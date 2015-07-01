//pragma once
#include "cocosbuilder/CocosBuilder.h"

class XTableView
	: public cocos2d::extension::TableView
	, public cocos2d::extension::TableViewDataSource
    , public cocosbuilder::NodeLoaderListener
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_WITH_INIT_METHOD(XTableView, create);
    DECLARE_LUA_CLASS(XTableView);
    
    XTableView();
    virtual ~XTableView();
    
    virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
    virtual void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
    virtual void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
    virtual void scrollViewDidScroll(cocos2d::extension::ScrollView* view);
        
    virtual cocos2d::Size tableCellSizeForIndex(cocos2d::extension::TableView* pTableView, ssize_t uIdx);

    virtual cocos2d::extension::TableViewCell* tableCellAtIndex(cocos2d::extension::TableView* pTableView, ssize_t uIdx);
    virtual ssize_t numberOfCellsInTableView(cocos2d::extension::TableView* pTableView);
    
    virtual void onNodeLoaded(cocos2d::Node* pNode, cocosbuilder::NodeLoader* pNodeLoader);
    
private:
    void UpdateVerticalScrollBar();
    
public:
    int LuaGetContainer(lua_State* L);
    
    int LuaSetVerticalFillOrder(lua_State* L);
    int LuaGetVerticalFillOrder(lua_State* L);
    
    int LuaInsertCellAtIndex(lua_State* L);
    int LuaUpdateCellAtIndex(lua_State* L);
    int LuaRemoveCellAtIndex(lua_State* L);
    
    int LuaGetViewSize(lua_State* L);
	int LuaSetViewSize(lua_State* L);
    int LuaGetContentSize(lua_State* L);
    int LuaSetContentSize(lua_State* L);
    int LuaGetContentOffset(lua_State* L);
    int LuaSetContentOffset(lua_State* L);
	int LuaReloadData(lua_State* L);
    
    int LuaGetOffsetFromIndex(lua_State* L);
    int LuaRefresh(lua_State* L);
    int LuaGetRootPanel(lua_State* L);

private:
    Node* m_pVerticalScrollBar;
	cocos2d::extension::ScrollView::Direction m_euCheckDirection;
    BOOL m_bLoadCompleted;
};

class XTableViewLoader : public cocosbuilder::ScrollViewLoader
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(XTableViewLoader, loader);

protected:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(XTableView);
};
