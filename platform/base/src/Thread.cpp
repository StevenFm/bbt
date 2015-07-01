#include "Base.h"

XThread::XThread()
{
    m_ThreadHandle      = INVALID_THREAD_HANDLE;
    m_ThreadFunction    = NULL;
    m_pvThreadArg       = NULL;
}

XThread::~XThread()
{
    assert(m_ThreadHandle == INVALID_THREAD_HANDLE);
}

void XThread::ThreadFunction()
{
    (*m_ThreadFunction)(m_pvThreadArg);
}

#ifdef _MSC_VER
static unsigned __stdcall MyThreadProc(void* pvArg)
{
    XThread* pThread = (XThread*)pvArg;
	
    pThread->ThreadFunction();
	
    return 0;
}

BOOL XThread::Create(ThreadFunctionPointer ThreadFunction, void* pvArg)
{
    unsigned uThreadID = 0;

    m_ThreadFunction = ThreadFunction;
    m_pvThreadArg    = pvArg;
	
    m_ThreadHandle = (ThreadHandle)_beginthreadex(
        NULL,           // SD
	    0,	            // initial stack size
	    MyThreadProc,   // thread function
	    (void*)this,    // thread argument
	    0,	            // creation option
	    &uThreadID      // thread identifier
    );	

    return (m_ThreadHandle != INVALID_THREAD_HANDLE);
}

void XThread::WaitForExit()
{
    if (m_ThreadHandle != INVALID_THREAD_HANDLE)
    {
        WaitForSingleObject(m_ThreadHandle, INFINITE);    	
        CloseHandle(m_ThreadHandle);
        m_ThreadHandle = INVALID_THREAD_HANDLE;
    }       
}
#endif

#if defined(__linux) || defined(__APPLE__)
static void* MyThreadProc(void* pvArg)
{
	XThread* pThread = (XThread*)pvArg;
	
	pThread->ThreadFunction();
	
    return NULL;
}

BOOL XThread::Create(ThreadFunctionPointer ThreadFunction, void* pvArg)
{
    BOOL            bResult     = false;
    int             nRetCode    = 0;
    pthread_attr_t  ThreadAttr; 

	m_ThreadFunction = ThreadFunction;
	m_pvThreadArg    = pvArg;
	
    nRetCode = pthread_attr_init(&ThreadAttr);
    XY_FAILED_JUMP(nRetCode == 0);
    
    nRetCode = pthread_attr_setstacksize(&ThreadAttr, 256 * 1024);
    XY_FAILED_JUMP(nRetCode == 0);

    nRetCode = pthread_create(&m_ThreadHandle, &ThreadAttr, MyThreadProc, this);
    XY_FAILED_JUMP(nRetCode == 0);
    
    pthread_attr_destroy(&ThreadAttr);    

    bResult = true;
Exit0:
    return bResult;
}

void XThread::WaitForExit()
{
    if (m_ThreadHandle != INVALID_THREAD_HANDLE)
    {
	    pthread_join(m_ThreadHandle, NULL);
	    m_ThreadHandle  = INVALID_THREAD_HANDLE;
    }      
}
#endif
