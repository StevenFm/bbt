#include <stdio.h>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include "Base.h"

using namespace std;

static XFileHelper g_FileHelper;

XFileHelper* g_pFileHelper = &g_FileHelper;

static BOOL IsAbsolutePath(const char szPath[])
{
#ifdef _MSC_VER
	int nLength = (int)strlen(szPath);
    if (nLength >= 2 && szPath[1] == ':')
		return true;
#endif

#if defined(__linux) || defined(__APPLE__)
	if (szPath[0] == '/')
		return true;
#endif

	return false;
}

static void DelEndSprit(std::string& strPath)
{
	int nLen = strPath.length();
	if (nLen < 1)
		return;

	if (strPath[nLen - 1] == '/' || strPath[nLen - 1] == '\\')
		strPath.erase(nLen - 1, 1);
}

// 把'/'和'\\'转换成平台对应的
static void TransformDirSprit(std::string& strPath)
{
	int nLength = (int)strPath.length();

	for (int i = 0; i <= nLength; ++i)
	{
#ifdef _MSC_VER
		if (strPath[i] == '/')
			strPath[i] = '\\';
#endif

#if defined(__linux) || defined(__APPLE__)
		if (strPath[i] == '\\')
			strPath[i] = '/';
#endif
	}
}

// 找出路径中文件名的开始位置
static const char* GetFileNamePos(const char* pszPath)
{
	int			nLen   = (int)strlen(pszPath);
	const char* pszPos = pszPath + nLen;

	while (pszPos-- > pszPath)
	{
		if (*pszPos == '\\' || *pszPos == '/')
			return pszPos + 1;
	}

	return pszPath;
}

// 拆分路径
static BOOL SplitPath(std::vector<std::string>& tokenList, const char* pszPath)
{
	BOOL		bResult		= false;
	const char* pszToken	= pszPath;
	const char* pszPos		= pszPath;

	tokenList.clear();
	tokenList.reserve(32);
	do
	{
		if (*pszPos == '/' || *pszPos == '\\' || *pszPos == '\0')
		{
			if (pszPos > pszToken)
			{
				tokenList.push_back(std::string(pszToken, pszPos));
			}
			pszToken = pszPos + 1;
		}
	} while (*pszPos++);

	bResult = true;
	return bResult;
}

#ifdef _MSC_VER
static BOOL ListDir(std::list<std::string>& retList, const char* pszDir, BOOL bDir, BOOL bRecursion)
{
	BOOL	bResult			 = false;
	int		nRetCode		 = 0;
	char*	pszPath			 = NULL;
	HANDLE	hFind			 = NULL;
	WIN32_FIND_DATAA findData;

	pszPath = (char*)malloc(XY_MAX_PATH);
	XY_FAILED_JUMP(pszPath);

	nRetCode = snprintf(pszPath, XY_MAX_PATH, "%s%s%s", pszDir, pszDir[0] ? "\\" : "", "*");
	XY_FAILED_JUMP(nRetCode > 0 && nRetCode < XY_MAX_PATH);

	hFind = FindFirstFileA(pszPath, &findData);
	XY_FAILED_JUMP(hFind != INVALID_HANDLE_VALUE);

	do
	{
		nRetCode = strcmp(findData.cFileName, ".");
		if (nRetCode == 0)
			continue;

		nRetCode = strcmp(findData.cFileName, "..");
		if (nRetCode == 0)
			continue;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			continue;

		nRetCode = snprintf(pszPath, XY_MAX_PATH, "%s%s%s", pszDir, pszDir[0] ? "\\" : "", findData.cFileName);
		XY_FAILED_JUMP(nRetCode > 0 && nRetCode < XY_MAX_PATH);

		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bDir)
			retList.push_back(std::string(pszPath));
		else if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) && !bDir)
			retList.push_back(std::string(pszPath));

		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bRecursion)
		{
			nRetCode = ListDir(retList, pszPath, bDir, bRecursion);
			XY_FAILED_JUMP(nRetCode);
		}
	} while (FindNextFileA(hFind, &findData));

	bResult = true;
