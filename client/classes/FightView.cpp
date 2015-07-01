
#include "FightView.h"
#include "ServerAgent.h"
#include "FightFangKuai.h"
#include "UIManager.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace cocosbuilder;

IMPL_LUA_CLASS_BEGIN(XFightView)
	EXPORT_LUA_FUNCTION(LuaAddToNode)
	EXPORT_LUA_FUNCTION(LuaSetup)
	EXPORT_LUA_FUNCTION(LuaCreateFangKuai)
	EXPORT_LUA_FUNCTION(LuaSetBoard)
	EXPORT_LUA_FUNCTION(LuaGetFangKuai)
	EXPORT_LUA_FUNCTION(LuaSetBombing)
	EXPORT_LUA_FUNCTION(LuaSetFKParentScale)
IMPL_LUA_CLASS_END()

XFightView::XFightView()
{
	m_nCurFangKuaiTag	 = 0;
	m_pBoard			 = NULL;
	m_pFangKuaiParent    = NULL;
	m_bBombing			 = false;

	this->scheduleUpdate();
}

XFightView::~XFightView()
{
	m_pBoard = NULL;
    m_MapRemoveFangKuai.clear();

	this->unscheduleUpdate();
	if (isTouchEnabled())
	{
		Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(this);
	}
}

void XFightView::Setup(const char* pszBoardName, const char* pszStartName, bool bOpenTouches)
{
	SetBoard(pszBoardName, pszStartName);
	setTouchEnabled(bOpenTouches);
	if (bOpenTouches)
	{
		SetTouchCallback();
	}
}

void XFightView::visit(Renderer *renderer, const Mat4& parentTransform, bool parentTransformUpdated)
{
	Layer::visit(renderer, parentTransform, parentTransformUpdated);
	OnActivateEnd();
}

void XFightView::update(float dt)
{
	OnActivateEnd();
}

