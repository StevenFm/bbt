#pragma once

#ifdef _MSC_VER
    #define __THIS_FUNCTION__   __FUNCTION__
#endif

#if defined(__linux) || defined(__APPLE__)
    #define __THIS_FUNCTION__   __PRETTY_FUNCTION__
#endif

#define XY_FAILED_JUMP(Condition) \
    do  \
    {   \
        if (!(Condition))   \
            goto Exit0;     \
    } while (false)

#define XY_FAILED_RET_CODE(Condition, nCode) \
    do  \
    {   \
        if (!(Condition))   \
        {   \
            nResult = nCode;    \
            goto Exit0;         \
        }   \
    } while (false)

#define XY_SUCCESS_JUMP(Condition) \
    do  \
    {   \
        if (Condition)      \
            goto Exit1;     \
    } while (false)
	
#define XYCOM_FAILED_JUMP(Condition) \
    do  \
    {   \
        if (FAILED(Condition))   \
            goto Exit0;     \
    } while (false)
	
#define XYLOG_FAILED_JUMP(Condition) \
    do  \
    {   \
        if (!(Condition))   \
        {                   \
            Log(            \
                eLogDebug,  \
                    "XYLOG_FAILED_JUMP(%s) at %s:%d in %s", #Condition, __FILE__, __LINE__, __THIS_FUNCTION__   \
            );              \
            goto Exit0;     \
        }                   \
    } while (false)

#define XY_DELETE_ARRAY(pArray)     \
    do  \
    {   \
        if (pArray)                 \
        {                           \
            delete[] (pArray);      \
            (pArray) = NULL;        \
        }                           \
    } while (false)


#define XY_DELETE(p)    \
    do  \
    {   \
        if (p)              \
        {                   \
            delete (p);     \
            (p) = NULL;     \
        }                   \
    } while (false)

#define XY_FREE(pvBuffer) \
    do  \
    {   \
        if (pvBuffer)               \
        {                           \
            free(pvBuffer);         \
            (pvBuffer) = NULL;      \
        }                           \
    } while (false)


#define XY_COM_RELEASE(pInterface) \
    do  \
    {   \
        if (pInterface)                 \
        {                               \
            (pInterface)->Release();    \
            (pInterface) = NULL;        \
        }                               \
    } while (false)

#define XY_CLOSE_FILE(p)    \
    do  \
    {   \
        if  (p) \
        {       \
            fclose(p);  \
            (p) = NULL; \
        }   \
    } while (false)

#define XY_CLOSE_FD(p)    \
    do  \
    {   \
        if  ((p) >= 0) \
        {       \
            close(p);  \
            (p) = -1; \
        }   \
    } while (false)


#define XY_FREE_STRING(pvString) \
    do \
    { \
    if (pvString) \
        { \
        PlatformFreeString(pvString); \
        pvString = NULL; \
        } \
    } while (false)


// 用来取代MS的MAKELONG,MAKEWORD
#define MAKE_DWORD(a, b)      ((DWORD)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#define MAKE_WORD(a, b)      ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))

#if defined(__linux) || defined(__APPLE__)
    #define LOWORD(l)           ((WORD)(((DWORD)(l)) & 0xffff))
    #define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
    #define LOBYTE(w)           ((BYTE)(((DWORD)(w)) & 0xff))
    #define HIBYTE(w)           ((BYTE)((((DWORD)(w)) >> 8) & 0xff))
#endif
