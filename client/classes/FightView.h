
#pragma once

#include "stdafx.h"
#include "FightFangKuai.h"

class XFightView : public cocos2d::Layer
{
public:
	XFightView();
	virtual ~XFightView();

	virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, bool parentTransformUpdated);
	virtual void update(float dt);
	virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onTouchCancelled(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);

	void                OnActivateEnd();
	void                CallActivateEnd();
	void				Setup(const char* pszCCBFile, const char* pszStartName, bool bOpenTouches = false);                
	void				SetBoard(const char* pszCCBFile, const char* pszStartName);
	XFightFangKuai*		CreateFangKuai();

	CREATE_FUNC(XFightView);
	DECLARE_LUA_CLASS(XFightView);

	int  LuaAddToNode(lua_State* L);
	int  LuaSetup(lua_State* L);
	int  LuaCreateFangKuai(lua_State* L);
	int  LuaSetBoard(lua_State* L);
	int  LuaGetFangKuai(lua_State* L);
	int  LuaSetBombing(lua_State* L);
	int  LuaSetFKParentScale(lua_State* L);

private:
	void SetTouchCallback();

public:
	typedef std::list<XFightFangKuai*>         XMapRemoveFangKuai;

	int								m_nCurFangKuaiTag;
	BasicPanel*						m_pBoard;
	XMapRemoveFangKuai              m_MapRemoveFangKuai; //避免方块在移动的时候删除方块
	cocos2d::Node*                m_pFangKuaiParent;
	bool							m_bBombing;
};