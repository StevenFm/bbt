#include "base/Base.h"
#include "Luna.h"
#include "LuaPacker.h"

static int LuaLog(lua_State* L)
{
    int         nTopIndex     = lua_gettop(L);
    int         nTextLen      = 0;
    char		szText[1024];

    lua_getglobal(L, "tostring");
    szText[0] = '\0';

    for (int i = 1; i <= nTopIndex; i++)
    {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);

        const char* pszString = lua_tostring(L, -1);  /* get result */
        if (pszString == NULL)
            pszString = "";

        int nStrLen = (int)strlen(pszString);
        int nLeftSize = _countof(szText) - nTextLen - 1;

        nStrLen = Min(nStrLen, nLeftSize);

        memcpy(szText + nTextLen, pszString, nStrLen + 1);
        nTextLen += nStrLen;

        if (i < nTopIndex && nTextLen < _countof(szText) - 1)
        {
            szText[nTextLen++] = '\t';
        } 
        lua_pop(L, 1);  /* pop result */
    }

    szText[_countof(szText) - 1] = '\0';
    Log(eLogDebug, "%s", szText);

    return 0;
}

static int LuaGetFileModifyTime(lua_State* L)
{
	time_t		nTime		  = 0;
    int         nTopIndex     = 0;
    const char* pszFileName   = NULL;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 1);

    pszFileName = lua_tostring(L, 1);
    XY_FAILED_JUMP(pszFileName);
	
	nTime = g_pFileHelper->GetFileModifyTime(pszFileName);
Exit0:
	if (nTime == 0)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushnumber(L, (double)nTime);
	}
	return 1;
}

static int LuaGetRandomData(lua_State* L)
{
	int 	nArgCount 	= lua_gettop(L);
	int 	nByteCount  = 16;
	BYTE*	pbyData 	= NULL;
	char*	pszText 	= NULL;
	char*	pszPos 		= 0;

	if (nArgCount == 1)
	{
		nByteCount = lua_tointeger(L, 1);
	}

	if (nByteCount < 1)
		nByteCount = 1;

	if (nByteCount > 1024 * 64)
	{
		nByteCount = 1024 * 64;
	}

	pbyData = new BYTE[nByteCount];
	pszText = new char[nByteCount * 2 + 1];

	CSPRandData(pbyData, (size_t)nByteCount);

	pszPos = pszText;
	for (int i = 0; i < nByteCount; i++)
	{
		sprintf(pszPos, "%02x", pbyData[i]);
		pszPos += 2;
	}
	*pszPos = '\0';

	lua_pushstring(L, pszText);

	XY_DELETE_ARRAY(pszText);
	XY_DELETE_ARRAY(pbyData);
	return 1;
}

static int LuaSetRandomSeed(lua_State* L)
{
	size_t uRandSeedLen = 0;
	const char* pszRandSeed = lua_tolstring(L, 1, &uRandSeedLen);
	WellSetRandSeed((uint32_t*)pszRandSeed);
	return 0;
}

static int LuaGetRandomSeed(lua_State* L)
{
	uint32_t uRandSeed[WELL_RAND_STATE_COUNT];
	WellGetRandSeed(uRandSeed);
	lua_pushlstring(L, (const char*)uRandSeed, sizeof(uRandSeed));
	return 1;
}

// 和math.random返回值相同 
// 0个参数：返回[0, 1)
// 1个参数:返回[1, n]
// 2个参数:返回[min, max]
static int LuaRandom(lua_State* L)
{
    int nTop = lua_gettop(L);
    unsigned uRand = WellRand();
    if (nTop == 0)
    {
        lua_pushnumber(L, (double)uRand / 0xFFFFFFFF);
    }
    else if (nTop == 1)
    {
        int nMax = lua_tointeger(L, 1);
        if (nMax)
            uRand = uRand % nMax + 1;
        else
            uRand = 0;
        lua_pushinteger(L, uRand);
    }
    else if (nTop == 2)
    {
        int nMin = lua_tointeger(L, 1);
        int nMax = lua_tointeger(L, 2);
        if (nMin > nMax)
        {
            int nTmp = nMin;
            nMin = nMax;
            nMax = nTmp;
        }
        int nSpan = nMax - nMin + 1;
        if (nSpan)
            uRand %= nSpan;
        else
            uRand = 0;
        uRand += nMin;
        lua_pushinteger(L, uRand);
    }

    return 1;
}

static int LuaGetTickCount(lua_State* L)
{
    int64_t nTime = XY_GetTickCount();
    lua_pushnumber(L, (double)nTime);
    return 1;
}

