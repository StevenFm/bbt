#pragma once

#include <list>
#include <string>

// FileHelper的设计初衷是为了解决基础库代码中文件访问需要在不同平台区别处理的问题
// 比如基础库里面的tab,ini,lua等模块做文件操作时会调用这个接口
// 具体平台(如iOS)下,如果需要对文件路径之类的作特殊处理,那么继承这个类,改写下面的g_pFileHelper指针即可
// 对于非基础库代码模块(如客户端,服务端)自己的文件操作,可以调用这个接口,也可以自己另行实现


struct XFileHelper
{
public:
	XFileHelper();
	virtual ~XFileHelper();

	// 取得相对路径的全路径
	// 如果传入的是全路径,则原样返回
	virtual BOOL GetFullPath(std::string& strFullPath, const char szPath[]);
	virtual BOOL GetRelativePath(std::string& strRelativePath, const char szRoot[], const char szPath[]);

	// 返回的扩展名是包含'.'的,比如".cpp"
	virtual BOOL GetFileExtName(std::string& strExtName, const char szPath[]);
	virtual BOOL GetFileNameFromPath(std::string& strFileName, const char szPath[]);
	virtual BOOL GetFileNameWithoutExt(std::string& strFileName, const char szPath[]);
	virtual BOOL GetDirFromPath(std::string& strDir, const char szPath[]);

	virtual time_t GetFileModifyTime(const char szFileName[]);

	// 读取整个文件的数据
	// 如果失败,返回空,只要是成功的,返回就不是空(哪怕文件为空)
	// uExtSize用于在文件大小的基础上额外扩大返回buffer的分配(扩大的这块内存数据是未定义的),避免在某些情况下无谓的数据拷贝
	// 调用者有责任释放返回的buffer: delete[] pbyBuffer;
	virtual BYTE* ReadFileData(size_t* puSize, const char szFileName[], size_t uExtSize = 0);

	virtual BOOL WriteFileData(const char szFileName[], const void* pvData, size_t uDataLen);

	// 注意,传入的dirList, fileList如果不是空的,函数不会自动清空
	// 默认返回的路径是全路径,如果指定了bRetRelativePath标志,则返回相对于szDir的路径
	virtual BOOL GetDirList(std::list<std::string>& dirList, const char szDir[], BOOL bRecursion = false, BOOL bRetRelativePath = false);
	virtual BOOL GetFileList(std::list<std::string>& fileList, const char szDir[], BOOL bRecursion = false, BOOL bRetRelativePath = false);
};

// 这个指针供基础库(tab,ini,lua)做文件操之用,外部如需要改变相关行为,请继承XFileHelper,改写这个指针即可
// 该指针默认指向一个内部全局对象
extern XFileHelper* g_pFileHelper;



