#include "BasicPanel.h"
#include "TableView.h"
#include "UIManager.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace cocosbuilder;

IMPL_LUA_CLASS_BEGIN(BasicPanel)
	EXPORT_LUA_FUNCTION(LuaSetSizeByName)
	EXPORT_LUA_FUNCTION(LuaGetSizeByName)
	EXPORT_LUA_FUNCTION(LuaGetSelfSize)
	EXPORT_LUA_FUNCTION(LuaSetScaleByName)
	EXPORT_LUA_FUNCTION(LuaSetPositionByName)
	EXPORT_LUA_FUNCTION(LuaGetPositionByName)
	EXPORT_LUA_FUNCTION(LuaSetImageByName)
	EXPORT_LUA_FUNCTION(LuaSetScale9ImageByName)
	EXPORT_LUA_FUNCTION(LuaSetAnchorPointByName)
	EXPORT_LUA_FUNCTION(LuaPlayCCBAction)
	EXPORT_LUA_FUNCTION(LuaSetVisibleByName)
	EXPORT_LUA_FUNCTION(LuaSetLabelTextByName)
	EXPORT_LUA_FUNCTION(LuaActionEaseSineIn)
	EXPORT_LUA_FUNCTION(LuaSetRotation)
	EXPORT_LUA_FUNCTION(LuaSetScrollOffSet)
    EXPORT_LUA_FUNCTION(LuaRemoveAnimationDelegate)
	EXPORT_LUA_FUNCTION(LuaSetOrder)
	EXPORT_LUA_FUNCTION(LuaGetOrder)
IMPL_LUA_CLASS_END()

BasicPanel::BasicPanel()
{
}

BasicPanel::~BasicPanel()
{
	/*	因为这个map里面有些变量并不是通过回调加入，所以此处不能这样写，但还是需要有方法释放
	MAP_NODE::iterator it;
	for (it = m_mapMemberVariable.begin(); it != m_mapMemberVariable.end(); it++)
	{
		CC_SAFE_RELEASE(it->second);
	}
	*/
	m_mapMemberVariable.clear();
//	m_mapMenuItem.clear();
//	m_mapButtonRootName.clear();
}

SEL_MenuHandler BasicPanel::onResolveCCBCCMenuItemSelector(Ref* pTarget, const char* pszSelectorName)
{
	XY_FAILED_JUMP(pTarget == this && strcmp(pszSelectorName, "") != 0);

	return menu_selector(BasicPanel::OnMenuItemClicked);
Exit0:
	return NULL;
}

//意味着所有有回调的按钮都是使用OnButtonClicked，在里面细分那个按钮触发事件
Control::Handler BasicPanel::onResolveCCBCCControlSelector(Ref* pTarget, const char* pszSelectorName)
{
	XY_FAILED_JUMP(pTarget == this && strcmp(pszSelectorName, "") != 0);

	return cccontrol_selector(BasicPanel::OnButtonClicked);
Exit0:
	return NULL;
}

SEL_CallFuncN BasicPanel::onResolveCCBCCCallFuncSelector(Ref* pTarget, const char* pszSelectorName)
{
	XY_FAILED_JUMP(pTarget == this && strcmp(pszSelectorName, "") != 0);

	m_pszCurrentAni = pszSelectorName;

	return callfuncN_selector(BasicPanel::OnFuncSelector);
Exit0:
	return NULL;
}

/*去除字符串首尾空格*/
int Trim(const char* pszInBuf, char* pszOutBuf)
{
    int nRet = 0;
    if (pszInBuf == NULL || pszOutBuf == NULL)
    {
        return -1;
    }
    const char *pszTempBuf = pszInBuf;
    int i = 0;
    int j = strlen(pszTempBuf) - 1;
    while (isspace(pszTempBuf[i]) && pszTempBuf[i] != '\0')
    {
        i++;
    }
    while (isspace(pszTempBuf[j]) && pszTempBuf[j] >=0)
    {
        j--;
    }
    int nSize = j - i + 1;
    memcpy(pszOutBuf, pszTempBuf + i, nSize);
    pszOutBuf[nSize] = '\0';
    return nRet;
}

bool BasicPanel::onAssignCCBMemberVariable(Ref* pTarget, const char* pszMemberVariableName, Node* pNode)
{
	BasicPanel*		pPanel					= NULL;
	char			szVariableName[32]		= {0};

	XYLOG_FAILED_JUMP(strcmp(pszMemberVariableName, ""));
    Trim(pszMemberVariableName, szVariableName);

	m_mapMemberVariable[szVariableName] = pNode;
	pPanel = dynamic_cast<BasicPanel*>(pNode);
	if (pPanel)
	{
		pPanel->m_pszRootName = pszMemberVariableName;
		return true;
	}

	if (dynamic_cast<ControlButton*>(pNode))
	{
		m_mapButtonRootName[pNode] = pszMemberVariableName;
		return true;
	}

	if (dynamic_cast<MenuItem*>(pNode))
	{
		m_mapMenuItem[pNode] = pszMemberVariableName;
		return true;
	}
	
	return true;
Exit0:
	return false;
}

