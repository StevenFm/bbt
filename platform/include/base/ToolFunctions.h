#pragma once

#ifdef _MSC_VER
inline BOOL MakeDir(const char cszDir[])
{
    return (_mkdir(cszDir) != -1);
}
#endif

#if defined(__linux) || defined(__APPLE__)
inline BOOL MakeDir(const char cszDir[])
{
    return (mkdir(cszDir, 0777) != -1);
}

FILE* wfopen(const wchar_t* pwFileName, const wchar_t* pwMode);
#endif

BOOL XY_GetFileSize(uint64_t* puFileSize, FILE* pFile);
BOOL XY_GetFileSize(uint64_t* puFileSize, const char szFileName[]);

#ifdef __cplusplus
template<typename T>
inline const T& Max(const T& a, const T& b)
{
    return (a < b) ? b : a;
}
template<typename T>
inline const T& Min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}
#else
#define Max(a,b)    (((a) > (b)) ? (a) : (b))
#define Min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

inline BOOL SafeCopyString(char* pszBuffer, size_t uBufferSize, const char* pszString)
{
    size_t  uLen  = strlen(pszString);

    if (uLen + 1 <= uBufferSize)
    {
        memcpy(pszBuffer, pszString, uLen + 1);
        return true;
    }

	if (uBufferSize > 0)
	{
		pszBuffer[0] = '\0';
	}

    return false;
}

template<size_t uBufferSize>
inline BOOL SafeCopyString(char (&szBuffer)[uBufferSize], const char* pszString)
{
    return SafeCopyString(szBuffer, uBufferSize, pszString);
}

inline BOOL SafeCopyString(wchar_t* pszBuffer, size_t uBufferSize, const wchar_t* pszString)
{
    size_t  uLen = wcslen(pszString);

    if (sizeof(wchar_t) * (uLen + 1) <= uBufferSize)
    {
        memcpy(pszBuffer, pszString, sizeof(wchar_t) * (uLen + 1));
        return true;
    }

    return false;
}

template<size_t uBufferSize>
inline BOOL SafeCopyString(wchar_t (&szBuffer)[uBufferSize], const wchar_t* pszString)
{
    return SafeCopyString(szBuffer, uBufferSize, pszString);
}

inline void* MemDup(const void* pvData, size_t uDataLen)
{
    void* pvBuffer = malloc(uDataLen);
    if (pvBuffer)
    {
        memcpy(pvBuffer, pvData, uDataLen);
    }
    return pvBuffer;
}

#ifdef _MSC_VER
inline struct tm* localtime_r(const time_t* timep, struct tm* result)
{
    errno_t nErr = localtime_s(result, timep);

    return (nErr == 0) ? result : NULL;
}
#endif

// 加密数据，再次调用解密数据
inline void EncryptData(BYTE* pbyData, size_t uDataLen, uint64_t uBaseOffset)
{
    for (size_t i = 0; i < uDataLen; i++)
    {
        pbyData[i] ^= (BYTE)(uBaseOffset + i);
    }
}
