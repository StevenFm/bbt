#include "stdafx.h"
#include "TableView.h"
#include "BasicPanel.h"
#include "ServerAgent.h"

using namespace std;
USING_NS_CC;
USING_NS_CC_EXT;

IMPL_LUA_CLASS_BEGIN(XTableView)    
    EXPORT_LUA_BIG_BOOL(m_bLoadCompleted)

    EXPORT_LUA_FUNCTION(LuaGetContainer)
    EXPORT_LUA_FUNCTION(LuaSetVerticalFillOrder)
    EXPORT_LUA_FUNCTION(LuaGetVerticalFillOrder)

    EXPORT_LUA_FUNCTION(LuaInsertCellAtIndex)
    EXPORT_LUA_FUNCTION(LuaUpdateCellAtIndex)
    EXPORT_LUA_FUNCTION(LuaRemoveCellAtIndex)

    EXPORT_LUA_FUNCTION(LuaGetViewSize)
	EXPORT_LUA_FUNCTION(LuaSetViewSize)
    EXPORT_LUA_FUNCTION(LuaGetContentSize)
    EXPORT_LUA_FUNCTION(LuaSetContentSize)
    EXPORT_LUA_FUNCTION(LuaGetContentOffset)
    EXPORT_LUA_FUNCTION(LuaSetContentOffset)
    EXPORT_LUA_FUNCTION(LuaReloadData)

    EXPORT_LUA_FUNCTION(LuaGetOffsetFromIndex)
    EXPORT_LUA_FUNCTION(LuaRefresh)
    EXPORT_LUA_FUNCTION(LuaGetRootPanel)
IMPL_LUA_CLASS_END()

BasicPanel* GetTableViewCell(TableViewCell* pTableViewCell)
{
    BasicPanel*		pResult		= NULL;
    Vector<Node*>	pChildren	= pTableViewCell->getChildren();

    if (pChildren.size() == 1)
        pResult = dynamic_cast<BasicPanel*>(pChildren.at(0));//objectAtIndex(0));
    return pResult;
}

class XTableViewDelegate : public TableViewDelegate
{
public:
    virtual void scrollViewDidScroll(ScrollView* pScrollView)
    {

    }

    virtual void scrollViewDidZoom(ScrollView* pScrollView)
    {

    }

    virtual void tableCellTouched(TableView* pTableView, TableViewCell* pTableViewCell)
    {
        BOOL bRetCode = false;
        XTableView* pCurTableView = dynamic_cast<XTableView*>(pTableView);
        BasicPanel* pCurPanel = GetTableViewCell(pTableViewCell);
        XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());

        XYLOG_FAILED_JUMP(pCurTableView);
        XYLOG_FAILED_JUMP(pCurPanel);

        bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), pCurTableView, "OnTableCellTouched");
        XY_FAILED_JUMP(bRetCode);

        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurTableView);
        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurPanel);

        bRetCode = Lua_XCall(luaSafeStack, 2, 0);
        XYLOG_FAILED_JUMP(bRetCode);

Exit0:
        return ;
    }

    virtual void tableCellHighlight(TableView* pTableView, TableViewCell* pTableViewCell)
    {
        BOOL bRetCode = false;
        XTableView* pCurTableView = dynamic_cast<XTableView*>(pTableView);
        BasicPanel* pCurPanel = GetTableViewCell(pTableViewCell);
        XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());

        XYLOG_FAILED_JUMP(pCurTableView);
        XYLOG_FAILED_JUMP(pCurPanel);

        bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), pCurTableView, "OnTableCellHighlight");
        XY_FAILED_JUMP(bRetCode);

        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurTableView);
		Lua_PushObject(g_pServerAgent->GetLuaState(), pCurPanel);

        bRetCode = Lua_XCall(luaSafeStack, 2, 0);
        XYLOG_FAILED_JUMP(bRetCode);

