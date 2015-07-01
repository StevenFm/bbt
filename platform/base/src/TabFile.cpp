#include "Base.h"
using namespace std;

XTabFile::XTabFile()
{
    m_nLineCount    = 0;
    m_nColumnCount  = 0;
    m_ulRefCount    = 1;
}

XTabFile::~XTabFile()
{
	Clear();
}

BOOL XTabFile::Load(const char cszFileName[])
{
    BOOL            bResult         = false;
	BYTE*           pbyBuffer       = NULL;
    char*           pszAnalyzeBegin = NULL;
    unsigned long   uSize           = 0;
    char*           pszCell         = NULL;
    char*           pszPos          = NULL;
    XTabLine*       pTabLine        = NULL;
    int             nLineCount      = 0;
    int             nColumn         = 0;

	Clear();

	pbyBuffer = g_pFileHelper->ReadFileData((size_t*)&uSize, cszFileName, 1);
    XY_FAILED_JUMP(pbyBuffer);
	pbyBuffer[uSize] = '\0';

    if (HasUtf8BomHeader((const char*)pbyBuffer, (int)uSize))
    {
        pszAnalyzeBegin = (char*)pbyBuffer + 3;
        uSize -= 3;
    }
    else
    {
        pszAnalyzeBegin = (char*)pbyBuffer;
        uSize = uSize;
    }

    pszPos  = pszAnalyzeBegin;
    while (pszPos < pszAnalyzeBegin + uSize)
    {
        if (*pszPos == '\t')
        {
            nColumn++;
        }
        else if (*pszPos == '\n')
        {
            nColumn++;
            nLineCount++;
            m_nColumnCount = max(nColumn, m_nColumnCount);
            nColumn = 0;
        }

        pszPos++;
    }
    nLineCount++;

    m_LineTable.reserve(nLineCount);

    pszCell = pszAnalyzeBegin;
    pszPos  = pszAnalyzeBegin;

    while (pszPos < pszAnalyzeBegin + uSize)
    {
        if (pTabLine == NULL)
        {
            pTabLine = new XTabLine;

            pTabLine->reserve(m_nColumnCount);
        }

        if (*pszPos == '\t')
        {
            *pszPos++ = '\0';

            pTabLine->push_back(pszCell[0] == '\0' ? NULL : strdup(pszCell));
            pszCell = pszPos;

            continue;
        }

        if (*pszPos == '\r')
        {
            *pszPos++ = '\0';
            continue;
        }

        if (*pszPos == '\n')
        {
            *pszPos++ = '\0';

            pTabLine->push_back(pszCell[0] == '\0' ? NULL : strdup(pszCell));
            pszCell = pszPos;

            if ((int)pTabLine->size() < m_nColumnCount)
            {
                pTabLine->resize(m_nColumnCount, NULL);
            }
            m_LineTable.push_back(pTabLine);
            pTabLine = NULL;

            continue;
        }

        pszPos++;
    }

    if (pTabLine)
    {
        if (pszPos > pszCell)
        {
            pTabLine->push_back(pszCell[0] == '\0' ? NULL : strdup(pszCell));
            pszCell = pszPos;
        }

        if ((int)pTabLine->size() < m_nColumnCount)
        {
            pTabLine->resize(m_nColumnCount, NULL);
        }
        m_LineTable.push_back(pTabLine);
        pTabLine = NULL;
    }

    m_nLineCount = (int)m_LineTable.size();

    BuildIndex();

	bResult = true;
Exit0:
    XY_DELETE(pTabLine);
	XY_DELETE_ARRAY(pbyBuffer);
	return bResult;
}

