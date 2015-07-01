#pragma once
#include "FileTree.h"

struct XFileBlock
{
    uint64_t uOffset;
    BOOL     bIsUsing;
    uint32_t uDataLen;
    uint32_t uBlockSize;
    uint32_t uLastModifyTime;   // time_t
};

class XFileIndex
{
public:
    BOOL Save();    
    BOOL Load(const char wszIndexFile[]);

    int         QueryFile(const char wszFilePath[]);
    XFileBlock* GetFile(int nFileIndex);
    XFileBlock* UpdateFile(const char wszFilePath[], time_t uModifyTime, size_t uFileSize);
    BOOL        DeleteFile(const char wszFilePath[]);
    uint32_t    GetPakMakeTime();
        
private:
    BOOL  SaveBlockTable(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BYTE* LoadBlockTable(BYTE* pbyData, size_t uDataLen, int nBlockCount);

    int  CreateNewFileBlock(size_t uDataSize);
    int  FindAvailableFileBlock(size_t uDataLen);

private:
    typedef std::vector<XFileBlock> XBlockTable;

    XBlockTable         m_BlockTable;
    XFileTreeFolder     m_FileTree;
    std::string         m_wstrIndexFileName;
    uint32_t            m_uMakePakTime;
};
