#include "stdafx.h"
#include "TouchDispatcher.h"

USING_NS_CC;
USING_NS_CC_EXT;

bool XTouchDispatcher::init(void)
{
    return CCTouchDispatcher::init();
}

void XTouchDispatcher::touchesBegan(CCSet* pTouches, CCEvent* pEvent)
{
    if (m_bDispatchEvents)
    {
        ProcessTouch(pTouches, pEvent, CCTOUCHBEGAN);
    }
}

void XTouchDispatcher::touchesEnded(CCSet* pTouches, CCEvent* pEvent)
{
    if (m_bDispatchEvents)
    {
        ProcessTouch(pTouches, pEvent, CCTOUCHENDED);
    }
}

void XTouchDispatcher::touchesCancelled(CCSet* pTouches, CCEvent* pEvent)
{
    if (m_bDispatchEvents)
    {
        ProcessTouch(pTouches, pEvent, CCTOUCHCANCELLED);
    }
}

void XTouchDispatcher::touchesMoved(CCSet* pTouches, CCEvent* pEvent)
{
    if (m_bDispatchEvents)
    {
        ProcessTouch(pTouches, pEvent, CCTOUCHMOVED);
    }
}

// TargetedHandler：用来做ui点击 StandardHandler用来做场景点击
void XTouchDispatcher::ProcessTouch(CCSet* pTouches, CCEvent* pEvent, unsigned uIndex)
{
    BOOL            bRetcode                = false;
    CCSet*          pMutableTouches         = NULL;
    unsigned int    uTargetedHandlersCount  = m_pTargetedHandlers->count();
    unsigned int    uStandardHandlersCount  = m_pStandardHandlers->count();
    bool            bNeedsMutableSet        = (uTargetedHandlersCount && uStandardHandlersCount);
    CCDirector*     pDirector               = CCDirector::sharedDirector();

    XYLOG_FAILED_JUMP(uIndex >= 0 && uIndex < 4);

    m_bLocked = true;
    pMutableTouches = (bNeedsMutableSet ? pTouches->mutableCopy() : pTouches);

    if (uTargetedHandlersCount > 0)
    {
        for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
        {
            CCTouch* pTouch = (CCTouch*)(*it);
            if (uIndex == CCTOUCHBEGAN)
            {
                bRetcode = ProcessTargetedHandlerTouchBegan(pDirector->getRunningScene(), pTouch, pEvent);
                if (bRetcode && bNeedsMutableSet)
                    pMutableTouches->removeObject(pTouch);
            }
            else
            {
                bRetcode = ProcessTargetedHandlerTouch(pTouch, pEvent, uIndex);
                if (bRetcode && bNeedsMutableSet)
                    pMutableTouches->removeObject(pTouch);
            }
        }
    }

    if (uStandardHandlersCount > 0 && pMutableTouches->count() > 0)
    {
        ProcessStandardHandlerTouch(pMutableTouches, pEvent, uIndex);
    }

    if (bNeedsMutableSet)
    {
        pMutableTouches->release();
    }

Exit0:
    m_bLocked = false;
    processDelegate();
}

BOOL XTouchDispatcher::ReverseTraversalChildren(CCNode* pNode, CCTouch* pTouch, CCEvent* pEvent)
{
    BOOL                    bResult                 = false;
    BOOL                    bRetCode                = false;
    CCObject*               pObject                 = NULL;

    CCARRAY_FOREACH_REVERSE(pNode->getChildren(), pObject)
    {
        CCNode* pChildNode = (CCNode*)(pObject);
        if (!pChildNode)
            break;

        bRetCode = ProcessTargetedHandlerTouchBegan(pChildNode, pTouch, pEvent);
        XY_SUCCESS_JUMP(bRetCode);
    }

    XY_FAILED_JUMP(false);

Exit1:
    bResult = true;
 Exit0:
    return bResult;
}

