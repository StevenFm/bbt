#pragma once
#include "touch_dispatcher/CCTouchDispatcher.h"

class XTouchDispatcher : public cocos2d::CCTouchDispatcher
{
public:
    CREATE_FUNC(XTouchDispatcher);   

    bool init(void);
    
    virtual void touchesBegan(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void touchesEnded(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void touchesCancelled(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);
    virtual void touchesMoved(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent);

private:
    void ProcessTouch(cocos2d::CCSet* pTouches, cocos2d::CCEvent* pEvent, unsigned int uIndex);
    BOOL ProcessTargetedHandlerTouchBegan(cocos2d::CCNode* pNode, cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
    BOOL ReverseTraversalChildren(cocos2d::CCNode* pNode, cocos2d::CCTouch* pTouch, cocos2d::CCEvent* pEvent);
    BOOL ProcessTargetedHandlerTouch(cocos2d::CCTouch *pTouch, cocos2d::CCEvent* pEvent, unsigned int uIndex);
    void ProcessStandardHandlerTouch(cocos2d::CCSet* pMutableTouches, cocos2d::CCEvent* pEvent, unsigned int uIndex);
};
