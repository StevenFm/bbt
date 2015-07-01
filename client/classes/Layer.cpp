#include "stdafx.h"
#include "Layer.h"

USING_NS_CC;
USING_NS_CC_EXT;

bool XLayer::onTouchBegan(Touch* pTouch, Event* pEvent)
{
	log("============ont ouch began");
	this->setTouchEnabled(false);
	return false;
}