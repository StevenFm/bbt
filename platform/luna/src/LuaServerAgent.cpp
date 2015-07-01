
#include "base/Base.h"
#include "Luna.h"
#include "LuaServerAgent.h"
#include "LuaPacker.h"

IMPL_LUA_CLASS_BEGIN(XConnectAgent)
	EXPORT_LUA_STD_STR_R(m_strIP)
	EXPORT_LUA_FUNCTION(LuaCall)
	EXPORT_LUA_FUNCTION(LuaClose)
IMPL_LUA_CLASS_END()


IMPL_LUA_CLASS_BEGIN(XLuaServer)
	EXPORT_LUA_STD_STR_R(m_strIP)
	EXPORT_LUA_FUNCTION(LuaBroadcast)
	EXPORT_LUA_FUNCTION(LuaGetAllAgents)
	EXPORT_LUA_FUNCTION(LuaListen)
IMPL_LUA_CLASS_END()

XConnectAgent::XConnectAgent(XLuaServer* pServerAgent, ISocketMgr* piSocketMgr, XSocket* pSocket, BYTE* pbyBuffer, size_t uBufferSize)
{
	piSocketMgr->AddRef();
	m_piSocketMgr = piSocketMgr;
	m_pSocket = pSocket;
	m_pbyBuffer = pbyBuffer;
	m_uBufferSize = uBufferSize;
	m_pServerAgent = pServerAgent;
}

XConnectAgent::~XConnectAgent()
{
	XY_COM_RELEASE(m_piSocketMgr);
}

int XConnectAgent::LuaClose(lua_State* L)
{
	m_piSocketMgr->CloseSocket(m_pSocket);
	m_pSocket = NULL;
	return 0;
}



int XConnectAgent::LuaCall(lua_State* L)
{
	BOOL        bResult         = false;
	int			nRetCode		= 0;
	int			nArgCount		= 0;
	int			nNameLen		= 0;
	size_t		uSize			= 0;
	const char* pszFuncName		= NULL;
	XLuaPaker	paker(m_uBufferSize);

	nArgCount = lua_gettop(L);
	XYLOG_FAILED_JUMP(nArgCount >= 1);

	pszFuncName = lua_tostring(L, 1);
	XYLOG_FAILED_JUMP(pszFuncName);

	nNameLen = (int)strlen(pszFuncName) + 1;
	XYLOG_FAILED_JUMP(nNameLen < (int)m_uBufferSize);

	memcpy(m_pbyBuffer, pszFuncName, nNameLen);

	nRetCode = paker.PushValue(L, 2, nArgCount - 1);
	XYLOG_FAILED_JUMP(nRetCode);

	nRetCode = paker.Save(&uSize, m_pbyBuffer + nNameLen, (size_t)(m_uBufferSize - nNameLen));
	XYLOG_FAILED_JUMP(nRetCode);

	nRetCode = m_piSocketMgr->Send(m_pSocket, m_pbyBuffer, (size_t)nNameLen + uSize);
	XYLOG_FAILED_JUMP(nRetCode);

	bResult = true;
Exit0:
	lua_pushboolean(L, bResult);
	return 1;
}


static void DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen)
{
	((XConnectAgent*)pvUsrData)->m_pServerAgent->DataCallback(pSocket, pvUsrData, pbyData, uDataLen);
}

static void ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr)
{
	((XConnectAgent*)pvUsrData)->m_pServerAgent->ErrorCallback(pSocket, pvUsrData, nSysErr, nUsrErr);
}

static void AcceptCallback(XSocket* pSocket, void* pvUsrData, XSocket* pNewSocket)
{
	((XLuaServer*)pvUsrData)->AcceptCallback(pNewSocket);
}

XLuaServer::XLuaServer()
{
	m_piSocketMgr = NULL;
}

XLuaServer::~XLuaServer()
{
	Clear();
}

BOOL XLuaServer::Setup(const char* pszModuleName, ISocketMgr* piSocketMgr, lua_State* L, const char* pszCallbackFile)
{
	BOOL				 bResult		= false;
	int					 nRetCode		= 0;

	piSocketMgr->AddRef();
	m_piSocketMgr = piSocketMgr;
	m_pLuaState = L;
	m_strCallbackScript = pszCallbackFile;
	m_strModuleName = pszModuleName;

	bResult = CallScript("Setup");
	XYLOG_FAILED_JUMP(bResult);

Exit0:
	return bResult;
}