Exit0:
	XY_FREE(pszPath);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		hFind = NULL;
	}
	return bResult;
}
#endif

#if defined(__linux) || defined(__APPLE__)
static BOOL ListDir(std::list<std::string>& retList, const char* pszDir, BOOL bDir, BOOL bRecursion)
{
	BOOL	bResult	 = false;
	int		nRetCode = false;
	DIR*	pDir	 = NULL;
	dirent*	pEntry	 = NULL;
	int		nType	 = bDir ? DT_DIR : DT_REG;
	char*	pszPath	 = NULL;

	pszPath = (char*)malloc(XY_MAX_PATH);
	XY_FAILED_JUMP(pszPath);

	pDir = pszDir[0] ? opendir(pszDir) : opendir(".");
	XY_FAILED_JUMP(pDir != NULL);

	while (true)
	{
		pEntry = readdir(pDir);
		if (pEntry == NULL)
			break;

		 if (pEntry->d_name[0] == '.')
			continue;

		nRetCode = snprintf(pszPath, XY_MAX_PATH, "%s%s%s", pszDir, pszDir[0] ? "/" : "", pEntry->d_name);
		XY_FAILED_JUMP(nRetCode > 0 && nRetCode < XY_MAX_PATH);

		if (pEntry->d_type == nType)
			retList.push_back(std::string(pszPath));

		if (pEntry->d_type == DT_DIR && bRecursion)
		{
			nRetCode = ListDir(retList, pszPath, bDir, bRecursion);
			XY_FAILED_JUMP(nRetCode);
		}
	}

	bResult = true;
Exit0:
	XY_FREE(pszPath);
	if (pDir != NULL)
	{
		closedir(pDir);
		pDir = NULL;
	}
	return bResult;
}
#endif

XFileHelper::XFileHelper( )
{
}

XFileHelper::~XFileHelper( )
{
}

BOOL XFileHelper::GetFullPath(std::string& strFullPath, const char szPath[])
{
	BOOL		bResult  = false;
	int			nRetCode = 0;
	int			nIndex	 = 0;
	int			nJump	 = 0;
    char*       pszCwd   = NULL;
    std::string strPath  = szPath;
	char		szCwd[XY_MAX_PATH];
	std::vector<std::string> tokenList;
	
	nRetCode = IsAbsolutePath(szPath);
    if (!nRetCode)
    {
        pszCwd = getcwd(szCwd, _countof(szCwd));
        XY_FAILED_JUMP(pszCwd);
        strPath = pszCwd;
        strPath.push_back(DIR_SPRIT);
        strPath += szPath;
    }
    
	// 拆分路径
	nRetCode = SplitPath(tokenList, strPath.c_str());
	XY_FAILED_JUMP(nRetCode);
	
	// 处理路径中的"..",把它们变成"",后续处理的时候会跳过
	nIndex = (int)tokenList.size();
	while (nIndex-- > 0)
	{
		nRetCode = strcmp(tokenList[nIndex].c_str(), "..");
		if (nRetCode == 0)
		{
			nJump++;
			XY_FAILED_JUMP(nJump <= nIndex);
			continue;
		}
        
		if (nJump > 0)
		{
			tokenList[nIndex].clear();
			nJump--;
		}
	}
    
	// 拼接全路径
	strFullPath.clear();
    
	for (int i = 0; i < (int)tokenList.size(); i++)
	{
		if (tokenList[i].empty())
			continue;
        
		nRetCode = strcmp(tokenList[i].c_str(), ".");
		if (nRetCode == 0)
			continue;
        
		nRetCode = strcmp(tokenList[i].c_str(), "..");
		if (nRetCode == 0)
			continue;
        
#if defined(__linux) || defined(__APPLE__)
		strFullPath += "/";
#endif
        
#ifdef _MSC_VER
		if (!strFullPath.empty())
			strFullPath += "\\";
#endif
        
		strFullPath += tokenList[i];
	}
    
	bResult = true;
Exit0:
	return bResult;
}

