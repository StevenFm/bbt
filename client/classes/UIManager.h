//
//  UIManager.h
//  BBT
//
//  Created by FM on 14-7-2.
//
//

#ifndef __BBT__UIManager__
#define __BBT__UIManager__

#include "stdafx.h"
#include "BasicPanel.h"
#include "cocosbuilder/CocosBuilder.h"

class UIManager;

extern UIManager* g_UIManager;

class UIManager
{
public:
	UIManager();
	~UIManager();
	bool Init();
	BasicPanel* GetInstanceByRootName(const char* pszRootName);

//提供给Lua的接口，提供给Lua的接口统一用Lua开头
	DECLARE_LUA_CLASS(UIManager);

//Scene
	int LuaChangeScene(lua_State* L);

//Layer
	int LuaAddLayer(lua_State* L);
	int LuaRemoveLayer(lua_State* L);
	int LuaChangeLayer(lua_State* L);
	int LuaAddDialog(lua_State* L);
	int LuaRemoveDialog(lua_State* L);
	
//BasicPanel
	int LuaAddBasicPanelOnNode(lua_State* L);
	int LuaGetInstanceByRootName(lua_State* L);

//Action
	int LuaPlayCCBAction(lua_State* L);

//TableView, EditBox等特殊组件
	int LuaGetTableView(lua_State* L);
	int LuaGetEditBox(lua_State* L);
	int LuaCreateCell(lua_State* L);
    int LuaMoveContentOffset(lua_State* L);
    int LuaGetCellIndex(lua_State* L);

	int LuaGetScrollLayer(lua_State* L);
	int LuaCreateScrollLayerNode(lua_State* L);
	int LuaAddLayerToScrollLayerArray(lua_State* L);
	int LuaInitScrollLayerByArray(lua_State* L);

	int LuaAddChild(lua_State* L);

//Node
	int LuaSetNodeAnchorPoint(lua_State* L);
	int LuaSetNodeVisible(lua_State* L);
	int LuaSetTouchEnabled(lua_State* L);
	int LuaSetKeypadEnabled(lua_State* L);
    int LuaRemoveTextureForKey(lua_State* L);
    int LuaAddTextureImage(lua_State* L);
    int LuaSetTexture(lua_State* L);
	int LuaSetLabelText(lua_State* L);
    int LuaSetPosition(lua_State* L);
    int LuaCreateAndSetSpriteToNode(lua_State* L);
	int LuaSetScaleX(lua_State* L);
	int LuaRemoveAllChildrenWithCleanup(lua_State* L);
	int LuaCreateAndAddSpriteChild(lua_State* L);

	int LuaChangeSpriteImg(lua_State* L);
	int LuaRemoveInstance(lua_State* L);

//MenuItem
	int LuaResetMenuItemImage(lua_State* L);
	int LuaSetMenuItemEnable(lua_State* L);

//CCProgressTimer
	int LuaAddProgressTimerToNode(lua_State* L);
	int LuaSetProgressTimerReverse(lua_State* L);
	int LuaSetProgressTimerMidpoint(lua_State* L);
	int LuaSetProgressTimerPecentage(lua_State* L);
	int LuaSetProgressTimerType(lua_State* L);
	int LuaSetProgressTimerChangeRate(lua_State* L);

//Others
	int LuaCCMessageBox(lua_State* L);
	int LuaSetAnimationDelegate(lua_State* L);
    
    int RemoveAnimationDelegate(const char* pszRootName);
    
	int LuaSetPreferredSize(lua_State* L);
	int LuaGetPlatForm(lua_State* L);
    
    int LuaRemoveTexture(lua_State* L);
    
    int LuaExit(lua_State* L);

//BasicPanel
	int LuaGetPanel(lua_State* L);

//提供给UI实例的接口，统一已UI开头命名
	bool UITouchBegan(const char* pszRootName);
	void UIButtonClicked(const char* pszRootName, const char* pszNodeName);
	void UIMenuItemClicked(const char* pszRootName, const char* pszNodeName);
	void UIOnEnter(const char* pszRootName);
	void UIOnExit(const char* pszRootName);
	void UIOnCompletedAnimationSequenceNamed(const char* pszRootName, const char* pszActionName);
	void UIOnFuncSelector(const char* pszRootName, const char* pszCallback);
	void UIOnActionCompleted(const char* pszRootName);

private:	
	void ChangeScene(const char* pszFileName, bool bPush);
	void ChangeLayer(const char* pszFileName);

	cocos2d::Node* GetUIStanceByFileName(const char* pszFileName);
	cocos2d::Node* CreateNodeByFile(const char* pszFileName);

	void GetCCBInstance(cocos2d::Node* pNode);
	void CleanCInstance();

//使用Lua函数，这些函数统一用DoLua开头
	int DoLuaInitManager();
	int	DoLuaOnNoParams(const char* pszFileName, const char* pszFunctionName);			//调用Lua无参函数
	int	DoLuaOnCreateUIInstance(const char* pszRootName, const char* pszClassName);		//创建Lua对象实例

	std::map<std::string, BasicPanel*> m_mapUIInstance;			//CCB界面文件创建出来的UI实例
	cocos2d::Node* m_pRootNode;
};

#endif /* defined(__BBT__UIManager__) */