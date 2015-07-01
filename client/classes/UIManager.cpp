//
//  UIManger.cpp
//  BBT
//
//  Created by FM on 14-7-2.
//
//

#include "UIManager.h"
#include "ServerAgent.h"
#include "TableView.h"
#include "EditBox.h"
#include "ScrollLayer.h"
#include "Audio.h"
#include "LayerColor.h"
#include "Layer.h"

using namespace std;
using namespace cocos2d;
using namespace cocos2d::extension;
using namespace cocosbuilder;

UIManager* g_UIManager;

IMPL_LUA_CLASS_BEGIN(UIManager)
	//Scene
	EXPORT_LUA_FUNCTION(LuaChangeScene)

	//Layer
	EXPORT_LUA_FUNCTION(LuaAddLayer)
	EXPORT_LUA_FUNCTION(LuaRemoveLayer)
	EXPORT_LUA_FUNCTION(LuaChangeLayer)
	EXPORT_LUA_FUNCTION(LuaAddDialog)
	EXPORT_LUA_FUNCTION(LuaRemoveDialog)
	EXPORT_LUA_FUNCTION(LuaGetScrollLayer)

	//basicPanel
	EXPORT_LUA_FUNCTION(LuaAddBasicPanelOnNode)
	EXPORT_LUA_FUNCTION(LuaGetInstanceByRootName)

	//Action
	EXPORT_LUA_FUNCTION(LuaPlayCCBAction)

	//TableView, EditBox等特殊组件
	EXPORT_LUA_FUNCTION(LuaGetTableView)
	EXPORT_LUA_FUNCTION(LuaGetEditBox)
	EXPORT_LUA_FUNCTION(LuaCreateCell)
	EXPORT_LUA_FUNCTION(LuaMoveContentOffset)


	EXPORT_LUA_FUNCTION(LuaCreateScrollLayerNode)
	EXPORT_LUA_FUNCTION(LuaAddLayerToScrollLayerArray)
	EXPORT_LUA_FUNCTION(LuaInitScrollLayerByArray)
	EXPORT_LUA_FUNCTION(LuaAddChild)

	//Node
	EXPORT_LUA_FUNCTION(LuaSetNodeAnchorPoint)
	EXPORT_LUA_FUNCTION(LuaSetTouchEnabled)
	EXPORT_LUA_FUNCTION(LuaSetKeypadEnabled)
	EXPORT_LUA_FUNCTION(LuaSetNodeVisible)
    EXPORT_LUA_FUNCTION(LuaRemoveTextureForKey)
    EXPORT_LUA_FUNCTION(LuaAddTextureImage)
    EXPORT_LUA_FUNCTION(LuaSetTexture)
	EXPORT_LUA_FUNCTION(LuaSetLabelText)
	EXPORT_LUA_FUNCTION(LuaSetPosition)
	EXPORT_LUA_FUNCTION(LuaSetScaleX)
	EXPORT_LUA_FUNCTION(LuaRemoveAllChildrenWithCleanup)
	EXPORT_LUA_FUNCTION(LuaCreateAndAddSpriteChild)
	EXPORT_LUA_FUNCTION(LuaRemoveInstance)

	EXPORT_LUA_FUNCTION(LuaChangeSpriteImg)
	//MenuItemIamge
	EXPORT_LUA_FUNCTION(LuaResetMenuItemImage)
	EXPORT_LUA_FUNCTION(LuaSetMenuItemEnable)

	//ProgressTimer
	EXPORT_LUA_FUNCTION(LuaAddProgressTimerToNode)
	EXPORT_LUA_FUNCTION(LuaSetProgressTimerReverse)
	EXPORT_LUA_FUNCTION(LuaSetProgressTimerMidpoint)
	EXPORT_LUA_FUNCTION(LuaSetProgressTimerPecentage)
	EXPORT_LUA_FUNCTION(LuaSetProgressTimerType)
	EXPORT_LUA_FUNCTION(LuaSetProgressTimerChangeRate)

	//Others
	EXPORT_LUA_FUNCTION(LuaCCMessageBox)
	EXPORT_LUA_FUNCTION(LuaSetAnimationDelegate)
    EXPORT_LUA_FUNCTION(LuaGetCellIndex)
    EXPORT_LUA_FUNCTION(LuaCreateAndSetSpriteToNode)
	EXPORT_LUA_FUNCTION(LuaSetPreferredSize)
	EXPORT_LUA_FUNCTION(LuaGetPlatForm)
    EXPORT_LUA_FUNCTION(LuaRemoveTexture)
    EXPORT_LUA_FUNCTION(LuaExit)

	//BasicPanel
	EXPORT_LUA_FUNCTION(LuaGetPanel)
IMPL_LUA_CLASS_END()

UIManager::UIManager()
{
	g_Audio = new Audio();
}

UIManager::~UIManager()
{
	XY_DELETE(g_Audio);
}

