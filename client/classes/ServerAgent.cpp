#include "stdafx.h"
#include "base/Base.h"
#include "luna/Luna.h"
#include "luna/LuaPacker.h"
#include "ServerAgent.h"
#include "ScriptDef.h"
#include "GameDef.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/third_party/android/prebuilt/libcurl/include/curl/curl.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "platform/third_party/ios/curl/curl.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include "curl/include/win32/curl/curl.h"
#endif

#include "WebClient.h"


USING_NS_CC;

XServerAgent* g_pServerAgent = NULL;
const int LEN_OF_NAME_BUFFER = 32;

IMPL_LUA_CLASS_BEGIN(XServerAgent)
    EXPORT_LUA_FUNCTION(LuaConnect)
    EXPORT_LUA_FUNCTION(LuaClose)
    EXPORT_LUA_FUNCTION(LuaCall)

	EXPORT_LUA_FUNCTION(LuaGetNetState)
	EXPORT_LUA_FUNCTION(LuaIsAlive)

	EXPORT_LUA_FUNCTION(LuaCreateClientAgent)
	EXPORT_LUA_FUNCTION(LuaCloseClientAgent)	

    EXPORT_LUA_INT_R(m_nLastCallTime)
IMPL_LUA_CLASS_END()



static void ConnectCallback(void*, BOOL bOK)
{
    g_pServerAgent->OnConnectComplete(bOK);
}

static void DataCallback(void*, BYTE* pbyData, size_t uDataLen)
{
    g_pServerAgent->OnDataCallback(pbyData, uDataLen);
}

static void ErrorCallback(void*, int nSysErr, int nUsrErr)
{
	Log(eLogDebug, "XServerAgent::ErrorCallback, nSysErr:%d, nUsrErr:%d", nSysErr, nUsrErr);
    g_pServerAgent->CallEvent("OnDisconnect");
}


static size_t _EncodeU64(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uValue)
{
	assert(uValue < 65536 && uBufferSize >= 2);
	pbyBuffer[0] = (uValue >> 8) & 0xff;
	pbyBuffer[1] = uValue & 0xff;
	return 2;
}

static size_t _DecodeU64(uint64_t* puValue, const BYTE* pbyData, size_t uDataLen)
{
	if (uDataLen < 2)
    {
		return 0;
    }

	*puValue = pbyData[0] << 8 | pbyData[1];
	return 2;
}

XServerAgent::XServerAgent() : m_Socket(CS_MAX_PACKAGE_SIZE)
{
    //m_Socket.SetHeaderEncodeHandle(EncodeU64, DecodeU64);
    m_Socket.SetConnectCallback(&ConnectCallback);
    m_Socket.SetDataCallback(&DataCallback);
    m_Socket.SetErrorCallback(&ErrorCallback);
    
    m_nLogicFrame = 0;
    m_nBeginTime = XY_GetTickCount();
    
    m_nLastCallTime = 0;
    m_pLuaState = NULL;

	this->scheduleUpdate();
}

XServerAgent::~XServerAgent()
{
    Clear();

	this->unscheduleUpdate();
}

BOOL XServerAgent::Setup()
{
    BOOL            bResult       = false;
    int             nRetCode      = false;
    CCDirector*     pDirector     = CCDirector::sharedDirector();
    
	XY_SetRandSeed((uint32_t)time(NULL));

	uint32_t uStateArray[WELL_RAND_STATE_COUNT];
	for (int i = 0; i < _countof(uStateArray); ++i)
		uStateArray[i] = XY_Rand();

	WellSetRandSeed(uStateArray);

	m_piSocketMgr = CreateSocketMgr(4, 16);
	XYLOG_FAILED_JUMP(m_piSocketMgr);

    m_pLuaState = luaL_newstate();
    XYLOG_FAILED_JUMP(m_pLuaState);
    
    pDirector->setNotificationNode(this);
    
    Lua_SetupEnv(m_pLuaState);

    SetupScriptFuncs(m_pLuaState);
    
    luaopen_base_webclient(m_pLuaState);

	nRetCode = Lua_LoadScript(m_pLuaState, "scripts/preload.lua");
	XYLOG_FAILED_JUMP(nRetCode);

    nRetCode = Lua_LoadScript(m_pLuaState, "scripts/main.lua");
    XYLOG_FAILED_JUMP(nRetCode);
    
	nRetCode = CallMainScript("Setup");
	XYLOG_FAILED_JUMP(nRetCode);
    
    bResult = true;
Exit0:
    return bResult;
}

