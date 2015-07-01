#pragma once

struct IFilePackage;

class XFilePackageMulti
{
public:
    XFilePackageMulti();
    ~XFilePackageMulti();

    BOOL    Load(const char wszIndexFile[]);
    BOOL    Reload();

    BOOL    IsFileExist(const char wszFilePath[]);
    BOOL    GetFileSize(size_t* puRetSize, const char wszFilePath[]);
    size_t  ReadFile(void* pvBuffer, size_t uBufferSize, const char wszFilePath[], size_t uFileOffset);
    time_t  GetFileModifyTime(const char wszFilePath[]);

    uint32_t GetPakMakeTime(int nPakIndex);

private:
    BOOL    QueryFile(const char wszFilePath[], int* pFileIndex, IFilePackage** ppFilePak);
    void    Close();

private:
    typedef std::vector<IFilePackage*> XPakFileTable;
    typedef std::vector<std::string> XPakFileNameTable;
    
    XPakFileTable m_PakTable;
    XPakFileNameTable m_PakFileNameTable;
};

extern XFilePackageMulti* g_pFilePackageMulti;
