#include "stdafx.h"
#include "luna/LuaPacker.h"
#include "ServerAgent.h"
#include "UIManager.h"
#include "Audio.h"
#include "FightView.h"

USING_NS_CC;
USING_NS_CC_EXT;

int LuaGetServerAgent(lua_State* L)
{
    Lua_PushObject(L, g_pServerAgent);
    return 1;
}

int LuaGetUIManager(lua_State* L)
{
	Lua_PushObject(L, g_UIManager);
	return 1;
}

int LuaGetAudio(lua_State* L)
{
	Lua_PushObject(L, g_Audio);
	return 1;
}

int LuaGetUserString(lua_State* L)
{
    const char* pszKey = lua_tostring(L, 1);
    std::string strValue = CCUserDefault::sharedUserDefault()->getStringForKey(pszKey);
    
    lua_pushstring(L, strValue.c_str());
    return 1;
}

int LuaGetUserNumber(lua_State* L)
{
    const char* pszKey = lua_tostring(L, 1);
    double fValue = CCUserDefault::sharedUserDefault()->getDoubleForKey(pszKey);
        
    lua_pushnumber(L, fValue);
    return 1;
}

int LuaGetUserBool(lua_State* L)
{
    const char* pszKey = lua_tostring(L, 1);
    BOOL bValue = CCUserDefault::sharedUserDefault()->getBoolForKey(pszKey, false);
        
    lua_pushboolean(L, bValue);
    return 1;
}

int LuaSetUserValue(lua_State* L)
{
    const char* pszKey = lua_tostring(L, 1);
    int nValueType = lua_type(L, 2);
    
    if (nValueType == LUA_TSTRING)
    {
        std::string strValue = lua_tostring(L, 2);
        CCUserDefault::sharedUserDefault()->setStringForKey(pszKey, strValue);
    }
    else if (nValueType == LUA_TNUMBER)
    {
        CCUserDefault::sharedUserDefault()->setDoubleForKey(pszKey, lua_tonumber(L, 2));
    }
    else if (nValueType == LUA_TBOOLEAN)
    {
        CCUserDefault::sharedUserDefault()->setBoolForKey(pszKey, lua_toboolean(L, 2));
    }
    else
    {
        Log(eLogError, "SetUserValue only suport value-type: string & numbers.");
    }
    
    CCUserDefault::sharedUserDefault()->flush();
    return 0;
}

int LuaMessageBox(lua_State* L)
{
    const char* pszMsg = lua_tostring(L, 1);
    
    if (pszMsg)
    {
//        cocos2d::CCMessageBox(pszMsg, "提醒");
    }
    
    return 0;
}

int LuaGetPlatform(lua_State* L)
{
    lua_pushnumber(L, CC_TARGET_PLATFORM);
    
    return 1;
}

int LuaReadFileData(lua_State* L)
{
    int nResultCount = 0;
    const char* pszFileName = lua_tostring(L, 1);
    size_t uFileSize = 0;
    BYTE* pbyFileData = g_pFileHelper->ReadFileData(&uFileSize, pszFileName);
    
    if (pbyFileData)
    {
        lua_pushlstring(L, (const char*)pbyFileData, uFileSize);
        lua_pushinteger(L, uFileSize);
        nResultCount = 2;
    }
    else
    {
        Log(eLogDebug, "[Lua] ReadFileData('%s') failed!", pszFileName);
        lua_pushnil(L);
        nResultCount = 1;
    }
    
    XY_DELETE_ARRAY(pbyFileData);
    
    return nResultCount;
}

int LuaWriteFileData(lua_State* L)
{
    BOOL bResult = false;
    const char* pszFileName = lua_tostring(L, 1);
    size_t uStrSize = 0;
    const char* pszStrData = lua_tolstring(L, 2, &uStrSize);
    
    if (pszStrData && uStrSize > 0)
    {
        bResult = g_pFileHelper->WriteFileData(pszFileName, pszStrData, uStrSize);
        if (!bResult)
        {
            Log(eLogDebug, "[Lua] WriteFileData('%s', '%s') failed!", pszFileName, pszStrData);
        }
    }
    
    lua_pushboolean(L, bResult);
    return 1;
}

int LuaPopScene(lua_State* L)
{
    CCDirector::sharedDirector()->popScene();
    return 0;
}

int LuaPopToRootScene(lua_State* L)
{
    CCDirector::sharedDirector()->popToRootScene();
    return 0;
}

int LuaSetTimeScale(lua_State* L)
{
    float fTimeScale = (float)lua_tonumber(L, 1);
    CCScheduler* pScheduler = CCDirector::sharedDirector()->getScheduler();
    pScheduler->setTimeScale(fTimeScale);
    
    return 0;
}

static int LuaGetGameFrame(lua_State* L)
{
	int nLogicFrame = g_pServerAgent->GetLogicFrame();
	lua_pushnumber(L, nLogicFrame);
	return 1;
}

static int LuaFightCreateView(lua_State* L)
{
	int			nResult		   = 0;
	XFightView* pFightView     = XFightView::create();

	XYLOG_FAILED_JUMP(pFightView);

	Lua_PushObject(L, pFightView);
	nResult = 1;
Exit0:
	return nResult;
}

static int LuaFullPathForFileName(lua_State* L)
{
	CCFileUtils*	pFileUtils	   = CCFileUtils::sharedFileUtils();
	const char*		pszFileName    = lua_tostring(L, 1);
	std::string		strFullName	   = pFileUtils->fullPathForFilename(pszFileName);
	lua_pushstring(L, strFullName.c_str());
	return 1;
}

luaL_Reg g_ScriptFuncs[] =
{
    {"GetServerAgent", LuaGetServerAgent},
    
	{"GetUIManager", LuaGetUIManager},
	{"GetAudio", LuaGetAudio},

    {"GetUserString", LuaGetUserString},
    {"GetUserNumber", LuaGetUserNumber},
    {"GetUserBool", LuaGetUserBool},
    {"SetUserValue", LuaSetUserValue},
    
    {"MessageBox", LuaMessageBox},
    {"GetPlatform", LuaGetPlatform},
    
    {"ReadFileData", LuaReadFileData},
    {"WriteFileData", LuaWriteFileData},

    {"PopScene", LuaPopScene},
    {"PopToRootScene", LuaPopToRootScene},
    
    {"SetTimeScale", LuaSetTimeScale},
	{"GetGameFrame", LuaGetGameFrame},
	{"FightCreateView", LuaFightCreateView},
	{"FullPathForFileName", LuaFullPathForFileName},
    {NULL, NULL}
};

void SetupScriptFuncs(lua_State* L)
{
    for (luaL_Reg* pReg = g_ScriptFuncs; pReg->func; pReg++)
    {
        lua_pushcfunction(L, pReg->func);
        lua_setglobal(L, pReg->name);
    }
}