void BasicPanel::onNodeLoaded(Node* pNode, NodeLoader* pNodeLoader)
{
	MAP_NODE::iterator it;
	for (it = m_mapMemberVariable.begin(); it != m_mapMemberVariable.end(); ++it)
	{
		if (it->second->getParent() == NULL)
		{
			m_pszSelfType = it->first;	
			m_mapMemberVariable.erase(it);
			break;
		}
	}

	this->setTouchMode(Touch::DispatchMode::ONE_BY_ONE);
}

bool BasicPanel::onTouchBegan(Touch* pTouch, CCEvent* pEvent)
{
	log("ontouchbegan:%s", m_pszRootName.c_str());
	return g_UIManager->UITouchBegan(m_pszRootName.c_str());
}

void BasicPanel::onEnter()
{
	g_UIManager->UIOnEnter(m_pszRootName.c_str());
	CCLayer::onEnter();
}

void BasicPanel::onExit()
{
	g_UIManager->UIOnExit(m_pszRootName.c_str());
	CCLayer::onExit();
}

void BasicPanel::completedAnimationSequenceNamed(const char *pszName)
{
	g_UIManager->UIOnCompletedAnimationSequenceNamed(m_pszRootName.c_str(), pszName);
}

void BasicPanel::OnButtonClicked(Ref* pObject, Control::EventType eEvent)
{
	g_UIManager->UIButtonClicked(m_pszRootName.c_str(), m_mapButtonRootName[pObject].c_str());
}


void BasicPanel::OnFuncSelector(Node* pNode)
{
	XYLOG_FAILED_JUMP(pNode == this);

	g_UIManager->UIOnFuncSelector(m_pszRootName.c_str(), m_pszCurrentAni.c_str());

Exit0:
	return;
}

void BasicPanel::OnMenuItemClicked(Ref* pObject)
{
	g_UIManager->UIMenuItemClicked(m_pszRootName.c_str(), m_mapMenuItem[pObject].c_str());
}



void BasicPanel::OnActionCompleted()
{

}

void BasicPanel::SetAnimationDelegate()
{
	CCBAnimationManager* pManager	= dynamic_cast<CCBAnimationManager*>(this->getUserObject());
	pManager->setDelegate(this);
}

void BasicPanel::PlayCCBAction(const char* pszActionName, float nDelayTime)
{
	CCBAnimationManager* pManager	= dynamic_cast<CCBAnimationManager*>(this->getUserObject());
	XY_FAILED_JUMP(pManager);

	pManager->runAnimationsForSequenceNamedTweenDuration(pszActionName, nDelayTime);
Exit0:
	return;
}

void BasicPanel::PrintAllCCBFileName()
{
	MAP_NODE::iterator it;
	for (it = m_mapMemberVariable.begin(); it != m_mapMemberVariable.end(); ++it)
	{
		bool isCCBFileNode = (NULL == dynamic_cast<BasicPanel*>(it->second)) ? false : true;
		if (isCCBFileNode)
		{
			CCLOG("%s",it->first.c_str());
		}
		else
		{
			CCLOG("not ccb %s", it->first.c_str());
		}
	}
}

int BasicPanel::LuaSetSizeByName(lua_State* L)
{
	int				nTop    = lua_gettop(L);
	const char*		pszName = NULL;
	float			fHeight = 0.0;
	float			fWidth  = 0.0;
	Node*			pNode   = NULL;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszName = lua_tostring(L, 1);

	pNode   = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	fWidth  = lua_tonumber(L, 2); 
	fHeight = lua_tonumber(L, 3);
	pNode->setContentSize(CCSize(fWidth, fHeight));
Exit0:
	return 0;
}

int BasicPanel::LuaGetSizeByName(lua_State* L)
{
	int				nTop    = lua_gettop(L);
	const char*		pszName = NULL;
	Node*			pNode   = NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszName = lua_tostring(L, 1);

	pNode   = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	lua_pushnumber(L, pNode->getContentSize().width);
	lua_pushnumber(L, pNode->getContentSize().height);
Exit0:
	return 2;
}

