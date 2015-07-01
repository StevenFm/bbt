#pragma once

#ifdef _MSC_VER
class XMutex
{
public:
    XMutex()
    { 
        BOOL bRetCode = InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 4000);
        if (!bRetCode)
        {
            assert(false && "InitializeCriticalSectionAndSpinCount failed !");
        }
    }

    ~XMutex()       
    { 
        DeleteCriticalSection(&m_CriticalSection);                
    }

    void Lock()      
    { 
        EnterCriticalSection(&m_CriticalSection);
    }

    void Unlock()    
    { 
        LeaveCriticalSection(&m_CriticalSection);
    }

private:
    CRITICAL_SECTION m_CriticalSection;
};
#endif

#if defined(__linux) || defined(__APPLE__)
class XMutex
{
public:
    XMutex()        
    { 
        pthread_mutex_init(&m_Mutex, NULL);                                             
    }

    ~XMutex()       
    { 
        pthread_mutex_destroy(&m_Mutex);                                          
    }

    void Lock()      
    { 
        pthread_mutex_lock(&m_Mutex);
    }

    void Unlock()    
    { 
        pthread_mutex_unlock(&m_Mutex);
    }

private:
    pthread_mutex_t m_Mutex;
};
#endif
