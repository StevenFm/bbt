#include "Base.h"

#define INI_FREE_NODE(p)    \
    do  \
    {   \
        if ((p) < m_pszFileBuffer || (p) >= m_pszFileBufferEnd) \
        free(p);    \
    } while (false)

static BOOL InvalidChar(char c)
{
    if (strchr("\\/:*?\"<>|\'#~!@%^&()+{}`,\a\b\f\t\v[];#=", c))
        return true;

    return false;
}

static char* SplitLine(char* pszLineBegin)
{
    char* pszPos = pszLineBegin;

    while (*pszPos != '\r' && *pszPos != '\n' && *pszPos != '\0')
        ++pszPos;

    while (*pszPos == '\r' || *pszPos == '\n')
        *pszPos++ = '\0';

    return pszPos;
}

XIniFile::XIniFile()
{
    m_pszFileBuffer     = NULL;
    m_pszFileBufferEnd  = NULL;
    m_ulRefCount        = 1;
}

XIniFile::~XIniFile()
{
    Clear();
}

BOOL XIniFile::Load(const char szFileName[])
{
    BOOL            bResult         = false;
    size_t          uFileSize       = 0;
    char*           pszNextLine     = NULL;
    XKeyList*       pKeyList        = NULL;

    Clear();

	m_pszFileBuffer = (char*)g_pFileHelper->ReadFileData(&uFileSize, szFileName, 1);
    XY_FAILED_JUMP(m_pszFileBuffer);
    XY_SUCCESS_JUMP(uFileSize == 0);

    m_pszFileBufferEnd  = m_pszFileBuffer + uFileSize + 1;
    m_pszFileBuffer[uFileSize] = '\0';

    if (HasUtf8BomHeader((const char*)m_pszFileBuffer, (int)uFileSize))
    {
        pszNextLine = m_pszFileBuffer + 3;
        uFileSize -= 3;
    }
    else
    {
        pszNextLine = m_pszFileBuffer;
        uFileSize = uFileSize;
    }

    while (pszNextLine < m_pszFileBufferEnd && *pszNextLine)
    {
        char* pszLineBegin = pszNextLine;

        pszNextLine = SplitLine(pszLineBegin);

        if (*pszLineBegin == '[')   
        {
            char*           pszSectBegin    = pszLineBegin + 1;
            char*           pszSectEnd      = NULL;
            std::pair<XSectionTable::iterator, bool> insRet;

            pszSectEnd = strrchr(pszSectBegin, ']');
			XYLOG_FAILED_JUMP(pszSectEnd); 
			XYLOG_FAILED_JUMP(pszSectEnd != pszSectBegin); 

            *pszSectEnd = '\0';

            insRet = m_SectionTable.insert(make_pair(pszSectBegin, XKeyList()));

            if (insRet.first != m_SectionTable.end())
                pKeyList = &insRet.first->second;

			XYLOG_FAILED_JUMP(insRet.second);

            m_SectionList.push_back(pszSectBegin);
            continue;
        }

        if (!InvalidChar(*pszLineBegin))
        {
            char*   pszValueBegin   = NULL;

            if (pKeyList == NULL)
                continue;

            pszValueBegin = strchr(pszLineBegin, '=');
            if (pszValueBegin == NULL)
                continue;

            *pszValueBegin++ = '\0';

            if (FindKeyIt(pKeyList, pszLineBegin) != pKeyList->end())
                continue;

            pKeyList->push_back(std::make_pair(pszLineBegin, pszValueBegin));
        }
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::Save(const char szFileName[])
{
    BOOL                        bResult         = false;
    int                         nRetCode        = 0;
	std::string					strText;
    XSectionTable::iterator     sectMapIt;
    XSectionList::iterator      sectListIt;

    sectListIt = m_SectionList.begin();
    XY_FAILED_JUMP(sectListIt != m_SectionList.end());

	strText.reserve(1024 * 8);

    while (true)
    {
        sectMapIt = m_SectionTable.find(*sectListIt);

		strText.push_back('[');
		strText += *sectListIt;
		strText.push_back(']');
		strText.push_back('\n');

        for (XKeyList::iterator keyIt = sectMapIt->second.begin(); keyIt != sectMapIt->second.end(); ++keyIt)
        {
			strText += keyIt->first;
			strText.push_back('=');
			strText += keyIt->second;
			strText.push_back('\n');
        }

        ++sectListIt;

        if (sectListIt == m_SectionList.end())
            break;

		strText.push_back('\n');
    }

	nRetCode = g_pFileHelper->WriteFileData(szFileName, strText.c_str(), strText.length());
	XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void XIniFile::Clear()
{
    for (XSectionTable::iterator sectIter = m_SectionTable.begin(); sectIter != m_SectionTable.end(); ++sectIter)
    {
        for (XKeyList::iterator keyIter = sectIter->second.begin(); keyIter != sectIter->second.end(); ++keyIter)
        {
            INI_FREE_NODE(keyIter->first);
            INI_FREE_NODE(keyIter->second);
        }

        sectIter->second.clear();
        INI_FREE_NODE(sectIter->first);
    }

    m_SectionList.clear();
    m_SectionTable.clear();

    XY_DELETE_ARRAY(m_pszFileBuffer);
    m_pszFileBufferEnd = NULL;
}

BOOL XIniFile::GetNextSection(const char szSection[], char* pszNextSection)
{
    BOOL                    bResult     = false;
    int                     nRetCode    = 0;
    XSectionList::iterator  it;

    assert(szSection);
    assert(pszNextSection);

    for (it = m_SectionList.begin(); it != m_SectionList.end(); ++it)
    {
        nRetCode = strcmp(*it, szSection);
        if (nRetCode == 0)
        {
            ++it;
            XY_FAILED_JUMP(it != m_SectionList.end());

            strcpy(pszNextSection, *it);
            goto Exit1;
        }
    }

    it = m_SectionList.begin();
    XY_FAILED_JUMP(it != m_SectionList.end());

    strcpy(pszNextSection, *it);
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::GetNextKey(const char szSection[], const char szKeyName[], char* pszNextKey)
{
    BOOL                    bResult     = false;
    XKeyList*               pKeyList    = NULL;
    XKeyList::iterator      it;

    assert(szSection);
    assert(pszNextKey);
    assert(szKeyName);

    pKeyList = GetKeyList(szSection);
    XY_FAILED_JUMP(pKeyList);

    if (szKeyName[0] == '\0')
    {
        it = pKeyList->begin();
        XY_FAILED_JUMP(it != pKeyList->end());

        strcpy(pszNextKey, it->first);   
    }
    else
    {
        it = FindKeyIt(pKeyList, szKeyName);
        XY_FAILED_JUMP(it != pKeyList->end());

        ++it;
        XY_FAILED_JUMP(it != pKeyList->end());

        strcpy(pszNextKey, it->first);    
    }

    bResult = true;
Exit0:
    return bResult;
}

int  XIniFile::GetSectionCount()
{
    return (int)m_SectionTable.size();
}

BOOL XIniFile::IsSectionExist(const char szSection[])
{
    return (GetKeyList(szSection) != NULL);
}

BOOL XIniFile::IsKeyExist(const char szSection[], const char szKeyName[])
{
    return (GetKeyValue(szSection, szKeyName) != NULL);
}

BOOL XIniFile::GetString(const char szSection[], const char szKeyName[], char* pszRetString, unsigned int uSize)
{
    BOOL            bResult     = false;
    size_t          uStrLen     = 0;
    const char*     pszValue    = NULL;

    assert(pszRetString);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    uStrLen = strlen(pszValue);
    XY_FAILED_JUMP(uStrLen < uSize);

    memcpy(pszRetString, pszValue, uStrLen + 1);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::GetInteger(const char szSection[], const char szKeyName[], int* pnValue)
{
    BOOL            bResult     = false;
    const char*     pszValue    = NULL;
    long long       llValue     = 0;
    char*           pszStop     = NULL;

    assert(pnValue);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    llValue = strtoll(pszValue, &pszStop, 0);
    XY_FAILED_JUMP(llValue <= (long long)INT_MAX);
    XY_FAILED_JUMP(llValue >= (long long)INT_MIN);
    XY_FAILED_JUMP(pszStop != pszValue);

    *pnValue = (int)llValue;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::GetInteger2(const char szSection[], const char szKeyName[], int* pnValue1, int* pnValue2)
{
    BOOL    bResult     = false;
    int     nRetCode    = 0;
    int     nValues[3];                      // 尝试读取3个，如果成功读取3个视为出错

    assert(pnValue1 && pnValue2);

    nRetCode = GetMultiInteger(szSection, szKeyName, nValues, 3);
    XY_FAILED_JUMP(nRetCode == 2);

    *pnValue1 = nValues[0];
    *pnValue2 = nValues[1];

    bResult = true;
Exit0:
    return bResult;
}

int  XIniFile::GetMultiInteger(const char szSection[], const char szKeyName[], int* pnValues, int nCount)
{
    int             nResult     = 0;
    const char*     pszValue    = NULL;
    const char*     pszPos      = NULL;
    long long       llValue     = 0;
    char*           pszStop     = NULL;

    assert(pnValues);
    XY_FAILED_JUMP(nCount > 0);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    pszPos = pszValue;

    while (pszPos && nCount && *pszPos != '\0')
    {
        while (*pszPos == ',')
        {
            ++pszPos;
        }

        llValue = strtoll(pszPos, &pszStop, 0);
        XY_FAILED_JUMP(llValue <= (long long)INT_MAX);
        XY_FAILED_JUMP(llValue >= (long long)INT_MIN);
        XY_FAILED_JUMP(pszPos != pszStop);

        *pnValues++ = (int)llValue;

        --nCount;
        ++nResult;

        pszPos = strchr(pszPos, ',');
    }

Exit0:
    return nResult;
}

int  XIniFile::GetMultiLong(const char szSection[], const char szKeyName[], long* pnValues, int nCount)
{
    int             nResult     = 0;
    const char*     pszValue    = NULL;
    const char*     pszPos      = NULL;
    long long       llValue     = 0;
    char*           pszStop     = NULL;

    assert(pnValues);
    XY_FAILED_JUMP(nCount > 0);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    pszPos = pszValue;

    while (pszPos && nCount && *pszPos != '\0')
    {
        while (*pszPos == ',')
        {
            ++pszPos;
        }

        llValue = strtoll(pszPos, &pszStop, 0);
        XY_FAILED_JUMP(llValue <= (long long)INT_MAX);
        XY_FAILED_JUMP(llValue >= (long long)INT_MIN);
        XY_FAILED_JUMP(pszPos != pszStop);

        *pnValues++ = (long)llValue;

        --nCount;
        ++nResult;

        pszPos = strchr(pszPos, ',');
    }

Exit0:
    return nResult;
}

BOOL XIniFile::GetFloat(const char szSection[], const char szKeyName[], float* pfValue)
{
    BOOL            bResult     = false;
    const char*     pszValue    = NULL;
    double          dValue      = 0.0;
    char*           pszStop     = NULL;

    assert(pfValue);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    dValue = strtod(pszValue, &pszStop);
    XY_FAILED_JUMP(pszStop != pszValue);
    XY_FAILED_JUMP(dValue <= (double)FLT_MAX);
    XY_FAILED_JUMP(dValue >= -(double)FLT_MAX);

    *pfValue = (float)dValue;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::GetFloat2(const char szSection[], const char szKeyName[], float* pfValue1, float* pfValue2)
{
    BOOL    bResult     = false;
    int     nRetCode    = 0;
    float   fValues[3];          

    assert(pfValue1 && pfValue2);

    nRetCode = GetMultiFloat(szSection, szKeyName, fValues, 3);
    XY_FAILED_JUMP(nRetCode == 2);

    *pfValue1 = fValues[0];
    *pfValue2 = fValues[1];

    bResult = true;
Exit0:
    return bResult;
}

int  XIniFile::GetMultiFloat(const char szSection[], const char szKeyName[], float* pfValues, int nCount)
{
    int             nResult     = 0;
    const char*     pszValue    = NULL;
    const char*     pszPos      = NULL;
    char*           pszStop     = NULL;
    double          dValue      = 0;

    assert(pfValues);
    XY_FAILED_JUMP(nCount > 0);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    pszPos = pszValue;

    while (pszPos && nCount && *pszPos != '\0')
    {
        while (*pszPos == ',')
        {
            ++pszPos;
        }

        dValue = strtod(pszPos, &pszStop);
        XY_FAILED_JUMP(pszPos != pszStop);
        XY_FAILED_JUMP(dValue <= (double)FLT_MAX);
        XY_FAILED_JUMP(dValue >= -(double)FLT_MAX);

        *pfValues++ = (float)dValue;

        --nCount;
        ++nResult;

        pszPos = strchr(pszPos, ',');
    }

Exit0:
    return nResult;
}

BOOL XIniFile::GetStruct(const char szSection[], const char szKeyName[], void* pStruct, unsigned int uSize)
{
    BOOL            bResult     = false;
    const char*     pszValue    = NULL;
    BYTE*           pbyDecode   = NULL;
    size_t          uValueLen   = 0;
    BYTE            byRead[2];

    assert(pStruct);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

    uValueLen = strlen(pszValue);
    XY_FAILED_JUMP(uValueLen == uSize * 2);

    pbyDecode = (BYTE*)pStruct;

    for (int i = 0; i < (int)uSize; i++)
    {
        byRead[0] = pszValue[i * 2];
        byRead[1] = pszValue[i * 2 + 1];
        if ('0' <= byRead[0] && byRead[0] <= '9')
            byRead[0] = byRead[0] - '0';
        else if ('A' <= byRead[0] && byRead[0] <= 'F')
            byRead[0] = byRead[0] - 'A' + 10;
        else
            goto Exit0;

        if ('0' <= byRead[1] && byRead[1] <= '9')
            byRead[1] = byRead[1] - '0';
        else if ('A' <= byRead[1] && byRead[1] <= 'F')
            byRead[1] = byRead[1] - 'A' + 10;
        else
            goto Exit0;

        *(pbyDecode + i) = byRead[0] * 16 + byRead[1];
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::GetBool(const char szSection[], const char szKeyName[], int* pBool)
{
    BOOL            bResult     = false;
    const char*     pszValue    = NULL;
	int 			nValueCount = 0;

    const char* pszValueArray[] = 
    {
        "yes",  "no",
        "1",    "0",
        "true", "false"
    };

    assert(pBool);

    pszValue = GetKeyValue(szSection, szKeyName);
    XY_FAILED_JUMP(pszValue);

	nValueCount = sizeof(pszValueArray) / sizeof(pszValueArray[0]);
	
    for (int i = 0; i < nValueCount; i++)
    {
        if (STR_CASE_CMP(pszValue, pszValueArray[i]) == 0)
		{
			*pBool = ((i % 2) == 0);
			bResult = true;
			break;
		}
    }

Exit0:
    return bResult;
}   

BOOL XIniFile::AddSection(const char szSection[])
{
    BOOL        bResult     = false;
    XKeyList*   pKeyList    = NULL;

    assert(szSection);

    XY_FAILED_JUMP(szSection[0] != '\0');

    pKeyList = GetKeyList(szSection);
    XY_FAILED_JUMP(pKeyList == NULL);

    pKeyList = InsertSection(szSection);
    XY_FAILED_JUMP(pKeyList);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::AddKey(const char szSection[], const char szKeyName[], const char szKeyValue[])
{
    BOOL                bResult         = false;
    XKeyList*           pKeyList        = NULL;
    char*               pszDupKey       = NULL;
    char*               pszDupValue     = NULL;
    XKeyList::iterator  it;

    assert(szSection);
    assert(szKeyName);
    assert(szKeyValue);

    XY_FAILED_JUMP(szSection[0] != '\0');
    XY_FAILED_JUMP(szKeyName[0] != '\0');

    pKeyList = GetKeyList(szSection);
    XY_FAILED_JUMP(pKeyList);

    it = FindKeyIt(pKeyList, szKeyName);
    XY_FAILED_JUMP(it == pKeyList->end());

    pszDupKey = strdup(szKeyName);
    XY_FAILED_JUMP(pszDupKey);

    pszDupValue = strdup(szKeyValue);
    XY_FAILED_JUMP(pszDupValue);

    pKeyList->push_back(std::make_pair(pszDupKey, pszDupValue));

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::RemoveSection(const char szSection[])
{
    BOOL                        bResult     = false;
    XKeyList*                   pKeyList    = NULL;
    XSectionTable::iterator     sectMapIt;
    XSectionList::iterator      sectListIt;

    assert(szSection);

    sectMapIt = m_SectionTable.find((char*)szSection);
    XY_FAILED_JUMP(sectMapIt != m_SectionTable.end());

    pKeyList = &sectMapIt->second;

    for (XKeyList::iterator keyIt = pKeyList->begin(); keyIt != pKeyList->end(); ++keyIt)
    {
        INI_FREE_NODE(keyIt->first);
        INI_FREE_NODE(keyIt->second);
    }
    pKeyList->clear();

    for (sectListIt = m_SectionList.begin(); sectListIt != m_SectionList.end(); ++sectListIt)
    {
        if (strcmp(szSection, *sectListIt) == 0)
            break;
    }

    assert(sectListIt != m_SectionList.end());
    m_SectionList.erase(sectListIt);

    INI_FREE_NODE(sectMapIt->first);
    m_SectionTable.erase(sectMapIt);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::RemoveKey(const char szSection[], const char szKeyName[])
{
    BOOL                    bResult     = false;
    XKeyList*               pKeyList    = NULL;
    XKeyList::iterator      keyIt;

    assert(szKeyName);

    pKeyList = GetKeyList(szSection);
    XY_FAILED_JUMP(pKeyList);

    keyIt = FindKeyIt(pKeyList, szKeyName);
    XY_FAILED_JUMP(keyIt != pKeyList->end());

    INI_FREE_NODE(keyIt->first);
    INI_FREE_NODE(keyIt->second);

    pKeyList->erase(keyIt);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::SetString(const char szSection[], const char szKeyName[], const char szString[])
{
    BOOL                    bResult         = false;
    int                     nRetCode        = 0;
    XKeyList*               pKeyList        = NULL;
    char*                   pszDupKey       = NULL;
    char*                   pszDupValue     = NULL;
    XKeyList::iterator      keyIt;

    assert(szSection);
    assert(szKeyName);
    assert(szString);

    XY_FAILED_JUMP(szSection[0] != '\0');
    XY_FAILED_JUMP(szKeyName[0] != '\0');

    pKeyList = GetKeyList(szSection);
    if (pKeyList == NULL)
        pKeyList = InsertSection(szSection);

    assert(pKeyList);

    keyIt = FindKeyIt(pKeyList, szKeyName);
    if (keyIt == pKeyList->end())
    {
        pszDupKey = strdup(szKeyName);
        XY_FAILED_JUMP(pszDupKey);

        pszDupValue = strdup(szString);
        XY_FAILED_JUMP(pszDupValue);

        pKeyList->push_back(std::make_pair(pszDupKey, pszDupValue));
        goto Exit1;
    }

    nRetCode = strcmp(keyIt->second, szString);
    XY_SUCCESS_JUMP(nRetCode == 0);

    pszDupValue = strdup(szString);
    XY_FAILED_JUMP(pszDupValue);

    INI_FREE_NODE(keyIt->second);
    keyIt->second = pszDupValue;

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        XY_FREE(pszDupKey);
        XY_FREE(pszDupValue);
    }
    return bResult;
}

BOOL XIniFile::SetInteger(const char szSection[], const char szKeyName[], int nValue)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    int     nWriteLen       = 0;
    char    szNumToStr[12];

    nWriteLen = snprintf(szNumToStr, _countof(szNumToStr), "%d", nValue);
    XY_FAILED_JUMP(nWriteLen > 0 && nWriteLen < _countof(szNumToStr));

    nRetCode = SetString(szSection, szKeyName, szNumToStr);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::SetInteger2(const char szSection[], const char szKeyName[], int nValue1, int nValue2)
{
    int nValues[2] = {nValue1, nValue2};

    return SetMultiInteger(szSection, szKeyName, nValues, 2);
}

BOOL XIniFile::SetMultiInteger(const char szSection[], const char szKeyName[], int* pnValues, int nCount)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    char*   pszNumToStr     = NULL;
    int     nNumPos         = 0;
    int     nNumMaxLen      = 0;

    assert(pnValues);
    XY_FAILED_JUMP(nCount > 0);

    nNumMaxLen = 12 * nCount; // 12: [负号] + 10个数字 + 逗号(或者结尾0)

    pszNumToStr = (char*)malloc(nNumMaxLen);   
    XY_FAILED_JUMP(pszNumToStr);

    nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen, "%d", *pnValues++);
    XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen);

    nNumPos += nRetCode;

    while (--nCount)
    {
        nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen - nNumPos, ",%d", *pnValues++);
        XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen - nNumPos);

        nNumPos += nRetCode;
    }

    nRetCode = SetString(szSection, szKeyName, pszNumToStr);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    XY_FREE(pszNumToStr);
    return bResult;
}

BOOL XIniFile::SetMultiLong(const char szSection[], const char szKeyName[], long* pValues, int nCount)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    char*   pszNumToStr     = NULL;
    int     nNumPos         = 0;
    int     nNumMaxLen      = 0;

    assert(pValues);
    XY_FAILED_JUMP(nCount > 0);

    nNumMaxLen = 12 * nCount; // 12: [负号] + 10个数字 + 逗号(或者结尾0)

    pszNumToStr = (char*)malloc(nNumMaxLen);   
    XY_FAILED_JUMP(pszNumToStr);

    nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen, "%ld", *pValues++);
    XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen);

    nNumPos += nRetCode;

    while (--nCount)
    {
        nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen - nNumPos, ",%ld", *pValues++);
        XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen - nNumPos);

        nNumPos += nRetCode;
    }

    nRetCode = SetString(szSection, szKeyName, pszNumToStr);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    XY_FREE(pszNumToStr);
    return bResult;
}

BOOL XIniFile::SetFloat(const char szSection[], const char szKeyName[], float fValue)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    int     nWriteLen       = 0;
    char    szNumToStr[25];     

    nWriteLen = snprintf(szNumToStr, _countof(szNumToStr), "%.18g", fValue);
    XY_FAILED_JUMP(nWriteLen > 0 && nWriteLen < _countof(szNumToStr));

    nRetCode = SetString(szSection, szKeyName, szNumToStr);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XIniFile::SetFloat2(const char szSection[],  const char szKeyName[], float fValue1, float fValue2)
{
    float fValues[2] = {fValue1, fValue2};

    return SetMultiFloat(szSection, szKeyName, fValues, 2);
}

BOOL XIniFile::SetMultiFloat(const char szSection[],  const char szKeyName[], float* pfValues, int nCount)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    char*   pszNumToStr     = NULL;
    int     nNumPos         = 0;
    int     nNumMaxLen      = 0;

    assert(pfValues);
    XY_FAILED_JUMP(nCount > 0);

    nNumMaxLen = 25 * nCount;   // 25: [负号] + 23 + 逗号(或者'\0')

    pszNumToStr = (char*)malloc(nNumMaxLen);
    XY_FAILED_JUMP(pszNumToStr);

    nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen, "%.18g", *pfValues++);
    XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen);
    nNumPos += nRetCode;

    while (--nCount)
    {
        nRetCode = snprintf(pszNumToStr + nNumPos, nNumMaxLen - nNumPos, ",%.18g", *pfValues++);
        XY_FAILED_JUMP(nRetCode > 0 && nRetCode < nNumMaxLen - nNumPos);
        nNumPos += nRetCode;
    }

    nRetCode = SetString(szSection, szKeyName, pszNumToStr);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    XY_FREE(pszNumToStr);
    return bResult;
}
BOOL XIniFile::SetStruct(const char szSection[], const char szKeyName[], void* lpStruct, unsigned int uSize)
{
    BOOL    bResult         = false;
    int     nRetCode        = 0;
    char*   pszEncode       = NULL;
    BYTE    byStructByte    = 0;

    assert(lpStruct);

    pszEncode = (char*)malloc(uSize * 2 + 1);
    XY_FAILED_JUMP(pszEncode);

    for (int i = 0; i < (int)uSize; i++)
    {
        byStructByte = *((BYTE*)lpStruct + i);
        pszEncode[i * 2] = (byStructByte & 0xf0) >> 4;
        pszEncode[i * 2 + 1] = byStructByte & 0x0f;

        if (pszEncode[i * 2] >= 10)
            pszEncode[i * 2] = pszEncode[i * 2] - 10 + 'A';
        else
            pszEncode[i * 2] = pszEncode[i * 2] + '0';

        if (pszEncode[i * 2 + 1] >= 10)
            pszEncode[i * 2 + 1] = pszEncode[i * 2 + 1] - 10 + 'A';
        else
            pszEncode[i * 2 + 1] = pszEncode[i * 2 + 1] + '0';
    }

    pszEncode[uSize * 2] = '\0';

    nRetCode = SetString(szSection, szKeyName, pszEncode);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    XY_FREE(pszEncode);
    return bResult;
}

