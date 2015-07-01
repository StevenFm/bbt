//
//  Audio.h
//  FangKuai
//
//  Created by X on 13-8-7.
//
//

#ifndef __FangKuai__MMAudio__
#define __FangKuai__MMAudio__

#include "stdafx.h"

class Audio;

extern Audio* g_Audio;

class Audio:
public cocos2d::CCObject
{
public:
    Audio();
    ~Audio();

	DECLARE_LUA_CLASS(Audio);

	//Lua
	int LuaPreLoadEffect(lua_State* L);
	int LuaUnLoadEffect(lua_State* L);
	int LuaPreLoadBGMusic(lua_State* L);

	int LuaPlayEffect(lua_State* L);
	int LuaPauseEffect(lua_State* L);
	int LuaResumeEffect(lua_State* L);
	int LuaStopEffect(lua_State* L);
	int LuaPauseAllEffects(lua_State* L);
	int LuaResumeAllEffects(lua_State* L);
	int LuaStopAllEffects(lua_State* L);
	
	int LuaPlayBGMusic(lua_State* L);
	int LuaPauseBGMusic(lua_State* L);
	int LuaResumeBGMusic(lua_State* L);
	int LuaStopBGMusic(lua_State* L);
    
	int LuaEnd(lua_State* L);
};

#endif /* defined(__FangKuai__MMAudio__) */
