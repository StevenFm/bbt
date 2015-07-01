//
//  Audio.cpp
//  FangKuai
//
//  Created by X on 13-8-7.
//
//

#include "Audio.h"

#include "SimpleAudioEngine.h"

using namespace std;
using namespace CocosDenshion;
USING_NS_CC;

Audio* g_Audio;

IMPL_LUA_CLASS_BEGIN(Audio)
	EXPORT_LUA_FUNCTION(LuaPreLoadEffect)
	EXPORT_LUA_FUNCTION(LuaUnLoadEffect)
	EXPORT_LUA_FUNCTION(LuaPreLoadBGMusic)

	EXPORT_LUA_FUNCTION(LuaPlayEffect)
	EXPORT_LUA_FUNCTION(LuaPauseEffect)
	EXPORT_LUA_FUNCTION(LuaResumeEffect)
	EXPORT_LUA_FUNCTION(LuaStopEffect)
	EXPORT_LUA_FUNCTION(LuaPauseAllEffects)
	EXPORT_LUA_FUNCTION(LuaResumeAllEffects)
	EXPORT_LUA_FUNCTION(LuaStopAllEffects)

	EXPORT_LUA_FUNCTION(LuaPlayBGMusic)
	EXPORT_LUA_FUNCTION(LuaPauseBGMusic)
	EXPORT_LUA_FUNCTION(LuaResumeBGMusic)
	EXPORT_LUA_FUNCTION(LuaStopBGMusic)

	EXPORT_LUA_FUNCTION(LuaEnd)
IMPL_LUA_CLASS_END()

Audio::Audio()
{
    
}

Audio::~Audio()
{

}

/*
使用音效前需要先载入，此处就是预载入。背景音乐的使用方法也是这样
*/
int Audio::LuaPreLoadEffect(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	const char* pszFileName	= NULL;
	
	XYLOG_FAILED_JUMP(nTop == 1);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName);

	SimpleAudioEngine::sharedEngine()->preloadEffect(pszFileName);
Exit0:
	return 0;
}

int Audio::LuaUnLoadEffect(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	const char* pszFileName	= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName);

	SimpleAudioEngine::sharedEngine()->unloadEffect(pszFileName);
Exit0:
	return 0;
}

int Audio::LuaPreLoadBGMusic(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	const char* pszFileName	= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName);

	SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic(pszFileName);
Exit0:
	return 0;
}

int Audio::LuaPlayEffect(lua_State* L)
{
	int				nTop		= lua_gettop(L);
	const char*		pszFileName	= NULL;
	unsigned int	nSoundID;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName);

	nSoundID = SimpleAudioEngine::sharedEngine()->playEffect(pszFileName);
	lua_pushinteger(L, nSoundID);
	return 1;
Exit0:
	return 0;
}

int Audio::LuaPauseEffect(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	int			nSoundID;

	XYLOG_FAILED_JUMP(nTop == 1);

	nSoundID = lua_tonumber(L, 1);
	XYLOG_FAILED_JUMP(nSoundID);

	SimpleAudioEngine::sharedEngine()->pauseEffect(nSoundID);
Exit0:
	return 0;
}

int Audio::LuaResumeEffect(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	int			nSoundID;

	XYLOG_FAILED_JUMP(nTop == 1);

	nSoundID = lua_tonumber(L, 1);
	XYLOG_FAILED_JUMP(nSoundID);

	SimpleAudioEngine::sharedEngine()->resumeEffect(nSoundID);
Exit0:
	return 0;
}
int Audio::LuaStopEffect(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	int			nSoundID;

	XYLOG_FAILED_JUMP(nTop == 1);

	nSoundID = lua_tonumber(L, 1);
	XYLOG_FAILED_JUMP(nSoundID);

	SimpleAudioEngine::sharedEngine()->stopEffect(nSoundID);
Exit0:
	return 0;
}

int Audio::LuaPauseAllEffects(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->pauseAllEffects();
	return 0;
}

int Audio::LuaResumeAllEffects(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->resumeAllEffects();
	return 0;
}
int Audio::LuaStopAllEffects(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->stopAllEffects();
	return 0;
}

int Audio::LuaPlayBGMusic(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	const char* pszFileName	= NULL;
	bool		bLoop;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName);

	bLoop = lua_toboolean(L, 2);
	SimpleAudioEngine::sharedEngine()->playBackgroundMusic(pszFileName, bLoop);
Exit0:
	return 0;
}

int Audio::LuaPauseBGMusic(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
	return 0;
}
int Audio::LuaResumeBGMusic(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
	return 0;
}
int Audio::LuaStopBGMusic(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	bool		bReleaseData;

	XYLOG_FAILED_JUMP(nTop == 1);

	bReleaseData = lua_toboolean(L, 1);
	SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(bReleaseData);
Exit0:
	return 0;
}

int Audio::LuaEnd(lua_State* L)
{
	SimpleAudioEngine::sharedEngine()->end();
	return 0;
}