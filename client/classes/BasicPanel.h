#ifndef __BASIC_PANEL_H__
#define __BASIC_PANEL_H__

#include "stdafx.h"
#include "extensions/cocos-ext.h"
#include "cocosbuilder/CocosBuilder.h"

typedef std::map<std::string, cocos2d::Node*> MAP_NODE; 

class BasicPanel :
public cocos2d::Layer,
public cocosbuilder::CCBSelectorResolver,
public cocosbuilder::NodeLoaderListener,
public cocosbuilder::CCBMemberVariableAssigner,
public cocosbuilder::CCBAnimationManagerDelegate
{
public:
	CCB_STATIC_NEW_AUTORELEASE_OBJECT_WITH_INIT_METHOD(BasicPanel, create);
	DECLARE_LUA_CLASS(BasicPanel);

	BasicPanel();
	~BasicPanel();

	virtual cocos2d::SEL_MenuHandler onResolveCCBCCMenuItemSelector(Ref* pTarget, const char* pszSelectorName);
    virtual cocos2d::extension::Control::Handler onResolveCCBCCControlSelector(Ref* pTarget, const char* pszSelectorName);
    virtual cocos2d::SEL_CallFuncN onResolveCCBCCCallFuncSelector(Ref* pTarget, const char* pszSelectorName);
    virtual bool onAssignCCBMemberVariable(Ref* pTarget, const char* pszMemberVariableName, Node* pNode);
    virtual void onNodeLoaded(Node* pNode, cocosbuilder::NodeLoader* pNodeLoader);
	virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onEnter();
	virtual void onExit();
	virtual void completedAnimationSequenceNamed(const char *name);

	void SetAnimationDelegate();
	void PlayCCBAction(const char* pszActionName, float nDelayTime);
	void PrintAllCCBFileName();

	int  LuaSetSizeByName(lua_State* L);
	int  LuaGetSizeByName(lua_State* L);
	int  LuaGetSelfSize(lua_State* L);
	int  LuaSetScaleByName(lua_State* L);
	int  LuaGetPositionByName(lua_State* L);
	int  LuaSetPositionByName(lua_State* L);
	int  LuaSetImageByName(lua_State* L);
	int  LuaSetScale9ImageByName(lua_State* L);
	int  LuaSetAnchorPointByName(lua_State* L);
	int	 LuaPlayCCBAction(lua_State* L);
	int  LuaSetVisibleByName(lua_State* L);
	int  LuaSetLabelTextByName(lua_State* L);
	int  LuaActionEaseSineIn(lua_State* L);
	int  LuaSetRotation(lua_State* L);
	int  LuaSetScrollOffSet(lua_State* L);
    int  LuaRemoveAnimationDelegate(lua_State* L);
	int  LuaSetOrder(lua_State* L);
	int  LuaGetOrder(lua_State* L);
public:
	MAP_NODE m_mapMemberVariable;		//成员
	std::string m_pszSelfType;			//类型，对应脚本文件。如start
	std::string m_pszRootName;			//具体的名字，由父节点及其本身拼接成，如start|settings

private:
	void OnButtonClicked(Ref* pObject, cocos2d::extension::Control::EventType handler);//cocos2d::extension::CCControlEvent eEvent);		//通知Manager某按钮被点击。
	void OnMenuItemClicked(Ref* pObject);
	void OnFuncSelector(Node* pNode);
	void OnActionCompleted();
	std::map<Ref*, std::string> m_mapButtonRootName;
	std::map<Ref*, std::string> m_mapMenuItem;
	std::string m_pszCurrentAni;
};

class BasicPanelLoader : public cocosbuilder::NodeLoader
{
public:
	CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(BasicPanelLoader, loader);
protected:
	CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(BasicPanel);
};

#endif