bool UIManager::Init()
{
	NodeLoaderLibrary* pLoaderLibrary = NodeLoaderLibrary::getInstance();

	pLoaderLibrary->unregisterNodeLoader("CCLayerColor");
	pLoaderLibrary->unregisterNodeLoader("CCLayer");

	pLoaderLibrary->registerNodeLoader("BasicPanel", BasicPanelLoader::loader());
	pLoaderLibrary->registerNodeLoader("TableView", XTableViewLoader::loader());
	pLoaderLibrary->registerNodeLoader("EditBox", XEditBoxLoader::loader());
    pLoaderLibrary->registerNodeLoader("CCLayerColor", XLayerColorLoader::loader());
    pLoaderLibrary->registerNodeLoader("CCLayer", XLayerLoader::loader());

	Scene* scene = Scene::create();
	m_pRootNode = Node::create();
	scene->addChild(m_pRootNode);
	scene->addChild(g_pServerAgent);
	Director::getInstance()->runWithScene(scene);

	DoLuaInitManager();
	return true;
}

BasicPanel* UIManager::GetInstanceByRootName(const char* pszRootName)
{ 
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	return m_mapUIInstance[pszRootName];
Exit0:
	return NULL;
}

void UIManager::ChangeScene(const char* pszFileName, bool bPush)
{
	Node*	pNode			= NULL;
// 	Scene*	pRunningScene	= Director::sharedDirector()->getRunningScene();
// 	Scene*	pScene			= Scene::create();

// 	if (!bPush)
// 	{
// 		if (pRunningScene)
// 		{
// 			pRunningScene->removeAllChildrenWithCleanup(true);
// 		}
		CleanCInstance();
//	}
	
	pNode = GetUIStanceByFileName(pszFileName);
	XYLOG_FAILED_JUMP(pNode);

	m_pRootNode->addChild(pNode);

// 	if (!pRunningScene)
// 	{
// 		Director::sharedDirector()->runWithScene(pScene);
// 		return;
// 	}
// 	if (bPush)
// 	{
// 		Director::sharedDirector()->pushScene(pScene);
// 	}
// 	else
// 	{
// 		Director::sharedDirector()->replaceScene(pScene);
// 	}
	
Exit0:
	return;
}

void UIManager::ChangeLayer(const char* pszFileName)
{
	//Scene*	pScene		= Director::sharedDirector()->getRunningScene();
	Node*	pNode		= NULL;

// 	if (!pScene)
// 	{
// 		pScene = Scene::create();
// 	}

	//pScene->removeAllChildrenWithCleanup(true);
	CleanCInstance();

	pNode = GetUIStanceByFileName(pszFileName);
	XY_FAILED_JUMP(pNode);

	m_pRootNode->addChild(pNode);
Exit0:
	return;
}

Node* UIManager::GetUIStanceByFileName(const char* pszFileName)
{
	Node*		pNode		= CreateNodeByFile(pszFileName);
	XYLOG_FAILED_JUMP(pNode);

	GetCCBInstance(pNode);

	return pNode;
Exit0:
	return NULL;
}

Node* UIManager::CreateNodeByFile(const char* pszFileName)
{
	NodeLoaderLibrary*		pLoaderLibrary	= NodeLoaderLibrary::getInstance();
	CCBReader*				pReader			= new CCBReader(pLoaderLibrary);
	Node*					pNode			= NULL;

	assert(FileUtils::getInstance()->isFileExist(pszFileName));
	pNode = pReader->readNodeGraphFromFile(pszFileName);
	XY_DELETE(pReader);
	return pNode;
}

void UIManager::GetCCBInstance(Node* pNode)
{
	BasicPanel*				pPanel			= dynamic_cast<BasicPanel*>(pNode);
	Node*					pParentPanel;
	MAP_NODE::iterator		itMember;
	XYLOG_FAILED_JUMP(pPanel);

	pParentPanel = (pPanel->getParent());
	while (pParentPanel != NULL)
	{
		if (dynamic_cast<BasicPanel*>(pParentPanel) != NULL)
		{
			string pszNewRootName = dynamic_cast<BasicPanel*>(pParentPanel)->m_pszRootName;
			pszNewRootName.append("|");
			pPanel->m_pszRootName = pszNewRootName.append(pPanel->m_pszRootName);
			break;
		}
		if (pParentPanel->getParent() == NULL)
		{
			break;
		}
		pParentPanel = pParentPanel->getParent();
	}

	for (itMember = pPanel->m_mapMemberVariable.begin(); itMember != pPanel->m_mapMemberVariable.end(); ++itMember)
	{
		BasicPanel* pPanelIT = dynamic_cast<BasicPanel*>(itMember->second);
		if (pPanelIT)
		{
			GetCCBInstance(itMember->second);	
		}
	}
    
    m_mapUIInstance[pPanel->m_pszRootName] = pPanel;
	DoLuaOnCreateUIInstance(pPanel->m_pszRootName.c_str(), pPanel->m_pszSelfType.c_str());
Exit0:
	return;
}

void UIManager::CleanCInstance()
{
	m_pRootNode->removeAllChildren();
	m_mapUIInstance.clear();
}

bool UIManager::UITouchBegan(const char* pszRootName)
{
	BOOL			bResult					= false;
	int				nRetCode				= 0;
	bool			bTouch					= false;
	lua_State*		pLuaState				= g_pServerAgent->GetLuaState();
	XLuaSafeStack   luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "LayerTouchBegan");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);

	nRetCode = Lua_XCall(luaSafeStack, 1, 1);
	XYLOG_FAILED_JUMP(nRetCode);

	bTouch = lua_toboolean(pLuaState, -1);
	return bTouch;
