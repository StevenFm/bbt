//  CCScrollLayer.h
//  Museum
//
//  Created by GParvaneh on 29/12/2010.
//  Copyright 2010 All rights reserved.
//  Ported to C++ by Lior Tamam on 03/04/2011
//  Cleaned for Cocos2d-x V2.x by @HermanJakobi (01/12/12)
//pragma once
#include "cocos2d.h"
#include "cocos-ext.h"
#include "stdafx.h"

class XScrollLayer : public cocos2d::Layer 
{
    
public:
	XScrollLayer();
	~XScrollLayer();
    
	static XScrollLayer* create(cocos2d::Array* layers,int widthOffset);
	DECLARE_LUA_CLASS(XScrollLayer);
	
    bool init(cocos2d::Array *layers,int widthOffset);
    
    void initPage(int page);

	void AddLayerToArray(Node* pLayer);

	void initScrollLayer(int widthOffset);
	int  GetLayerCount();

    int LuaInitPage(lua_State* L);
	int LuaGetCurrentPage(lua_State* L);
public:
	cocos2d::__Array   m_Array;
    
private:
    
    // Holds the current height and width of the screen
	int scrollHeight;
	int scrollWidth;
	
	// Holds the height and width of the screen when the class was inited
	int startHeight;
	int startWidth;
	
	// Holds the current page being displayed
	int currentScreen;
	
	// A count of the total screens available
	int totalScreens;
	
	// The initial point the user starts their swipe
	int startSwipe;
    
    cocos2d::Vec2 selfPosition;
    
    cocos2d::Array* selfLayers;

	cocos2d::EventListenerTouchOneByOne* _listener;
    
	void moveToPage(int page);
	void moveToNextPage();
	void moveToPreviousPage();
	
    
    virtual bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);
	virtual void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent);	
    
    //virtual void onEnter();
	//virtual void onExit();
    
};