void XServerAgent::Clear()
{
    CCDirector::sharedDirector()->setNotificationNode(NULL);
    
    LUA_CLEAR_REF();
    if (m_pLuaState)
    {
        lua_close(m_pLuaState);
        m_pLuaState = NULL;
    }
}


void XServerAgent::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, bool parentTransformUpdated)//update(float dt)
{
    //Node::visit();
	//Node::visit(renderer, parentTransform, parentTransformUpdated);
	log("-- visit --");
    
    m_Socket.Activate();
	m_piSocketMgr->Query();
        
    int64_t nTimeNow = XY_GetTickCount();
    if ((nTimeNow - m_nBeginTime) * GAME_LOGIC_FPS > m_nLogicFrame * 1000)
    {
        Activate();
        m_nLogicFrame++;
    }
}

void XServerAgent::update(float dt)
{
	m_Socket.Activate();
	m_piSocketMgr->Query();

	int64_t nTimeNow = XY_GetTickCount();
	if ((nTimeNow - m_nBeginTime) * GAME_LOGIC_FPS > m_nLogicFrame * 1000)
	{
		Activate();
		m_nLogicFrame++;
	}
}

void XServerAgent::Activate()
{
    XLuaSafeStack luaSafeStack(m_pLuaState);
    
    CallMainScript("Activate");

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	if (m_nLogicFrame % 4 == 0)
	{
		Lua_ReloadModifiedFiles(m_pLuaState);
	}
#endif
}

int XServerAgent::LuaConnect(lua_State* L)
{
	int			nArgCount		= 0;
	const char* pszIP           = NULL;
    int         nPort           = 0;
    int         nTimeout        = 3000;
    
	nArgCount = lua_gettop(L);
	XYLOG_FAILED_JUMP(nArgCount >= 2);
    
	pszIP = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszIP);
    
	nPort = lua_tointeger(L, 2);
    
    if (nArgCount >= 3)
    {
        nTimeout = lua_tointeger(L, 3);
    }
    
    m_strIP = pszIP;
    m_nPort = nPort;
 
    m_Socket.ConnectAsync(pszIP, nPort, nTimeout);
    m_Socket.SetRecvBufferSize(CS_MAX_SEND_SOCKET_BUFFER_SIZE);
    m_Socket.SetSendBufferSize(CS_MAX_RECV_SOCKET_BUFFER_SIZE);

Exit0:
	return 0;
}

int XServerAgent::LuaClose(lua_State* L)
{
    m_Socket.Close();
	return 0;
}

int XServerAgent::LuaCall(lua_State* L)
{
    BOOL        bResult         = false;
	int			nRetCode		= 0;
	int			nArgCount		= 0;
	int			nNameLen		= 0;
	size_t		uSize			= 0;
	const char* pszFuncName		= NULL;
	XLuaPaker	paker(sizeof(m_byBuffer));
    
	nArgCount = lua_gettop(L);
	XYLOG_FAILED_JUMP(nArgCount >= 1);
    
	pszFuncName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFuncName);
    
	nNameLen = (int)strlen(pszFuncName) + 1;
	XYLOG_FAILED_JUMP(nNameLen < sizeof(m_byBuffer));
    
	memcpy(m_byBuffer, pszFuncName, nNameLen);
    
	nRetCode = paker.PushValue(L, 2, nArgCount - 1);
	XYLOG_FAILED_JUMP(nRetCode);
    
	nRetCode = paker.Save(&uSize, m_byBuffer + nNameLen, (size_t)(sizeof(m_byBuffer) - nNameLen));
	XYLOG_FAILED_JUMP(nRetCode);
    
	nRetCode = m_Socket.Send(m_byBuffer, (size_t)nNameLen + uSize);
    XYLOG_FAILED_JUMP(nRetCode);

    m_nLastCallTime = time(NULL);

    bResult = true;
Exit0:
    lua_pushboolean(L, bResult);
	return 1;
}

int XServerAgent::LuaGetNetState(lua_State* L)
{
    const char* pszState = m_Socket.GetState();
    lua_pushstring(L, pszState);
    return 1;
}

