#include "Base.h"
#include <stdarg.h>

#define LOG_DIR            "logs"
#define LOG_LINE_MAX        (1024 * 2)

struct LogParam
{
    FILE*           pFile;
    int             nLineCount;
    int             nMaxFileLine;
    void*           pvUsrData;
    XLogCallback*   pCallback;
    XMutex          Mutex;
    char            szDir[MAX_PATH];
};

static LogParam*    g_pLog       = NULL;
static const char*  g_pszLevel[] = 
{
    "Err",
    "War",
    "Inf",
    "Dbg"
};

BOOL LogOpen(const char cszName[], int nMaxLine)
{
    BOOL    bResult     = false;
    int     nRetCode    = false;
    char*   pszRet      = NULL;
    char    szCWD[MAX_PATH];

    assert(g_pLog == NULL);

    g_pLog = new LogParam;

    pszRet = getcwd(szCWD, sizeof(szCWD));    
    XY_FAILED_JUMP(pszRet);    

    nRetCode = snprintf(g_pLog->szDir, sizeof(g_pLog->szDir), "%s/%s/%s", szCWD, LOG_DIR, cszName);
    XY_FAILED_JUMP(nRetCode > 0 && nRetCode < (int)sizeof(g_pLog->szDir));

    g_pLog->pFile           = NULL;
    g_pLog->nLineCount      = 0;
    g_pLog->nMaxFileLine    = nMaxLine;
    g_pLog->pCallback       = NULL;
    g_pLog->pvUsrData       = NULL;

    bResult = true;
Exit0:
    if (!bResult)
    {
        XY_DELETE(g_pLog);
    }
    return bResult;
}

void LogClose()
{
    if (g_pLog == NULL)
        return;

    if (g_pLog->pFile)
    {
        fclose(g_pLog->pFile);
        g_pLog->pFile = NULL;
    }

    XY_DELETE(g_pLog);
}

static FILE* CreateLogFile(const char cszPath[])
{
    FILE*   pFile       = NULL;
    int     nRetCode    = 0;
    char*   pszBuffer   = NULL;
    char*   pszPos      = NULL;
    int     nPreChar    = 0;

    pszBuffer = strdup(cszPath);
    XY_FAILED_JUMP(pszBuffer);

    pszPos = pszBuffer;

    while (*pszPos)
    {
        if (*pszPos == '/' || *pszPos == '\\')
        {
            *pszPos = '\0';

            if (nPreChar != 0 && nPreChar != ':')
            {
                nRetCode = MakeDir(pszBuffer);
                XY_FAILED_JUMP(nRetCode == 0 || (errno == EEXIST));
            }

            *pszPos = DIR_SPRIT;
        }

        nPreChar = *pszPos++;
    }

    pFile = fopen(pszBuffer, "w");
Exit0:
    XY_FREE(pszBuffer);
    return pFile;
}


static BOOL ResetLogFile()
{
    BOOL    bResult     = false;
    int     nRetCode    = false;
    time_t  uTimeNow    = 0;
    tm*     pTimeNow    = NULL;
    FILE*   pFile       = NULL;
    char    szPath[MAX_PATH];

    uTimeNow = time(NULL);
    pTimeNow = localtime(&uTimeNow);

    assert(pTimeNow);
    assert(g_pLog);

    nRetCode = snprintf(
        szPath, sizeof(szPath), 
        "%s/%2.2d_%2.2d_%2.2d_%2.2d_%2.2d.log",
        g_pLog->szDir,
        pTimeNow->tm_mon + 1,
        pTimeNow->tm_mday,
        pTimeNow->tm_hour,
        pTimeNow->tm_min,
        pTimeNow->tm_sec
    );
    XY_FAILED_JUMP(nRetCode > 0 && nRetCode < (int)sizeof(szPath));

    pFile = CreateLogFile(szPath);
    XY_FAILED_JUMP(pFile);

    if (g_pLog->pFile)
    {
        fclose(g_pLog->pFile);
        g_pLog->pFile = NULL;
    }

    g_pLog->pFile = pFile;
    pFile = NULL;

    g_pLog->nLineCount = 0;

    bResult = true;
Exit0:
    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }
    return bResult;
}

