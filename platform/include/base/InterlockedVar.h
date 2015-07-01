#pragma once

// 使用 XY_Interlocked*** 函数族时,会自动根据变量32/64位调用不同版本的指令

#ifdef _MSC_VER
template <int nArch> struct _interlocked_arch_;

#define INTERLOCKED_ARCH(p) ((_interlocked_arch_<sizeof(*(p))>*)NULL)

template <typename T>  
T _XY_InterlockedExchangeAdd(volatile T* pAddend, T Value, _interlocked_arch_<4>*)
{
    return (T)InterlockedExchangeAdd((volatile long*)pAddend, Value);
}

template <typename T>  
T _XY_InterlockedExchangeAdd(volatile T* pAddend, T Value, _interlocked_arch_<8>*)
{
    return (T)InterlockedExchangeAdd64((volatile long long*)pAddend, Value);
}

template <typename T>  
T _XY_InterlockedIncrement(volatile T* pValue, _interlocked_arch_<4>*)
{
    return (T)InterlockedIncrement((volatile long*)pValue);
}

template <typename T>  
T _XY_InterlockedIncrement(volatile T* pValue, _interlocked_arch_<8>*)
{
    return (T)InterlockedIncrement64((volatile long long*)pValue);
}

template <typename T>  
T _XY_InterlockedDecrement(volatile T* pValue, _interlocked_arch_<4>*)
{
    return (T)InterlockedDecrement((volatile long*)pValue);
}

template <typename T>  
T _XY_InterlockedDecrement(volatile T* pValue, _interlocked_arch_<8>*)
{
    return (T)InterlockedDecrement64((volatile long long*)pValue);
}

#define XY_InterlockedExchangeAdd(A, B) _XY_InterlockedIncrement(A, B, INTERLOCKED_ARCH(A))
#define XY_InterlockedIncrement(A) _XY_InterlockedIncrement(A, INTERLOCKED_ARCH(A))
#define XY_InterlockedDecrement(A) _XY_InterlockedDecrement(A, INTERLOCKED_ARCH(A))

#endif

#if defined(__linux) || defined(__APPLE__)
#define XY_InterlockedExchangeAdd(A, B) __sync_fetch_and_add(A, B)
#define XY_InterlockedIncrement(A) __sync_add_and_fetch(A, 1)
#define XY_InterlockedDecrement(A)  __sync_add_and_fetch(A, -1)
#endif
