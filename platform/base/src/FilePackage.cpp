#include "Base.h"
#include "FileIndex.h"
#include "FileTree.h"
#include <time.h>

using namespace std;

#define MAX_PACK_FILE_SIZE  ((uint64_t)1000 * 1000 * 1000 * 2)

class XFilePackage : public IFilePackage
{
public:
    XFilePackage();
    ~XFilePackage();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) { return -1; }
    virtual ULONG STDMETHODCALLTYPE AddRef(){ return XY_InterlockedIncrement(&m_ulRefCount); }
    virtual ULONG STDMETHODCALLTYPE Release();

    BOOL    Save();    
    BOOL    Load(const char wszIndexFile[]);

    int     QueryFile(const char wszFilePath[]);
    BOOL    GetFileSize(size_t* puRetSize, int nFileIndex);
    size_t  ReadFile(void* pvBuffer, size_t uBufferSize, int nFileIndex, size_t uFileOffset);
    time_t  GetFileModifyTime(int nFileIndex);

    BOOL    UpdateFile(const char wszFilePath[], time_t uModifyTime, BYTE* pbyData, size_t uDataLen);
    BOOL    DeleteFile(const char wszFilePath[]);

    uint32_t GetPakMakeTime();
    
private:
    BOOL    ReadFileData(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uOffset, size_t uRequestSize);
    BOOL    WriteFileData(uint64_t uOffset, const BYTE* pbyData, size_t uDataLen);
    FILE*   OpenPakFile(int nFileIndex, BOOL bCreateIfNotExist = false);

private:
    typedef std::vector<FILE*> XPakFileTable;

    XPakFileTable       m_PakFileTable;
    XFileIndex          m_FileIndex;
    string              m_wstrDataFilePrefix;
    XMutex              m_Mutex;
    volatile ULONG      m_ulRefCount;
};

IFilePackage* CreateFilePackInterface()
{	
    return new XFilePackage();
}

XFilePackage::XFilePackage()
{
    m_ulRefCount = 1;
}

XFilePackage::~XFilePackage()
{
    for (size_t i = 0; i < m_PakFileTable.size(); ++i)
    {
        XY_CLOSE_FILE(m_PakFileTable[i]);
    }
    m_PakFileTable.clear();
}

ULONG XFilePackage::Release()
{
    ULONG ulRefCount = XY_InterlockedDecrement(&m_ulRefCount);

    if (ulRefCount == 0)
    {
        delete this;
    }

    return ulRefCount;
}

BOOL XFilePackage::Save()
{
    m_Mutex.Lock();
    for (size_t i = 0; i < m_PakFileTable.size(); ++i)
    {
        FILE* pFile = m_PakFileTable[i];

        if (pFile)
        {
            fflush(pFile);
        }
    }
    m_Mutex.Unlock();

    return m_FileIndex.Save();
}
     
BOOL XFilePackage::Load(const char wszIndexFile[])
{
    const char*  pwszPos = strrchr(wszIndexFile, '.');
    int             nLen    = 0;

    m_wstrDataFilePrefix = wszIndexFile;

    if (pwszPos)
    {
        nLen = (int)(pwszPos - wszIndexFile);

        m_wstrDataFilePrefix.resize(nLen);
    }

    return m_FileIndex.Load(wszIndexFile);
}

int XFilePackage::QueryFile(const char wszFilePath[])
{
    return m_FileIndex.QueryFile(wszFilePath);
}

BOOL XFilePackage::GetFileSize(size_t* puRetSize, int nFileIndex)
{
    XFileBlock* pFileBlock = m_FileIndex.GetFile(nFileIndex);
    if (pFileBlock && pFileBlock->bIsUsing)
    {
        *puRetSize = pFileBlock->uDataLen;
        return true;
    }
    
    return false;
}

size_t XFilePackage::ReadFile(void* pvBuffer, size_t uBufferSize, int nFileIndex, size_t uFileOffset)
{
    size_t          uResult         = 0;
    size_t          uReadLen        = 0;
    int 			nRetCode        = 0;
    XFileBlock*     pFileBlock      = m_FileIndex.GetFile(nFileIndex);

    XYLOG_FAILED_JUMP(pFileBlock);
    XYLOG_FAILED_JUMP(uFileOffset <= pFileBlock->uDataLen);

    uReadLen = Min(pFileBlock->uDataLen - uFileOffset, uBufferSize);

    nRetCode = ReadFileData((BYTE*)pvBuffer, uBufferSize, pFileBlock->uOffset + uFileOffset, uReadLen);
    XYLOG_FAILED_JUMP(nRetCode);
    
    uResult = uReadLen;
Exit0:
    return uResult;
}

time_t XFilePackage::GetFileModifyTime(int nFileIndex)
{
    XFileBlock* pFileBlock = m_FileIndex.GetFile(nFileIndex);
    if (pFileBlock)
    {
        return (time_t)pFileBlock->uLastModifyTime;
    }

    return 0;
}