Exit0:
	return bResult;
}

void UIManager::UIButtonClicked(const char* pszRootName, const char* pszNodeName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "ButtonClicked");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);
	lua_pushstring(pLuaState, pszNodeName);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIMenuItemClicked(const char* pszRootName, const char* pszNodeName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "MenuItemClicked");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);
	lua_pushstring(pLuaState, pszNodeName);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIOnEnter(const char* pszRootName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "OnEnter");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);

	nRetCode = Lua_XCall(luaSafeStack, 1, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIOnExit(const char* pszRootName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "OnExit");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);

	nRetCode = Lua_XCall(luaSafeStack, 1, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIOnCompletedAnimationSequenceNamed(const char* pszRootName, const char* pszActionName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "OnCompletedAnimationSequenceNamed");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);
	lua_pushstring(pLuaState, pszActionName);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIOnFuncSelector(const char* pszRootName, const char* pszCallback)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "OnFuncSelector");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);
	lua_pushstring(pLuaState, pszCallback);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void UIManager::UIOnActionCompleted(const char* pszRootName)
{
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack	luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "OnFuncSelector");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);

	nRetCode = Lua_XCall(luaSafeStack, 1, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

int UIManager::DoLuaInitManager()
{
	BOOL			bResult         = false;
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack   luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "Init");
	XYLOG_FAILED_JUMP(nRetCode);

	nRetCode = Lua_XCall(luaSafeStack, 0, 0);
	XYLOG_FAILED_JUMP(nRetCode);

	bResult = true;
Exit0:
	return bResult;
}

int UIManager::DoLuaOnNoParams(const char* pszFileName, const char* pszFunctionName)
{
	BOOL			bResult         = false;
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack   luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, pszFileName, pszFunctionName);
	XYLOG_FAILED_JUMP(nRetCode);

	nRetCode = Lua_XCall(luaSafeStack, 0, 0);
	XYLOG_FAILED_JUMP(nRetCode);

	bResult = true;
Exit0:
	return bResult;
}

int UIManager::DoLuaOnCreateUIInstance(const char* pszRootName, const char* pszClassName)
{
	BOOL			bResult         = false;
	int				nRetCode		= 0;
	lua_State*		pLuaState		= g_pServerAgent->GetLuaState();
	XLuaSafeStack   luaSafeStack(pLuaState);

	nRetCode = Lua_GetFunction(pLuaState, "scripts/uimanager.lua", "CreateUIClass");
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushstring(pLuaState, pszRootName);
	lua_pushstring(pLuaState, pszClassName);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);

	bResult = true;
Exit0:
	return bResult;
}

int UIManager::LuaChangeScene(lua_State* L)
{
	const char*		pszFileName		= lua_tostring(L, 1);
	bool			bPush			= lua_toboolean(L, 2);

	ChangeScene(pszFileName, bPush);
	return 0;
}

/*
int UIManager::LuaAddNodeToScene(lua_State* L)
{

	return 0;
}
*/
int UIManager::LuaAddLayer(lua_State* L)
{
	const char*				pszFileName			= lua_tostring(L, 1);		//界面文件
	const char*				pszParentName		= lua_tostring(L, 2);		//父节点名字
	const char*				pszRootName			= lua_tostring(L, 3);		//自己的名字
	BasicPanel*				pParent;
	Node*					pAddNode;
	BasicPanel*				pPanel;
	std::string				szAppendName;
	MAP_NODE::iterator		itMember;

	XYLOG_FAILED_JUMP(pszFileName && pszParentName && pszRootName);

	pParent = m_mapUIInstance[pszParentName];
	XYLOG_FAILED_JUMP(pParent);

	pAddNode = CreateNodeByFile(pszFileName);
	XYLOG_FAILED_JUMP(pAddNode);

	pPanel = dynamic_cast<BasicPanel*>(pAddNode);
	XYLOG_FAILED_JUMP(pPanel);

	szAppendName			= pszParentName;
	(szAppendName.append("|")).append(pszRootName);
	pPanel->m_pszRootName	= szAppendName;

	pParent->m_mapMemberVariable[pszRootName]	= pPanel;
	m_mapUIInstance[pPanel->m_pszRootName]		= pPanel;

	DoLuaOnCreateUIInstance(pPanel->m_pszRootName.c_str(), pPanel->m_pszSelfType.c_str());

	for (itMember = pPanel->m_mapMemberVariable.begin(); itMember != pPanel->m_mapMemberVariable.end(); ++itMember)
	{
		if (dynamic_cast<BasicPanel*>(itMember->second))
		{
			GetCCBInstance(itMember->second);
		}
	}
	
	pParent->addChild(pPanel);
	return 0;
Exit0:
	return 0;
}