static int LuaLoadIniFile(lua_State* L)
{
    int         nResult         = 0;
    BOOL        bRetCode        = false;
	const char* pszFileName     = lua_tostring(L, 1);
    char*       pszSection      = NULL;
    char*       pszKey          = NULL;
    char*       pszValue        = NULL;
    const int   nSectionLen     = 256;
    const int   nKeyLen         = 256;
    const int   nValueLen       = 2048;
    XIniFile iniFile;
    
    XYLOG_FAILED_JUMP(pszFileName);
    
    bRetCode = iniFile.Load(pszFileName);
    XY_FAILED_JUMP(bRetCode);
    
    pszSection = new char[nSectionLen];
    pszKey = new char[nKeyLen];
    pszValue = new char[nValueLen];
    
    lua_newtable(L);
    
    pszSection[0] = '\0';
    while (iniFile.GetNextSection(pszSection, pszSection))
    {
        pszSection[nSectionLen - 1] = '\0';
        
        lua_newtable(L);
        
        pszKey[0] = '\0';
        while (iniFile.GetNextKey(pszSection, pszKey, pszKey))
        {
            pszKey[nKeyLen - 1] = '\0';
            
            bRetCode = iniFile.GetString(pszSection, pszKey, pszValue, nValueLen);
            XYLOG_FAILED_JUMP(bRetCode);
            
            pszValue[nValueLen - 1] = '\0';
            
            lua_pushstring(L, pszValue);
            lua_setfield(L, -2, pszKey);
        }
        lua_setfield(L, -2, pszSection);
    }
    
    nResult = 1;
Exit0:
    XY_DELETE_ARRAY(pszSection);
    XY_DELETE_ARRAY(pszKey);
    XY_DELETE_ARRAY(pszValue);
	return nResult;
}

static int LuaGetStringMD5(lua_State* L)
{
    size_t uStringlen = 0;
    const char* pszText = lua_tolstring(L, 1, &uStringlen);
    char szMd5[64];

    GetDataMD5(szMd5, pszText, uStringlen);
    szMd5[_countof(szMd5) - 1] = '\0';

    lua_pushstring(L, szMd5);
    return 1;
}

#ifndef MAX_BLOCK_SIZE
#define	MAX_BLOCK_SIZE	(1024 * 1024 * 32)
#endif

static int LuaPack(lua_State* L)
{
    int nResult = 0;
    BOOL bRetCode = 0;
    int nArgCount = lua_gettop(L);
    BYTE* pbyBuffer = (BYTE*)malloc(MAX_BLOCK_SIZE); 
    size_t uDataLen = 0;
    XLuaPaker paker(MAX_BLOCK_SIZE);

    XYLOG_FAILED_JUMP(pbyBuffer);

    bRetCode = paker.PushValue(L, 1, nArgCount);
    XYLOG_FAILED_JUMP(bRetCode);

    bRetCode = paker.Save(&uDataLen, pbyBuffer, MAX_BLOCK_SIZE);
    XYLOG_FAILED_JUMP(bRetCode);

    lua_pushlstring(L, (char*)pbyBuffer, uDataLen);
    nResult = 1;
Exit0:
    XY_FREE(pbyBuffer);
    return nResult;
}

static int LuaUnpack(lua_State* L)
{
    int nResult = 0;
    size_t uDataLen = 0;
    const char* pszData = lua_tolstring(L, 1, &uDataLen);
    XLuaUnpaker unpaker(MAX_BLOCK_SIZE);

    XYLOG_FAILED_JUMP(pszData);

    nResult = unpaker.Expand(L, (BYTE*)pszData, uDataLen);
Exit0:
    return nResult;
}


void RegisterHelperFunctions(lua_State* L)
{
    lua_register(L, "Log", LuaLog);
    lua_register(L, "GetFileModifyTime", LuaGetFileModifyTime);
    lua_register(L, "GetRandomData", LuaGetRandomData);
    lua_register(L, "SetRandomSeed", LuaSetRandomSeed);
    lua_register(L, "GetRandomSeed", LuaGetRandomSeed);
    lua_register(L, "Random", LuaRandom);
    lua_register(L, "GetTickCount", LuaGetTickCount);
    lua_register(L, "LoadIniFile", LuaLoadIniFile);
    lua_register(L, "GetStringMD5", LuaGetStringMD5);
    lua_register(L, "Pack", LuaPack);
    lua_register(L, "Unpack", LuaUnpack);
}
