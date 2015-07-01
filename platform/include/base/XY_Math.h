#pragma once

#include <math.h>
#include "Base.h"

//骰子随机数定义
#define DICE_MAX				100
#define DICE_MIN				1
#define XY_PI                   3.1415926f
#define DIRECTION_BIT_NUM       8
#define DIRECTION_COUNT			(1 << DIRECTION_BIT_NUM)

template<typename T>
inline T g_GetDistance3(T nDistance2, T nSourceZ, T nDestZ)
{
	T nDistance3 = nDistance2 + (nSourceZ - nDestZ) * (nSourceZ - nDestZ);

	return nDistance3;
}

template<typename T>
inline T g_GetDistance3(T nSourceX, T nSourceY, T nSourceZ, T nDestX, T nDestY, T nDestZ)
{
	T nDistance3 = 
        (nSourceX - nDestX) * (nSourceX - nDestX) + 
		(nSourceY - nDestY) * (nSourceY - nDestY) + 
        (nSourceZ - nDestZ) * (nSourceZ - nDestZ);

	return nDistance3;
}

template<typename T>
inline T g_GetDistance2(T nSourceX, T nSourceY, T nDestX, T nDestY)
{
	T nDistance2 = 
        (nSourceX - nDestX) * (nSourceX - nDestX) + 
        (nSourceY - nDestY) * (nSourceY - nDestY);

	return nDistance2;
}

int g_GetDirection(int nX, int nY);

template<typename T>
inline T g_GetDirection(T nSourceX, T nSourceY, T nDestX, T nDestY)
{
    return (T)g_GetDirection((int)(nDestX - nSourceX), (int)(nDestY - nSourceY));
}

inline DWORD g_Dice(void)
{
	return XY_Rand() % (DICE_MAX - DICE_MIN + 1) + DICE_MIN;
}

#define SIN_COS_NUMBER	4096

int g_Sin(int nDirection);

inline int g_Cos(int nDirection)
{
    return g_Sin(DIRECTION_COUNT / 4 - nDirection);
}

inline BOOL g_RandPercent(int nPercent)
{
	return ((int)XY_Rand() % 100 < nPercent);
}

inline BOOL g_InRange(int nXa, int nYa, int nZa, int nXb, int nYb, int nZb, int nRange)
{
    BOOL        bResult     = false;
    long long   llDistance  = 0;
    long long   llRange     = (long long)nRange * nRange;

    llDistance = g_GetDistance3(
        (long long)nXa, (long long)nYa, (long long)nZa, 
        (long long)nXb, (long long)nYb, (long long)nZb
    );

    XY_FAILED_JUMP(llDistance <= llRange);

    bResult = true;
Exit0:
    return bResult;
}

enum IN_RANGE_RESULT
{
	irrInvalid = 0,

    irrInRange,
    irrTooClose,
    irrTooFar,

	irrTotal
};

inline IN_RANGE_RESULT g_InRange(int nXa, int nYa, int nZa, int nXb, int nYb, int nZb, int nMinRange, int nMaxRange)
{
    IN_RANGE_RESULT nResult     = irrInvalid;
    long long       llDistance  = 0;
    long long       llMinRange  = (long long)nMinRange * nMinRange;
    long long       llMaxRange  = (long long)nMaxRange * nMaxRange;

    assert(nMinRange >= 0);
    assert(nMinRange <= nMaxRange);

    llDistance = g_GetDistance3(
        (long long)nXa, (long long)nYa, (long long)nZa, 
        (long long)nXb, (long long)nYb, (long long)nZb
    );

    XY_FAILED_RET_CODE(llDistance >= llMinRange, irrTooClose);
    XY_FAILED_RET_CODE(llDistance <= llMaxRange, irrTooFar);

    nResult = irrInRange;

Exit0:
    return nResult;
}

// 从a到b的射线上,取距离a为nRange的点
// 如果算出的点超出了b点,则取b,返回false
// 否则取算出的中间点坐标,返回true
inline BOOL GetRayPointInRange(POINT* pOut, POINT& a, POINT& b, int nRange)
{
    int nDeltaX = b.x - a.x;
    int nDeltaY = b.y - a.y;
    int nDistanceQ = nDeltaX * nDeltaX + nDeltaY * nDeltaY;

    assert(nRange >= 0);

    if (nDistanceQ <= nRange * nRange)
    {
        *pOut = b;
        return false;
    }

    float fDistance = sqrtf((float)nDistanceQ);

    pOut->x = (int)((a.x * (fDistance - nRange) + b.x * nRange) / fDistance);
    pOut->y = (int)((a.y * (fDistance - nRange) + b.y * nRange) / fDistance);

    return true;
}

inline BOOL GetRayPoint(POINT* pOut, POINT& a, POINT& b, int nRange)
{
    int nDeltaX = b.x - a.x;
    int nDeltaY = b.y - a.y;
    int nDistanceQ = nDeltaX * nDeltaX + nDeltaY * nDeltaY;
    float fDistance = sqrtf((float)nDistanceQ);

    assert(nRange >= 0);
    if (fDistance < 0.0001)
        return false;

    pOut->x = (int)(a.x + nDeltaX * nRange / fDistance);
    pOut->y = (int)(a.y + nDeltaY * nRange / fDistance);

    return true;
}

