#include "stdafx.h"
#include "platform/CCFileUtils.h"
#include "FileAssist.h"

USING_NS_CC;

XFileAssist::XFileAssist()
{
    CCFileUtils* pFileUtils = CCFileUtils::sharedFileUtils();
    
    std::string strWritePath = pFileUtils->getWritablePath();
    
    std::vector<std::string> pathTable;
    
    pathTable.push_back(strWritePath);
    
    pFileUtils->setSearchPaths(pathTable);
}

XFileAssist::~XFileAssist()
{
}

BYTE* XFileAssist::ReadFileData(size_t* puSize, const char szFileName[], size_t uExtSize)
{
    CCFileUtils* pFileUtils = CCFileUtils::sharedFileUtils();
    
	// 注意有个坑: 安卓下面,fullPathForFilename返回的不见得是fullPath
	// 如果根据规则(searchPath + RelativePath + Resolution + FileName),在APK(Zip)包里面找到了这个文件
	// 那么返回的就只是一个相对路径(相对于APK的root)
    //std::string strFullPath = pFileUtils->fullPathForFilename(szFileName);

	BYTE* pbyData = pFileUtils->getFileData(szFileName, "rb", (ssize_t*)puSize);
	if (pbyData == NULL || uExtSize == 0)
		return pbyData;

	BYTE* pbyBuffer = new BYTE[*puSize + uExtSize];

	memcpy(pbyBuffer, pbyData, *puSize);

	CC_SAFE_DELETE_ARRAY(pbyData);

	return pbyBuffer;
}

BOOL XFileAssist::WriteFileData(const char szFileName[], const void* pvData, size_t uDataLen)
{
    CCFileUtils* pFileUtils = CCFileUtils::sharedFileUtils();
    
    std::string strWritePath = pFileUtils->getWritablePath();
    
    strWritePath.push_back(DIR_SPRIT);
    
    strWritePath += szFileName;
    
    return XFileHelper::WriteFileData(strWritePath.c_str(), pvData, uDataLen);
}

BOOL XFileAssist::GetDirList(std::list<std::string>& dirList, const char szDir[], BOOL bRecursion, BOOL bRetRelativePath)
{
    // 基于cocos2d-x的文件访问设计,目前遍历目录有很多坑,暂时不要调用这个方法:)
    return false;
}

BOOL XFileAssist::GetFileList(std::list<std::string>& fileList, const char szDir[], BOOL bRecursion, BOOL bRetRelativePath)
{
    // 基于cocos2d-x的文件访问设计,目前遍历目录有很多坑,暂时不要调用这个方法:)
    return false;
}

time_t XFileAssist::GetFileModifyTime(const char szFileName[])
{
    CCFileUtils* pFileUtils = CCFileUtils::sharedFileUtils();
    
	// 注意有个坑: 安卓下面,fullPathForFilename返回的不见得是fullPath
	// 如果根据规则(searchPath + RelativePath + Resolution + FileName),在APK(Zip)包里面找到了这个文件
	// 那么返回的就只是一个相对路径(相对于APK的root)
    std::string strFullPath = pFileUtils->fullPathForFilename(szFileName);

    return XFileHelper::GetFileModifyTime(strFullPath.c_str());
}