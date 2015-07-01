
#include "FightFangKuai.h"
#include "cocos-ext.h"
#include "GameDef.h"
#include "ServerAgent.h"
#include "FightView.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace cocosbuilder;

IMPL_LUA_CLASS_BEGIN(XFightFangKuai)
	EXPORT_LUA_FLOAT(m_fFallSpeed)
	EXPORT_LUA_FLOAT_R(m_fCurFallHeight)
	EXPORT_LUA_FUNCTION(LuaChangeFangKuai)
	EXPORT_LUA_FUNCTION(LuaGetPostion)
	EXPORT_LUA_FUNCTION(LuaSetPostion)
	EXPORT_LUA_FUNCTION(LuaGetCellSize)
	EXPORT_LUA_FUNCTION(LuaMoveCell)
	EXPORT_LUA_FUNCTION(LuaRemove)
	EXPORT_LUA_FUNCTION(LuaMoveFullCell)
	EXPORT_LUA_FUNCTION(LuaSetVisible)
	EXPORT_LUA_FUNCTION(LuaGetVisible)
	EXPORT_LUA_FUNCTION(LuaGetUIFangKuai)
IMPL_LUA_CLASS_END()

XFightFangKuai::XFightFangKuai()
: m_pUIFangKuai(NULL)
, m_fFallSpeed(0.0f)
, m_CellSize(Size::ZERO)
, m_fCurFallHeight(0.0f)
, m_pFightView(NULL)
{
}

XFightFangKuai::~XFightFangKuai()
{
    m_pFightView = NULL;
    m_pUIFangKuai = NULL;
}

void XFightFangKuai::ChangeFangKuai(const char* pszCCBFile, const char* pszImageName, const char* pszImageFile)
{
	CCNode*				 pNode      = NULL;
	CCBAnimationManager* pManager   = NULL;
	CCBReader*			 pCCBReader = new CCBReader(NodeLoaderLibrary::getInstance());

	XYLOG_FAILED_JUMP(pCCBReader);

	pCCBReader->autorelease();

	if(m_pUIFangKuai)
	{
		removeChild(m_pUIFangKuai);
	}

	m_pUIFangKuai = (BasicPanel*)pCCBReader->readNodeGraphFromFile(pszCCBFile);
	XYLOG_FAILED_JUMP(m_pUIFangKuai);

	m_pUIFangKuai->setTouchEnabled(false);
	addChild(m_pUIFangKuai);
    
    //pNode must be not null, because m_CellSize must not null
    pNode = m_pUIFangKuai->m_mapMemberVariable[pszImageName];
    if(pNode)
    {
        const CCSize& NodeSize = pNode->getContentSize();
        m_CellSize = NodeSize;
        
        if(pszImageFile && strcmp(pszImageFile, "") != 0)
        {
            CCSprite*  pSprite  = dynamic_cast<CCSprite*>(pNode);
            if(pSprite)
            {
                pSprite->setTexture(pszImageFile);
                const CCSize& size = pSprite->getContentSize();
                pSprite->setScaleX(m_CellSize.width / size.width);
                pSprite->setScaleY(m_CellSize.height / size.height);
            }
            else
            {
                Scale9Sprite*  pS9Sprite = dynamic_cast<Scale9Sprite*>(pNode);
                if(pS9Sprite)
                {
                    pS9Sprite->initWithFile(pszImageFile);
                    pS9Sprite->setContentSize(m_CellSize);
                }
            }
        }
        
        pNode->setAnchorPoint(CCPoint(0,0));
    }

	pManager = dynamic_cast<CCBAnimationManager*>(m_pUIFangKuai->getUserObject());
	if(pManager)
	{
		pManager->setDelegate(this);
	}
Exit0:
	return;
}

void XFightFangKuai::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
	Node::visit(renderer, parentTransform, parentFlags);

	bool			bBombing	= m_pFightView->m_bBombing;
	if(m_fFallSpeed > 0.00001 && !bBombing)
	{
		float fY		  = getPositionY();
		float fX		  = getPositionX();
		float fFallHeight = m_CellSize.height / m_fFallSpeed;
		m_fCurFallHeight  = m_fCurFallHeight + fFallHeight;
		if(m_fCurFallHeight >= m_CellSize.height)
		{
			fFallHeight      = fFallHeight - (m_fCurFallHeight - m_CellSize.height);
			m_fCurFallHeight = 0.0f;
			OnFallFullCell();
		}

		fY = fY - fFallHeight;
		setPosition(fX, fY);
		setZOrder(fX + fY * 100000);
	}
}

void XFightFangKuai::DrawInRange()
{
	glEnable(GL_SCISSOR_TEST);
	//这里的坐标应该是可调的
	glScissor(0.0f, 0.0f, 1280.0f, 635.0f);

	CCNode::visit();

	glDisable(GL_SCISSOR_TEST);  
}

void XFightFangKuai::completedAnimationSequenceNamed(const char* pszName)
{
	OnCompletedAction(pszName);
}

