#include "Base.h"

static uint32_t gs_uState[WELL_RAND_STATE_COUNT] = {
    0x3da2b566, 0xce3c485f, 0xfce75007, 0xb4c67390,
    0x9cda237e, 0x9a5fe1d0, 0xbaff11a8, 0x21375ddf,
    0x58c6064b, 0xd23db660, 0xa56ccdc4, 0x367952f1,
    0x45674300, 0x553838de, 0x1bec6c85, 0x1e7c7fa1
};
static uint32_t gs_uIndex = 0;


// Thanks to Game Programming Gems 7, by Scott Jacobs
uint32_t WellRand()
{
    uint32_t a, b, c, d;

    a = gs_uState[gs_uIndex];
    c = gs_uState[(gs_uIndex + 13) & 15];
    b = a ^ c ^ (a << 16) ^ (c << 15);
    c = gs_uState[(gs_uIndex + 9) & 15];
    c ^= (c >> 11);
    a = gs_uState[gs_uIndex] = b ^ c;
    d = a ^ ((a << 5) & 0xDA442D20UL);
    gs_uIndex = (gs_uIndex + 15) & 15;
    a = gs_uState[gs_uIndex];
    gs_uState[gs_uIndex] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);

    return gs_uState[gs_uIndex];
}

void WellSetRandSeed(uint32_t uStateArray[])
{
    memcpy(gs_uState, uStateArray, sizeof(gs_uState));
}

void WellGetRandSeed(uint32_t uStateArray[WELL_RAND_STATE_COUNT])
{
    memcpy(uStateArray, gs_uState, sizeof(gs_uState));
}