BOOL XFileHelper::GetRelativePath(std::string& strRelativePath, const char szRoot[], const char szPath[])
{
	BOOL	bResult			= false;
	int		nRetCode		= 0;
	int		nIndex			= 0;
	int		nSize			= 0;
	int		nRootListSize	= 0;
	int		nPathListSize	= 0;
	std::string strRoot;
	std::string strPath;
	std::vector<std::string> rootTokenList;
	std::vector<std::string> pathTokenList;

	nRetCode = GetFullPath(strRoot, szRoot);
	XY_FAILED_JUMP(nRetCode);
	nRetCode = GetFullPath(strPath, szPath);
	XY_FAILED_JUMP(nRetCode);

	nRetCode = SplitPath(rootTokenList, strRoot.c_str());
	XY_FAILED_JUMP(nRetCode);
	nRetCode = SplitPath(pathTokenList, strPath.c_str());
	XY_FAILED_JUMP(nRetCode);

#ifdef _MSC_VER
	// 判断Windows中盘符不同的情况
	nRetCode = STR_CASE_CMP(rootTokenList[0].c_str(), pathTokenList[0].c_str());
	XY_FAILED_JUMP(nRetCode == 0);
#endif

	nRootListSize = (int)rootTokenList.size();
	nPathListSize = (int)pathTokenList.size();
	nSize = min(nRootListSize, nPathListSize);

	for (nIndex = 0; nIndex < nSize; nIndex++)
	{
		nRetCode = STR_CASE_CMP(rootTokenList[nIndex].c_str(), pathTokenList[nIndex].c_str());
		if (nRetCode != 0)
			break;
	}

	strRelativePath.clear();

	if (nIndex != nSize)
	{
		while (nRootListSize-- > nIndex)
		{
#ifdef _MSC_VER
			strRelativePath.append("..\\");
#endif

#if defined(__linux) || defined(__APPLE__)
			strRelativePath.append("../");
#endif
		}
	}

	for (; nIndex < nPathListSize; nIndex++)
	{
		strRelativePath.append(pathTokenList[nIndex]);
		if (nIndex + 1 < nPathListSize)
			strRelativePath.push_back(DIR_SPRIT);
	}

	bResult = true;
Exit0:
	return bResult;
}

BOOL XFileHelper::GetFileExtName(std::string& strExtName, const char szPath[])
{
	const char* pszPos = NULL;

	pszPos = strrchr(szPath, '.');
	if (pszPos != NULL)
	{
		strExtName = pszPos;
		return true;
	}
	return false;
}

BOOL XFileHelper::GetFileNameFromPath(std::string& strFileName, const char szPath[])
{
	const char* pszPos = GetFileNamePos(szPath);

	if (pszPos != NULL && pszPos[0] != '\0')
	{
		strFileName = pszPos;
		return true;
	}
	return false;
}

BOOL XFileHelper::GetFileNameWithoutExt(std::string& strFileName, const char szPath[])
{
	BOOL		bResult	= false;
	const char* pszPos  = NULL;
	const char* pszDot  = NULL;

	pszPos = GetFileNamePos(szPath);
	XY_FAILED_JUMP(pszPos);

	pszDot = strrchr(pszPos, '.');
	if (pszDot)
	{
		strFileName.assign(pszPos, pszDot);
	}
	else
	{
		strFileName = pszPos;
	}

	bResult = true;
Exit0:
	return bResult;
}

BOOL XFileHelper::GetDirFromPath(std::string& strDir, const char szPath[])
{
	const char* pszPos = GetFileNamePos(szPath);

	if (pszPos != NULL)
	{
		if (pszPos > szPath)
			pszPos--;

		strDir.assign(szPath, pszPos);
		return true;
	}
	return false;
}