int XServerAgent::LuaIsAlive(lua_State* L)
{
	BOOL bRetCode = m_Socket.IsAlive();
	
	lua_pushboolean(L, bRetCode);
	return 1;
}

void XServerAgent::OnConnectComplete(BOOL bOK)
{
    CallEvent(bOK ? "OnConnectOK" : "OnConnectFailed");
}

void XServerAgent::OnDataCallback(BYTE* pbyData, size_t uDataLen)
{
	int				nRetCode		= 0;
	BYTE*			pbyPos			= pbyData;
	BYTE*			pbyEnd			= pbyData + uDataLen;
	int				nArgCount		= 0;
	XLuaUnpaker		unpaker(CS_MAX_PACKAGE_SIZE);
	XLuaSafeStack	safeStack(m_pLuaState);
    
	while (pbyPos < pbyEnd && *pbyPos)
    {
		pbyPos++;
    }
	XYLOG_FAILED_JUMP(pbyPos < pbyEnd);
    
	nRetCode = Lua_GetFunction(m_pLuaState, "scripts/server_agent.lua", (char*)pbyData);
    if (!nRetCode)
    {
        Log(eLogError, "remote call server_agent:%s not exist !", (char*)pbyData);
        goto Exit0;
    }
    
	pbyPos++; // skip '\0'
	nArgCount = unpaker.Expand(m_pLuaState, pbyPos, (size_t)(pbyEnd - pbyPos));
    
	nRetCode = Lua_XCall(safeStack, nArgCount, 0);
    if (!nRetCode)
    {
        Log(eLogDebug, "remote call server_agent:%s failed !", (char*)pbyData);
    }
    
Exit0:
	return;
}

BOOL XServerAgent::CallMainScript(const char szFunction[])
{
    BOOL        bResult         = false;
	int			nRetCode		= 0;
    
	XLuaSafeStack   luaSafeStack(m_pLuaState);
    
	nRetCode = Lua_GetFunction(m_pLuaState, "scripts/main.lua", szFunction);
	XYLOG_FAILED_JUMP(nRetCode);
    
	nRetCode = Lua_XCall(luaSafeStack, 0, 0);
	XYLOG_FAILED_JUMP(nRetCode);
    
	bResult = true;
Exit0:
	return bResult;
}

void XServerAgent::CallEvent(const char szEvent[])
{
	XLuaSafeStack	safeStack(m_pLuaState);
    
    int nRetCode = Lua_GetObjFunction(m_pLuaState, this, szEvent);
    if (!nRetCode)
    {
        return;
    }
    
    Lua_PushObject(m_pLuaState, this);
    
    Lua_XCall(safeStack, 1, 0);
}

int XServerAgent::LuaCreateClientAgent(lua_State* L)
{
	const char* szModelName = NULL;
	const char* szScriptFile = NULL;
	XLuaClient* pClient = NULL;

	XYLOG_FAILED_JUMP(lua_gettop(L) >= 2);
	XYLOG_FAILED_JUMP(lua_isstring(L, 1));
	XYLOG_FAILED_JUMP(lua_isstring(L, 2));
	szModelName = lua_tostring(L, 1);
	szScriptFile = lua_tostring(L, 2);
	XYLOG_FAILED_JUMP(m_mapClient.find(szModelName) == m_mapClient.end());
	pClient = new XLuaClient();
	XYLOG_FAILED_JUMP(pClient);
	pClient->Setup(szModelName, m_piSocketMgr, L, szScriptFile);

Exit0:
	Lua_PushObject(L, pClient);
	if (pClient)
	{
		m_mapClient[szModelName] = pClient;
	}
	return 1;
}

int XServerAgent::LuaCloseClientAgent(lua_State* L)
{
	const char* szModelName = NULL;
	XLuaClient* pClient = NULL;
	std::map<std::string, XLuaClient*>::iterator itor;

	XYLOG_FAILED_JUMP(lua_gettop(L) >= 1);
	XYLOG_FAILED_JUMP(lua_isstring(L, 1));
	szModelName = lua_tostring(L, 1);
	itor = m_mapClient.find(szModelName);
	XYLOG_FAILED_JUMP(itor != m_mapClient.end());
	pClient = itor->second;
	pClient->Close();
	XY_DELETE(pClient);
	m_mapClient.erase(itor);
Exit0:
	return 0;
}
