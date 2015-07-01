//pragma once
#include "cocosbuilder/CocosBuilder.h"

struct XEditBoxCCBParam
{
    cocos2d::extension::Scale9Sprite* pNormalSprite;
    cocos2d::extension::Scale9Sprite* pPressedSprite;
    cocos2d::extension::Scale9Sprite* pDisabledSprite;
    std::string strText;
    std::string strFont;
    int nFontSize;
    cocos2d::Color3B fontColor;
    cocos2d::Size contentSize;
    cocos2d::Vec2 position;
    cocos2d::Vec2 anchorPoint;
    float scaleX;
    float scaleY;
    float skewX;
    float skewY;
    bool visible;
    bool ignoreAnchorPointForPosition;
};

class XEditBox 
    : public cocos2d::extension::EditBox
    , public cocosbuilder::NodeLoaderListener
    , public cocos2d::extension::EditBoxDelegate    
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(XEditBox, create);
    DECLARE_LUA_CLASS(XEditBox);

    XEditBox();
    ~XEditBox();
    
    virtual void onNodeLoaded(cocos2d::Node* pNode, cocosbuilder::NodeLoader* pNodeLoader);
	virtual void editBoxEditingDidBegin(cocos2d::extension::EditBox* editBox);
    virtual void editBoxReturn(cocos2d::extension::EditBox* pEditBox);

    int LuaSetInputMode(lua_State* L);
    int LuaSetInputFlag(lua_State* L);
    int LuaSetReturnType(lua_State* L);
    int LuaSetMaxLength(lua_State* L);    
    int LuaGetText(lua_State* L);
    int LuaSetText(lua_State* L);
    int LuaGetRootPanel(lua_State* L);

public:
    XEditBoxCCBParam* m_pCCBParam;
};

class XEditBoxLoader : public cocosbuilder::NodeLoader
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(XEditBoxLoader, loader);

protected:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(XEditBox);
    
    virtual void parseProperties(cocos2d::Node * pNode, cocos2d::Node * pParent, cocosbuilder::CCBReader * pCCBReader);
};