Exit0:
        return ;
    }

    virtual void tableCellUnhighlight(TableView* pTableView, TableViewCell* pTableViewCell)
    {
        BOOL bRetCode = false;
        XTableView* pCurTableView = dynamic_cast<XTableView*>(pTableView);
        BasicPanel* pCurPanel = GetTableViewCell(pTableViewCell);
        XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());

        XYLOG_FAILED_JUMP(pCurTableView);
        XYLOG_FAILED_JUMP(pCurPanel);

        bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), pCurTableView, "OnTableCellUnhighlight");
        XY_FAILED_JUMP(bRetCode);

        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurTableView);
		Lua_PushObject(g_pServerAgent->GetLuaState(), pCurPanel);

        bRetCode = Lua_XCall(luaSafeStack, 2, 0);
        XYLOG_FAILED_JUMP(bRetCode);

Exit0:
        return ;
    }

    virtual void tableCellWillRecycle(TableView* pTableView, TableViewCell* pTableViewCell)
    {
        BOOL bRetCode = false;
        XTableView* pCurTableView = dynamic_cast<XTableView*>(pTableView);
        BasicPanel* pCurPanel = GetTableViewCell(pTableViewCell);
        XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());

        XYLOG_FAILED_JUMP(pCurTableView);
        XYLOG_FAILED_JUMP(pCurPanel);

        bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), pCurTableView, "OnTableCellWillRecycle");
        XY_FAILED_JUMP(bRetCode);

        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurTableView);
        Lua_PushObject(g_pServerAgent->GetLuaState(), pCurPanel);
        bRetCode = Lua_XCall(luaSafeStack, 2, 0);
        XYLOG_FAILED_JUMP(bRetCode);

Exit0:
        return ;
    }
};

XTableViewDelegate g_TableViewDelegate;

XTableView::XTableView()
{
    m_pVerticalScrollBar = NULL;
    m_bLoadCompleted = false;
}

XTableView::~XTableView()
{
    CC_SAFE_RELEASE_NULL(m_pVerticalScrollBar);
}

bool XTableView::onTouchBegan(Touch* pTouch, Event* pEvent)
{
    if (!m_bLoadCompleted)
        return false;

    if (!TableView::onTouchBegan(pTouch, pEvent))
        return false;

    m_euCheckDirection = Direction::NONE;
    return true;
}

void XTableView::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
	BOOL bRetCode = false;

	if (!m_bLoadCompleted)
        return;

    TableView::onTouchEnded(pTouch, pEvent);
 
    m_euCheckDirection = Direction::NONE;

	bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "TouchEnded");
	XYLOG_FAILED_JUMP(bRetCode);

    bRetCode = Lua_XCall(luaSafeStack, 0, 0);
	XYLOG_FAILED_JUMP(bRetCode);
Exit0:
    return;
}


void XTableView::onTouchMoved(Touch* pTouch, Event* pEvent)
{
    if (!m_bLoadCompleted)
        return;

    Direction euCurDirection = this->getDirection();
    
    if (euCurDirection == Direction::BOTH || euCurDirection == m_euCheckDirection)
    {
        TableView::onTouchMoved(pTouch, pEvent);
        return;
    }

    if (m_euCheckDirection == Direction::NONE)
    {
        CCPoint subPoint = pTouch->getLocationInView() - pTouch->getStartLocationInView();
        subPoint.x = fabs(subPoint.x);
        subPoint.y = fabs(subPoint.y);
        
        if (subPoint.x < 50 && subPoint.y < 50)
            return;
       
        if (subPoint.x > subPoint.y)
            m_euCheckDirection = Direction::HORIZONTAL;// kCCScrollViewDirectionHorizontal;
        else
            m_euCheckDirection = Direction::VERTICAL; //kCCScrollViewDirectionVertical;
    }
}

void XTableView::scrollViewDidScroll(ScrollView* pScrollView)
{
	XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
	BOOL bRetCode = false;

    if (!m_bLoadCompleted)
        return;

    TableView::scrollViewDidScroll(pScrollView);
    UpdateVerticalScrollBar();

	bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "ScrollViewDidScroll");
	XYLOG_FAILED_JUMP(bRetCode);

    bRetCode = Lua_XCall(luaSafeStack, 0, 0);
	XYLOG_FAILED_JUMP(bRetCode);
