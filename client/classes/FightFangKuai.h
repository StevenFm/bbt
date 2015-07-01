
#pragma once

#include "stdafx.h"
#include "BasicPanel.h"

class XFightView;

class XFightFangKuai : 
public cocos2d::Node,
public cocosbuilder::CCBAnimationManagerDelegate
{
public:
	XFightFangKuai();
	virtual	~XFightFangKuai();

	virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags);
	virtual void completedAnimationSequenceNamed(const char* pszName);

	void ChangeFangKuai(const char* pszCCBFile, const char* pszImageName = "fkbg", const char* pszImageFile = NULL);
	void OnFallFullCell();
	void OnCompletedAction(const char* pszName);

	CREATE_FUNC(XFightFangKuai);
	DECLARE_LUA_CLASS(XFightFangKuai);

	int  LuaChangeFangKuai(lua_State* L);
	int  LuaMoveCell(lua_State* L);
	int  LuaMoveFullCell(lua_State* L);
	int  LuaSetPostion(lua_State* L);
	int  LuaGetPostion(lua_State* L);
	int  LuaGetCellSize(lua_State* L);
	int  LuaRemove(lua_State* L);
	int  LuaSetVisible(lua_State* L);
	int  LuaGetVisible(lua_State* L);
	int  LuaGetUIFangKuai(lua_State* L);
	int  LuaUpdate(lua_State* L);

private:
	void DrawInRange();
	void RemoveDelegate();

public:
	float               m_fCurFallHeight;
	float               m_fFallSpeed;			//多少帧下满一个格子
	cocos2d::Size		m_CellSize;				//一个格子的大小
	BasicPanel*			m_pUIFangKuai;
	XFightView*         m_pFightView;
};