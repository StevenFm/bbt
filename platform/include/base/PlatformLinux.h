#pragma once

#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <getopt.h>
#include <dirent.h>

#define DECLSPEC_ALIGN(x) __attribute__((aligned(x)))

#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT

typedef float float32_t;
typedef double float64_t;

typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef int BOOL;
typedef unsigned long u_long;
typedef unsigned long ULONG;
typedef void* HANDLE;
typedef void* HMODULE;
typedef long HRESULT;

#define fseek64     fseeko64
#define ftell64     ftello64

#define MAX_PATH    512

#define CONTAINING_RECORD(address, type, field) ((type*)( \
    (char*)(address) - \
    (ptrdiff_t)(&((type*)0)->field)))

typedef struct _GUID 
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} GUID, IID;

#define STDMETHODCALLTYPE
#define REFIID const IID&

struct IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual u_long  STDMETHODCALLTYPE AddRef(void) = 0;
    virtual u_long  STDMETHODCALLTYPE Release(void) = 0;
};


#if !defined(__cplusplus)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
extern "C++"
{
	template <typename _CountofType, size_t _SizeOfArray>
	char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
}
#endif

#define snwprintf   swprintf

#define DIR_SPRIT    '/'

#define STR_CASE_CMP(szA, szB) strcasecmp(szA, szB)
#define WSTR_CASE_CMP(szA, szB) wcscasecmp((szA), (szB))

typedef     int             SOCKET;
#define     INVALID_SOCKET  (-1)
#define     SOCKET_ERROR    (-1)

struct POINT 
{
    int x;
    int y;
};