Exit0:
    return;
}

Size XTableView::tableCellSizeForIndex(TableView* pTableView, ssize_t uIdx)
{
    Size result(0, 0);
    BOOL bRetCode = false;
    XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
    
    assert(this == pTableView);
    
    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "TableCellSizeForIndex");
    XYLOG_FAILED_JUMP(bRetCode);
    
    Lua_PushObject(g_pServerAgent->GetLuaState(), this);
    lua_pushinteger(g_pServerAgent->GetLuaState(), uIdx + 1);
    
    bRetCode = Lua_XCall(luaSafeStack, 2, 2);
    XYLOG_FAILED_JUMP(bRetCode);
    
    result.width = lua_tonumber(g_pServerAgent->GetLuaState(), -2);
    result.height = lua_tonumber(g_pServerAgent->GetLuaState(), -1);
    
Exit0:
    return result;
}

TableViewCell* XTableView::tableCellAtIndex(TableView* pTableView, ssize_t uIdx)
{
    TableViewCell* pResult = NULL;
    BOOL bRetCode = false;
    BasicPanel* pOldPanel = NULL;
    BasicPanel* pNewPanel = NULL;
    XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
    
    assert(this == pTableView);
    
    // 无论如何都要return一个CCTableViewCell，否则会空指针访问！
    pResult = this->dequeueCell();
    if (!pResult)
    {
        pResult = new TableViewCell();
        pResult->autorelease();
    }
    
    Size size = this->tableCellSizeForIndex(pTableView, uIdx);
    pResult->setContentSize(size);

    pOldPanel = GetTableViewCell(pResult);
    
    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "TableCellAtIndex");
    XYLOG_FAILED_JUMP(bRetCode);
    
    Lua_PushObject(g_pServerAgent->GetLuaState(), this);
    lua_pushinteger(g_pServerAgent->GetLuaState(), uIdx + 1);
    Lua_PushObject(g_pServerAgent->GetLuaState(), pOldPanel);

    bRetCode = Lua_XCall(luaSafeStack, 3, 1);
    XYLOG_FAILED_JUMP(bRetCode);
    
    pNewPanel = Lua_ToObject<BasicPanel>(g_pServerAgent->GetLuaState(), -1);
    XYLOG_FAILED_JUMP(pNewPanel);

   if (pOldPanel != pNewPanel)
   {
        pResult->removeAllChildren();
        
        pNewPanel->setPosition(Point::ZERO);
        pNewPanel->setAnchorPoint(Point::ZERO);
		pResult->addChild(pNewPanel);
    }
    
Exit0:
    return pResult;
}

ssize_t XTableView::numberOfCellsInTableView(TableView* pTableView)
{
    unsigned int uResult = 0;
    BOOL bRetCode = false;
    XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
    
    assert(this == pTableView);
    
    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "NumberOfCellsInTableView");
    XY_FAILED_JUMP(bRetCode);
    
    Lua_PushObject(g_pServerAgent->GetLuaState(), this);
    
    bRetCode = Lua_XCall(luaSafeStack, 1, 1);
    XYLOG_FAILED_JUMP(bRetCode);
    
    uResult = lua_tointeger(g_pServerAgent->GetLuaState(), -1);
Exit0:
    return uResult;
}

void XTableView::onNodeLoaded(Node* pNode, cocosbuilder::NodeLoader* pNodeLoader)
{
    XTableView* pTableView  = dynamic_cast<XTableView*>(pNode);
    
    XYLOG_FAILED_JUMP(pTableView);
    
    // 摘自 CCTableView::initWithViewSize
    
    pTableView->_cellsUsed      = Vector<TableViewCell*>();//new ArrayForObjectSorting();
    pTableView->_cellsFreed     = Vector<TableViewCell*>();
    pTableView->_indices        = new std::set<ssize_t>();
    pTableView->_vordering      = TableView::VerticalFillOrder::BOTTOM_UP;// kCCTableViewFillBottomUp;
    // pTableView->setDirection(kCCScrollViewDirectionVertical); // 不需要
    
    ScrollView::setDelegate(this);
    
    pTableView->setDataSource(pTableView);
    pTableView->_updateCellPositions();
    pTableView->_updateContentSize();

    pTableView->setDelegate(&g_TableViewDelegate);
                
Exit0:
    return;
}