time_t XFileHelper::GetFileModifyTime(const char szFileName[])
{
	time_t		nResult	 = 0;
	int			nRetCode = 0;
	struct stat	fileInfo;

	nRetCode = stat(szFileName, &fileInfo);
	XY_FAILED_JUMP(nRetCode != -1);

#ifdef __APPLE__
	nResult = fileInfo.st_mtimespec.tv_sec;
#endif

#if defined(_MSC_VER) || defined(__linux)
	nResult = fileInfo.st_mtime;
#endif

Exit0:
	return nResult;
}

BYTE* XFileHelper::ReadFileData(size_t* puSize, const char szFileName[], size_t uExtSize)
{
	BYTE*	 pbyResult	 = NULL;
	BOOL 	 bRetCode	 = 0;
	FILE*	 pFile		 = NULL;
	uint64_t uFileSize	 = 0;
	BYTE*	 pbyBuffer	 = NULL;
	size_t	 uRead		 = 0;

	pFile = fopen(szFileName, "rb");
	XY_FAILED_JUMP(pFile != NULL);

	bRetCode = XY_GetFileSize(&uFileSize, pFile);
	XY_FAILED_JUMP(bRetCode);

	pbyBuffer = new BYTE[(unsigned)uFileSize + uExtSize];
	XY_FAILED_JUMP(pbyBuffer != NULL);

	uRead = fread(pbyBuffer, (size_t)uFileSize, 1, pFile);
	XY_FAILED_JUMP(uRead == 1);

	*puSize = (size_t)uFileSize;
	pbyResult = pbyBuffer;
Exit0:
	if (pbyResult == NULL)
	{
		XY_DELETE_ARRAY(pbyBuffer);
	}
	XY_CLOSE_FILE(pFile);
	return pbyResult;
}

BOOL XFileHelper::WriteFileData(const char szFileName[], const void* pvData, size_t uDataLen)
{
    BOOL   bResult = false;
	size_t uWrite  = 0;
	FILE*  pFile   = NULL;

	pFile = fopen(szFileName, "wb");
	XY_FAILED_JUMP(pFile != NULL);

	uWrite = fwrite(pvData, uDataLen, 1, pFile);
	XY_FAILED_JUMP(uWrite == 1);

	bResult = true;
Exit0:
	XY_CLOSE_FILE(pFile);
	return bResult;
}

BOOL XFileHelper::GetDirList(std::list<std::string>& dirList, const char szDir[], BOOL bRecursion, BOOL bRetRelativePath)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;
	std::string strPath(szDir);
    
	bRetCode = GetFullPath(strPath, szDir);
	XY_FAILED_JUMP(bRetCode);
    
	bRetCode = ListDir(dirList, strPath.c_str(), true, bRecursion);
	XY_FAILED_JUMP(bRetCode);
    
    if (!bRetRelativePath)
        goto Exit1;
    
    for (std::list<std::string>::iterator it = dirList.begin(); it != dirList.end(); ++it)
    {
        bRetCode = GetRelativePath(*it, strPath.c_str(), it->c_str());
        XY_FAILED_JUMP(bRetCode);
    }
    
Exit1:
	bResult = true;
Exit0:
	return bResult;
}

BOOL XFileHelper::GetFileList(std::list<std::string>& fileList, const char szDir[], BOOL bRecursion, BOOL bRetRelativePath)
{
    BOOL bResult  = false;
	BOOL bRetCode = false;
	std::string strPath(szDir);

	bRetCode = GetFullPath(strPath, szDir);
	XY_FAILED_JUMP(bRetCode);

	bRetCode = ListDir(fileList, strPath.c_str(), false, bRecursion);
	XY_FAILED_JUMP(bRetCode);
    
    if (!bRetRelativePath)
        goto Exit1;
    
    for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
    {
        bRetCode = GetRelativePath(*it, strPath.c_str(), it->c_str());
        XY_FAILED_JUMP(bRetCode);
    }

Exit1:
	bResult = true;
Exit0:
	return bResult;
}