int UIManager::LuaRemoveLayer(lua_State* L)
{
	const char*									pszNodeName;
	string										szParentName;
	string										szSelfName;
	string										szChildName;
	int											nBackPosition;
	map<std::string, BasicPanel*>::iterator		it;

	pszNodeName		= lua_tostring(L, 1);
	XY_FAILED_JUMP(pszNodeName && strcmp(pszNodeName, ""));

	szChildName		= pszNodeName;
	nBackPosition	= szChildName.find_last_of("|");
	if (nBackPosition != -1)
	{
		szParentName = szChildName.substr(0, nBackPosition);
		szSelfName	 = szChildName.substr(nBackPosition + 1, szChildName.length());
		m_mapUIInstance[szParentName]->m_mapMemberVariable[szSelfName] = NULL;
	}

	m_mapUIInstance[pszNodeName]->removeFromParentAndCleanup(true);

	for (it = m_mapUIInstance.begin(); it != m_mapUIInstance.end();)
	{
		int nPosition = it->first.find(szChildName);
		if (nPosition != -1)
		{
			//DoLuaOnDeleteUIInstance(it->first.c_str());
			m_mapUIInstance.erase(it++);
			continue;
		}
		it++;
	}
Exit0:
	return 0;
}

int UIManager::LuaChangeLayer(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszFileName;
	
	XYLOG_FAILED_JUMP(nTop == 1);

	pszFileName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFileName && strlen(pszFileName) > 0);

	ChangeLayer(pszFileName);
Exit0:
	return 0;
}

int UIManager::LuaAddDialog(lua_State* L)
{
	const char*				pszFileName			= lua_tostring(L, 1);		//界面文件
	const char*				pszRootName			= lua_tostring(L, 2);		//自己的名字
	Node*					pAddNode			= NULL;
	BasicPanel*				pPanel				= NULL;
	Scene*					pScene				= NULL;

	XYLOG_FAILED_JUMP(pszFileName && pszRootName);

	pAddNode = CreateNodeByFile(pszFileName);
	XYLOG_FAILED_JUMP(pAddNode);

	pPanel = dynamic_cast<BasicPanel*>(pAddNode);
	XYLOG_FAILED_JUMP(pPanel);

	pPanel->m_pszRootName						= pszRootName;
	m_mapUIInstance[pPanel->m_pszRootName]		= pPanel;

	pScene = CCDirector::sharedDirector()->getRunningScene();
	XYLOG_FAILED_JUMP(pScene);
	pScene->addChild(pPanel);

	DoLuaOnCreateUIInstance(pPanel->m_pszRootName.c_str(), pPanel->m_pszSelfType.c_str());
	return 0;
Exit0:
	return 0;
}

int UIManager::LuaRemoveDialog(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszRootName		= NULL;
	Node*			pNode			= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszRootName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszRootName);

	pNode = m_mapUIInstance[pszRootName];
	XYLOG_FAILED_JUMP(pNode);

	m_mapUIInstance.erase(pszRootName);
	pNode->removeFromParent();
Exit0:
	return 0;
}

int UIManager::LuaAddBasicPanelOnNode(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszPanelPath	= NULL;
	const char*		pszParentName	= NULL;
	const char*		pszNodeName		= NULL;
	const char*		pszRootName		= NULL;
	BasicPanel*				pParent;
	Node*					pParentNode;
	Node*					pAddNode;
	BasicPanel*				pPanel;
	string					szAppendName;
	MAP_NODE::iterator		itMember;

	XYLOG_FAILED_JUMP(nTop == 4);

	pszPanelPath	= lua_tostring(L, 1);
	pszParentName	= lua_tostring(L, 2);
	pszNodeName		= lua_tostring(L, 3);
	pszRootName		= lua_tostring(L, 4);
	XYLOG_FAILED_JUMP(pszPanelPath && pszParentName && pszNodeName && pszRootName);

	pParent = m_mapUIInstance[pszParentName];
	XYLOG_FAILED_JUMP(pParent);

	pParentNode = pParent->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pParentNode);

	pAddNode = CreateNodeByFile(pszPanelPath);
	XYLOG_FAILED_JUMP(pAddNode);

	pPanel = dynamic_cast<BasicPanel*>(pAddNode);
	XYLOG_FAILED_JUMP(pPanel);

	szAppendName			= pszParentName;
	(szAppendName.append("|")).append(pszRootName);
	pPanel->m_pszRootName	= szAppendName;

	pParent->m_mapMemberVariable[pszRootName]	= pPanel;
	m_mapUIInstance[pPanel->m_pszRootName]		= pPanel;

	DoLuaOnCreateUIInstance(pPanel->m_pszRootName.c_str(), pPanel->m_pszSelfType.c_str());

	for (itMember = pPanel->m_mapMemberVariable.begin(); itMember != pPanel->m_mapMemberVariable.end(); ++itMember)
	{
		if (dynamic_cast<BasicPanel*>(itMember->second))
		{
			GetCCBInstance(itMember->second);
		}
	}

	pParentNode->addChild(pPanel);
Exit0:
	return 0;
}

int UIManager::LuaGetInstanceByRootName(lua_State* L)
{
	int nTop					= lua_gettop(L);
	const char*	pszRootName		= NULL;
	BasicPanel*	pInstance		= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszRootName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszRootName);

	pInstance = GetInstanceByRootName(pszRootName);
	XYLOG_FAILED_JUMP(pInstance);

	Lua_PushObject(L, pInstance);
	return 1;
Exit0:
	log("找不到节点%s", pszRootName);
	return 0;
}

