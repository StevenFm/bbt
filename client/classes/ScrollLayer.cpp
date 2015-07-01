//  XScrollLayer.cpp
//  Museum
//
//  Created by GParvaneh on 29/12/2010.
//  Copyright 2010. All rights reserved.
//  Ported to C++ by Lior Tamam on 03/04/2011
//  Cleaned for Cocos2d-x V2.x by @HermanJakobi (01/12/12)

#include "ScrollLayer.h"

#include "stdafx.h"
#include "BasicPanel.h"
#include "ServerAgent.h"

using namespace std;

USING_NS_CC;
USING_NS_CC_EXT;

IMPL_LUA_CLASS_BEGIN(XScrollLayer)    
	EXPORT_LUA_FUNCTION(LuaInitPage)
	EXPORT_LUA_FUNCTION(LuaGetCurrentPage)
IMPL_LUA_CLASS_END()

XScrollLayer::XScrollLayer()
{
	EventDispatcher* dispatcher = Director::getInstance()->getEventDispatcher();
	_listener = EventListenerTouchOneByOne::create();
	_listener->onTouchBegan = CC_CALLBACK_2(XScrollLayer::onTouchBegan, this);
	_listener->onTouchMoved = CC_CALLBACK_2(XScrollLayer::onTouchMoved, this);
	_listener->onTouchEnded = CC_CALLBACK_2(XScrollLayer::onTouchEnded, this);
	dispatcher->addEventListenerWithSceneGraphPriority(_listener, this);
	this->setTouchEnabled(true);

	m_Array.init();
}

XScrollLayer::~XScrollLayer()
{
	CC_SAFE_RELEASE(_listener);
}

XScrollLayer* XScrollLayer::create(CCArray *layers, int widthOffset)
{	
	XScrollLayer *pRet = new XScrollLayer();
	if (pRet && pRet->init(layers, widthOffset))
	{
		pRet->autorelease();
		return pRet;
	}
	CC_SAFE_DELETE(pRet);
	return NULL;
}

bool XScrollLayer::init(CCArray *layers, int widthOffset)
{	
	if (CCLayer::init())
	{		
		
		
		// Set up the starting variables
		if(!widthOffset)
		{
			widthOffset = 0;
		}	
		currentScreen = 1;
		
		// offset added to show preview of next/previous screens
        CCSize s=CCDirector::sharedDirector()->getWinSize();
        
		scrollWidth  = s.width - widthOffset;
		scrollHeight = s.height;
		startWidth = scrollWidth;
		startHeight = scrollHeight;
		
		// Loop through the array and add the screens
		unsigned int i;
		for (i=0; i<layers->count(); i++)
		{
			CCLayer* l = static_cast<CCLayer*>(layers->objectAtIndex(i));
			l->setAnchorPoint(ccp(0,0));
			l->setPosition(ccp((i*scrollWidth),0));
			addChild(l);			
		}
		
		// Setup a count of the available screens
		totalScreens = layers->count();
        
        selfLayers = layers;
        selfLayers->retain();
        
        selfPosition = CCPointMake(this->getPositionX(), this->getPositionY());
        
        
		return true;	
	}
	
    
    return false;
    
}

void XScrollLayer::initPage(int page)
{
    moveToPage(page);
}

/*
void XScrollLayer::onEnter()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, false);
    CCLayer::onEnter();
}

void XScrollLayer::onExit()
{
	CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
    CCLayer::onExit();
}	
*/

void XScrollLayer::moveToPage(int page)
{	
	//EaseBounce* changePage = EaseBounce::create(CCMoveTo::create(0.3f, ccp(-((page-1)*scrollWidth),0)));
	EaseBounceIn* changePage = EaseBounceIn::create(MoveTo::create(0.3f, ccp(-((page-1)*scrollWidth),0)));
	this->runAction(changePage);
	currentScreen = page;	
}

void XScrollLayer::moveToNextPage()
{	
	EaseBounceIn* changePage = EaseBounceIn::create(CCMoveTo::create(0.3f, ccp(-(((currentScreen+1)-1)*scrollWidth),0)));
	
	this->runAction(changePage);
	currentScreen = currentScreen+1;

	BOOL bRetCode = false;
    XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
    
    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "MoveToNextPage");
    XY_FAILED_JUMP(bRetCode);
    
    Lua_PushObject(g_pServerAgent->GetLuaState(), this);
    
    bRetCode = Lua_XCall(luaSafeStack, 1, 0);
    XYLOG_FAILED_JUMP(bRetCode);
    
Exit0:
    return;
}

void XScrollLayer::moveToPreviousPage()
{	
	EaseBounceIn* changePage = EaseBounceIn::create(CCMoveTo::create(0.3f, ccp(-(((currentScreen-1)-1)*scrollWidth),0)));
	this->runAction(changePage);
	currentScreen = currentScreen-1;

    BOOL bRetCode = false;
    XLuaSafeStack luaSafeStack(g_pServerAgent->GetLuaState());
    
    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "MoveToPreviousPage");
    XY_FAILED_JUMP(bRetCode);
    
    Lua_PushObject(g_pServerAgent->GetLuaState(), this);
    
    bRetCode = Lua_XCall(luaSafeStack, 1, 0);
    XYLOG_FAILED_JUMP(bRetCode);
    
Exit0:
    return;
}


bool XScrollLayer::onTouchBegan(CCTouch *touch, CCEvent *withEvent)
{
	
    CCPoint touchPoint = touch->getLocation(); // Get the touch position
    touchPoint = this->getParent()->convertToNodeSpace(touchPoint);
    
	
	startSwipe = (int)touchPoint.x;
	return true;
}

void XScrollLayer::onTouchMoved(CCTouch *touch, CCEvent *withEvent)
{	
    
    CCPoint touchPoint = touch->getLocation(); // Get the touch position
    touchPoint = this->getParent()->convertToNodeSpace(touchPoint);
	
	this->setPosition(ccp((-(currentScreen-1)*scrollWidth)+(touchPoint.x-startSwipe),0));
}

void XScrollLayer::onTouchEnded(CCTouch *touch, CCEvent *withEvent)
{   
    CCPoint touchPoint = touch->getLocation(); // Get the touch position
    touchPoint = this->getParent()->convertToNodeSpace(touchPoint);
	
	int newX = (int)touchPoint.x;
    
	if ( (newX - startSwipe) < -scrollWidth / 3 && (currentScreen+1) <= totalScreens )
	{
		this->moveToNextPage();
	}
	else if ( (newX - startSwipe) > scrollWidth / 3 && (currentScreen-1) > 0 )
	{
		this->moveToPreviousPage();
	}
	else
	{
		this->moveToPage(currentScreen);		
	}
}


void XScrollLayer::AddLayerToArray(CCNode* pLayer)
{
	m_Array.addObject(pLayer);
}

void XScrollLayer::initScrollLayer(int widthOffset)
{
	this->init(&m_Array, widthOffset);
}

int XScrollLayer::GetLayerCount()
{
	return m_Array.count();
}

int XScrollLayer::LuaInitPage(lua_State* L)
{
	float fPage = (float)lua_tonumber(L, 1);
	this->initPage((int)fPage);
	return 0;
}

int XScrollLayer::LuaGetCurrentPage(lua_State* L)
{
	int nCurrentPage = currentScreen;
	lua_pushinteger(L, nCurrentPage);
	return 1;
}