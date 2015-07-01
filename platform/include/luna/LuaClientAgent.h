
#pragma 

#define XD_MAX_SOCKET_SEND_BUFFER_SIZE		(1024 * 256)
#define XD_MAX_SOCKET_RECV_BUFFER_SIZE		(1024 * 128)
#define XD_MAX_PACKAGE_SIZE					(1024 * 1024)


class XLuaClient
{
public:
	XLuaClient();
	~XLuaClient();

	BOOL Setup(const char* pszModuleName, ISocketMgr* piSocketMgr, lua_State* L, const char* pszCallbackScript);
	void Clear();

	void DataCallback(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen);
	void ErrorCallback(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr);

	BOOL Connect(const char* pszIP, int nPort);
	void Close();

	int LuaCall(lua_State* L);
	int LuaConnect(lua_State* L);
	int LuaClose(lua_State* L);
	
	BOOL CallScript(const char szFunction[]);

	void Activate();

	DECLARE_LUA_CLASS(XLuaClient);

private:
	ISocketMgr* m_piSocketMgr;
	XSocket*	m_pSocket;
	BYTE		m_byBuffer[XD_MAX_PACKAGE_SIZE];
	time_t		m_nNextConnectTime;

	lua_State*	m_pLuaState;
	std::string	m_strModuleName;
	std::string m_strCallbackScript;

};

