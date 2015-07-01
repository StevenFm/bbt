#pragma once

// LCG24
// 随机种子只需要一个uint32_t来存储
// 速度最快,随机分布一般
#define XY_RAND_MAX 0x00FFFFFF

uint32_t XY_Rand();
void XY_SetRandSeed(uint32_t uSeed);
uint32_t XY_GetRandSeed();

// WELL512
// 随机种子需要16个uint32_t来存储
// 速度较快,分布较好
#define WELL_RAND_STATE_COUNT 16

uint32_t WellRand();
void WellSetRandSeed(uint32_t uStateArray[WELL_RAND_STATE_COUNT]);
void WellGetRandSeed(uint32_t uStateArray[WELL_RAND_STATE_COUNT]);

// CSP Random-Number-Generator
// 具有最好的随机分布
// 没有随机种子,速度稍慢
BOOL CSPRandData(void* pvBuffer, size_t uBufferLen);

