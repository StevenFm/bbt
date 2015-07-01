#include "Base.h"
#include "FileIndex.h"

#define MAX_INDEX_FILE_SIZE     (1024 * 1024 * 16)
#define INDEX_FILE_VERSION      (0)

#pragma pack(1)
struct XIndexFileHeader
{
    int32_t     nVersion;
    int32_t     nBlockCount;
    uint32_t    uMakePakTime;
};

struct XFileBlockData
{
    uint64_t uOffset;
    BYTE     byIsUsing;
    uint32_t uDataLen;
    uint32_t uBlockSize;
    uint32_t uLastModifyTime;   // time_t
};
#pragma pack()
// 索引文件格式:
// XIndexFileHeader + XFileBlock[...] + XTreeNodeData[...];

extern BOOL GetFileSize(int64_t* pnFileSize, FILE* pFile);

static BOOL IsBlockTooLarge(size_t uBlockSize, size_t uDataLen)
{
    if (uBlockSize < 1024)
    {
        return false;
    }

    if (uBlockSize > uDataLen / 8 * 9)
    {
        return true;
    }

    return false;
}

BOOL XFileIndex::Save()
{
    BOOL			    bResult			    = false;
    int			        nRetCode			= false;
    BYTE*               pbyBuffer           = (BYTE*)malloc(MAX_INDEX_FILE_SIZE);
    BYTE*               pbyPos              = pbyBuffer;
    size_t              uLeftSize           = MAX_INDEX_FILE_SIZE;
    size_t              uUsedSize           = 0;
    const char*      pwszIndexFileName   = m_wstrIndexFileName.c_str();
    FILE*               pFile               = NULL;
    XIndexFileHeader*   pFileHeader         = NULL;

    XYLOG_FAILED_JUMP(pbyBuffer);
    XYLOG_FAILED_JUMP(uLeftSize >= sizeof(XIndexFileHeader));
    pFileHeader = (XIndexFileHeader*)pbyPos;
    pFileHeader->nVersion    = INDEX_FILE_VERSION;
    pFileHeader->nBlockCount = (int)m_BlockTable.size();
    pFileHeader->uMakePakTime = (uint32_t)time(NULL);

    pbyPos += sizeof(XIndexFileHeader);
    uLeftSize -= sizeof(XIndexFileHeader);

    nRetCode = SaveBlockTable(&uUsedSize, pbyPos, uLeftSize);
    XYLOG_FAILED_JUMP(nRetCode);

    pbyPos += uUsedSize;
    uLeftSize -= uUsedSize;

    nRetCode = m_FileTree.Save(&uUsedSize, pbyPos, uLeftSize);
    XYLOG_FAILED_JUMP(nRetCode);

    pbyPos += uUsedSize;
    uLeftSize -= uUsedSize;

    EncryptData(pbyBuffer, MAX_INDEX_FILE_SIZE - uLeftSize, 0);

    pFile = fopen(pwszIndexFileName, "wb");
    XYLOG_FAILED_JUMP(pFile);

    nRetCode = (int)fwrite(pbyBuffer, MAX_INDEX_FILE_SIZE - uLeftSize, 1, pFile);
    XYLOG_FAILED_JUMP(nRetCode == 1);

    bResult = true;
Exit0:
    XY_CLOSE_FILE(pFile);
    XY_FREE(pbyBuffer);
    return bResult;
}

