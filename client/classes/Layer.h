#include "cocosbuilder/CocosBuilder.h"

class XLayer : public cocos2d::CCLayer
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_WITH_INIT_METHOD(XLayer, create);
    
    virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
};

class XLayerLoader : public cocosbuilder::LayerLoader
{
public:
    CCB_STATIC_NEW_AUTORELEASE_OBJECT_METHOD(XLayerLoader, loader);

protected:
    CCB_VIRTUAL_NEW_AUTORELEASE_CREATECCNODE_METHOD(XLayer);
};