BOOL XTabFile::Save(const char cszFileName[])
{
    BOOL    bResult     = false;
	BOOL	bRetCode	= false;
	string	strText;

    assert(m_nLineCount == (int)m_LineTable.size());

    for (int nLine = 0; nLine < m_nLineCount; nLine++)
    {
        XTabLine* pTabLine = m_LineTable[nLine];

        assert(m_nColumnCount == (int)pTabLine->size());
        
        for (int nColumn = 0; nColumn < m_nColumnCount; nColumn++)        
        {
            const char* pszCell = (*pTabLine)[nColumn];
            
            if (pszCell == NULL)
                pszCell = "";

			strText += pszCell;
			strText.push_back((nColumn == m_nColumnCount - 1) ? '\n' : '\t');
        }
    }
	
	bRetCode = g_pFileHelper->WriteFileData(cszFileName, strText.c_str(), strText.length());
	XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void XTabFile::Clear()
{
    for (int nLine = 0; nLine < m_nLineCount; nLine++)
    {
        XTabLine* pTabLine = m_LineTable[nLine];
        
        for (int nColumn = 0; nColumn < m_nColumnCount; nColumn++)        
        {
            free((*pTabLine)[nColumn]);
        }

        delete pTabLine;
    }

    m_LineTable.clear();

    m_LineIndex.clear();
    m_ColIndex.clear();

    m_nLineCount    = 0;
    m_nColumnCount  = 0;
}

int	XTabFile::GetLineCount()
{    
    return m_nLineCount;
}

int	XTabFile::GetColumnCount()
{
    return m_nColumnCount;
}

int XTabFile::FindLine(const char cszLine[])
{
    int nLine = -1;

    assert(cszLine);

    XNameTable::iterator it = m_LineIndex.find(cszLine);
    if (it != m_LineIndex.end())
    {
        nLine = it->second;
    }

    return nLine;
}

int XTabFile::FindColumn(const char cszCol[])
{
    int nCol = -1;

    assert(cszCol);

    XNameTable::iterator it = m_ColIndex.find(cszCol);
    if (it != m_ColIndex.end())
    {
        nCol = it->second;
    }

    return nCol;
}

BOOL XTabFile::InsertLine(int nLine)
{
    BOOL        bResult     = false;
    XTabLine*   pTabLine    = NULL;

    XY_FAILED_JUMP(nLine >= 1);
    XY_FAILED_JUMP(nLine <= m_nLineCount);

    pTabLine = new XTabLine;

    pTabLine->resize(m_nColumnCount, NULL);

    m_LineTable.insert(m_LineTable.begin() + nLine - 1, pTabLine);

    m_nLineCount++;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}


BOOL XTabFile::AddLine(int nAddLineCount)
{
    BOOL bResult = false;

    XY_FAILED_JUMP(nAddLineCount >= 0);

    for (int i = 0; i < nAddLineCount; i++)
    {
        XTabLine*   pTabLine = new XTabLine;

        pTabLine->resize(m_nColumnCount, NULL);

        m_LineTable.push_back(pTabLine);
    }

    m_nLineCount += nAddLineCount;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::DelLine(int nLine)
{
    BOOL        bResult     = false;
    XTabLine*   pTabLine    = NULL;

    XY_FAILED_JUMP(nLine >= 1);
    XY_FAILED_JUMP(nLine <= m_nLineCount);

    pTabLine = m_LineTable[nLine - 1];

    for (int i = 0; i < m_nColumnCount; i++)
    {
        free((*pTabLine)[i]);
    }

    delete pTabLine;

    m_LineTable.erase(m_LineTable.begin() + nLine - 1);

    m_nLineCount--;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::InsertColumn(int nColumn)
{
    BOOL bResult = false;

    XY_FAILED_JUMP(nColumn >= 1);
    XY_FAILED_JUMP(nColumn <= m_nColumnCount);

    for (int i = 0; i < m_nLineCount; i++)
    {
        XTabLine* pTabLine = m_LineTable[i];

        pTabLine->insert(pTabLine->begin() + nColumn - 1, NULL);
    }

    m_nColumnCount++;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::AddColumn(int nAddColumnCount)
{
    BOOL bResult = false;

    XY_FAILED_JUMP(nAddColumnCount >= 0);

    for (int i = 0; i < m_nLineCount; i++)
    {
        XTabLine* pTabLine = m_LineTable[i];

        pTabLine->resize(m_nColumnCount + nAddColumnCount, NULL);
    }

    m_nColumnCount += nAddColumnCount;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::DelColumn(int nColumn)
{
    BOOL bResult = false;

    XY_FAILED_JUMP(nColumn >= 1);
    XY_FAILED_JUMP(nColumn <= m_nColumnCount);

    for (int i = 0; i < m_nLineCount; i++)
    {
        XTabLine* pTabLine = m_LineTable[i];
        
        free((*pTabLine)[nColumn - 1]);

        pTabLine->erase(pTabLine->begin() + nColumn - 1);
    }

    m_nColumnCount--;

    BuildIndex();

    bResult = true;
Exit0:
    return bResult;
}

/****************************  行列均为数字以进行检索 ***************************/
const char* XTabFile::GetString(int nLine, int nCol)
{
    return GetCell(nLine, nCol);
}

BOOL XTabFile::GetString(int nLine, int nCol, char* pszBuffer, size_t uBufferSize)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    const char* pszCell     = NULL;

    pszCell = GetCell(nLine, nCol);
    XY_FAILED_JUMP(pszCell);

    bRetCode = SafeCopyString(pszBuffer, uBufferSize, pszCell);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetString(int nLine, int nCol, const char cszValue[])
{
    return SetCell(nLine, nCol, cszValue);
}

BOOL XTabFile::GetInt16(int nLine, int nCol, int16_t* pnRetValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int64_t     nValue      = 0;

    assert(pnRetValue);

    bRetCode = GetInt64(nLine, nCol, &nValue);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(nValue >= SHRT_MIN && nValue <= SHRT_MAX);

    *pnRetValue = (int16_t)nValue;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetInt16(int nLine, int nCol, int16_t nSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[16];

	sprintf(szIntNum, "%d", nSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetUInt16(int nLine, int nCol, uint16_t* puRetValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int64_t     nValue      = 0;

    assert(puRetValue);

    bRetCode = GetInt64(nLine, nCol, &nValue);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(nValue >= 0 && nValue <= USHRT_MAX);

    *puRetValue = (uint16_t)nValue;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetUInt16(int nLine, int nCol, uint16_t uSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[16];

	sprintf(szIntNum, "%u", uSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetInt32(int nLine, int nCol, int32_t* pnRetValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int64_t     n64Value    = 0;

    assert(pnRetValue);

    bRetCode = GetInt64(nLine, nCol, &n64Value);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(n64Value >= INT_MIN && n64Value <= INT_MAX);

    *pnRetValue = (int32_t)n64Value;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetInt32(int nLine, int nCol, int32_t nSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[16];

	sprintf(szIntNum, "%d", nSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetUInt32(int nLine, int nCol, uint32_t* puRetValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    int64_t     n64Value    = 0;

    assert(puRetValue);

    bRetCode = GetInt64(nLine, nCol, &n64Value);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(n64Value >= 0 && n64Value <= UINT_MAX);

    *puRetValue = (uint32_t)n64Value;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetUInt32(int nLine, int nCol, uint32_t uSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[16];

	sprintf(szIntNum, "%u", uSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetInt64(int nLine, int nCol, int64_t* pnRetValue)
{
    BOOL            bResult         = false;
	const char*     pszString       = NULL;
    int64_t         n64Value        = 0;
    char*           pszEndPointer   = NULL;

    assert(pnRetValue);

    pszString = GetCell(nLine, nCol);
    XY_FAILED_JUMP(pszString);

    errno = 0;

    n64Value = strtoll(pszString, &pszEndPointer, 0);

    XY_FAILED_JUMP(errno == 0);
    XY_FAILED_JUMP(pszEndPointer != pszString);

    *pnRetValue = n64Value;

    bResult = true;
Exit0:
	return bResult; 
}

BOOL XTabFile::SetInt64(int nLine, int nCol, int64_t nSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[32];

	sprintf(szIntNum, "%lld", nSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetUInt64(int nLine, int nCol, uint64_t* puRetValue)
{
    BOOL            bResult         = false;
	const char*     pszString       = NULL;
    uint64_t        u64Value        = 0;
    char*           pszEndPointer   = NULL;

    assert(puRetValue);

    pszString = GetCell(nLine, nCol);
    XY_FAILED_JUMP(pszString);

    errno = 0;

    u64Value = strtoull(pszString, &pszEndPointer, 0);

    XY_FAILED_JUMP(errno == 0);
    XY_FAILED_JUMP(pszEndPointer != pszString);

    *puRetValue = u64Value;

    bResult = true;
Exit0:
	return bResult; 
}

BOOL XTabFile::SetUInt64(int nLine, int nCol, uint64_t  uSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szIntNum[32];

	sprintf(szIntNum, "%llu", uSetValue);

	bRetCode = SetCell(nLine, nCol, szIntNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetFloat(int nLine, int nCol, float* pfRetValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    double      fValue      = 0.0f;

    assert(pfRetValue);

    bRetCode = GetDouble(nLine, nCol, &fValue);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(fValue >= -FLT_MAX && fValue <= FLT_MAX);

    *pfRetValue = (float)fValue;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetFloat(int nLine, int nCol, float  fSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szFloatNum[32];

	sprintf(szFloatNum, "%.18g", fSetValue);

	bRetCode = SetCell(nLine, nCol, szFloatNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetDouble(int nLine, int nCol, double* pfRetValue)
{ 
    BOOL            bResult         = false;
	const char*     pszString       = NULL;
    double          fValue          = 0.0f;
    char*           pszEndPointer   = NULL;

    assert(pfRetValue);

    pszString = GetCell(nLine, nCol);
    XY_FAILED_JUMP(pszString);

    errno = 0;

    fValue = strtod(pszString, &pszEndPointer);

    XY_FAILED_JUMP(errno == 0);
    XY_FAILED_JUMP(pszEndPointer != pszString);

    *pfRetValue = fValue;

    bResult = true;
Exit0:
	return bResult;    
}

BOOL XTabFile::SetDouble(int nLine, int nCol, double  fSetValue)
{
    BOOL bResult    = false;
    BOOL bRetCode   = false; 
	char szFloatNum[32];

	sprintf(szFloatNum, "%.18g", fSetValue);

	bRetCode = SetCell(nLine, nCol, szFloatNum);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::GetInt(int nLine, int nCol, int* pnValue)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;
    int64_t nValue64    = 0;

    assert(pnValue);

    bRetCode = GetInt64(nLine, nCol, &nValue64);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(nValue64 >= INT_MIN && nValue64 <= INT_MAX);

    *pnValue = (int)nValue64;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetInt(int nLine, int nCol, int nValue)
{
    return SetInt64(nLine, nCol, nValue);
}

BOOL XTabFile::GetDword(int nLine, int nCol, DWORD* pdwValue)
{
    BOOL        bResult     = false;
    BOOL        bRetCode    = false;
    uint64_t    uValue64    = 0;

    assert(pdwValue);

    bRetCode = GetUInt64(nLine, nCol, &uValue64);
    XY_FAILED_JUMP(bRetCode);

    XY_FAILED_JUMP(uValue64 <= UINT_MAX);

    *pdwValue = (DWORD)uValue64;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XTabFile::SetDword(int nLine, int nCol, DWORD dwValue)
{
    return SetUInt64(nLine, nCol, dwValue);
}

/****************************  行为数字列为字串以进行检索 ***************************/
const char* XTabFile::GetString(int  nLine, const char cszCol[])
{
    return GetString(nLine, FindColumn(cszCol));
}

BOOL XTabFile::GetString(int  nLine, const char cszCol[], char* pszBuffer, size_t uBufferSize)
{
    return GetString(nLine, FindColumn(cszCol), pszBuffer, uBufferSize);
}

BOOL XTabFile::SetString(int  nLine, const char cszCol[], const char cszValue[])
{
    return SetString(nLine, FindColumn(cszCol), cszValue);
}

BOOL XTabFile::GetInt16(int  nLine, const char cszCol[], int16_t* pnRetValue)
{
    return GetInt16(nLine, FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt16(int  nLine, const char cszCol[], int16_t  nSetValue)
{
    return SetInt16(nLine, FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt16(int  nLine, const char cszCol[], uint16_t* puRetValue)
{
    return GetUInt16(nLine, FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt16(int  nLine, const char cszCol[], uint16_t  uSetValue)
{
    return SetUInt16(nLine, FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetInt32(int  nLine, const char cszCol[], int32_t* pnRetValue)
{
    return GetInt32(nLine, FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt32(int  nLine, const char cszCol[], int32_t  nSetValue)
{
    return SetInt32(nLine, FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt32(int  nLine, const char cszCol[], uint32_t* puRetValue)
{
    return GetUInt32(nLine, FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt32(int  nLine, const char cszCol[], uint32_t  uSetValue)
{
    return SetUInt32(nLine, FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetInt64(int  nLine, const char cszCol[], int64_t* pnRetValue)
{
    return GetInt64(nLine, FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt64(int  nLine, const char cszCol[], int64_t  nSetValue)
{
    return SetInt64(nLine, FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt64(int  nLine, const char cszCol[], uint64_t* puRetValue)
{
    return GetUInt64(nLine, FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt64(int  nLine, const char cszCol[], uint64_t  uSetValue)
{
    return SetUInt64(nLine, FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetFloat(int  nLine, const char cszCol[], float* pfRetValue)
{
    return GetFloat(nLine, FindColumn(cszCol), pfRetValue);
}

BOOL XTabFile::SetFloat(int  nLine, const char cszCol[], float  fSetValue)
{
    return SetFloat(nLine, FindColumn(cszCol), fSetValue);
}

BOOL XTabFile::GetDouble(int  nLine, const char cszCol[], double* pfRetValue)
{
    return GetDouble(nLine, FindColumn(cszCol), pfRetValue);
}

BOOL XTabFile::SetDouble(int  nLine, const char cszCol[], double  fSetValue)
{
    return SetDouble(nLine, FindColumn(cszCol), fSetValue);
}

BOOL XTabFile::GetInt(int nLine, const char cszCol[], int* pnValue)
{
    return GetInt(nLine, FindColumn(cszCol), pnValue);
}

BOOL XTabFile::SetInt(int nLine, const char cszCol[], int nValue)
{
    return SetInt(nLine, FindColumn(cszCol), nValue);
}

BOOL XTabFile::GetDword(int nLine, const char cszCol[], DWORD* pdwValue)
{
    return GetDword(nLine, FindColumn(cszCol), pdwValue);
}

BOOL XTabFile::SetDword(int nLine, const char cszCol[], DWORD dwValue)
{
    return SetDword(nLine, FindColumn(cszCol), dwValue);
}

/************************** 行列均为字串以进行检索 ***********************/
const char* XTabFile::GetString(const char cszLine[], const char cszCol[])
{
    return GetString(FindLine(cszLine), FindColumn(cszCol));
}

BOOL XTabFile::GetString(const char cszLine[], const char cszCol[], char* pszBuffer, size_t uBufferSize)
{
    return GetString(FindLine(cszLine), FindColumn(cszCol), pszBuffer, uBufferSize);
}

BOOL XTabFile::SetString(const char cszLine[], const char cszCol[], const char cszValue[])
{
    return SetString(FindLine(cszLine), FindColumn(cszCol), cszValue);
}

BOOL XTabFile::GetInt16(const char cszLine[], const char cszCol[], int16_t* pnRetValue)
{
    return GetInt16(FindLine(cszLine), FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt16(const char cszLine[], const char cszCol[], int16_t  nSetValue)
{
    return SetInt16(FindLine(cszLine), FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt16(const char cszLine[], const char cszCol[], uint16_t* puRetValue)
{
    return GetUInt16(FindLine(cszLine), FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt16(const char cszLine[], const char cszCol[], uint16_t  uSetValue)
{
    return SetUInt16(FindLine(cszLine), FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetInt32(const char cszLine[], const char cszCol[], int32_t* pnRetValue)
{
    return GetInt32(FindLine(cszLine), FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt32(const char cszLine[], const char cszCol[], int32_t  nSetValue)
{
    return SetInt32(FindLine(cszLine), FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt32(const char cszLine[], const char cszCol[], uint32_t* puRetValue)
{
    return GetUInt32(FindLine(cszLine), FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt32(const char cszLine[], const char cszCol[], uint32_t  uSetValue)
{
    return SetUInt32(FindLine(cszLine), FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetInt64(const char cszLine[], const char cszCol[], int64_t* pnRetValue)
{
    return GetInt64(FindLine(cszLine), FindColumn(cszCol), pnRetValue);
}

BOOL XTabFile::SetInt64(const char cszLine[], const char cszCol[], int64_t  nSetValue)
{
    return SetInt64(FindLine(cszLine), FindColumn(cszCol), nSetValue);
}

BOOL XTabFile::GetUInt64(const char cszLine[], const char cszCol[], uint64_t* puRetValue)
{
    return GetUInt64(FindLine(cszLine), FindColumn(cszCol), puRetValue);
}

BOOL XTabFile::SetUInt64(const char cszLine[], const char cszCol[], uint64_t  uSetValue)
{
    return SetUInt64(FindLine(cszLine), FindColumn(cszCol), uSetValue);
}

BOOL XTabFile::GetFloat(const char cszLine[], const char cszCol[], float* pfRetValue)
{
    return GetFloat(FindLine(cszLine), FindColumn(cszCol), pfRetValue);
}

BOOL XTabFile::SetFloat(const char cszLine[], const char cszCol[], float  fSetValue)
{
    return SetFloat(FindLine(cszLine), FindColumn(cszCol), fSetValue);
}

BOOL XTabFile::GetDouble(const char cszLine[], const char cszCol[], double* pfRetValue)
{
    return GetDouble(FindLine(cszLine), FindColumn(cszCol), pfRetValue);
}

BOOL XTabFile::SetDouble(const char cszLine[], const char cszCol[], double  fSetValue)
{
    return SetDouble(FindLine(cszLine), FindColumn(cszCol), fSetValue);
}

BOOL XTabFile::GetInt(const char cszLine[], const char cszCol[], int* pnValue)
{
    return GetInt(FindLine(cszLine), FindColumn(cszCol), pnValue);
}

BOOL XTabFile::SetInt(const char cszLine[], const char cszCol[], int nValue)
{
    return SetInt(FindLine(cszLine), FindColumn(cszCol), nValue);
}

BOOL XTabFile::GetDword(const char cszLine[], const char cszCol[], DWORD* pdwValue)
{
    return GetDword(FindLine(cszLine), FindColumn(cszCol), pdwValue);
}

BOOL XTabFile::SetDword(const char cszLine[], const char cszCol[], DWORD dwValue)
{
    return SetDword(FindLine(cszLine), FindColumn(cszCol), dwValue);
}

const char* XTabFile::GetCell(int nLine, int nCol)
{
    const char* pszResult   = NULL;
    XTabLine*   pTabLine    = NULL;

    XY_FAILED_JUMP(nLine >= 1 && nLine <= m_nLineCount);
    XY_FAILED_JUMP(nCol >= 1 && nCol <= m_nColumnCount);

    pTabLine = m_LineTable[nLine - 1];

    pszResult = (*pTabLine)[nCol - 1];

    if (pszResult == NULL)
    {
        pszResult = "";
    }

Exit0:
    return pszResult;
}

BOOL XTabFile::SetCell(int nLine, int nCol, const char cszValue[])
{
    BOOL        bResult     = false;
    XTabLine*   pTabLine    = NULL;

    XY_FAILED_JUMP(nLine >= 1 && nLine <= m_nLineCount);
    XY_FAILED_JUMP(nCol >= 1 && nCol <= m_nColumnCount);

    pTabLine = m_LineTable[nLine - 1];

    free((*pTabLine)[nCol - 1]);
    (*pTabLine)[nCol - 1] = NULL;

    if (cszValue != NULL && cszValue[0] != '\0')
    {
        (*pTabLine)[nCol - 1] =  strdup(cszValue);
    }

    if (nLine == 1 || nCol == 1)
    {
        BuildIndex();
    }

    bResult = true;
Exit0:
    return bResult;
}

void XTabFile::BuildIndex()
{
    m_LineIndex.clear();

    if (m_nColumnCount >= 1)
    {
        for (int nLine = 1; nLine <= m_nLineCount; nLine++)
        {
            m_LineIndex.insert(std::make_pair(GetCell(nLine, 1), nLine));
        }
    }

    m_ColIndex.clear();

    if (m_nLineCount >= 1)
    {
        for (int nCol = 1; nCol <= m_nColumnCount; nCol++)
        {
            m_ColIndex.insert(std::make_pair(GetCell(1, nCol), nCol));
        }
    }
}
    