int UIManager::LuaPlayCCBAction(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszRootName		= NULL;
	const char*		pszActionName	= NULL;
	float			fDelay			= 0.0f;
	BasicPanel*		pPanel			= NULL;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszRootName		= lua_tostring(L,1);
	pszActionName	= lua_tostring(L, 2);
	fDelay			= lua_tonumber(L, 3);

	XYLOG_FAILED_JUMP(pszRootName && pszActionName);

	pPanel			= m_mapUIInstance[pszRootName];
	XYLOG_FAILED_JUMP(pPanel);

	pPanel->PlayCCBAction(pszActionName, fDelay);
Exit0:
	return 0;
}

int UIManager::LuaGetTableView(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	XTableView*		pNode			= dynamic_cast<XTableView*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	XY_FAILED_JUMP(pNode);

	Lua_PushObject(L, pNode);
	return 1;
Exit0:
	log("找不到节点%s, %s", pszRootName, pszNodeName);
	return 0;
}

int UIManager::LuaGetEditBox(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	XEditBox*		pEditBox;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	pEditBox = dynamic_cast<XEditBox*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pEditBox);

	Lua_PushObject(L, pEditBox);
	return 1;
Exit0:
	return 0;
}

int UIManager::LuaCreateCell(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszCellName		= lua_tostring(L, 2);
	const char*		pszFileName		= lua_tostring(L, 3);
	string			szCellName		= pszRootName;
	BasicPanel*		pCell;

	XYLOG_FAILED_JUMP(pszRootName && pszCellName && pszFileName);

	pCell = dynamic_cast<BasicPanel*>(CreateNodeByFile(pszFileName));
	XYLOG_FAILED_JUMP(pCell);

	(szCellName.append("|")).append(pszCellName);
	m_mapUIInstance[szCellName] = pCell;
	pCell->m_pszRootName = szCellName;

	DoLuaOnCreateUIInstance(pCell->m_pszRootName.c_str(), pCell->m_pszSelfType.c_str());
	Lua_PushObject(L, pCell);
	return 1;
Exit0:
	return 0;
}

int UIManager::LuaMoveContentOffset(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	float    		fOffset  		= lua_tonumber(L, 3);
	TableView*		pTableView;
	CCPoint         point;

	XYLOG_FAILED_JUMP(pszRootName && pszNodeName && fOffset);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	pTableView = dynamic_cast<TableView*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTableView);

	point = pTableView->getContentOffset();
	pTableView->setContentOffset(ccpSub(point, ccp(fOffset,0)));
Exit0:
	return 0;
}

int UIManager::LuaSetTouchEnabled(lua_State* L)
{
    const char*		pszParentRootName	= lua_tostring(L, 1);
	bool			bEnable				= lua_toboolean(L, 2);
	BasicPanel*		pPanel				= m_mapUIInstance[pszParentRootName];
	XY_FAILED_JUMP(pPanel);

	pPanel->setTouchEnabled(bEnable);
Exit0:
	return 0;
}

int UIManager::LuaSetKeypadEnabled(lua_State* L)
{
    const char*		pszParentRootName	= lua_tostring(L, 1);
	bool			bEnable				= lua_toboolean(L, 2);
	BasicPanel*		pPanel				= m_mapUIInstance[pszParentRootName];
	XY_FAILED_JUMP(pPanel);

	pPanel->setKeypadEnabled(bEnable);
Exit0:
	return 0;
}

int UIManager::LuaSetNodeAnchorPoint(lua_State* L)
{
	int			nTop			= lua_gettop(L);
	const char* pszParentNode	= NULL;
	const char* pszNode			= NULL;
	float		fX;
	float		fY;
	BasicPanel* pParent			= NULL;
	Node*		pNode			= NULL;

	XYLOG_FAILED_JUMP(nTop == 4);

	pszParentNode	= lua_tostring(L, 1);
	pszNode			= lua_tostring(L, 2);
	fX				= lua_tonumber(L, 3);
	fY				= lua_tonumber(L, 4);

	pParent = m_mapUIInstance[pszParentNode];
	XYLOG_FAILED_JUMP(pParent);

	pNode = pParent->m_mapMemberVariable[pszNode];
	XYLOG_FAILED_JUMP(pNode);

	pNode->setAnchorPoint(ccp(fX, fY));
Exit0:
	return 0;
}

int UIManager::LuaSetNodeVisible(lua_State* L)
{
	const char*		pszParentRootName	 = lua_tostring(L, 1);
	const char*		pszRootName			 = lua_tostring(L, 2);
	bool			bVisible			 = lua_toboolean(L, 3);

	BasicPanel* pPanel = m_mapUIInstance[pszParentRootName];
	XYLOG_FAILED_JUMP(pPanel);

	pPanel->m_mapMemberVariable[pszRootName]->setVisible(bVisible);
Exit0:
	return 0;
}

int UIManager::LuaRemoveTextureForKey(lua_State* L)
{
    const char*		pszFileName			 = lua_tostring(L, 1);
    
    CCTextureCache::sharedTextureCache()->removeTextureForKey(pszFileName);
    
    return 0;
}

int UIManager::LuaAddTextureImage(lua_State* L)
{
	const char*		pszFileName			 = lua_tostring(L, 1);

	CCTextureCache::sharedTextureCache()->addImage(pszFileName);

	return 0;
}