BOOL XFileIndex::Load(const char wszIndexFile[])
{
    BOOL	            bResult			    = false;
    int			        nRetCode			= false;
    uint64_t            uFileSize           = 0;
    BYTE*               pbyBuffer           = NULL;
    FILE*               pFile               = NULL;
    BYTE*               pbyPos              = NULL;
    BYTE*               pbyEnd              = NULL;
    size_t              uLeftDataLen        = 0;
    XIndexFileHeader*   pFileHeader         = NULL;

    m_wstrIndexFileName = wszIndexFile;

    pFile = fopen(wszIndexFile, "rb");
    if(!pFile)
    {
        int nErrorNo = errno;
        if (nErrorNo != ENOENT)
        {
            Log(eLogDebug, "XFileIndex::Load open file failed:%d.", nErrorNo);
        }
        goto Exit0;
    }

    nRetCode = XY_GetFileSize(&uFileSize, pFile);
    XYLOG_FAILED_JUMP(nRetCode);

    pbyBuffer = (BYTE*)malloc((size_t)uFileSize);
    XYLOG_FAILED_JUMP(pbyBuffer);

    nRetCode = (int)fread(pbyBuffer, (size_t)uFileSize, 1, pFile);
    XYLOG_FAILED_JUMP(nRetCode == 1);

    EncryptData(pbyBuffer, (size_t)uFileSize, 0);

    pbyPos = pbyBuffer;
    pbyEnd = pbyPos + uFileSize;
    uLeftDataLen = (size_t)uFileSize;

    XYLOG_FAILED_JUMP(uLeftDataLen >= sizeof(XIndexFileHeader));
    pFileHeader = (XIndexFileHeader*)pbyPos;
    XYLOG_FAILED_JUMP(pFileHeader->nVersion == INDEX_FILE_VERSION);
    XYLOG_FAILED_JUMP(pFileHeader->nBlockCount >= 0);
    m_uMakePakTime = pFileHeader->uMakePakTime;

    pbyPos += sizeof(XIndexFileHeader);
    uLeftDataLen -= sizeof(XIndexFileHeader);

    pbyPos = LoadBlockTable(pbyPos, uLeftDataLen, pFileHeader->nBlockCount);
    XYLOG_FAILED_JUMP(pbyPos);

    uLeftDataLen = (size_t)(pbyEnd - pbyPos);

    nRetCode = m_FileTree.Load(pbyPos, uLeftDataLen);
    XYLOG_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    XY_FREE(pbyBuffer);
    XY_CLOSE_FILE(pFile);
    return bResult;
}

BOOL XFileIndex::SaveBlockTable(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL		        bResult		    = false;
    BYTE*               pbyPos          = pbyBuffer;
    size_t              uLeftSize       = uBufferSize;
    int                 nBlockCount     = (int)m_BlockTable.size();
    size_t              uDataLen        = sizeof(XFileBlockData) * nBlockCount;
    XFileBlockData*     pBlockData      = (XFileBlockData*)pbyBuffer;

    XYLOG_FAILED_JUMP(uLeftSize >= uDataLen);

    for (int i = 0; i < nBlockCount; i++)
    {
        XFileBlock* pBlock = &m_BlockTable[i];

        pBlockData->uOffset         = pBlock->uOffset;
        pBlockData->byIsUsing       = (BYTE)pBlock->bIsUsing;
        pBlockData->uDataLen        = pBlock->uDataLen;
        pBlockData->uBlockSize      = pBlock->uBlockSize;
        pBlockData->uLastModifyTime = pBlock->uLastModifyTime;

        pBlockData++;
    }

    pbyPos += uDataLen;
    uLeftSize -= uDataLen;

    *puUsedSize = uBufferSize - uLeftSize;

    bResult = true;
Exit0:
    return bResult;
}

BYTE* XFileIndex::LoadBlockTable(BYTE* pbyData, size_t uDataLen, int nBlockCount)
{
    BYTE*               pbyResult           = NULL;
    size_t              uBlockDataLen       = sizeof(XFileBlockData) * nBlockCount;
    XFileBlockData*     pBlockData          = (XFileBlockData*)pbyData;
    XFileBlock          Block;

    XYLOG_FAILED_JUMP(nBlockCount >= 0);
    XYLOG_FAILED_JUMP(uDataLen >= uBlockDataLen);

    m_BlockTable.clear();
    m_BlockTable.reserve(nBlockCount);

    for (int i = 0; i < nBlockCount; i++)
    {
        Block.uOffset           = pBlockData->uOffset;
        Block.bIsUsing          = pBlockData->byIsUsing;
        Block.uDataLen          = pBlockData->uDataLen;
        Block.uBlockSize        = pBlockData->uBlockSize;
        Block.uLastModifyTime   = pBlockData->uLastModifyTime;

        m_BlockTable.push_back(Block);

        pBlockData++;
    }

    pbyResult = pbyData + uBlockDataLen;
Exit0:
    return pbyResult;
}

int XFileIndex::QueryFile(const char wszFilePath[])
{
    XTreeFileNode* pFileNode = m_FileTree.GetFileNode(wszFilePath, false);

    if (pFileNode)
    {
        return pFileNode->nFileIndex;
    }

    return INVALID_FILE_INDEX;    
}

XFileBlock* XFileIndex::GetFile(int nFileIndex)
{
    if (nFileIndex >= 0 && nFileIndex < (int)m_BlockTable.size())
    {
        return &m_BlockTable[nFileIndex];
    }
    return NULL;
}