void XFightView::CallActivateEnd()
{
	lua_State*          pLuaState = g_pServerAgent->GetLuaState();
	int                 nRetCode  = 0;
	XLuaSafeStack       luaSafeStack(pLuaState);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnAllFangKuaiFallEnd");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);

	nRetCode = Lua_XCall(luaSafeStack, 1, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	return;
}

void XFightView::OnActivateEnd()
{
	CallActivateEnd();

	XMapRemoveFangKuai::iterator  itRemove = m_MapRemoveFangKuai.begin();
	for(; itRemove != m_MapRemoveFangKuai.end(); ++itRemove)
	{
		XFightFangKuai* pFangKuai = *itRemove;
		pFangKuai->cleanup();
		pFangKuai->removeFromParent();
	}

	m_MapRemoveFangKuai.clear();
}

void XFightView::SetBoard(const char* pszBoardName, const char* pszStartName)
{
	CCBReader* pCCBReader = new CCBReader(NodeLoaderLibrary::getInstance());
	CCNode*    pNode      = NULL;

	XYLOG_FAILED_JUMP(pCCBReader);

	pCCBReader->autorelease();

	if(m_pBoard)
	{
		removeChild(m_pBoard);
	}

	if(m_pFangKuaiParent)
	{
		removeChild(m_pFangKuaiParent);
	}

	m_pBoard = g_UIManager->GetInstanceByRootName(pszBoardName);//(BasicPanel*)pCCBReader->readNodeGraphFromFile(pszCCBFile);
	XYLOG_FAILED_JUMP(m_pBoard);


	m_pFangKuaiParent = m_pBoard->m_mapMemberVariable[pszStartName];
Exit0:
	return;
}

bool XFightView::onTouchBegan(CCTouch* pTouch, CCEvent* pEvent)
{
	lua_State*          pLuaState = g_pServerAgent->GetLuaState();
	int                 nRetCode  = 0;
	CCPoint             location  = pTouch->getLocation();
	XLuaSafeStack       luaSafeStack(pLuaState);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnTouchBegan");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);
	lua_pushnumber(pLuaState, location.x);
	lua_pushnumber(pLuaState, location.y);

	nRetCode = Lua_XCall(luaSafeStack, 3, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	return true;
}

void XFightView::onTouchMoved(CCTouch* pTouch, CCEvent* pEvent)
{
	lua_State*          pLuaState = g_pServerAgent->GetLuaState();
	int                 nRetCode  = 0;
	CCPoint             location  = pTouch->getLocation();
	XLuaSafeStack       luaSafeStack(pLuaState);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnTouchMoved");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);
	lua_pushnumber(pLuaState, location.x);
	lua_pushnumber(pLuaState, location.y);

	nRetCode = Lua_XCall(luaSafeStack, 3, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	return;
}

void XFightView::onTouchEnded(CCTouch* pTouch, CCEvent* pEvent)
{
	lua_State*          pLuaState = g_pServerAgent->GetLuaState();
	int                 nRetCode  = 0;
	CCPoint             location  = pTouch->getLocation();
	XLuaSafeStack       luaSafeStack(pLuaState);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnTouchEnded");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);
	lua_pushnumber(pLuaState, location.x);
	lua_pushnumber(pLuaState, location.y);

	nRetCode = Lua_XCall(luaSafeStack, 3, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	return;
}

void XFightView::onTouchCancelled(CCTouch* pTouch, CCEvent* pEvent)
{
	onTouchEnded(pTouch, pEvent);
}

XFightFangKuai* XFightView::CreateFangKuai()
{
	XFightFangKuai* pFangKuai = XFightFangKuai::create();
	CCNode*         pNode     = NULL;

	XYLOG_FAILED_JUMP(pFangKuai && m_pFangKuaiParent);

	m_nCurFangKuaiTag++;
	pFangKuai->setTag(m_nCurFangKuaiTag);
	pFangKuai->m_pFightView = this;
	m_pFangKuaiParent->addChild(pFangKuai);
Exit0:
	return pFangKuai;
}

int XFightView::LuaCreateFangKuai(lua_State* L)
{
	int             nResult		 = 0;
	int             nCellX		 = 0;
	int             nCellY		 = 0;
	float           fX			 = 0;
	float           fY			 = 0;
	float           fFallSpeed   = 0.0f;
	const char*     pszFile		 = NULL;
	const char*     pszImgeFile  = NULL;
	const char*     pszImageName = NULL;
	int             nTop		 = lua_gettop(L);

	XFightFangKuai* pFangKuai = CreateFangKuai();
	XY_FAILED_JUMP(pFangKuai && nTop >= 3);

	pszFile        = lua_tostring(L, 1);
	pszImageName   = lua_tostring(L, 2);
	pszImgeFile    = lua_tostring(L, 3);
	pFangKuai->ChangeFangKuai(pszFile, pszImageName, pszImgeFile);

	if(nTop >= 4)
	{
		fFallSpeed = lua_tonumber(L, 4);
		pFangKuai->m_fFallSpeed = fFallSpeed;
	}

	if(nTop >= 5)
	{
		nCellX	   = lua_tonumber(L, 5);
		nCellY	   = lua_tonumber(L, 6);
		fX		   = nCellX * pFangKuai->m_CellSize.width;
		fY		   = nCellY * pFangKuai->m_CellSize.height;
		pFangKuai->setPosition(fX, fY);
	}

	Lua_PushObject(L, pFangKuai);
	nResult = 1;
Exit0:
	return nResult;
}

int XFightView::LuaSetBoard(lua_State* L)
{
	int			  nTop			= lua_gettop(L);
	const char*   pszFile		= NULL;
   const char*    pszStartName	= NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszFile      = lua_tostring(L, 1);
	pszStartName = lua_tostring(L, 2);

	SetBoard(pszFile, pszStartName);
Exit0:
	return 0;
}

int XFightView::LuaGetFangKuai(lua_State* L)
{
	int					nTop		= lua_gettop(L);
	int					nTag		= 0;
	int                 nResult     = 0;
	XFightFangKuai*		pFangKuai	= NULL;

	XYLOG_FAILED_JUMP(nTop == 1 && m_pFangKuaiParent);

	nTag = lua_tonumber(L, 1);

	pFangKuai = (XFightFangKuai*)m_pFangKuaiParent->getChildByTag(nTag);
	XY_FAILED_JUMP(pFangKuai);

	Lua_PushObject(L, pFangKuai);
	nResult = 1;
Exit0:
	return nResult;
}

int XFightView::LuaAddToNode(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszInstanceName	= NULL;
	const char*		pszNodeName		= NULL;
	BasicPanel*		pInstance		= NULL;
	CCNode*			pNode			= NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszInstanceName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszInstanceName);

	pInstance = g_UIManager->GetInstanceByRootName(pszInstanceName);
	XYLOG_FAILED_JUMP(pInstance);

	pszNodeName = lua_tostring(L, 2);
	XYLOG_FAILED_JUMP(pszNodeName);

	pNode = pInstance->m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pNode);

	pNode->addChild(this);
Exit0:
	return 0;
}

int XFightView::LuaSetup(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszBoardName	= NULL;
	const char*     pszStartName	= NULL;
	bool            bOpenTouch		= false;

	XYLOG_FAILED_JUMP(nTop >= 2);

	pszBoardName = lua_tostring(L, 1);
	pszStartName = lua_tostring(L, 2);
	bOpenTouch	 = nTop >= 3 ? lua_tointeger(L, 3) : false;

	Setup(pszBoardName, pszStartName, bOpenTouch);
Exit0:
	return 0;
}

int XFightView::LuaSetBombing(lua_State* L)
{
	m_bBombing = lua_toboolean(L, 1);
	return 0;
}

int XFightView::LuaSetFKParentScale(lua_State* L)
{
	int		nTop = lua_gettop(L);
	float	fX   = 0.0f;
	float   fY   = 0.0f;

	XYLOG_FAILED_JUMP(nTop == 2 && m_pFangKuaiParent);

	fX = lua_tonumber(L, 1);
	fY = lua_tonumber(L, 2);
	m_pFangKuaiParent->setScale(fX, fY);
Exit0:
	return 0;
}

void XFightView::SetTouchCallback()
{
	auto listen = EventListenerTouchOneByOne::create();
	listen->setSwallowTouches(true); 
	listen->onTouchBegan		= CC_CALLBACK_2(XFightView::onTouchBegan, this);
	listen->onTouchMoved		= CC_CALLBACK_2(XFightView::onTouchMoved, this);
	listen->onTouchEnded		= CC_CALLBACK_2(XFightView::onTouchEnded, this);
	listen->onTouchCancelled	= CC_CALLBACK_2(XFightView::onTouchEnded, this);

	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listen, this);
}