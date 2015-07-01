#include "Base.h"

using namespace std;

XFilePackageMulti* g_pFilePackageMulti = NULL;

XFilePackageMulti::XFilePackageMulti()
{
}

XFilePackageMulti::~XFilePackageMulti()
{
    Close();    
}

BOOL XFilePackageMulti::Load(const char wszIndexFile[])
{    
    BOOL bRetCode = false;
    BOOL bResult  = false;

    m_PakFileNameTable.push_back(wszIndexFile);

    IFilePackage* pPak = CreateFilePackInterface();
    bRetCode = pPak->Load(wszIndexFile);
    XY_FAILED_JUMP(bRetCode);

    m_PakTable.push_back(pPak);

    bResult = true;
Exit0:
    if (!bResult)
    {
        XY_COM_RELEASE(pPak);
    }

    return bResult;
}

BOOL XFilePackageMulti::Reload()
{    
    BOOL bRetCode = false;
    BOOL bResult  = false;
    
    Close();

    for (XPakFileNameTable::iterator it = m_PakFileNameTable.begin(); it != m_PakFileNameTable.end(); ++it)
    {
        IFilePackage* pPak = CreateFilePackInterface();
        bRetCode = pPak->Load((*it).c_str());
        if (!bRetCode)
        {
            XY_COM_RELEASE(pPak);
            continue;
        }

        m_PakTable.push_back(pPak);
    }

    bResult = true;
    return bResult;
}

BOOL XFilePackageMulti::IsFileExist(const char wszFilePath[])
{
    BOOL            bRetCode = false;
    BOOL            bResult  = false;
    int             nIndex   = INVALID_FILE_INDEX;
    IFilePackage*   pPak     = NULL;

    bRetCode = QueryFile(wszFilePath, &nIndex, &pPak);
    XY_FAILED_JUMP(bRetCode);
      
    bResult = true;
Exit0:
    return bResult;
}

BOOL XFilePackageMulti::GetFileSize( size_t* puRetSize, const char wszFilePath[] )
{
    BOOL            bRetCode = false;
    size_t          uResult  = 0;
    int             nIndex   = INVALID_FILE_INDEX;
    IFilePackage*   pPak     = NULL;

    bRetCode = QueryFile(wszFilePath, &nIndex, &pPak);
    XY_FAILED_JUMP(bRetCode);

    uResult = pPak->GetFileSize(puRetSize, nIndex);
Exit0:
    return uResult;
}

BOOL XFilePackageMulti::QueryFile(const char wszFilePath[], int* pFileIndex, IFilePackage** ppFilePak)
{
    BOOL bResult  = false;

    *pFileIndex = INVALID_FILE_INDEX;

    for (XPakFileTable::iterator it = m_PakTable.begin(); it != m_PakTable.end(); ++it)
    {
        int nIndex = (*it)->QueryFile(wszFilePath);
        if (nIndex != INVALID_FILE_INDEX)
        {
            *pFileIndex = nIndex;
            *ppFilePak = *it;

            break;
        }        
    }
    XY_FAILED_JUMP(*pFileIndex != INVALID_FILE_INDEX);

    bResult = true;
Exit0:
    return bResult;

}

size_t XFilePackageMulti::ReadFile(void* pvBuffer, size_t uBufferSize, const char wszFilePath[], size_t uFileOffset)
{
    BOOL            bRetCode = false;
    size_t          uResult  = 0;
    int             nIndex   = INVALID_FILE_INDEX;
    IFilePackage*   pPak     = NULL;

    bRetCode = QueryFile(wszFilePath, &nIndex, &pPak);
    XY_FAILED_JUMP(bRetCode);

    uResult = pPak->ReadFile(pvBuffer, uBufferSize, nIndex, uFileOffset);

Exit0:
    return uResult;
}

time_t XFilePackageMulti::GetFileModifyTime(const char wszFilePath[])
{
    BOOL            bRetCode = false;
    int             nIndex   = INVALID_FILE_INDEX;
    IFilePackage*   pPak     = NULL;

    bRetCode = QueryFile(wszFilePath, &nIndex, &pPak);
    XY_FAILED_JUMP(bRetCode);

    return pPak->GetFileModifyTime(nIndex);
Exit0:
    return 0;
}

uint32_t XFilePackageMulti::GetPakMakeTime(int nPakIndex)
{
    if (nPakIndex >= 0 && nPakIndex < (int)m_PakTable.size())
    {
        return m_PakTable[nPakIndex]->GetPakMakeTime();
    }
    return 0;
}

void XFilePackageMulti::Close()
{
    for (XPakFileTable::iterator it = m_PakTable.begin(); it != m_PakTable.end(); ++it)
    {
        XY_COM_RELEASE(*it);
    }
    m_PakTable.clear();
}