BOOL XTouchDispatcher::ProcessTargetedHandlerTouchBegan(CCNode* pNode, CCTouch* pTouch, CCEvent* pEvent)
{
    BOOL                    bResult                 = false;
    BOOL                    bRetCode                = false;
    BOOL                    bClaimed                = false;
    CCTouchDelegate*        pTouchDelegate          = NULL;
    CCTouchHandler*         pTouchHandler           = NULL;
    CCTargetedTouchHandler* pTargetedTouchHandler   = NULL;
    CCSet*                  pClaimedTouches         = NULL;
    BOOL                    bChildPriorityFlag      = true;

    bRetCode = pNode->isVisible();
    XY_FAILED_JUMP(bRetCode);

    if (dynamic_cast<CCMenu*>(pNode) || dynamic_cast<CCScrollView*>(pNode))
    {
        bChildPriorityFlag = false;
    }

    if (bChildPriorityFlag)
    {
        bRetCode = ReverseTraversalChildren(pNode, pTouch, pEvent);
        XY_SUCCESS_JUMP(bRetCode);
    }
    
    pTouchDelegate = dynamic_cast<CCTouchDelegate*>(pNode);
    XY_FAILED_JUMP(pTouchDelegate);

    pTouchHandler = findHandler(m_pTargetedHandlers, pTouchDelegate);
    XY_FAILED_JUMP(pTouchHandler);

    pTargetedTouchHandler = dynamic_cast<CCTargetedTouchHandler*>(pTouchHandler);
    XY_FAILED_JUMP(pTargetedTouchHandler);

    pClaimedTouches = pTargetedTouchHandler->getClaimedTouches();

    bClaimed = pTouchDelegate->ccTouchBegan(pTouch, pEvent);
    if (bClaimed) 
    {
        pClaimedTouches->addObject(pTouch);
    }

    XY_FAILED_JUMP(bClaimed && pTargetedTouchHandler->isSwallowsTouches());
    
Exit1:
    bResult = true;
Exit0:
    if (!bResult && !bChildPriorityFlag)
    {
        bResult = ReverseTraversalChildren(pNode, pTouch, pEvent);
    }
    return bResult;
}

BOOL XTouchDispatcher::ProcessTargetedHandlerTouch(CCTouch* pTouch, CCEvent* pEvent, unsigned uIndex)
{
    BOOL                    bResult                 = false;
    BOOL                    bRetCode                = false;
    CCObject*               pObject                 = NULL;
    CCTargetedTouchHandler* pTargetedTouchHandler   = NULL;
    CCTouchDelegate*        pTouchDelegate          = NULL;
    CCSet*                  pClaimedTouches         = NULL;
    BOOL                    bSwallowsFlag           = false;

    assert(uIndex != CCTOUCHBEGAN);

    CCARRAY_FOREACH(m_pTargetedHandlers, pObject)
    {
        pTargetedTouchHandler = (CCTargetedTouchHandler*)(pObject);
        if (!pTargetedTouchHandler)
            continue;

        pTouchDelegate = pTargetedTouchHandler->getDelegate();
        pClaimedTouches = pTargetedTouchHandler->getClaimedTouches();
        
        bRetCode = pClaimedTouches->containsObject(pTouch);
        if (!bRetCode)
            continue;

        if (!bSwallowsFlag && pTargetedTouchHandler->isSwallowsTouches())
        {
            bSwallowsFlag = true;
        }

        switch (uIndex)
        {
        case CCTOUCHMOVED:
            pTouchDelegate->ccTouchMoved(pTouch, pEvent);
            break;
                
        case CCTOUCHENDED:
            pTouchDelegate->ccTouchEnded(pTouch, pEvent);
            pClaimedTouches->removeObject(pTouch);
            break;
                
        case CCTOUCHCANCELLED:
            pTouchDelegate->ccTouchCancelled(pTouch, pEvent);
            pClaimedTouches->removeObject(pTouch);
            break;
        }
    }

    XY_FAILED_JUMP(bSwallowsFlag);

    bResult = true;
Exit0:
    return bResult;
}

void XTouchDispatcher::ProcessStandardHandlerTouch(CCSet* pMutableTouches, CCEvent* pEvent, unsigned uIndex)
{
    CCObject*   pObject     = NULL;

    CCARRAY_FOREACH(m_pStandardHandlers, pObject)
    {
        CCStandardTouchHandler* pHandler = (CCStandardTouchHandler*)(pObject);
        if (!pHandler)
            break;

        switch (uIndex)
        {
        case CCTOUCHBEGAN:
            pHandler->getDelegate()->ccTouchesBegan(pMutableTouches, pEvent);
            break;
        case CCTOUCHMOVED:
            pHandler->getDelegate()->ccTouchesMoved(pMutableTouches, pEvent);
            break;
        case CCTOUCHENDED:
            pHandler->getDelegate()->ccTouchesEnded(pMutableTouches, pEvent);
            break;
        case CCTOUCHCANCELLED:
            pHandler->getDelegate()->ccTouchesCancelled(pMutableTouches, pEvent);
            break;
        }
    }
}
