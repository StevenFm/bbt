#include "stdafx.h"
#include "EditBox.h"
#include "ServerAgent.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace cocosbuilder;

#define PROPERTY_POSITION "position"
#define PROPERTY_CONTENTSIZE "contentSize"
#define PROPERTY_SKEW "skew"
#define PROPERTY_ANCHORPOINT "anchorPoint"
#define PROPERTY_SCALE "scale"
#define PROPERTY_ROTATION "rotation"
#define PROPERTY_ROTATIONX "rotationX"
#define PROPERTY_ROTATIONY "rotationY"
#define PROPERTY_TAG "tag"
#define PROPERTY_IGNOREANCHORPOINTFORPOSITION "ignoreAnchorPointForPosition"
#define PROPERTY_VISIBLE "visible"

#define PROPERTY_COLOR "color"
#define PROPERTY_OPACITY "opacity"
#define PROPERTY_BLENDFUNC "blendFunc"
#define PROPERTY_FNTFILE "fntFile"
#define PROPERTY_STRING "string"

#define PROPERTY_ZOOMONTOUCHDOWN "zoomOnTouchDown"
#define PROPERTY_TITLE_NORMAL "title|1"
#define PROPERTY_TITLE_HIGHLIGHTED "title|2"
#define PROPERTY_TITLE_DISABLED "title|3"
#define PROPERTY_TITLECOLOR_NORMAL "titleColor|1"
#define PROPERTY_TITLECOLOR_HIGHLIGHTED "titleColor|2"
#define PROPERTY_TITLECOLOR_DISABLED "titleColor|3"
#define PROPERTY_TITLETTF_NORMAL "titleTTF|1"
#define PROPERTY_TITLETTF_HIGHLIGHTED "titleTTF|2"
#define PROPERTY_TITLETTF_DISABLED "titleTTF|3"
#define PROPERTY_TITLETTFSIZE_NORMAL "titleTTFSize|1"
#define PROPERTY_TITLETTFSIZE_HIGHLIGHTED "titleTTFSize|2"
#define PROPERTY_TITLETTFSIZE_DISABLED "titleTTFSize|3"
#define PROPERTY_LABELANCHORPOINT "labelAnchorPoint"
#define PROPERTY_PREFEREDSIZE "preferedSize" // TODO Should be "preferredSize". This is a typo in cocos2d-iphone, cocos2d-x and CocosBuilder!
#define PROPERTY_BACKGROUNDSPRITEFRAME_NORMAL "backgroundSpriteFrame|1"
#define PROPERTY_BACKGROUNDSPRITEFRAME_HIGHLIGHTED "backgroundSpriteFrame|2"
#define PROPERTY_BACKGROUNDSPRITEFRAME_DISABLED "backgroundSpriteFrame|3"

IMPL_LUA_CLASS_BEGIN(XEditBox)    
    EXPORT_LUA_FUNCTION(LuaSetInputMode) 
    EXPORT_LUA_FUNCTION(LuaSetInputFlag) 
    EXPORT_LUA_FUNCTION(LuaSetReturnType)     
    EXPORT_LUA_FUNCTION(LuaSetMaxLength)     
    EXPORT_LUA_FUNCTION(LuaGetText)
    EXPORT_LUA_FUNCTION(LuaSetText)
    EXPORT_LUA_FUNCTION(LuaGetRootPanel)
IMPL_LUA_CLASS_END()

XEditBox::XEditBox()
{
    m_pCCBParam = new XEditBoxCCBParam;
}

XEditBox::~XEditBox()
{
    XY_DELETE(m_pCCBParam);
}