void Log(LogType eType, const char cszFormat[], ...)
{
    int             nRetCode    = 0;
    time_t          uTimeNow    = 0;
    tm*             pTimeNow    = NULL;
    DWORD           dwThread    = GetCurrentThreadID();
    int             nLogLen     = 0;
    XLogCallback*   pCallback   = NULL;
    BOOL            bLockFlag   = false;
    va_list         marker;
    static char     s_szLog[LOG_LINE_MAX];

    XY_FAILED_JUMP(g_pLog);

    assert(eType >= eLogError);
    assert(eType <= eLogDebug);

    uTimeNow = time(NULL);
    pTimeNow = localtime(&uTimeNow);

    assert(pTimeNow);

    g_pLog->Mutex.Lock();
    bLockFlag = true;

    nLogLen = snprintf(
        s_szLog, sizeof(s_szLog),
        "%2.2d-%2.2d,%2.2d:%2.2d:%2.2d<%s,%u>: ",
        pTimeNow->tm_mon + 1,
        pTimeNow->tm_mday,
        pTimeNow->tm_hour,
        pTimeNow->tm_min,
        pTimeNow->tm_sec,
        g_pszLevel[eType],
        dwThread
    );

    assert(nLogLen > 0);
    assert(nLogLen < sizeof(s_szLog));

    va_start(marker, cszFormat);
    nRetCode = vsnprintf(s_szLog + nLogLen, sizeof(s_szLog) - nLogLen, cszFormat, marker);
    va_end(marker);

    if (nRetCode > 0 && nRetCode < (int)sizeof(s_szLog) - nLogLen)
    {
        nLogLen += nRetCode;
    }
    
    if (nLogLen + 1 < sizeof(s_szLog) && s_szLog[nLogLen - 1] != '\n')
    {
        s_szLog[nLogLen++] = '\n';
        s_szLog[nLogLen] = '\0';
    }

    pCallback = g_pLog->pCallback;
    if (pCallback != NULL)
    {
        nRetCode = (*pCallback)(g_pLog->pvUsrData, eType, s_szLog);
        XY_FAILED_JUMP(nRetCode);
    }
    else
    {
#ifdef _MSC_VER
        int nUtf16Len = MultiByteToWideChar(CP_UTF8, 0, s_szLog, -1, NULL, 0);
        wchar_t* pwszLog = new wchar_t[nUtf16Len + 1];

        MultiByteToWideChar(CP_UTF8, 0, s_szLog, -1, pwszLog, nUtf16Len);
        pwszLog[nUtf16Len] = L'\0';

        int nAnsiLen = WideCharToMultiByte(CP_ACP, 0, pwszLog, -1, NULL, 0, NULL, NULL);
        char* pszLog = new char[nAnsiLen + 1];

        WideCharToMultiByte(CP_ACP, 0, pwszLog, -1, pszLog, nAnsiLen, NULL, FALSE);
        pszLog[nAnsiLen] = '\0';

        printf("%s", pszLog);
        
        XY_DELETE_ARRAY(pszLog);
        XY_DELETE_ARRAY(pwszLog);
#else
        fwrite(s_szLog, nLogLen, 1, stdout);
        fflush(stdout);
#endif

    }

    if (g_pLog->pFile == NULL || g_pLog->nLineCount >= g_pLog->nMaxFileLine)
    {
        ResetLogFile();
    }

    if (g_pLog->pFile)
    {
        fwrite(s_szLog, nLogLen, 1, g_pLog->pFile);
        fflush(g_pLog->pFile);
        g_pLog->nLineCount++;
    }

Exit0:
    if (bLockFlag)
    {
        g_pLog->Mutex.Unlock();
        bLockFlag = false;
    }
}

void LogHook(void* pvUsrData, XLogCallback* pCallback)
{
    g_pLog->pvUsrData = pvUsrData;
    g_pLog->pCallback = pCallback;
}

