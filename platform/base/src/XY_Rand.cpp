#include "Base.h"

static uint32_t gs_uRandSeed = 0;

uint32_t XY_Rand()
{
    gs_uRandSeed = (gs_uRandSeed * 16598013 + 12820163) & XY_RAND_MAX;
    return gs_uRandSeed;
}

void XY_SetRandSeed(uint32_t uSeed)
{
    gs_uRandSeed = uSeed;
}

uint32_t XY_GetRandSeed()
{
    return gs_uRandSeed;
}

