#pragma once

#include <io.h>
#include <direct.h>
#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>
#include <Winbase.h>
#include <Unknwn.h>
#include <conio.h>
#include <process.h>
#include <intrin.h>
#include <sys/stat.h>
#include <mbstring.h>

#ifndef getcwd
#define getcwd      _getcwd
#endif
#define snprintf    _snprintf
#define snwprintf   _snwprintf
#define tzset       _tzset
#define open        _open
#define close       _close
#define chsize      _chsize
#define fileno      _fileno
#define stricmp     _stricmp
#ifndef strdup
#define strdup      _strdup
#endif
#ifndef wcsdup
#define wcsdup      _wcsdup
#endif
#define getch       _getch
#define strtoll     _strtoi64
#define strtoull    _strtoui64
#define strtok_r    strtok_s
#define wfopen      _wfopen
#define getpid      _getpid
#define fseek64     _fseeki64
#define ftell64     _ftelli64
#define stat		_stat

#define DIR_SPRIT    '\\'

#define STR_CASE_CMP(szA, szB) _mbsicmp((unsigned char*)(szA), (unsigned char*)(szB))
#define WSTR_CASE_CMP(szA, szB) _wcsicmp((szA), (szB))

typedef int socklen_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef float float32_t;
typedef double float64_t;

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

