#include "Base.h"

#if defined(__linux) || defined(__APPLE__)
FILE* wfopen(const wchar_t* pwFileName, const wchar_t* pwMode)
{
    FILE*   pFile       = NULL;
    size_t  uRetCode    = 0;
    char    szFileName[MAX_PATH];
    char    szMode[MAX_PATH];

    uRetCode = wcstombs(szFileName, pwFileName, sizeof(szFileName));
    XY_FAILED_JUMP(uRetCode != (size_t)-1);

    uRetCode = wcstombs(szMode, pwMode, sizeof(szMode));
    XY_FAILED_JUMP(uRetCode != (size_t)-1);

    pFile = fopen(szFileName, szMode);
Exit0:
    return pFile;
}
#endif

BOOL XY_GetFileSize(uint64_t* puFileSize, FILE* pFile)
{
    BOOL    bResult     = false;
    int     nRetCode    = 0;
    int64_t nOrgPos     = 0;
    int64_t nFileLen    = 0;

    nOrgPos = ftell(pFile);
    XY_FAILED_JUMP(nOrgPos != -1);

    nRetCode = fseek(pFile, 0, SEEK_END);
    XY_FAILED_JUMP(nRetCode == 0);

    nFileLen = ftell(pFile);
    XY_FAILED_JUMP(nFileLen != -1);

    nRetCode = fseek(pFile, (long)nOrgPos, SEEK_SET);
    XY_FAILED_JUMP(nRetCode == 0);

    XY_FAILED_JUMP(nFileLen >= 0);

    *puFileSize = (uint64_t)nFileLen;

    bResult = true;
Exit0:
    return bResult;
}

BOOL XY_GetFileSize(uint64_t* puFileSize, const char szFileName[])
{
	BOOL bResult  = false;
	int  nRetCode = 0;
	struct stat info;

	nRetCode = stat(szFileName, &info);
	XY_FAILED_JUMP(nRetCode == 0);

	*puFileSize = (uint64_t)info.st_size;

	bResult = true;
Exit0:
	return bResult;
}
