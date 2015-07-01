#pragma once

#ifdef _MSC_VER
typedef HANDLE ThreadHandle;

#define INVALID_THREAD_HANDLE   NULL

inline void ThreadSleep(int nMilliSecond)
{
    Sleep((DWORD)nMilliSecond);
}

inline DWORD GetCurrentThreadID()
{
    return GetCurrentThreadId();
}
#endif

#if defined(__linux) || defined(__APPLE__)
typedef pthread_t ThreadHandle;
#define INVALID_THREAD_HANDLE   0

inline void ThreadSleep(int nMilliSecond)
{
    usleep((unsigned)nMilliSecond * 1000);
}

inline DWORD GetCurrentThreadID()
{
	// mac 64 bit环境下,pthread_t被定义成一个指针,将64位指针强转到DWORD会报错
	// 所以先转为uint64_t,当然,这个ID也只能拿来显示了:)
    return (DWORD)(uint64_t)pthread_self();
}
#endif

typedef void (*ThreadFunctionPointer)(void* pvArg);

class XThread
{
public:
	XThread();
	~XThread();

    BOOL  Create(ThreadFunctionPointer ThreadFunction, void* pvArg);
    void  WaitForExit();

    void  ThreadFunction();

private:
    ThreadHandle          m_ThreadHandle;
	ThreadFunctionPointer m_ThreadFunction;
    void*                 m_pvThreadArg;
};