BOOL XFilePackage::UpdateFile(const char wszFilePath[], time_t uModifyTime, BYTE* pbyData, size_t uDataLen)
{
    BOOL             bResult         = false;
    int              nRetCode        = false;
    XFileBlock*      pFileBlock      = NULL;

    pFileBlock = m_FileIndex.UpdateFile(wszFilePath, uModifyTime, uDataLen);
    XYLOG_FAILED_JUMP(pFileBlock);

    nRetCode = WriteFileData(pFileBlock->uOffset, pbyData, pFileBlock->uDataLen);
    XYLOG_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XFilePackage::DeleteFile(const char wszFilePath[])
{
    return m_FileIndex.DeleteFile(wszFilePath);
}

uint32_t XFilePackage::GetPakMakeTime()
{
    return m_FileIndex.GetPakMakeTime();
}

BOOL XFilePackage::ReadFileData(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uOffset, size_t uRequestSize)
{
    BOOL        bResult         = false;
    int         nRetCode        = 0;
    size_t      uBufferPos      = 0;

    m_Mutex.Lock();

    XYLOG_FAILED_JUMP(uRequestSize <= uBufferSize);

    while (uRequestSize > 0)
    {
        int         nPakFileIndex   = (int)(uOffset / MAX_PACK_FILE_SIZE);
        int32_t     nPakFileOffset  = (int32_t)(uOffset % MAX_PACK_FILE_SIZE);
        size_t      uRead           = Min((size_t)(MAX_PACK_FILE_SIZE - nPakFileOffset), uRequestSize);
        FILE*       pFile           = OpenPakFile(nPakFileIndex);

        assert(nPakFileOffset >= 0);
        assert(nPakFileOffset <= MAX_PACK_FILE_SIZE);

        XYLOG_FAILED_JUMP(pFile);

        nRetCode = fseek(pFile, nPakFileOffset, SEEK_SET);
        XYLOG_FAILED_JUMP(nRetCode == 0);

        nRetCode = (int)fread(pbyBuffer + uBufferPos, uRead, 1, pFile);
        // XYLOG_FAILED_JUMP(nRetCode == 1);
        if (nRetCode != 1)
        {
            Log(eLogDebug, "errno:%d \t uRead:%d \t uBufferPos:%d \t nPakFileOffset:%lld", errno, uRead, uBufferPos, nPakFileOffset);
            
            goto Exit0;
        }

        EncryptData(pbyBuffer + uBufferPos, uRead, uOffset);

        uBufferPos      += uRead;
        uOffset         += uRead;
        uRequestSize    -= uRead;
    }

    bResult = true;
Exit0:
    m_Mutex.Unlock();
    return bResult;
}

BOOL XFilePackage::WriteFileData(uint64_t uOffset, const BYTE* pbyData, size_t uDataLen)
{
    BOOL        bResult         = false;
    int         nRetCode        = 0;
    size_t      uBufferPos      = 0;
    BYTE*       pbyBuffer       = (BYTE*)MemDup(pbyData, uDataLen);

    EncryptData(pbyBuffer, uDataLen, uOffset);

    m_Mutex.Lock();

    while (uDataLen > 0)
    {
        int         nPakFileIndex   = (int)(uOffset / MAX_PACK_FILE_SIZE);
        int32_t     nPakFileOffset  = (int32_t)(uOffset % MAX_PACK_FILE_SIZE);
        size_t      uWrite          = Min((size_t)(MAX_PACK_FILE_SIZE - nPakFileOffset), uDataLen);
        FILE*       pFile           = OpenPakFile(nPakFileIndex, true);

        assert(nPakFileOffset >= 0);
        assert(nPakFileOffset <= MAX_PACK_FILE_SIZE);

        XYLOG_FAILED_JUMP(pFile);

        nRetCode = fseek(pFile, nPakFileOffset, SEEK_SET);
        XYLOG_FAILED_JUMP(nRetCode == 0);

        nRetCode = (int)fwrite(pbyBuffer + uBufferPos, uWrite, 1, pFile);
        XY_FAILED_JUMP(nRetCode == 1);

        uBufferPos      += uWrite;
        uOffset         += uWrite;
        uDataLen        -= uWrite;
    }

    bResult = true;
Exit0:
    m_Mutex.Unlock();
    XY_FREE(pbyBuffer);
    return bResult;
}

FILE* XFilePackage::OpenPakFile(int nFileIndex, BOOL bCreateIfNotExist)
{
    FILE*   pFile           = NULL;
    int     nRetCode        = 0;
    int     nFileCount      = (int)m_PakFileTable.size();
    char wszPakFileName[XY_MAX_PATH];

    for (int i = nFileCount; i <= nFileIndex; i++)
    {
        m_PakFileTable.push_back(NULL);
    }

    pFile = m_PakFileTable[nFileIndex];
    if (pFile == NULL)
    {
        nRetCode = snprintf(wszPakFileName, _countof(wszPakFileName), "%s%d.dat", m_wstrDataFilePrefix.c_str(), nFileIndex);
        XYLOG_FAILED_JUMP(nRetCode > 0 && nRetCode < _countof(wszPakFileName));

        if (bCreateIfNotExist)
        {
            pFile = fopen(wszPakFileName, "wb");
        }
        else
        {
            pFile = fopen(wszPakFileName, "rb");
        }
        
        m_PakFileTable[nFileIndex] = pFile;
    }

Exit0:
    return pFile;
}
