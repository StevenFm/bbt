//pragma once

struct XFileAssist : XFileHelper
{
    XFileAssist();
	virtual ~XFileAssist();
    
	virtual BYTE* ReadFileData(size_t* puSize, const char szFileName[], size_t uExtSize = 0);
	virtual BOOL WriteFileData(const char szFileName[], const void* pvData, size_t uDataLen);
    
	virtual BOOL GetDirList(std::list<std::string>& dirList, const char szDir[], BOOL bRecursion = false, BOOL bRetRelativePath = false);
	virtual BOOL GetFileList(std::list<std::string>& fileList, const char szDir[], BOOL bRecursion = false, BOOL bRetRelativePath = false);
    
    virtual time_t GetFileModifyTime(const char szFileName[]);
};