int UIManager::LuaSetTexture(lua_State* L)
{
	int				nTop				 = lua_gettop(L);
    const char*		pszParentRootName	 = NULL;
    const char*		pszRootName			 = NULL;
	const char*		pszFileName			 = NULL;
    BasicPanel*     pPanel               = NULL;
	CCTexture2D*	pTexture;
	CCSprite*		pSprite;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszParentRootName	 = lua_tostring(L, 1);
	pszRootName			 = lua_tostring(L, 2);
	pszFileName			 = lua_tostring(L, 3);
	XYLOG_FAILED_JUMP(pszParentRootName && pszRootName && pszFileName);
    
    pPanel = m_mapUIInstance[pszParentRootName];
	XYLOG_FAILED_JUMP(pPanel);
    
    pTexture = CCTextureCache::sharedTextureCache()->textureForKey(pszFileName);
    if (pTexture == NULL)
    {
        pTexture = CCTextureCache::sharedTextureCache()->addImage(pszFileName);
    }
	XYLOG_FAILED_JUMP(pTexture);

	pSprite	= dynamic_cast<CCSprite*>(pPanel->m_mapMemberVariable[pszRootName]);
	XYLOG_FAILED_JUMP(pSprite);

    pSprite->setTexture(pTexture);
    pSprite->setTextureRect(CCRect(pSprite->getTextureRect().origin.x, pSprite->getTextureRect().origin.y, pTexture->getPixelsWide(), pTexture->getPixelsHigh())); //使用这个方法不会造成图片变形的问题
Exit0:
    return 0;
}

int UIManager::LuaChangeSpriteImg(lua_State* L)
{
	const char*		pszParentRootName	 = lua_tostring(L, 1);
    const char*		pszRootName			 = lua_tostring(L, 2);
	const char*		pszFileName			 = lua_tostring(L, 3);
    BasicPanel*     pPanel               = NULL;
    CCTexture2D *   pTexture             = NULL;
    
    pPanel = m_mapUIInstance[pszParentRootName];
	XYLOG_FAILED_JUMP(pPanel);
    
	  //更换图片  
	pTexture = CCTextureCache::sharedTextureCache()->addImage(pszFileName);
    ((cocos2d::CCSprite*)(pPanel->m_mapMemberVariable[pszRootName]))->setTexture(pTexture);
    
Exit0:
    return 0;
}

int UIManager::LuaRemoveInstance(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszInstanceName	= NULL;
	BasicPanel*		pInstance		= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszInstanceName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszInstanceName);

	pInstance = m_mapUIInstance[pszInstanceName];
	XYLOG_FAILED_JUMP(pInstance);

    RemoveAnimationDelegate(pszInstanceName);
    
	pInstance->removeFromParentAndCleanup(true);
    
    m_mapUIInstance[pszInstanceName] = NULL;
    m_mapUIInstance.erase(pszInstanceName);
Exit0:
	return 0;
}

int UIManager::LuaSetLabelText(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszRootName		= NULL;
	const char*		pszNodeName		= NULL;
	const char*		pszText			= NULL;
	BasicPanel*		pParent			= NULL;
	Label*			pNode			= NULL;

	pszRootName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszRootName && pszRootName[0] != '\0');

	pszNodeName = lua_tostring(L, 2);
	XYLOG_FAILED_JUMP(pszNodeName && pszNodeName[0] != '\0');

	pszText		= lua_tostring(L, 3);
	XYLOG_FAILED_JUMP(pszText && pszText[0] != '\0');

	pParent		= m_mapUIInstance[pszRootName];
	XYLOG_FAILED_JUMP(pParent);

	pNode		= dynamic_cast<Label*>(pParent->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pNode);

	pNode->setString(pszText);
Exit0:
	return 0;
}

int UIManager::LuaSetPosition(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszParentName	= NULL;
	const char*		pszNodeName		= NULL;
	BasicPanel*		pParent			= NULL;
	Node*			pNode			= NULL;
	float			fX;
	float			fY;

	XYLOG_FAILED_JUMP(nTop == 4);

	pszParentName = lua_tostring(L, 1);
	pszNodeName = lua_tostring(L, 2);
	XYLOG_FAILED_JUMP(pszParentName && pszNodeName);

	fX = lua_tonumber(L, 3);
	fY = lua_tonumber(L, 4);

	pParent = m_mapUIInstance[pszParentName];
	XYLOG_FAILED_JUMP(pParent);

	pNode = pParent->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pNode);

	pNode->setPosition(fX, fY);
Exit0:
	return 0;
}

int UIManager::LuaSetScaleX(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	float			fFlip			= lua_tonumber(L, 3);
	Node*			pFlip			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pFlip = m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pFlip);

	pFlip->setScaleX(fFlip);
Exit0:
	return 0;
}

int UIManager::LuaRemoveAllChildrenWithCleanup(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	float			bMove			= lua_tonumber(L, 3);
	Node*			pNode			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pNode = m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pNode);

	pNode->removeAllChildrenWithCleanup(bMove);
Exit0:
	return 0;
}

int UIManager::LuaCreateAndAddSpriteChild(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	const char*	    pszFile			= lua_tostring(L, 3);
	float           fPointX         = lua_tonumber(L, 4);
	float           fPointY         = lua_tonumber(L, 5);
	Node*			pNode			= NULL;
	CCSprite*       sprite          = NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pNode = m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pNode);

	sprite = CCSprite::create(pszFile);
	sprite->setPosition(ccp(fPointX, fPointY));//sprite->setScale(100);
    pNode->addChild(sprite);

