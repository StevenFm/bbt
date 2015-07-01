#pragma once
#include "stdafx.h"
#define CS_MAX_SEND_SOCKET_BUFFER_SIZE	(1024 * 128)
#define CS_MAX_RECV_SOCKET_BUFFER_SIZE	(1024 * 128)
#define CS_MAX_PACKAGE_SIZE             (1024 * 96)
#include "luna/LuaClientAgent.h"

// 这个类的功能有点乱，需要重新规划一下
class XServerAgent : public cocos2d::Node
{
public:
	XServerAgent();
	virtual ~XServerAgent();

    virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, bool parentTransformUpdated);

	virtual void update(float dt);

    BOOL Setup();
    void Clear();
    
	void Activate();
    
    BOOL CallMainScript(const char szFunction[]);

	int LuaConnect(lua_State* L);
	int LuaClose(lua_State* L);
	int LuaCall(lua_State* L);
	int LuaGetNetState(lua_State* L);
	int LuaIsAlive(lua_State* L);
	int LuaCreateClientAgent(lua_State* L);
	int LuaCloseClientAgent(lua_State* L);

    void OnConnectComplete(BOOL bOK);

    void OnDataCallback(BYTE* pbyData, size_t uDataLen);
    
    void CallEvent(const char szEvent[]);
    
    void SetAssetsVersion(const std::string& strVersion) {m_strAssetsVersion = strVersion;};
    
    lua_State* GetLuaState(){return m_pLuaState;}
	int  GetLogicFrame()	{return m_nLogicFrame;}
    
    CREATE_FUNC(XServerAgent);
  	DECLARE_LUA_CLASS(XServerAgent);
private:
	ISocketMgr*				m_piSocketMgr;		// 模块连接
	XSampleSocket           m_Socket;			// 主连接
    BYTE                    m_byBuffer[CS_MAX_PACKAGE_SIZE];
    std::string             m_strIP;
    int                     m_nPort;
    int                     m_nLogicFrame;
    int64_t                 m_nBeginTime;
    std::string             m_strAssetsVersion;
    int                     m_nLastCallTime;
    lua_State*              m_pLuaState;
	std::map<std::string, XLuaClient*> m_mapClient;
};

extern XServerAgent* g_pServerAgent;