void XEditBox::onNodeLoaded(Node* pNode, NodeLoader* pNodeLoader)
{
    BOOL            bRetCode    = false;
    XEditBox*       pEditBox    = dynamic_cast<XEditBox*>(pNode);  

    XYLOG_FAILED_JUMP(pEditBox);
    XYLOG_FAILED_JUMP(pEditBox->m_pCCBParam);
    
    bRetCode = pEditBox->initWithSizeAndBackgroundSprite(pEditBox->m_pCCBParam->contentSize, pEditBox->m_pCCBParam->pNormalSprite);
    XYLOG_FAILED_JUMP(bRetCode);
    
//     pEditBox->setBackgroundSpriteForState(pEditBox->m_pCCBParam->pPressedSprite, Control::State::HIGH_LIGHTED);
//     pEditBox->setBackgroundSpriteForState(pEditBox->m_pCCBParam->pDisabledSprite, Control::State::DISABLED);
    
  //  this->setTouchPriority(0); // CCControl set the touch dispatcher priority by default to 1

    pEditBox->setAnchorPoint(pEditBox->m_pCCBParam->anchorPoint);
    pEditBox->setPosition(pEditBox->m_pCCBParam->position);
     
    pEditBox->setFont(pEditBox->m_pCCBParam->strFont.c_str(), pEditBox->m_pCCBParam->nFontSize);
    pEditBox->setFontColor(pEditBox->m_pCCBParam->fontColor);
    
    pEditBox->setPlaceHolder(pEditBox->m_pCCBParam->strText.c_str());
    pEditBox->setPlaceholderFont(pEditBox->m_pCCBParam->strFont.c_str(), pEditBox->m_pCCBParam->nFontSize);
    pEditBox->setPlaceholderFontColor(pEditBox->m_pCCBParam->fontColor);

    pEditBox->setDelegate(pEditBox);

Exit0:
    if (pEditBox)
    {
        CC_SAFE_RELEASE_NULL(pEditBox->m_pCCBParam->pNormalSprite);
        CC_SAFE_RELEASE_NULL(pEditBox->m_pCCBParam->pPressedSprite);
        CC_SAFE_RELEASE_NULL(pEditBox->m_pCCBParam->pDisabledSprite);
        XY_DELETE(pEditBox->m_pCCBParam);
    }
    return ;
}

void XEditBox::editBoxEditingDidBegin(EditBox* editBox)
{
	BOOL            bRetCode    = 0;
	XLuaSafeStack   luaSafeStack(g_pServerAgent->GetLuaState());

	bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "OnEditBoxEditingDidBegin");
	XY_FAILED_JUMP(bRetCode);

	Lua_PushObject(g_pServerAgent->GetLuaState(), this); 

	bRetCode = Lua_XCall(luaSafeStack, 1, 0);
	XYLOG_FAILED_JUMP(bRetCode);

Exit0:
	return;
}

void XEditBox::editBoxReturn(EditBox* pEditBox)
{
    BOOL            bRetCode    = 0;
    XLuaSafeStack   luaSafeStack(g_pServerAgent->GetLuaState());

    bRetCode = Lua_GetObjFunction(g_pServerAgent->GetLuaState(), this, "OnEditBoxReturn");
    XY_FAILED_JUMP(bRetCode);

    Lua_PushObject(g_pServerAgent->GetLuaState(), this); 

    bRetCode = Lua_XCall(luaSafeStack, 1, 0);
    XYLOG_FAILED_JUMP(bRetCode);

Exit0:
    return;
}

int XEditBox::LuaSetInputMode(lua_State* L)
{
    InputMode euInputMode = (InputMode)lua_tointeger(L, 1);
    this->setInputMode(euInputMode);
    return 0;
}

int XEditBox::LuaSetInputFlag(lua_State* L)
{
    InputFlag euInputFlag = (InputFlag)lua_tointeger(L, 1);
    this->setInputFlag(euInputFlag);
    return 0;
}

int XEditBox::LuaSetReturnType(lua_State* L)
{
    KeyboardReturnType euReturnType = (KeyboardReturnType)lua_tointeger(L, 1);
    this->setReturnType(euReturnType);
    return 0;
}

int XEditBox::LuaSetMaxLength(lua_State* L)
{
    int nMaxLength = lua_tointeger(L, 1);
    this->setMaxLength(nMaxLength);
    return 0;
}

int XEditBox::LuaGetText(lua_State* L)
{
    const char* pszText = this->getText();
    lua_pushstring(L, pszText);
    return 1;
}

int XEditBox::LuaSetText(lua_State* L)
{
    const char* pszText = lua_tostring(L, 1);
    this->setText(pszText);
    return 0;
}

int XEditBox::LuaGetRootPanel(lua_State* L)
{
	/* FM 注释与14-7-15。原因：不知道GetRootPanel从何而来，如何使用，为了编译通过，先注释掉
    Lua_PushObject(L, GetRootPanel(this));
    return 1;
	*/
	return 0;
}