void XTableView::UpdateVerticalScrollBar()
{   
    Direction direction = this->getDirection();
    Size contentSize = this->getContentSize();
    Size viewSize = this->getViewSize();
    Vec2 offset = this->getContentOffset();
    float fMaxOffset = contentSize.height - viewSize.height;
    float fPercentage = 0;
    Sequence* pAction = NULL;
    Size scrollBarSize;
    
    XY_FAILED_JUMP(direction == Direction::VERTICAL || direction == Direction::BOTH);
    XY_FAILED_JUMP(fMaxOffset > 0 && contentSize.height > 0);
    
    if (m_pVerticalScrollBar && !m_pVerticalScrollBar->getParent())
    {
        CC_SAFE_RELEASE_NULL(m_pVerticalScrollBar);
    }
    
    scrollBarSize = CCSizeMake(5, viewSize.height * viewSize.height / contentSize.height);
    
    if (!m_pVerticalScrollBar)
    {
        m_pVerticalScrollBar = CCLayerColor::create(ccc4(205, 186, 150, 255), scrollBarSize.width,scrollBarSize.height);
        m_pVerticalScrollBar->ignoreAnchorPointForPosition(true);
        
        // 注意，不能直接调用addChild
        CCLayer::addChild(m_pVerticalScrollBar, m_pVerticalScrollBar->getZOrder(), m_pVerticalScrollBar->getTag());
        CC_SAFE_RETAIN(m_pVerticalScrollBar);
    }
    
    XYLOG_FAILED_JUMP(m_pVerticalScrollBar);
    
    fPercentage = 1 - (fMaxOffset + offset.y) / fMaxOffset;
    fPercentage = MAX(fPercentage, 0.01f);
    fPercentage = MIN(fPercentage, 0.99f);
    
    m_pVerticalScrollBar->setPosition(viewSize.width - scrollBarSize.width, fPercentage * viewSize.height);
    
    m_pVerticalScrollBar->stopAllActions();
    
    pAction = CCSequence::createWithTwoActions(CCDelayTime::create(0.1f), CCRemoveSelf::create());
    m_pVerticalScrollBar->runAction(pAction);
    
Exit0:
    return;
}

int XTableView::LuaGetContainer(lua_State* L)
{
    CCNode* pNode = this->getContainer();
    BasicPanel* pPanel = dynamic_cast<BasicPanel*>(pNode);
    
    Lua_PushObject(L, pPanel);
    return 1;
}

int XTableView::LuaSetVerticalFillOrder(lua_State* L)
{
	VerticalFillOrder euOrder = (VerticalFillOrder)lua_tointeger(L, 1);
    
    this->setVerticalFillOrder(euOrder);
    
    return 0;
}

int XTableView::LuaGetVerticalFillOrder(lua_State* L)
{
    int euOrder =  (int)(this->getVerticalFillOrder());
    
    lua_pushinteger(L, euOrder);
    
    return 1;
}

int XTableView::LuaInsertCellAtIndex(lua_State* L)
{
    unsigned int uIdx = lua_tointeger(L, 1);
    
    this->insertCellAtIndex(uIdx - 1);
    
    return 0;
}

int XTableView::LuaUpdateCellAtIndex(lua_State* L)
{
    unsigned int uIdx = lua_tointeger(L, 1);
    
    this->updateCellAtIndex(uIdx - 1);
    
    return 0;
}

int XTableView::LuaRemoveCellAtIndex(lua_State* L)
{
    unsigned int uIdx = lua_tointeger(L, 1);
    
    this->removeCellAtIndex(uIdx - 1);
    
    return 0;
}

