#include "base/Base.h"
#include "luna/Luna.h"
#include "luna/LuaPacker.h"
#include "LuaClientAgent.h"

IMPL_LUA_CLASS_BEGIN(XLuaClient)
	EXPORT_LUA_FUNCTION(LuaCall)
	EXPORT_LUA_FUNCTION(LuaConnect)
	EXPORT_LUA_FUNCTION(LuaClose)
IMPL_LUA_CLASS_END()

static void DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen)
{
	((XLuaClient*)pvUsrData)->DataCallback(pSocket, pvUsrData, pbyData, uDataLen);
}

static void ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr)
{
	((XLuaClient*)pvUsrData)->ErrorCallback(pSocket, pvUsrData, nSysErr, nUsrErr);
}

XLuaClient::XLuaClient()
{
	m_piSocketMgr = NULL;
	m_pSocket = NULL;
	m_nNextConnectTime = 0;
}

XLuaClient::~XLuaClient()
{
	Clear();
}

BOOL XLuaClient::Setup(const char* pszModuleName, ISocketMgr* piSocketMgr, lua_State* L, const char* pszCallbackScript)
{
	BOOL bResult = false;

	piSocketMgr->AddRef();
	m_piSocketMgr = piSocketMgr;
	m_strModuleName = pszModuleName;
	m_pLuaState = L;
	m_strCallbackScript = pszCallbackScript;

	bResult = true;
	// Exit0:
	return bResult;
}

void XLuaClient::Clear()
{
	if (m_pSocket)
	{
		m_piSocketMgr->CloseSocket(m_pSocket);
		m_pSocket = NULL;
	}

	XY_COM_RELEASE(m_piSocketMgr);
}

void XLuaClient::Activate()
{
	CallScript("Activate");
}

BOOL XLuaClient::Connect(const char* pszIP, int nPort)
{
	BOOL bResult = false;

	assert(m_pSocket == NULL);

	m_pSocket = m_piSocketMgr->Connect(pszIP, nPort, 300, XD_MAX_PACKAGE_SIZE);
	XY_FAILED_JUMP(m_pSocket);

	m_piSocketMgr->SetSendBufferSize(m_pSocket, XD_MAX_SOCKET_SEND_BUFFER_SIZE);
	m_piSocketMgr->SetRecvBufferSize(m_pSocket, XD_MAX_SOCKET_RECV_BUFFER_SIZE);
	m_piSocketMgr->SetStreamDataCallback(m_pSocket, ::DataCallback);
	m_piSocketMgr->SetStreamErrorCallback(m_pSocket, ::ErrorCallback);
	m_piSocketMgr->SetUsrData(m_pSocket, this);

	CallScript("OnConnected");

	bResult = true;
Exit0:
	return bResult;
}

void XLuaClient::DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen)
{
	int				nRetCode		= 0;
	BYTE*			pbyPos			= pbyData;
	BYTE*			pbyEnd			= pbyData + uDataLen;
	int				nArgCount		= 0;
	static XLuaUnpaker		unpaker(XD_MAX_PACKAGE_SIZE);
	XLuaSafeStack	safeStack(m_pLuaState);

	while (pbyPos < pbyEnd && *pbyPos)
		pbyPos++;
	XYLOG_FAILED_JUMP(pbyPos < pbyEnd);

	nRetCode = Lua_GetFunction(m_pLuaState, m_strCallbackScript.c_str(), (char*)pbyData);
	if (!nRetCode)
	{
		Log(eLogDebug, "%s function is not found! %s", m_strCallbackScript.c_str(), (const char*)pbyData);
		goto Exit0;
	}

	pbyPos++; // skip '\0'
	nArgCount = unpaker.Expand(m_pLuaState, pbyPos, (size_t)(pbyEnd - pbyPos));

	nRetCode = Lua_XCall(safeStack, nArgCount, 0);
	XYLOG_FAILED_JUMP(nRetCode);

Exit0:
	unpaker.Reset();
	return;
}

void XLuaClient::ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr)
{
	m_piSocketMgr->CloseSocket(pSocket);
	m_pSocket = NULL;
	CallScript("OnDisconnected");
	Log(eLogDebug, "Gateway lost, sys_err = %d, usr_err = %d !", nSysErr, nUsrErr);
}

int XLuaClient::LuaCall(lua_State* L)
{
	BOOL        bResult         = false;
	int			nRetCode		= 0;
	int			nArgCount		= 0;
	int			nNameLen		= 0;
	size_t		uSize			= 0;
	const char* pszFuncName		= NULL;
	XLuaPaker	paker(sizeof(m_byBuffer));

	XYLOG_FAILED_JUMP(m_pSocket);

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

	nRetCode = m_piSocketMgr->Send(m_pSocket, m_byBuffer, (size_t)nNameLen + uSize);
	XYLOG_FAILED_JUMP(nRetCode);

	bResult = true;
Exit0:
	lua_pushboolean(L, bResult);
	return 1;
}

int XLuaClient::LuaConnect(lua_State* L)
{
	int nTop = lua_gettop(L);
	const char* pszIP = NULL;
	int nPort = 0;
	XYLOG_FAILED_JUMP(nTop >= 2);
	pszIP = lua_tostring(L, 1);
	nPort = (int)lua_tonumber(L, 2);
	// m_nNextConnectTime = time(NULL);
	Connect(pszIP, nPort);

	Log(eLogDebug, "%s Connect to IP %s:%d ... ... [%s]", m_strModuleName.c_str(), pszIP, nPort, m_pSocket ? "OK" : "Failed");
Exit0:
	lua_pushboolean(L, m_pSocket != NULL);
	return 1;
}

BOOL XLuaClient::CallScript(const char szFunction[])
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

void XLuaClient::Close()
{
	m_piSocketMgr->CloseSocket(m_pSocket);
	m_pSocket = NULL;
}

int XLuaClient::LuaClose(lua_State* L)
{
	Close();
	return 0;
}