void XFightFangKuai::OnFallFullCell()
{
	int					nRetCode	= 0;
	lua_State*          pLuaState	= g_pServerAgent->GetLuaState();
	XLuaSafeStack       luaSafeStack(pLuaState);

	XYLOG_FAILED_JUMP(m_pFightView);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnFallFullCell");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);
	Lua_PushObject(pLuaState, m_pFightView);

	nRetCode = Lua_XCall(luaSafeStack, 2, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void XFightFangKuai::OnCompletedAction(const char* pszName)
{
	int					nRetCode	= 0;
	lua_State*          pLuaState	= g_pServerAgent->GetLuaState();
	XLuaSafeStack       luaSafeStack(pLuaState);

	XYLOG_FAILED_JUMP(m_pFightView);

	nRetCode = Lua_GetObjFunction(pLuaState, this, "OnCompletedAction");
	XY_FAILED_JUMP(nRetCode);

	Lua_PushObject(pLuaState, this);
	Lua_PushObject(pLuaState, m_pFightView);
	lua_pushstring(pLuaState, pszName);

	nRetCode = Lua_XCall(luaSafeStack, 3, 0);
	XYLOG_FAILED_JUMP(nRetCode);
Exit0:
	return;
}

void XFightFangKuai::RemoveDelegate()
{
	CCBAnimationManager* pManager = dynamic_cast<CCBAnimationManager*>(m_pUIFangKuai->getUserObject());
	if(pManager)
	{
		pManager->setDelegate(NULL);
	}
}

int XFightFangKuai::LuaChangeFangKuai(lua_State* L)
{
	int				nTop			= lua_gettop(L);
	const char*		pszFile			= NULL;
	const char*     pszImgeFile     = NULL;
	const char*     pszImageName    = NULL;     

	XYLOG_FAILED_JUMP(nTop >= 1);

	pszFile        = lua_tostring(L, 1);
	pszImageName   = lua_tostring(L, 2);
	pszImgeFile    = lua_tostring(L, 3);
	ChangeFangKuai(pszFile, pszImageName, pszImgeFile);
Exit0:
	return 0;
}

int XFightFangKuai::LuaSetPostion(lua_State* L)
{
	int    nTop		= lua_gettop(L);
	float  fPosX	= 0;
	float  fPosY	= 0;

	XYLOG_FAILED_JUMP(nTop == 2);

	fPosX = lua_tonumber(L, 1);
	fPosY = lua_tonumber(L, 2);
	setPosition(fPosX, fPosY);
Exit0:
	return 0;
}

int XFightFangKuai::LuaGetPostion(lua_State* L)
{
	float fX = getPositionX();
	float fY = getPositionY();
	lua_pushnumber(L, fX);
	lua_pushnumber(L, fY);
	return 2;
}

int XFightFangKuai::LuaGetCellSize(lua_State* L)
{
	lua_pushnumber(L, m_CellSize.width);
	lua_pushnumber(L, m_CellSize.height);
	return 2;
}

int XFightFangKuai::LuaRemove(lua_State* L)
{
	XYLOG_FAILED_JUMP(m_pFightView);

	m_fFallSpeed = 0.0f;
	m_pUIFangKuai->cleanup();
	RemoveDelegate();
    this->removeFromParentAndCleanup(true);
	//m_pFightView->m_MapRemoveFangKuai.push_back(this);
Exit0:	
	return 0;
}

int XFightFangKuai::LuaMoveCell(lua_State* L)
{
	int		nTop			= lua_gettop(L);
	int		nMoveCellX		= 0;
	int     nMoveCellY      = 0;
	float   fX              = 0.0;
	float   fY              = 0.0;

	XYLOG_FAILED_JUMP(nTop == 2);

	nMoveCellX = lua_tointeger(L, 1);
	nMoveCellY = lua_tointeger(L, 2);
	fY		   = getPositionY();
	fX		   = getPositionX();
	fY         = fY + nMoveCellY * m_CellSize.height;
	fX         = fX + nMoveCellX * m_CellSize.width;
	setPosition(fX, fY);
Exit0:
	return 0;
}

int XFightFangKuai::LuaMoveFullCell(lua_State* L)
{
	int		nTop			= lua_gettop(L);
	int		nMoveCellX		= 0;
	int     nMoveCellY      = 0;
	float   fX              = 0.0;
	float   fY              = 0.0;

	XYLOG_FAILED_JUMP(nTop == 2);

	nMoveCellX = lua_tointeger(L, 1);
	nMoveCellY = lua_tointeger(L, 2);
	fY		   = getPositionY();
	fX		   = getPositionX();

	if(m_fCurFallHeight > 0.0001)
	{
		if(nMoveCellY < 0)
		{
			fY = fY - (m_CellSize.height - m_fCurFallHeight);
			++nMoveCellY;
		}
		else if(nMoveCellY > 0)
		{
			fY = fY + m_fCurFallHeight;
			--nMoveCellY;
		}

		m_fCurFallHeight = 0.0f;
	}

	fY         = fY + nMoveCellY * m_CellSize.height;
	fX         = fX + nMoveCellX * m_CellSize.width;
	setPosition(fX, fY);
Exit0:
	return 0;
}

int XFightFangKuai::LuaSetVisible(lua_State* L)
{
	int  nTop		= lua_gettop(L);
	int  nVisible   = 0;

	XYLOG_FAILED_JUMP(nTop == 1 && m_pUIFangKuai);

	nVisible = lua_tointeger(L, 1);
	m_pUIFangKuai->setVisible((bool)nVisible);
Exit0:
	return 0;
}

int XFightFangKuai::LuaGetVisible(lua_State* L)
{
	lua_pushnumber(L, (int)m_pUIFangKuai->isVisible());
	return 1;
}

int XFightFangKuai::LuaGetUIFangKuai(lua_State* L)
{
	int  nResult = 0;
	XYLOG_FAILED_JUMP(m_pUIFangKuai);

	Lua_PushObject(L, m_pUIFangKuai);
	nResult = 1;
Exit0:
	return nResult;
}