int XTableView::LuaGetViewSize(lua_State* L)
{
    CCSize size = this->getViewSize();
    
    lua_pushnumber(L, size.width);
    lua_pushnumber(L, size.height);
    
    return 2;
}

int XTableView::LuaSetViewSize(lua_State* L)
{
	CCSize size;
    
	size.width = (float)lua_tonumber(L, 1);
	size.height = (float)lua_tonumber(L, 2);
    
	this->setViewSize(size);
    return 0;
}

int XTableView::LuaGetContentSize(lua_State* L)
{
    const CCSize& size =this->getContentSize();
    
    lua_pushnumber(L, size.width);
    lua_pushnumber(L, size.height);
    
    return 2;
}

int XTableView::LuaSetContentSize(lua_State* L)
{
    CCSize size;
    
    size.width = (float)lua_tonumber(L, 1);
    size.height = (float)lua_tonumber(L, 2);
    
    this->setContentSize(size);
    
    return 0;
}

int XTableView::LuaGetContentOffset(lua_State* L)
{
    const CCPoint point =this->getContentOffset();
    
    lua_pushnumber(L, point.x);
    lua_pushnumber(L, point.y);
    
    return 2;
}


int XTableView::LuaSetContentOffset(lua_State* L)
{
    CCPoint point;
    
    point.x = (float)lua_tonumber(L, 1);
    point.y = (float)lua_tonumber(L, 2);
    
    this->setContentOffset(point);
    return 0;
}

int XTableView::LuaReloadData(lua_State* L)
{
    this->reloadData();
    return 0;
}

int XTableView::LuaGetOffsetFromIndex(lua_State* L)
{
    unsigned int uIdx = lua_tointeger(L, 1);
    CCPoint point = this->_offsetFromIndex(uIdx - 1);
    
    lua_pushnumber(L, point.x);
    lua_pushnumber(L, point.y);
    return 2;
}

int XTableView::LuaRefresh(lua_State* L)
{
    BOOL bReloadFlag = lua_toboolean(L, 1);
    CCPoint offset = this->getContentOffset();
    CCSize oldContentSize = this->getContentSize();

    bReloadFlag = !bReloadFlag && m_bLoadCompleted;
    m_bLoadCompleted = true;
    this->reloadData();

    if (bReloadFlag)
    {
        CCSize viewSize = this->getViewSize();
        CCSize contentSize = this->getContentSize();
        Direction direction = this->getDirection();

        if (direction == Direction::HORIZONTAL)
        {
            if (viewSize.width >= contentSize.width)
            {
                offset.x = 0;
                offset.y = 0;
            }
            else 
            {
                offset.x = MIN(0, MAX(viewSize.width - contentSize.width, offset.x));
                offset.y = 0;
            }
        }
        else if(direction == Direction::VERTICAL)
        {
            VerticalFillOrder euOrder = this->getVerticalFillOrder();
            if (euOrder == VerticalFillOrder::TOP_DOWN)
            {
                if (viewSize.height >= contentSize.height)
                {
                    offset.x = 0;
                    offset.y = viewSize.height - contentSize.height;
                }
                else 
                {
                    offset.x = 0;
                    offset.y = MIN(0, MAX(viewSize.height - contentSize.height, offset.y));
                }
            }
            else
            {
                if (viewSize.height >= contentSize.height)
                {
                    offset.x = 0;
                    offset.y = 0;
                }
                else 
                {
                    offset.x = 0;
                    offset.y = MIN(0, MAX(viewSize.height - contentSize.height, offset.y));
                }
            }
        }
        else
        {
            offset.x = 0;
            offset.y = 0;
        }
        this->setContentOffset(offset);
    }

    return 0;
}

int XTableView::LuaGetRootPanel(lua_State* L)
{
	/* FM 注释与14-7-15。原因：不知道GetRootPanel从何而来，如何使用，为了编译通过，先注释掉
    Lua_PushObject(L, GetRootPanel(this));
    return 1;
	*/
	return 0;
}