XFileBlock* XFileIndex::UpdateFile(const char wszFilePath[], time_t uModifyTime, size_t uFileSize)
{
    XFileBlock*    pResult      = NULL;
    int            nRetCode     = 0;
    XTreeFileNode* pFileNode    = m_FileTree.GetFileNode(wszFilePath, true);
    int            nNewBlock    = INVALID_FILE_INDEX;
    XFileBlock*    pFileBlock   = NULL;

    XYLOG_FAILED_JUMP(pFileNode);

    if (pFileNode->nFileIndex != INVALID_FILE_INDEX)
    {
        pFileBlock = &m_BlockTable[pFileNode->nFileIndex];

        if (uFileSize <= pFileBlock->uBlockSize)
        {
            nRetCode = IsBlockTooLarge(pFileBlock->uBlockSize, uFileSize);
            XY_SUCCESS_JUMP(!nRetCode);
        }

        pFileBlock->bIsUsing   = false;
        pFileNode->nFileIndex = INVALID_FILE_INDEX;
    }

    assert(pFileNode->nFileIndex == INVALID_FILE_INDEX);

    nNewBlock = FindAvailableFileBlock(uFileSize);
    if (nNewBlock == INVALID_FILE_INDEX)
    {
        nNewBlock = CreateNewFileBlock(uFileSize);        
    }
    XYLOG_FAILED_JUMP(nNewBlock != INVALID_FILE_INDEX);

    pFileNode->nFileIndex = nNewBlock;

Exit1:
    pFileBlock = &m_BlockTable[pFileNode->nFileIndex];

    pFileBlock->uDataLen        = (uint32_t)uFileSize;
    pFileBlock->bIsUsing        = true;
    pFileBlock->uLastModifyTime = (uint32_t)uModifyTime;

    pResult = pFileBlock;
Exit0:
    return pResult;
}

BOOL XFileIndex::DeleteFile(const char wszFilePath[])
{
    BOOL            bResult     = false;
    int             nFileIndex  = m_FileTree.DelFileNode(wszFilePath);

    XYLOG_FAILED_JUMP(nFileIndex != INVALID_FILE_INDEX);

    m_BlockTable[nFileIndex].bIsUsing = false;

    bResult = true;
Exit0:
    return bResult;
}   

uint32_t XFileIndex::GetPakMakeTime()
{
    return m_uMakePakTime;
}

int XFileIndex::CreateNewFileBlock(size_t uDataSize)
{
    int            nBlockCount    = (int)m_BlockTable.size();
    XFileBlock*    pLastFileBlock = NULL;
    uint64_t       uOffset        = 0;
    XFileBlock     NewFileBlock;

    if (nBlockCount > 0)
    {
        pLastFileBlock = &m_BlockTable[nBlockCount - 1]; 

        uOffset = pLastFileBlock->uOffset + pLastFileBlock->uBlockSize;
    }

    NewFileBlock.uOffset    = uOffset;
    NewFileBlock.uBlockSize = (uint32_t)uDataSize;
    NewFileBlock.uDataLen   = 0;
    NewFileBlock.bIsUsing   = false;

    m_BlockTable.push_back(NewFileBlock);

    return nBlockCount;
}

int XFileIndex::FindAvailableFileBlock(size_t uDataLen)
{
    int            nResult          = INVALID_FILE_INDEX;
    int            nRetCode         = false;
    int            nBlockCount      = (int)m_BlockTable.size();
    XFileBlock*     pFileBlock       = NULL;
    int            nSelectBlock     = INVALID_FILE_INDEX;

    for (int i = 0; i < nBlockCount; ++i)
    {
        pFileBlock = &m_BlockTable[i];

        if (pFileBlock->bIsUsing)
            continue;

        if (pFileBlock->uBlockSize < uDataLen)
            continue;

        if (nSelectBlock == INVALID_FILE_INDEX || m_BlockTable[nSelectBlock].uBlockSize > pFileBlock->uBlockSize)
        {
            nSelectBlock = i;
        }
    }

    XY_FAILED_JUMP(nSelectBlock != INVALID_FILE_INDEX);

    nRetCode = IsBlockTooLarge(m_BlockTable[nSelectBlock].uBlockSize, uDataLen);
    XY_FAILED_JUMP(!nRetCode);

    nResult = nSelectBlock;
Exit0:
    return nResult;
}