void XEditBoxLoader::parseProperties(CCNode * pNode, CCNode * pParent, CCBReader * pCCBReader)
{
    int numRegularProps = pCCBReader->readInt(false);
    int numExturaProps = pCCBReader->readInt(false);
    int propertyCount = numRegularProps + numExturaProps;
    
    XEditBox* pEditBox = dynamic_cast<XEditBox*>(pNode); 
    XEditBoxCCBParam* pCCBParam = NULL; 
    
    assert(pEditBox);
    
    pCCBParam = pEditBox->m_pCCBParam;
        
    for(int i = 0; i < propertyCount; i++)
    {
        int type = pCCBReader->readInt(false);
		std::string propertyName = pCCBReader->readCachedString();              /*int platform = */pCCBReader->readByte();
        
		/*
		RELATIVE_BOTTOM_LEFT,
			RELATIVE_TOP_LEFT,
			RELATIVE_TOP_RIGHT,
			RELATIVE_BOTTOM_RIGHT,
			PERCENT,
			MULTIPLY_RESOLUTION,
			*/
        switch(type)
        {
			case CCBReader::PropertyType::POSITION:// kCCBPropTypePosition:
            {
                float x = pCCBReader->readFloat();
                float y = pCCBReader->readFloat();
                int type = pCCBReader->readInt(false);
				CCBReader::PositionType positionType = static_cast<CCBReader::PositionType>(type);
                
                CCSize containerSize = pCCBReader->getAnimationManager()->getContainerSize(pParent);
                CCPoint point = getAbsolutePosition(ccp(x,y), positionType, containerSize, propertyName.c_str());
                
                if (propertyName == PROPERTY_POSITION)
                {
                    pCCBParam->position = point;
                }
                
                break;
            }
			case CCBReader::PropertyType::POINT://kCCBPropTypePoint:
            {
                CCPoint point = this->parsePropTypePoint(pNode, pParent, pCCBReader);
                
                if (propertyName == PROPERTY_ANCHORPOINT)
                {
                    pCCBParam->anchorPoint = point;
                }
                break;
            }
            case CCBReader::PropertyType::POINT_LOCK://kCCBPropTypePointLock:
            {
                this->parsePropTypePointLock(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::SIZE://kCCBPropTypeSize: 
			{
                CCSize size = this->parsePropTypeSize(pNode, pParent, pCCBReader);
                
                if (propertyName == PROPERTY_PREFEREDSIZE)
                {
                    pCCBParam->contentSize = size;
                }
                break;
            }
            case CCBReader::PropertyType::SCALE_LOCK://kCCBPropTypeScaleLock:
            {
                float * scaleLock = this->parsePropTypeScaleLock(pNode, pParent, pCCBReader, propertyName.c_str());
                
                if (propertyName == PROPERTY_SCALE)
                {
                    pCCBParam->scaleX = scaleLock[0];
                    pCCBParam->scaleY = scaleLock[1];
                }
                CC_SAFE_DELETE_ARRAY(scaleLock);
                break;
            }
            case CCBReader::PropertyType::FLOAT://kCCBPropTypeFloat:
            {
                this->parsePropTypeFloat(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::FLOAT_XY://kCCBPropTypeFloatXY:
            {
                float * xy =  this->parsePropTypeFloatXY(pNode, pParent, pCCBReader);
                if (propertyName == PROPERTY_SKEW)
                {
                    pCCBParam->skewX = xy[0];
                    pCCBParam->skewY = xy[1];
                }
                CC_SAFE_DELETE_ARRAY(xy);
                break;
            }
                
            case CCBReader::PropertyType::DEGREES://kCCBPropTypeDegrees:
            {
                this->parsePropTypeDegrees(pNode, pParent, pCCBReader, propertyName.c_str());
                break;
            }
            case CCBReader::PropertyType::FLOAT_SCALE://kCCBPropTypeFloatScale:
            {
                float floatScale = this->parsePropTypeFloatScale(pNode, pParent, pCCBReader);
                if(propertyName == PROPERTY_TITLETTFSIZE_NORMAL)
                {
                    pCCBParam->nFontSize = (int)floatScale;
                }
                break;
            }
            case CCBReader::PropertyType::INTEGER://kCCBPropTypeInteger:
            {
                this->parsePropTypeInteger(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::INTEGER_LABELED://kCCBPropTypeIntegerLabeled:
            {
                this->parsePropTypeIntegerLabeled(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::FLOAT_VAR://kCCBPropTypeFloatVar:
            {
                float * floatVar = this->parsePropTypeFloatVar(pNode, pParent, pCCBReader);
                CC_SAFE_DELETE_ARRAY(floatVar);
                break;
            }
            case CCBReader::PropertyType::CHECK://kCCBPropTypeCheck:
            {
                bool check = this->parsePropTypeCheck(pNode, pParent, pCCBReader, propertyName.c_str());
                if(propertyName == PROPERTY_VISIBLE)
                {
                    pCCBParam->visible = check;
                }
                else if(propertyName == PROPERTY_IGNOREANCHORPOINTFORPOSITION)
                {
                    pCCBParam->ignoreAnchorPointForPosition = check;
                }
                break;
            }
            case CCBReader::PropertyType::SPRITEFRAME://kCCBPropTypeSpriteFrame:
            {
                SpriteFrame * ccSpriteFrame = this->parsePropTypeSpriteFrame(pNode, pParent, pCCBReader, propertyName.c_str());
                Scale9Sprite* pScale9Sprite = Scale9Sprite::createWithSpriteFrame(ccSpriteFrame);
                
                if(propertyName == PROPERTY_BACKGROUNDSPRITEFRAME_NORMAL) 
                {
                    pCCBParam->pNormalSprite = pScale9Sprite;
                    CC_SAFE_RETAIN(pScale9Sprite);
                }
                else if(propertyName == PROPERTY_BACKGROUNDSPRITEFRAME_HIGHLIGHTED)
                {
                    pCCBParam->pPressedSprite = pScale9Sprite;
                    CC_SAFE_RETAIN(pScale9Sprite);
                }
                else if(propertyName == PROPERTY_BACKGROUNDSPRITEFRAME_DISABLED) 
                {
                    pCCBParam->pDisabledSprite = pScale9Sprite;
                    CC_SAFE_RETAIN(pScale9Sprite);
                }
                break;
            }
            case CCBReader::PropertyType::ANIMATION://kCCBPropTypeAnimation:
            {
                this->parsePropTypeAnimation(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::TEXTURE://kCCBPropTypeTexture:
            {
                this->parsePropTypeTexture(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::BYTE://kCCBPropTypeByte:
            {
                this->parsePropTypeByte(pNode, pParent, pCCBReader, propertyName.c_str());
                break;
            }
            case CCBReader::PropertyType::COLOR3://kCCBPropTypeColor3:
            {
                ccColor3B color3B = this->parsePropTypeColor3(pNode, pParent, pCCBReader, propertyName.c_str());
                if(propertyName == PROPERTY_TITLECOLOR_NORMAL)
                {
                    pCCBParam->fontColor = color3B;
                }
                break;
            }
            case CCBReader::PropertyType::COLOR4F_VAR://kCCBPropTypeColor4FVar:
            {
                ccColor4F * color4FVar = this->parsePropTypeColor4FVar(pNode, pParent, pCCBReader);
                CC_SAFE_DELETE_ARRAY(color4FVar);
                break;
            }
            case CCBReader::PropertyType::FLIP://kCCBPropTypeFlip: 
			{
                bool * flip = this->parsePropTypeFlip(pNode, pParent, pCCBReader);
                CC_SAFE_DELETE_ARRAY(flip);
                break;
            }
            case CCBReader::PropertyType::BLEND_MODE://kCCBPropTypeBlendmode:
            {
                this->parsePropTypeBlendFunc(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::FNT_FILE: //kCCBPropTypeFntFile:
            {
                std::string fntFile = pCCBReader->getCCBRootPath() + this->parsePropTypeFntFile(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::FONT_TTF://kCCBPropTypeFontTTF:
            {
                std::string fontTTF = this->parsePropTypeFontTTF(pNode, pParent, pCCBReader);
                if(propertyName == PROPERTY_TITLETTF_NORMAL)
                {
                    pCCBParam->strFont = fontTTF;
                }
                break;
            }
            case CCBReader::PropertyType::STRING://kCCBPropTypeString:
            {
                std::string string = this->parsePropTypeString(pNode, pParent, pCCBReader);
                if(propertyName == PROPERTY_TITLE_NORMAL)
                {
                    pCCBParam->strText = string;

                }
                break;
            }
            case CCBReader::PropertyType::TEXT://kCCBPropTypeText:
            {
                std::string text = this->parsePropTypeText(pNode, pParent, pCCBReader);
                break;
            }
            case CCBReader::PropertyType::BLOCK://kCCBPropTypeBlock: 
			{
                BlockData * blockData = this->parsePropTypeBlock(pNode, pParent, pCCBReader);
                CC_SAFE_DELETE(blockData);
                break;
            }
            case CCBReader::PropertyType::BLOCK_CONTROL://kCCBPropTypeBlockCCControl: 
			{
                BlockControlData * blockCCControlData = this->parsePropTypeBlockControl(pNode, pParent, pCCBReader);
                CC_SAFE_DELETE(blockCCControlData);
                break;
            }
            case CCBReader::PropertyType::CCB_FILE: //kCCBPropTypeCCBFile: 
			{
                this->parsePropTypeCCBFile(pNode, pParent, pCCBReader);
                break;
            }
            default:
                ASSERT_FAIL_UNEXPECTED_PROPERTYTYPE(type);
                break;
        }
    }
}
