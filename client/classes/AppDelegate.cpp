#include "stdafx.h"
#include "FileAssist.h"
#include "AppDelegate.h"
#include "ServerAgent.h"
#include "SimpleAudioEngine.h"
//#include "TouchDispatcher.h"
#include "UIManager.h"

USING_NS_CC;

BOOL LogAssist(void* pvUsrData, LogType eLevel, const char cszMsg[])
{
    char* pszDup = strdup(cszMsg);
    int nLen = (int)strlen(pszDup);
    
    // 因为CCLOG会强制在后面加个'\n',为了输出显示的问题,去掉原来的'\n'
    if (nLen > 0 && pszDup[nLen - 1] == '\n')
        pszDup[nLen - 1] = '\0';
    
    switch (eLevel)
    {
        case eLogError:
            // cocos2d::CCLog(pszDup); 会有编译警告 format string is not a string literal
            log("%s", pszDup);
            break;
            
        case eLogWarning:
            log("%s", pszDup);
            break;
            
        case eLogInfo:
            log("%s", pszDup);
            break;
            
        default:
            log("%s", pszDup);
            break;
    }
    
    XY_FREE(pszDup);
    fflush(stdout);
    
#ifdef _MSC_VER // Win下写File方便调试
    return true;
#else 
    return false; // do not write file
#endif
}


AppDelegate::AppDelegate() {
    g_pFilePackageMulti = new XFilePackageMulti();
    
#if !defined(_DEBUG) && !defined(_MSC_VER)
    std::string strWritePath = CCFileUtils::sharedFileUtils()->getWritablePath();
    strWritePath.append("/lastLog.log");
    
    freopen(strWritePath.c_str(), "w", stdout);
#endif
    
    g_pFileHelper = new XFileAssist();
    CCLOG("File assist sucess:%d", g_pFileHelper != NULL);
    
    BOOL bRetCode = LogOpen("bbt", 10000);
    if (bRetCode)
    {
        LogHook(NULL, &LogAssist);
    }
    else
    {
        CCLOG("LogOpen failed!");
    }
    
    Log(eLogInfo, "%s", "build at " __TIME__ " " __DATE__);
}

AppDelegate::~AppDelegate() {
	XY_DELETE(g_UIManager);
    CC_SAFE_RELEASE_NULL(g_pServerAgent);
    LogClose();
    XY_DELETE(g_pFileHelper);
}

typedef struct tagResource{
    CCSize size;
    char directory[100];
}Resource;

static Resource		ipadResource			= {Size(1280, 720),"resources-ipad"};
static Resource		ipadhdResource			= {Size(2048, 1536),"resources-ipadhd"};
static Resource		ccbiResource			= {Size(1280, 720), "ccbis"};
static Size			designResolutionSize	= Size(1280, 720);

bool AppDelegate::applicationDidFinishLaunching() {
    bool bResult = false;
    bool bRetCode = false;
    // initialize director
	auto director = Director::getInstance();
	auto glview = director->getOpenGLView();
	if(!glview) {
		glview = GLViewImpl::create("My Game");
		director->setOpenGLView(glview);
	}

	director->setDisplayStats(true);
	director->setAnimationInterval(1.0 / 60);

	Size screenSize = glview->getFrameSize();
	glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, kResolutionShowAll);

	std::vector<std::string> resDirOrders;
	resDirOrders.push_back(ccbiResource.directory);
	FileUtils::sharedFileUtils()->setSearchResolutionsOrder(resDirOrders);

	/*
    Director* pDirector = Director::sharedDirector();
    GLView* pEGLView	= GLView::sharedOpenGLView();
	XTouchDispatcher*       pTouchDispatche     = XTouchDispatcher::create();
	
    pDirector->setTouchDispatcher(pTouchDispatche); // 注意顺序，setTouchDispatcher要在setOpenGLView前面
    
    pDirector->setOpenGLView(pEGLView);
    pEGLView->setDesignResolutionSize(designResolutionSize.width,
                                          designResolutionSize.height,
                                          kResolutionShowAll);

    CCSize frameSize=pEGLView->getFrameSize();

        std::vector<std::string> searchPaths;
        std::vector<std::string> resDirOrders;
    
//    if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
        resDirOrders.push_back(ipadResource.directory);
        resDirOrders.push_back(ccbiResource.directory);
        pDirector->setContentScaleFactor(ipadResource.size.width/designResolutionSize.width);
//    }else if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
//        if (frameSize.width>ipadResource.size.width) {
//            resDirOrders.push_back(ipadhdResource.directory);
//            pDirector->setContentScaleFactor(ipadhdResource.size.width/designResolutionSize.width);
//        }else{
//            resDirOrders.push_back(ipadResource.directory);
//            pDirector->setContentScaleFactor(ipadResource.size.width/designResolutionSize.width);
//        }
//    }else{
//        resDirOrders.push_back(ipadResource.directory);
//        pDirector->setContentScaleFactor(ipadResource.size.width/designResolutionSize.width);
//    }
    CCFileUtils::sharedFileUtils()->setSearchResolutionsOrder(resDirOrders);

    // turn on display FPS
    pDirector->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);
    */
	g_UIManager = new UIManager();
    g_pServerAgent = XServerAgent::create();
    XYLOG_FAILED_JUMP(g_pServerAgent);
    CC_SAFE_RETAIN(g_pServerAgent);
    
    bRetCode = (bool)g_pServerAgent->Setup();
    XYLOG_FAILED_JUMP(bRetCode);

	/*
	RegisterLoader();
	// create a scene. it's an autorelease object
	pScene = StartScene::createScene();
	// run
	pDirector->runWithScene(pScene);
	*/


	g_UIManager->Init();  

	bResult = true;
Exit0:
    return bResult;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    CCDirector::sharedDirector()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseAllEffects();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    CCDirector::sharedDirector()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeAllEffects();
}