XIniFile::XKeyList* XIniFile::GetKeyList(const char szSection[])
{
    XKeyList*                   pKeyList   = NULL;
    XSectionTable::iterator     it;

    assert(szSection);

    XY_FAILED_JUMP(szSection[0] != '\0');

    it = m_SectionTable.find((char*)szSection);
    XY_FAILED_JUMP(it != m_SectionTable.end());

    pKeyList = &it->second;

Exit0:
    return pKeyList;
}

const char* XIniFile::GetKeyValue(const char szSection[], const char szKeyName[])
{
    XKeyList*           pKeyList    = NULL;
    const char*         pszValue    = NULL;
    XKeyList::iterator  it;

    assert(szSection);
    assert(szKeyName);

    XY_FAILED_JUMP(szKeyName[0] != '\0');

    pKeyList = GetKeyList(szSection);
    XY_FAILED_JUMP(pKeyList);

    it = FindKeyIt(pKeyList, szKeyName);
    XY_FAILED_JUMP(it != pKeyList->end());

    pszValue = it->second;

Exit0:
    return pszValue;
}

XIniFile::XKeyList::iterator XIniFile::FindKeyIt(XKeyList* pKeyList, const char szKeyName[])
{
    for (XKeyList::iterator it = pKeyList->begin(); it != pKeyList->end(); ++it)
    {
        if (strcmp(it->first, szKeyName) == 0)
            return it;
    }
    return pKeyList->end();
}

XIniFile::XKeyList* XIniFile::InsertSection(const char szSection[])
{
    BOOL            bResult         = false;
    XKeyList*       pKeyList        = NULL;
    char*           pszBuffer       = NULL;
    std::pair<XSectionTable::iterator, bool> insRet;

    pszBuffer = strdup(szSection);
    XY_FAILED_JUMP(pszBuffer);

    insRet = m_SectionTable.insert(make_pair(pszBuffer, XKeyList()));
    XY_FAILED_JUMP(insRet.second);

    pKeyList = &insRet.first->second;

    m_SectionList.push_back(pszBuffer);

    bResult = true;
Exit0:
    if (!bResult)
        XY_FREE(pszBuffer);

    return pKeyList;
}