Exit0:
	return 0;
}
int UIManager::LuaResetMenuItemImage(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	const char*			pszNormal		= lua_tostring(L, 3);
	const char*			pszSelect		= lua_tostring(L, 4);
	const char*			pszDisable		= lua_tostring(L, 5);
	CCMenuItemImage*	pMenuItem		= NULL;
	CCSprite*			pNormal			= NULL;
	CCSprite*			pSelect			= NULL;
	CCSprite*			pDisable		= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	pMenuItem	= dynamic_cast<CCMenuItemImage*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pMenuItem);

	pNormal		= CCSprite::create(pszNormal);
	pSelect		= CCSprite::create(pszSelect);
	pDisable	= CCSprite::create(pszDisable);
	XYLOG_FAILED_JUMP(pNormal && pSelect && pDisable);

	pMenuItem->setNormalImage(pNormal);
	pMenuItem->setSelectedImage(pSelect);
	pMenuItem->setDisabledImage(pDisable);
Exit0:
	return 0;
}

int UIManager::LuaSetMenuItemEnable(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	bool				bEnable			= lua_toboolean(L, 3);
	CCMenuItem*			pItem			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pItem = dynamic_cast<CCMenuItem*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pItem);

	pItem->setEnabled(bEnable);
Exit0:
	return 0;
}

int UIManager::LuaAddProgressTimerToNode(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	const char*			pszSelfName		= lua_tostring(L, 3);
	const char*			pszImageFile	= lua_tostring(L, 4);
	CCSprite*			pImageForTimer	= NULL;
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	XYLOG_FAILED_JUMP(!m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszSelfName]);

	pImageForTimer = CCSprite::create(pszImageFile);
	XYLOG_FAILED_JUMP(pImageForTimer);

	pTimer = CCProgressTimer::create(pImageForTimer);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setAnchorPoint(ccp(0.0f, 0.0f));

	m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszSelfName] = pTimer;
	m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]->addChild(pTimer);
Exit0:
	return 0;
}

int UIManager::LuaSetProgressTimerReverse(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	bool				bReverse		= lua_tostring(L, 3);
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pTimer = dynamic_cast<CCProgressTimer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setReverseProgress(bReverse);
Exit0:
	return 0;
}

int UIManager::LuaSetProgressTimerMidpoint(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	float				fX				= lua_tonumber(L, 3);
	float				fY				= lua_tonumber(L, 4);
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pTimer = dynamic_cast<CCProgressTimer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setMidpoint(ccp(fX, fY));
Exit0:
	return 0;
}

int UIManager::LuaSetProgressTimerPecentage(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	float				fPecentage		= lua_tonumber(L, 3);
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pTimer = dynamic_cast<CCProgressTimer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setPercentage(fPecentage);
Exit0:
	return 0;
}int UIManager::LuaCCMessageBox(lua_State* L)
{
	const char*		pszMsg	  = lua_tostring(L, 1);
	const char*	    pszTitle  = lua_tostring(L, 2);

	MessageBox(pszMsg, pszTitle);

	return 0;
}

int UIManager::LuaSetProgressTimerType(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	int 				nType			= lua_tonumber(L, 3);
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pTimer = dynamic_cast<CCProgressTimer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setType((CCProgressTimerType)nType);
Exit0:
	return 0;
}

int UIManager::LuaSetProgressTimerChangeRate(lua_State* L)
{
	const char*			pszRootName		= lua_tostring(L, 1);
	const char*			pszNodeName		= lua_tostring(L, 2);
	float 				nW				= lua_tonumber(L, 3);
	float				nH				= lua_tonumber(L, 4);
	CCProgressTimer*	pTimer			= NULL;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pTimer = dynamic_cast<CCProgressTimer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pTimer);

	pTimer->setBarChangeRate(ccp(nW, nH));
Exit0:
	return 0;
}

int UIManager::LuaSetAnimationDelegate(lua_State* L)
{
	const char*		pszRootName	= lua_tostring(L, 1);

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

	m_mapUIInstance[pszRootName]->SetAnimationDelegate();
Exit0:
	return 0;
}

int UIManager::RemoveAnimationDelegate(const char* pszRootName)
{
    CCBAnimationManager* pManager = NULL;
    
    XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);
    
    pManager = dynamic_cast<CCBAnimationManager*>(m_mapUIInstance[pszRootName]->getUserObject());
    pManager->setDelegate(NULL);
Exit0:
    return 0;
}

int UIManager::LuaCreateAndSetSpriteToNode(lua_State* L)
{
	int				nTop			= lua_gettop(L);
    const char*		pszRootName		= NULL;
	const char*		pszNodeName		= NULL;
	const char*		pszFile			= NULL;
	Node*			pNode			= NULL;
    CCSprite*		pSprite			= NULL;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszRootName = lua_tostring(L, 1);
	pszNodeName = lua_tostring(L, 2);
	pszFile		= lua_tostring(L, 3);

	XYLOG_FAILED_JUMP(pszRootName && pszNodeName && pszFile);
    
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);
    
	pNode = dynamic_cast<Node*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pNode);
    
    pSprite = CCSprite::create(pszFile);
    XYLOG_FAILED_JUMP(pSprite);
    
	pNode->addChild(pSprite);