int BasicPanel::LuaGetSelfSize(lua_State* L)
{
	int				nTop    = lua_gettop(L);

	XYLOG_FAILED_JUMP(nTop == 0);

	lua_pushnumber(L, this->getContentSize().width);
	lua_pushnumber(L, this->getContentSize().height);
Exit0:
	return 2;
}

int BasicPanel::LuaGetPositionByName(lua_State* L)
{
	int				nTop     = lua_gettop(L);
	float			fX       = 0.0;
	float           fY       = 0.0;
	int             nResult  = 0;
	Node*			pNode    = NULL;
	const char*     pszName  = NULL;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszName);

	pNode = m_mapMemberVariable[pszName];
	XY_FAILED_JUMP(pNode);

	pNode->getPosition(&fX, &fY);
	lua_pushnumber(L, fX);
	lua_pushnumber(L, fY);
	nResult = 2;
Exit0:
	return nResult;
}

int BasicPanel::LuaSetPositionByName(lua_State* L)
{
	int         nTop			= lua_gettop(L);
	Node*     pNode           = this;
	const char* pszName	        = NULL;
	float       fX              = 0.0f;
	float       fY              = 0.0f;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszName  = lua_tostring(L, 1);
	if (pszName)
	{
		pNode = m_mapMemberVariable[pszName];
		XYLOG_FAILED_JUMP(pNode);
	}

	fX       = lua_tonumber(L, 2);
	fY       = lua_tonumber(L, 3);
	pNode->setPosition(fX, fY);
Exit0:
	return 0;
}

int BasicPanel::LuaSetScaleByName(lua_State* L)
{
	int			nTop    = lua_gettop(L);
	const char* pszName = NULL;
	Node*		pNode   = NULL;
	float		fX	    = 0.0;
	float		fY      = 0.0;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszName = lua_tostring(L, 1);
	pNode   = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	fX  = lua_tonumber(L, 2); 
	fY  = lua_tonumber(L, 3);
	pNode->setScaleX(fX);
	pNode->setScaleY(fY);
Exit0:
	return 0;
}

int BasicPanel::LuaSetImageByName(lua_State* L)
{
	int					nTop		= lua_gettop(L);
	const char*			pszName		= NULL;
	const char*			pszFile     = NULL;
	Node*				pNode       = NULL;
	Sprite*				pSprite     = NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszName = lua_tostring(L, 1);

	pNode = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	pSprite = dynamic_cast<Sprite*>(pNode);
	XYLOG_FAILED_JUMP(pSprite);

	pszFile = lua_tostring(L, 2);
	XYLOG_FAILED_JUMP(pszFile);

	pSprite->setTexture(pszFile);
Exit0:
	return 0;
}

int BasicPanel::LuaSetScale9ImageByName(lua_State* L)
{
	int					nTop		= lua_gettop(L);
	const char*			pszName		= NULL;
	const char*			pszFile     = NULL;
	Node*				pNode       = NULL;
	Scale9Sprite*	    pS9Sprite   = NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszName = lua_tostring(L, 1);

	pNode = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	pS9Sprite = dynamic_cast<Scale9Sprite*>(pNode);
	XYLOG_FAILED_JUMP(pS9Sprite);

	pszFile = lua_tostring(L, 2);
	pS9Sprite->initWithFile(pszFile);
Exit0:
	return 0;
}

int BasicPanel::LuaSetAnchorPointByName(lua_State* L)
{
	int				nTop	 = lua_gettop(L);
	float           fX       = 0.0f;
	float           fY       = 0.0f;
	const char*		pszName  = NULL;
	Node*			pNode    = NULL;

	XYLOG_FAILED_JUMP(nTop == 3);

	pszName = lua_tostring(L, 1);
	pNode   = m_mapMemberVariable[pszName];
	XYLOG_FAILED_JUMP(pNode);

	fX  = lua_tonumber(L, 2);
	fY  = lua_tonumber(L, 3);

	pNode->setAnchorPoint(CCPoint(fX, fY));
Exit0:
	return 0;
}

int BasicPanel::LuaPlayCCBAction(lua_State* L)
{
	const char*		pszActionName	= lua_tostring(L, 1);
	float			fDelay			= lua_tonumber(L, 2);

	PlayCCBAction(pszActionName, fDelay);
	return 0;
}

int BasicPanel::LuaSetVisibleByName(lua_State* L)
{
	int				nTop		= lua_gettop(L);
	const char*		pszNodeName = lua_tostring(L, 1);
	bool			bVisible	= lua_toboolean(L, 2);
	Node*			pNode;

	XYLOG_FAILED_JUMP(nTop == 2);

	pNode = m_mapMemberVariable[pszNodeName];
	XYLOG_FAILED_JUMP(pNode);

	pNode->setVisible(bVisible);
Exit0:
	return 0;
}