void XLuaServer::Activate()
{
	CallScript("Activate");
}

BOOL XLuaServer::CallScript(const char szFunction[])
{
	BOOL			bRet = false;
	int				nRetCode = false;
	XLuaSafeStack	safeStack(m_pLuaState);

	nRetCode = Lua_GetFunction(m_pLuaState, m_strCallbackScript.c_str(), szFunction);
	XYLOG_FAILED_JUMP(nRetCode);

	nRetCode = Lua_XCall(safeStack, 0, 0);
	XYLOG_FAILED_JUMP(nRetCode);

	bRet = true;
Exit0:
	return bRet;
}

void XLuaServer::Clear()
{
// 	for (int i = 0; i < (int)m_ListenTable.size(); i++)
// 	{
// 		m_piSocketMgr->CloseSocket(m_ListenTable[i]);
// 	}
// 	m_ListenTable.clear();

	m_piSocketMgr->CloseSocket(m_pSocket);

	for (XConnectTable::iterator it = m_ConnectTable.begin(); it != m_ConnectTable.end(); ++it)
	{
		XY_DELETE(it->second);
	}
	m_ConnectTable.clear();

	XY_COM_RELEASE(m_piSocketMgr);
}

BOOL XLuaServer::Listen(const char* szIP, int nPort)
{
	BOOL				 bResult		= false;
	int					 nRetCode		= 0;
	int					 nCount			= 0;
		
	m_pSocket = m_piSocketMgr->Listen(szIP, nPort, XD_MAX_PACKAGE_SIZE);
	XYLOG_FAILED_JUMP(m_pSocket);

	Log(eLogInfo, "Listen %s:%d for %s connection !", szIP, nPort, m_strModuleName.c_str());

	m_piSocketMgr->SetSendRetryCount(m_pSocket, 3, 20);
	m_piSocketMgr->SetAcceptCallback(m_pSocket, ::AcceptCallback);
	m_piSocketMgr->SetUsrData(m_pSocket, this);

	bResult = true;
Exit0:
	return bResult;
}

// BOOL XLuaServer::Listen(XELISTEN_TYPE eListenType, int nPort)
// {
// 	BOOL				 bResult		= false;
// 	int					 nRetCode		= 0;
// 	int					 nCount			= 0;
// 	XNetworkAdapterTable adapterTable;
// 
// 	nRetCode = QueryNetworkAdaptersInfo(adapterTable);
// 	XYLOG_FAILED_JUMP(nRetCode);
// 
// 	nCount = (int)adapterTable.size();
// 
// 	for (int i = 0; i < nCount; i++)
// 	{
// 		XNetworkAdapterInfo& info = adapterTable[i];
// 		XSocket* pSocket = NULL;
// 
// 		nRetCode = IsInternalIP(info.strIpAddr.c_str());
// 		if ((eListenType == emLISTEN_INNER_IP && !nRetCode) ||
// 			(eListenType == emLISTEN_INTERNET_IP && nRetCode))	// 仅监听内网或外网IP
// 			continue;
// 
// 		pSocket = m_piSocketMgr->Listen(info.strIpAddr.c_str(), nPort, XD_MAX_PACKAGE_SIZE);
// 		if (pSocket == NULL)
// 			continue;
// 
// 		Log(eLogInfo, "Listen %s:%d for %s connection !", info.strIpAddr.c_str(), nPort, m_strModuleName.c_str());
// 
// 		m_piSocketMgr->SetSendRetryCount(pSocket, 3, 20);
// 		m_piSocketMgr->SetAcceptCallback(pSocket, ::AcceptCallback);
// 		m_piSocketMgr->SetUsrData(pSocket, this);
// 
// 		m_ListenTable.push_back(pSocket);
// 	}
// 
// 	bResult = true;
// Exit0:
// 	return bResult;
// }

int XLuaServer::LuaBroadcast(lua_State* L)
{
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

	for (XConnectTable::iterator it = m_ConnectTable.begin(); it != m_ConnectTable.end(); ++it)
	{
		m_piSocketMgr->Send(it->first, m_byBuffer, (size_t)nNameLen + uSize);
	}

Exit0:
	return 0;
}