Exit0:
	return 0;
}

int UIManager::LuaGetCellIndex(lua_State* L)
{
    BasicPanel*		panel		= Lua_ToObject<BasicPanel>(L, 1);
	Node*		    pNode       = panel->getParent();
    TableViewCell* pCell      = dynamic_cast<TableViewCell*>(pNode);
    int             nIndex       = pCell->getIdx();
    
    lua_pushinteger(L, nIndex);
Exit0:
	return 1;
}
//SetPreferredSize(szRootName, szNodeName, nHight, fValue)
int UIManager::LuaSetPreferredSize(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	float		    fHight			= lua_tonumber(L, 3);
	float		    fValue			= lua_tonumber(L, 4);
	cocos2d::extension::Scale9Sprite*		    pSprite;
	CCSize          Size            = CCSize(fValue , fHight);
    
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);
    
	pSprite = dynamic_cast<cocos2d::extension::Scale9Sprite*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pSprite);
	
	pSprite->setPreferredSize(Size);
Exit0:
	return 0;
}

int UIManager::LuaGetPlatForm(lua_State* L)
{
	TargetPlatform target = Application::getInstance()->getTargetPlatform();
	lua_pushinteger(L,(int)target);
	return 1;
}

int UIManager::LuaRemoveTexture(lua_State* L)
{
	/*
    CCTextureCache::sharedTextureCache()->removeAllTextures();
    CCSpriteFrameCache::sharedSpriteFrameCache()->removeUnusedSpriteFrames();
	*/
    return 0;
}

int UIManager::LuaExit(lua_State *L)
{
    exit(0);
    return 0;
}

int UIManager::LuaGetScrollLayer(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	XScrollLayer*	pNode			= dynamic_cast<XScrollLayer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	XY_FAILED_JUMP(pNode);

	Lua_PushObject(L, pNode);
	return 1;
Exit0:
	CCLOGERROR("找不到节点%s, %s", pszRootName, pszNodeName);
	return 0;
}

int UIManager::LuaCreateScrollLayerNode(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	XScrollLayer*   pScrollLayer    = NULL;
	pScrollLayer = new XScrollLayer;

	XYLOG_FAILED_JUMP(pszRootName &&  pszNodeName);
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

    (m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]) = pScrollLayer;

Exit0:
	return 0;
}

int UIManager::LuaAddLayerToScrollLayerArray(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);
	const char*		pszFileName		= lua_tostring(L, 3);
	const char*		pszLayerName	= lua_tostring(L, 4);

	XScrollLayer*   pScrollLayer    = NULL;
	BasicPanel*		pLayer;

	char            cszNum[5] = {0};
	string			szLayerName		= pszRootName;


	XYLOG_FAILED_JUMP(pszRootName && pszNodeName && pszLayerName && pszFileName);
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);

    pScrollLayer = dynamic_cast<XScrollLayer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

    pLayer = dynamic_cast<BasicPanel*>(CreateNodeByFile(pszFileName));
	XYLOG_FAILED_JUMP(pLayer);

	//itoa(pScrollLayer->GetLayerCount() + 1, cszNum, 10);
	sprintf(cszNum,"%d", pScrollLayer->GetLayerCount() + 1);
	((szLayerName.append("|")).append(pszLayerName)).append(cszNum);
	m_mapUIInstance[szLayerName] = pLayer;
	pLayer->m_pszRootName = szLayerName;
    GetCCBInstance(pLayer);

	pScrollLayer->AddLayerToArray(pLayer);
Exit0:
	return 0;
}

int UIManager::LuaInitScrollLayerByArray(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszNodeName		= lua_tostring(L, 2);

	XScrollLayer*   pScrollLayer    = NULL;

	XYLOG_FAILED_JUMP(pszRootName &&  pszNodeName);
	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);


	pScrollLayer = dynamic_cast<XScrollLayer*>(m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszNodeName]);

	pScrollLayer->initScrollLayer(0);
Exit0:
	return 0;
}

int UIManager::LuaAddChild(lua_State* L)
{
	const char*		pszRootName		= lua_tostring(L, 1);
	const char*		pszParentName	= lua_tostring(L, 2);
	const char*		pszChildName	= lua_tostring(L, 3);

	Node*  pParentNode;
	Node*  pChildNode;

	XYLOG_FAILED_JUMP(m_mapUIInstance[pszRootName]);
    
	pParentNode = (m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszParentName]);
	XYLOG_FAILED_JUMP(pParentNode);

	pChildNode = (m_mapUIInstance[pszRootName]->m_mapMemberVariable[pszChildName]);
	XYLOG_FAILED_JUMP(pChildNode);

	pParentNode->addChild(pChildNode);
Exit0:
	return 0;
}

int UIManager::LuaGetPanel(lua_State* L)
{
	const int		nTop		= lua_gettop(L);
	const char*		pszInstance = NULL;
	BasicPanel*		pPanel		= NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszInstance = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszInstance);

	pPanel = m_mapUIInstance[pszInstance];
	XYLOG_FAILED_JUMP(pPanel);

	Lua_PushObject(L, pPanel);
	return 1;
Exit0:
	return 0;
}