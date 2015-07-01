#pragma once

#define INVALID_FILE_INDEX (-1)

struct IFilePackage : IUnknown
{
    virtual BOOL    Save() = 0;    
    virtual BOOL    Load(const char wszIndexFile[]) = 0;

    virtual int     QueryFile(const char wszFilePath[]) = 0;
    virtual BOOL    GetFileSize(size_t* puRetSize, int nFileIndex) = 0;
    virtual size_t  ReadFile(void* pvBuffer, size_t uBufferSize, int nFileIndex, size_t uFileOffset) = 0;
    virtual time_t  GetFileModifyTime(int nFileIndex) = 0;

    virtual BOOL    UpdateFile(const char wszFilePath[], time_t uModifyTime, BYTE* pbyData, size_t uDataLen) = 0;
    virtual BOOL    DeleteFile(const char wszFilePath[]) = 0;

    virtual uint32_t GetPakMakeTime() = 0;
};

IFilePackage* CreateFilePackInterface();
