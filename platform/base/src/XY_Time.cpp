#include "Base.h"
#include "Mutex.h"

static XMutex  g_StartTickCountLock;

#if defined(__linux) || defined(__APPLE__)
DWORD XY_GetTickCount()
{
    DWORD           dwResult      = 0;
    int             nRetCode      = 0;
    static uint64_t s_uBaseTime   = 0;
    uint64_t        uTime         = 0;
    timeval         timeVal;

    nRetCode = gettimeofday(&timeVal, NULL);
    if (nRetCode == 0)
    {
        uTime = ((uint64_t)timeVal.tv_sec) * 1000 + timeVal.tv_usec / 1000;
        if (s_uBaseTime == 0)
        {
            g_StartTickCountLock.Lock();
            if (s_uBaseTime == 0)
            {
                s_uBaseTime = uTime;
            }
            g_StartTickCountLock.Unlock();
        }
        dwResult = (DWORD)(uTime - s_uBaseTime) + 1;
    }

    return dwResult;
}
#endif

#ifdef _MSC_VER

struct XTimePeriodCtrl 
{
    XTimePeriodCtrl(unsigned int uPeriod)
    {
        timeBeginPeriod(uPeriod);
        m_uPeriod = uPeriod;
    }

    ~XTimePeriodCtrl()
    {
        timeEndPeriod(m_uPeriod);
    }

    unsigned int m_uPeriod;
};

#pragma comment(lib, "Winmm.lib")

static XTimePeriodCtrl s_TimePeriodCtrl(1);

DWORD XY_GetTickCount()
{
    DWORD                dwResult              = 0;
    static uint32_t      s_uStartTickCount     = 0;
    static int           s_nStartFlag          = false;
    uint32_t             uTime                 = 0;

    uTime = timeGetTime();

    if (!s_nStartFlag)
    {
        g_StartTickCountLock.Lock();
        if (!s_nStartFlag)
        {
            s_uStartTickCount = uTime;
            s_nStartFlag = true;
        }
        g_StartTickCountLock.Unlock();
    }
    
    dwResult = (DWORD)(uTime - s_uStartTickCount) + 1;      // at least 1, maybe some people use 0 for error  

    return dwResult;    
}
#endif


