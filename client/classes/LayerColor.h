#include "cocosbuilder/CocosBuilder.h"

class XLayerColor : public cocos2d::LayerColor
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_WITH_INIT_METHOD(XLayerColor, create);

    virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
};

class XLayerColorLoader : public cocosbuilder::LayerColorLoader
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(XLayerColorLoader, loader);

protected:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(XLayerColor);
};
