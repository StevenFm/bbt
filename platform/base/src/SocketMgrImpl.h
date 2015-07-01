#pragma once

#include "SocketHelper.h"

struct XSocketNode
{
    BOOL    bListener;
    SOCKET  nSocket;
    BOOL    bUsrClose;
	int		nSendRetryCount;
	int		nSendRetrySleep;
    void*   pvUsrData;
};

struct XListenNode : XSocketNode
{
    XSocketAcceptCallback*  pCallback;
    size_t                  uMaxRecvPackSize;
    XListenNode*            pNext;
};

struct XStreamNode : XSocketNode
{
    BYTE*                   pbyBuffer;      // recv buffer
    size_t                  uBufferSize;
    BYTE*                   pbyDataBegin;   // recv data
    BYTE*                   pbyDataEnd;
    char                    szRemoteIP[32];
    int                     nRemotePort;
    BOOL                    bErrNotified;
    int                     nSysErr;
    int                     nUsrErr;
    int                     nTimeoutSeconds;
    time_t                  uLastPackTime;
    XSocketDataCallback*    pDataProc;
    XSocketErrorCallback*   pErrProc;
    XStreamNode*            pNext;

#ifdef _MSC_VER
    volatile BOOL           bComplete;
    WSABUF                  wsBuf;
    WSAOVERLAPPED           WSOVL;
#endif
};

class XSocketMgr : public ISocketMgr
{
public:
    XSocketMgr();
	~XSocketMgr();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return -1; };
    virtual u_long  STDMETHODCALLTYPE AddRef(void) {  return XY_InterlockedIncrement(&m_ulRefCount); };
    virtual u_long  STDMETHODCALLTYPE Release(void);

    virtual BOOL        Setup(int nMaxListen, int nMaxStream);
    virtual void        Clear();

    virtual XSocket*    Listen(const char cszIP[], int nPort, size_t uMaxRecvPackSize, size_t uRecvBufferSize);
    virtual XSocket*    Connect(const char cszIP[], int nPort, int nTimeout, size_t uMaxPackLen, size_t uRecvBufferSize);

    virtual void        SetAcceptCallback(XSocket* pSocket, XSocketAcceptCallback* pAcceptProc);
    virtual void        SetStreamDataCallback(XSocket* pSocket, XSocketDataCallback* pDataProc);
    virtual void        SetStreamErrorCallback(XSocket* pSocket, XSocketErrorCallback* pErrProc);

	// 设置系统缓冲区满导致发送失败后的重试的次数,默认为0,Retry前会sleep nSleep(默认1)毫秒
	void				SetSendRetryCount(XSocket* pSocket, int nCount, int nSleep);

    virtual void        SetUsrData(XSocket* pSocket, void* pvUsrData);
    virtual void*       GetUsrData(XSocket* pSocket);

    virtual BOOL        SetSendBufferSize(XSocket* pSocket, size_t uBufferSize);
    virtual BOOL        SetRecvBufferSize(XSocket* pSocket, size_t uBufferSize);

    virtual void        SetStreamTimeout(XSocket* pSocket, int nSeconds);
    virtual void        SetLinger(XSocket* pSocket, BOOL bEnable, int nSeconds);

    virtual BOOL        Send(XSocket* pSocket, void* pvData, size_t uDataLen);
    virtual void        CloseSocket(XSocket* pSocket);

    virtual const char* GetStreamRemoteIP(XSocket* pSocket);
    virtual int         GetStreamRemotePort(XSocket* pSocket);

	virtual void		SetQueryTimeout(int nTimeout);
    virtual int			Query();

private:
    int                 ProcessNewSocket(XListenNode* pNode, time_t uTimeNow);
    XStreamNode*        CreateStream(SOCKET nSocket, const char cszRemoteIP[], int nRemotePort, time_t uTimeNow, size_t uMaxPackLen);
    BOOL                ProcessReceive(XStreamNode* pNode, time_t uTimeNow);
    void                ProcessCycle(time_t uTimeNow);
    BOOL                StreamSend(XStreamNode* pNode, BYTE* pbyData, size_t uDataLen);

    volatile u_long     m_ulRefCount;

#ifdef __linux
    int                 m_nEpoll;
    epoll_event*        m_pEventTable;
    int                 m_nEventCount;
#endif

#ifdef __APPLE__
	int					m_nKqueue;
	struct kevent*		m_pEventTable;
	int					m_nEventCount;
#endif

    XListenNode*        m_pListenNodeArray;
    XListenNode*        m_pListenNodeFree;
    XListenNode         m_ListenHeadNode;

    XStreamNode*        m_pStreamNodeArray;
    XStreamNode*        m_pStreamNodeFree;
    XStreamNode         m_StreamHeadNode;

    time_t              m_uNextCycleProcess;
	int					m_nQueryTimeout;
};

