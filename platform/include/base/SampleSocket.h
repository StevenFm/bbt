#pragma once

typedef void (XSampleSocketConnectCallback)(void* pvUsrData, BOOL bConnected);
typedef void (XSampleSocketDataCallback)(void* pvUsrData, BYTE* pbyData, size_t uDataLen);
typedef void (XSampleSocketErrorCallback)(void* pvUsrData, int nSysErr, int nUsrErr);

typedef size_t XEncodeHandle(BYTE* pbyBuffer, size_t uBufferSize, uint64_t uValue);
typedef size_t XDecodeHandle(uint64_t* puValue, const BYTE* pbyData, size_t uDataLen);

// SampleSocket是可以重入的,也就是说,用户可以在回调函数里面Close或者Connect...

class XSampleSocket
{
public:
    XSampleSocket(size_t uMaxRecvPackSize = USHRT_MAX);
    ~XSampleSocket();

    // 同步模式连接,不会调用ConnectCallback.
    BOOL Connect(const char szIP[], int nPort, int nTimeout/*单位: ms*/);

    void ConnectAsync(const char szIP[], int nPort, int nTimeout/*单位: ms*/);

    BOOL Send(void* pvData, size_t uDataLen);

    // 默认disable,单位: s
    void SetLinger(BOOL bEnable, int nSeconds);

    void Close();

    BOOL SetSendBufferSize(size_t uBufferSize);
	BOOL SetRecvBufferSize(size_t uBufferSize);

	// 设置系统缓冲区满导致发送失败后的重试的次数,默认为0,Retry前会sleep nSleep(默认0)毫秒
	void SetSendRetryCount(int nCount, int nSleep);

    void Activate();

    void SetUserData(void* pvUsrData);
    void SetConnectCallback(XSampleSocketConnectCallback* pConnectProc);
    void SetDataCallback(XSampleSocketDataCallback* pDataProc);
    void SetErrorCallback(XSampleSocketErrorCallback* pErrProc);
    
    BOOL IsAvailable() { return m_eState == eConnected; }
    const char* GetState();

	BOOL IsAlive();

	void SetHeaderEncodeHandle(XEncodeHandle* pEncodeHandle, XDecodeHandle* pDecodeHandle);

private:
    void SetError(int nSysErr, int nUsrErr);
    void Reset();
    BOOL StreamSend(BYTE* pbyData, size_t uDataLen);
	void ProcessReceive();
	void ProcessConnect();

private:
    SOCKET              m_nSocket;
    BYTE*               m_pbyBuffer;
    size_t              m_uBufferSize;
    BYTE*               m_pbyDataBegin;
    BYTE*               m_pbyDataEnd;
	DWORD				m_dwConnectEndTime;
	int					m_nSendRetryCount;
	int					m_nSendRetrySleep;

    // 连接序列号,参见ProcessReceive的注释
    int                 m_nSequence;
    int                 m_nSysErr;
    int                 m_nUsrErr;

    void*                           m_pvUsrData;
	XSampleSocketConnectCallback*	m_pConnectProc;
	XSampleSocketDataCallback*		m_pDataProc;
	XSampleSocketErrorCallback*		m_pErrProc;
	XEncodeHandle*					m_pEncodeHandle;
	XDecodeHandle*					m_pDecodeHandle;
	    
    enum XState
    {
        eClosed,
        eConnecting,
        eConnected,
        eUserClosed,
        eError,
    } m_eState;
};
