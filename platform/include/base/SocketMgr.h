#pragma once

enum SocketUsrErr
{
    eSocketNormal = 0,
    eSocketDisconnect,
    eSocketTimeout,
    eSocketStreamErr
};

typedef void XSocket;
typedef void (XSocketAcceptCallback)(XSocket* pListen, void* pvUsrData, XSocket* pNewSocket);
typedef void (XSocketDataCallback)(XSocket* pSocket, void* pvUsrData, BYTE* pbyData, size_t uDataLen);
typedef void (XSocketErrorCallback)(XSocket* pSocket, void* pvUsrData, int nSysErr, int nUsrErr);

struct ISocketMgr : IUnknown
{
    // uMaxRecvPackSize指应用层Recv数据包时每个包的最大大小, uRecvBufferSize指系统层(TCP)的接收缓冲区大小
    virtual XSocket*    Listen(const char cszIP[], int nPort, size_t uMaxRecvPackSize, size_t uRecvBufferSize = 0) = 0;
    virtual XSocket*    Connect(const char cszIP[], int nPort, int nTimeout /*单位: ms*/ , size_t uMaxPackLen, size_t uRecvBufferSize = 0) = 0;

    virtual void        SetAcceptCallback(XSocket* pSocket, XSocketAcceptCallback* pAcceptProc) = 0;
    virtual void        SetStreamDataCallback(XSocket* pSocket, XSocketDataCallback* pDataProc) = 0;
    virtual void        SetStreamErrorCallback(XSocket* pSocket, XSocketErrorCallback* pErrProc) = 0;

	// 设置系统缓冲区满导致发送失败后的重试的次数,默认为0,Retry前会sleep nSleep(默认0)毫秒
	virtual void		SetSendRetryCount(XSocket* pSocket, int nCount, int nSleep) = 0;

    virtual void        SetUsrData(XSocket* pSocket, void* pvUsrData) = 0;
    virtual void*       GetUsrData(XSocket* pSocket) = 0;

    virtual BOOL        SetSendBufferSize(XSocket* pSocket, size_t uBufferSize) = 0;
    // 在某些平台上,Recv buf大小并不能随时设置,最好是在Listen和Connect时设置
    virtual BOOL        SetRecvBufferSize(XSocket* pSocket, size_t uBufferSize) = 0;

    virtual void        SetStreamTimeout(XSocket* pSocket, int nSeconds) = 0;
    virtual void        SetLinger(XSocket* pSocket, BOOL bEnable, int nSeconds) = 0; // 默认disable,单位: s

    virtual BOOL        Send(XSocket* pSocket, void* pvData, size_t uDataLen) = 0;
    virtual void        CloseSocket(XSocket* pSocket) = 0;

    virtual const char* GetStreamRemoteIP(XSocket* pSocket) = 0; // never return null
    virtual int         GetStreamRemotePort(XSocket* pSocket) = 0;

	// 当没有任何IO调用成功,Query阻塞的时间,单位: ms,默认值0
	virtual void		SetQueryTimeout(int nTimeout) = 0;
	// 返回本次调用的IO系统调用(accept, recv)成功次数
	virtual int			Query() = 0;
};

ISocketMgr* CreateSocketMgr(int nMaxListen, int nMaxStream);
