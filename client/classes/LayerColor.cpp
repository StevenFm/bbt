#include "stdafx.h"
#include "LayerColor.h"

USING_NS_CC;

bool XLayerColor::onTouchBegan(Touch* pTouch, Event* pEvent)
{
	log("ddddd");
	setTouchEnabled(false);
	return false;
}