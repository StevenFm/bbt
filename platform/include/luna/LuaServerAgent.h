#pragma once

#include "Luna.h"

#define XD_MAX_SOCKET_SEND_BUFFER_SIZE		(1024 * 256)
#define XD_MAX_SOCKET_RECV_BUFFER_SIZE		(1024 * 128)
//#define XD_MAX_PACKAGE_SIZE					(1024 * 1024 *16) ÄÚ´æ³Ô²»Ïû
#define XD_MAX_PACKAGE_SIZE					(1024 * 1024) 
// enum XELISTEN_TYPE
// {
// 	emLISTEN_ALL_IP = 0,
// 	emLISTEN_INNER_IP = 1,
// 	emLISTEN_INTERNET_IP = 2,
// };

class XLuaServer;

class XConnectAgent
{

public:
	XConnectAgent(XLuaServer* pServerAgent, ISocketMgr* piSocketMgr, XSocket* pSocket, BYTE* pbyBuffer, size_t uBufferSize);
	~XConnectAgent();

	int LuaCall(lua_State* L);
	int LuaClose(lua_State* L);

	std::string m_strIP;
	ISocketMgr*	m_piSocketMgr;
	XSocket*	m_pSocket;
	BYTE*		m_pbyBuffer;
	size_t		m_uBufferSize;

	XLuaServer* m_pServerAgent;

	DECLARE_LUA_CLASS(XConnectAgent);
};


class XLuaServer
{
public:
	XLuaServer();
	~XLuaServer();

	BOOL Setup(const char* pszModuleName, ISocketMgr* piSocketMgr, lua_State* L, const char* pszCallbackScript);
	void Activate();
	BOOL CallScript(const char szFunction[]);
	void Clear();

	BOOL Listen(const char* szIP, int nPort);
//	BOOL Listen(XELISTEN_TYPE eListenType, int nPort);

	int LuaBroadcast(lua_State* L);
	int LuaGetAllAgents(lua_State* L);
	int LuaListen(lua_State* L);

	void AcceptCallback(XSocket* pNewSocket);
	void DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen);
	void ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr);

	BOOL HasConnection();

	DECLARE_LUA_CLASS(XLuaServer);

private:
	BYTE		m_byBuffer[XD_MAX_PACKAGE_SIZE];
	ISocketMgr* m_piSocketMgr;
	lua_State*	m_pLuaState;

	std::string			m_strModuleName;
	std::string			m_strCallbackScript;

	std::string			m_strIP;
	// std::vector<XSocket*> m_ListenTable;
	XSocket*			m_pSocket;

	typedef std::map<XSocket*, XConnectAgent*> XConnectTable;
	XConnectTable	m_ConnectTable;
};