int XLuaServer::LuaGetAllAgents(lua_State* L)
{
	int nIndex = 1;

	lua_newtable(L);

	for (XConnectTable::iterator it = m_ConnectTable.begin(); it != m_ConnectTable.end(); ++it)
	{
		lua_pushinteger(L, nIndex++);
		Lua_PushObject(L, it->second);
		lua_settable(L, -3);
	}

	return 1;
}

int XLuaServer::LuaListen(lua_State* L)
{
	BOOL bRet = 0;
	int nPort = 0;
	const char* pszIP = NULL;
	XYLOG_FAILED_JUMP(lua_gettop(L) == 2);
	XYLOG_FAILED_JUMP(lua_isstring(L, 1));
	XYLOG_FAILED_JUMP(lua_isnumber(L, 2));
	pszIP = lua_tostring(L, 1);
	nPort = (int)lua_tonumber(L, 2);

	bRet = Listen(pszIP, nPort);

Exit0:
	lua_pushboolean(L, bRet);
	return 1;
}

void XLuaServer::AcceptCallback(XSocket* pNewSocket)
{
	int 			nRetCode = 0;
	XConnectAgent*	pConnect 	 = new XConnectAgent(this, m_piSocketMgr, pNewSocket, m_byBuffer, sizeof(m_byBuffer));

	pConnect->m_strIP = m_piSocketMgr->GetStreamRemoteIP(pNewSocket);
	m_ConnectTable[pNewSocket] = pConnect;

	m_piSocketMgr->SetSendBufferSize(pNewSocket, XD_MAX_SOCKET_SEND_BUFFER_SIZE);
	m_piSocketMgr->SetRecvBufferSize(pNewSocket, XD_MAX_SOCKET_RECV_BUFFER_SIZE);
	m_piSocketMgr->SetStreamDataCallback(pNewSocket, ::DataCallback);
	m_piSocketMgr->SetStreamErrorCallback(pNewSocket, ::ErrorCallback);
	m_piSocketMgr->SetUsrData(pNewSocket, pConnect);

	XLuaSafeStack	safeStack(m_pLuaState);

	nRetCode = Lua_GetFunction(m_pLuaState, m_strCallbackScript.c_str(), "OnConnect");
	if (nRetCode)
	{
		Lua_PushObject(m_pLuaState, pConnect);
		Lua_XCall(safeStack, 1, 0);
	}	
}

void XLuaServer::DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen)
{
	int				nRetCode		= 0;
	XConnectAgent*	pConnect		= (XConnectAgent*)pvUsrData;
	BYTE*			pbyPos			= pbyData;
	BYTE*			pbyEnd			= pbyData + uDataLen;
	int				nArgCount		= 0;
	static XLuaUnpaker	unpaker(XD_MAX_PACKAGE_SIZE);

	XLuaSafeStack	safeStack(m_pLuaState);

	while (pbyPos < pbyEnd && *pbyPos)
		pbyPos++;
	XYLOG_FAILED_JUMP(pbyPos < pbyEnd);

	nRetCode = Lua_GetFunction(m_pLuaState, m_strCallbackScript.c_str(), (const char*)pbyData);
	if (!nRetCode)
	{
		Log(eLogDebug, "%s function is not found! %s", m_strCallbackScript.c_str(), (const char*)pbyData);
		goto Exit0;
	}

	Lua_PushObject(m_pLuaState, pConnect);

	pbyPos++; // skip '\0'
	nArgCount = unpaker.Expand(m_pLuaState, pbyPos, (size_t)(pbyEnd - pbyPos));

	nRetCode = Lua_XCall(safeStack, 1 + nArgCount, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	unpaker.Reset();
	return;
}

void XLuaServer::ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr)
{
	int				nRetCode	= 0;
	XConnectAgent*	pConnect		= (XConnectAgent*)pvUsrData;
	XLuaSafeStack	safeStack(m_pLuaState);

	nRetCode = Lua_GetFunction(m_pLuaState, m_strCallbackScript.c_str(), "OnDisconnect");
	if (nRetCode)
	{
		Lua_PushObject(m_pLuaState, pConnect);
		Lua_XCall(safeStack, 1, 0);
	}

	XY_DELETE(pConnect);
	m_ConnectTable.erase(pSocket);

	m_piSocketMgr->CloseSocket(pSocket);
	Log(eLogDebug, "%s lost, sys_err = %d, usr_err = %d !", m_strModuleName.c_str(), nSysErr, nUsrErr);
}

BOOL XLuaServer::HasConnection()
{
	return !m_ConnectTable.empty();
}