int BasicPanel::LuaSetLabelTextByName(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszNodeName		= NULL;
	const char*		pszText			= NULL;
	CCLabelTTF*		pLabel			= NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszNodeName	= lua_tostring(L, 1);
	pszText		= lua_tostring(L, 2);

	pLabel = dynamic_cast<CCLabelTTF*>(m_mapMemberVariable[pszNodeName]);
	XYLOG_FAILED_JUMP(pLabel);

	pLabel->setString(pszText);
Exit0:
	return 0;
}

int BasicPanel::LuaActionEaseSineIn(lua_State* L)
{
	int					nTop		= lua_gettop(L);
	float				fTime		= 0.0f;
	float				fEndPointX  = 0.0f;
	float				fEndPointY  = 0.0f;
	CCActionInterval*	pMove		= NULL;
	CCActionInterval*	pEaseSineI	= NULL;

	XYLOG_FAILED_JUMP(nTop == 3);

	fTime		= lua_tonumber(L, 1);
	fEndPointX	= lua_tonumber(L, 2);
	fEndPointY	= lua_tonumber(L, 3);

	pMove		= CCMoveTo::create(fTime, ccp(fEndPointX, fEndPointY));
	pEaseSineI	= CCEaseSineIn::create(pMove);
	//CCCallFunc* funcall= CCCallFunc::create(this, callfunc_selector(BasicPanel::OnActionCompleted));
	//CCFiniteTimeAction * seq = CCSequence::create(easeSineIn,funcall,NULL);
	this->runAction(pEaseSineI);
	return 0;
Exit0:
	return 0;
}

int BasicPanel::LuaSetRotation(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	float		fAngle;
	const char* pszNodeName = NULL;
	Node*		pNode		= this;

	XYLOG_FAILED_JUMP(nTop >= 1);

	fAngle = lua_tonumber(L, 1);

	pszNodeName = lua_tostring(L, 2);
	if (pszNodeName)
	{
		pNode = m_mapMemberVariable[pszNodeName];
	}

	pNode->setRotation(fAngle);
Exit0:
	return 0;
}

int BasicPanel::LuaSetScrollOffSet(lua_State* L)
{
	const int		nTop		= lua_gettop(L);
	const char*		pszScroll	= NULL;
	float			fOffX		= 0.0f;
	float			fOffY		= 0.0f;
	BOOL			bAnimated	= 0;
	ScrollView*		pScroll		= NULL;

	XYLOG_FAILED_JUMP(nTop == 4);
	
	pszScroll	 = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszScroll);

	pScroll		 = dynamic_cast<ScrollView*>(m_mapMemberVariable[pszScroll]);
	XYLOG_FAILED_JUMP(pScroll);

	fOffX		 = lua_tointeger(L, 2);
	fOffY		 = lua_tointeger(L, 3);
	bAnimated    = lua_tointeger(L, 4);

	pScroll->setContentOffset(ccp(fOffX, fOffY), bAnimated);
Exit0:
	return 0;
}

int BasicPanel::LuaRemoveAnimationDelegate(lua_State *L)
{
    CCBAnimationManager* pManager = NULL;
    
    XYLOG_FAILED_JUMP(lua_gettop(L) == 0);
    
    pManager = dynamic_cast<CCBAnimationManager*>(this->getUserObject());
    pManager->setDelegate(NULL);
Exit0:
    return 0;
}

/*
注意：Order 使用的前提是节点的父节点相同。ArrivalOfOrder 在 Order 相同的情况下使用，如果 Order 不相同，那么修改 Order 即可
*/
int BasicPanel::LuaSetOrder(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	int			nOrder		= 0;
	const char* pszNode		= NULL;
	Node*		pNode		= NULL;

	XYLOG_FAILED_JUMP(nTop == 2);

	pszNode = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszNode && pszNode[0] != '\0');

	pNode = m_mapMemberVariable[pszNode];
	XYLOG_FAILED_JUMP(pNode);

	nOrder = lua_tointeger(L, 2);
	pNode->setZOrder(nOrder);
Exit0:
	return 0;
}

int BasicPanel::LuaGetOrder(lua_State* L)
{
	int			nTop		= lua_gettop(L);
	const char* pszNode		= NULL;
	Node*		pNode		= NULL;
	int			nOrder		= 0;

	XYLOG_FAILED_JUMP(nTop == 1);

	pszNode		= lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszNode && pszNode[0] != '\0');

	pNode		= m_mapMemberVariable[pszNode];
	XYLOG_FAILED_JUMP(pNode);
	
	nOrder		= pNode->getZOrder();
	lua_pushnumber(L, nOrder);

	return 1;
Exit0:
	return 0;
}