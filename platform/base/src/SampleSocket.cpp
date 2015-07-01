#include "Base.h"
#include "SocketHelper.h"
#include "SampleSocket.h"
#include "XY_Time.h"

XSampleSocket::XSampleSocket(size_t uMaxRecvPackSize)
{
#ifdef _MSC_VER
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    m_uBufferSize = MAX_HEADER_LEN + uMaxRecvPackSize;
    m_pbyBuffer = (BYTE*)malloc(m_uBufferSize);
    m_nSocket = INVALID_SOCKET;
    m_pbyDataBegin = m_pbyBuffer;
    m_pbyDataEnd = m_pbyBuffer;
	m_nSendRetryCount = 0;
	m_nSendRetrySleep = 0;
	m_pConnectProc = NULL;
	m_pDataProc = NULL;
	m_pErrProc = NULL;
    m_nSequence = 0;
    m_eState = eClosed;

	m_pEncodeHandle = EncodeU64;
	m_pDecodeHandle = DecodeU64;
}

XSampleSocket::~XSampleSocket()
{
    Reset();
    XY_FREE(m_pbyBuffer);

#ifdef _MSC_VER
    WSACleanup();
#endif
}

BOOL XSampleSocket::Connect(const char szIP[], int nPort, int nTimeout)
{
    BOOL bResult = false;
    
    Reset();

    m_nSocket = ConnectSocket(szIP, nPort, nTimeout, 1024 * 8);
    XY_FAILED_JUMP(m_nSocket != INVALID_SOCKET);
    
    m_eState = eConnected;

    bResult = true;
Exit0:
    return bResult;
}

void XSampleSocket::ConnectAsync(const char szIP[], int nPort, int nTimeout)
{
    int                 nRetCode        = 0;
    SOCKET              nSocket         = INVALID_SOCKET;
    sockaddr_in         serverAddr;

    assert(szIP);

    Reset();
    
    m_eState = eConnecting;

    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    XY_FAILED_JUMP(nSocket != INVALID_SOCKET);

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_addr.s_addr  = (unsigned long)inet_addr(szIP);
    serverAddr.sin_port         = htons((uint16_t)nPort);

    nRetCode = SetSocketNoneBlock(nSocket);
    XY_FAILED_JUMP(nRetCode);

	nRetCode = connect(nSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (nRetCode == SOCKET_ERROR)
	{
        nRetCode = GetSocketError();

#ifdef _MSC_VER
		XY_FAILED_JUMP(nRetCode == WSAEWOULDBLOCK);
#endif

#if defined(__linux) || defined(__APPLE__)
		XY_FAILED_JUMP(nRetCode == EINPROGRESS);
#endif
	}

    m_dwConnectEndTime = nTimeout >= 0 ? XY_GetTickCount() + (DWORD)nTimeout : UINT_MAX;
    m_nSocket = nSocket;
Exit0:
    if (m_nSocket == INVALID_SOCKET)
    {
        if (nSocket != INVALID_SOCKET)
        {
            CloseSocketHandle(nSocket);
            nSocket = INVALID_SOCKET;
        }
    }
    return;
}

BOOL XSampleSocket::Send(void* pvData, size_t uDataLen)
{
    BOOL                bResult    = false;
    int                 nRetCode   = 0;
    size_t				uHeaderLen = 0;
    BYTE                byDataHeader[MAX_HEADER_LEN];
    
    XY_FAILED_JUMP(m_eState == eConnected);

	uHeaderLen = m_pEncodeHandle(byDataHeader, sizeof(byDataHeader), uDataLen);
	XY_FAILED_JUMP(uHeaderLen > 0);

    nRetCode = StreamSend(byDataHeader, uHeaderLen);
    XY_FAILED_JUMP(nRetCode);

    nRetCode = StreamSend((BYTE*)pvData, uDataLen);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void XSampleSocket::SetLinger(BOOL bEnable, int nSeconds)
{
    linger       param;

    param.l_onoff  = (u_short)bEnable;
    param.l_linger = (u_short)nSeconds;

    setsockopt(m_nSocket, SOL_SOCKET, SO_LINGER, (char*)&param, sizeof(param));
}

void XSampleSocket::Close()
{
    if (m_eState != eClosed)
    {
        m_eState = eUserClosed;
    }
}

BOOL XSampleSocket::SetSendBufferSize(size_t uBufferSize)
{
    int nOptRet = 0;

    assert(uBufferSize > 0);

    nOptRet = setsockopt(m_nSocket, SOL_SOCKET, SO_SNDBUF, (char*)&uBufferSize, sizeof(uBufferSize));
    return (nOptRet == 0);
}

BOOL XSampleSocket::SetRecvBufferSize(size_t uBufferSize)
{
    int nOptRet = 0;
    
    assert(uBufferSize > 0);
#ifdef __APPLE__ 
    nOptRet = setsockopt(m_nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&uBufferSize, sizeof(uBufferSize));
#endif
    
#ifdef _MSC_VER
    nOptRet = setsockopt(m_nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&uBufferSize, sizeof(uBufferSize));
#endif
    
#ifdef __linux
    nOptRet = setsockopt(m_nSocket, SOL_SOCKET, SO_RCVBUFFORCE, (char*)&uBufferSize, sizeof(uBufferSize));
#endif
    return (nOptRet == 0);
}

void XSampleSocket::SetSendRetryCount(int nCount, int nSleep)
{
	m_nSendRetryCount = nCount;
	m_nSendRetrySleep = nSleep;
}

void XSampleSocket::Activate()
{
    if (m_eState == eConnecting)
    {
        ProcessConnect();
    }
    
    if (m_eState == eConnected)
    {
        ProcessReceive();
    }
    
    if (m_eState == eError)
    {
        if (m_pErrProc)
        {
            Reset();
            (*m_pErrProc)(m_pvUsrData, m_nSysErr, m_nUsrErr);
        }
    }
    
    if (m_eState == eUserClosed)
    {
        Reset();
    }
}

void XSampleSocket::SetUserData(void* pvUsrData)
{
    m_pvUsrData = pvUsrData;
}

void XSampleSocket::SetConnectCallback(XSampleSocketConnectCallback* pConnectProc)
{
    m_pConnectProc = pConnectProc;
}

void XSampleSocket::SetDataCallback(XSampleSocketDataCallback* pDataProc)
{
    m_pDataProc = pDataProc;
}

void XSampleSocket::SetErrorCallback(XSampleSocketErrorCallback* pErrProc)
{
    m_pErrProc = pErrProc;
}

const char* XSampleSocket::GetState()
{
    switch (m_eState)
    {
    case eClosed:
        return "Closed";

    case XSampleSocket::eConnecting:
        return "Connecting";

    case XSampleSocket::eConnected:
        return "Connected";

    case XSampleSocket::eUserClosed:
        return "UserClosed";

    case XSampleSocket::eError:
        return "Error";

    default:
        return "Unkown";
    }
}

void XSampleSocket::SetError(int nSysErr, int nUsrErr)
{
    m_nSysErr = nSysErr;
    m_nUsrErr = nUsrErr;
    m_eState = eError;
}

void XSampleSocket::Reset()
{
    m_eState = eClosed;
    
    if (m_nSocket != INVALID_SOCKET)
    {
        CloseSocketHandle(m_nSocket);
        m_nSocket = INVALID_SOCKET;
    }
    
    m_pbyDataBegin = m_pbyBuffer;
    m_pbyDataEnd = m_pbyBuffer;
    
    m_nSequence++;
}

BOOL XSampleSocket::StreamSend(BYTE* pbyData, size_t uDataLen)
{
    int     nRetCode = 0;
	int		nCount	 = 0;
    
    if (m_eState != eConnected)
        return false;

    while (uDataLen != 0)
    {
		size_t uTrySend = Min(MAX_SIZE_PER_SEND, uDataLen);
        nRetCode = send(m_nSocket, (const char*)pbyData, uTrySend, 0);
        if (nRetCode == -1)
        {
            int nErr = GetSocketError();
            
#ifdef _MSC_VER
			if (nErr == WSAEWOULDBLOCK && nCount < m_nSendRetryCount)
			{
				nCount++;
				ThreadSleep(m_nSendRetrySleep);
				continue;
			}
#endif
            
#if defined(__linux) || defined(__APPLE__)
            if (nErr == EINTR)
                continue;

			if (nErr == EAGAIN && nCount < m_nSendRetryCount)
			{
				nCount++;
				ThreadSleep(m_nSendRetrySleep);
				continue;
			}
#endif
            
            SetError(nErr, 0);

            return false;
        }

        pbyData  += nRetCode;
        uDataLen -= nRetCode;
    }
    return true;
}

void XSampleSocket::ProcessReceive()
{
    int			nRetCode            = 0;
    size_t		uLeftBufferSize     = 0;
    size_t		uDataLen            = 0;
    size_t		uHeaderLen          = 0;
    uint64_t	uUserDataLen        = 0;
    int         nSequence           = m_nSequence;
    
    assert(m_eState == eConnected);
    
    while (true)
    {
        uLeftBufferSize = (size_t)(m_pbyBuffer + m_uBufferSize - m_pbyDataEnd);
        assert(uLeftBufferSize > 0); // 理论上不可能出现这种情况,除非是后面的缓冲区处理写错了:)
        
        nRetCode = recv(m_nSocket, (char*)m_pbyDataEnd, Min(uLeftBufferSize, MAX_SIZE_PER_RECV), 0);
        if (nRetCode == 0)
        {
            SetError(0, eSocketDisconnect);
            goto Exit0;
        }
        
        if (nRetCode < 0)
        {
            int nError = GetSocketError();
            
#ifdef _MSC_VER
            XY_FAILED_JUMP(nError != WSAEWOULDBLOCK);
#endif

#if defined(__linux) || defined(__APPLE__)
            if (nError == EINTR)
                continue;

            if (nError == EAGAIN || nError == EWOULDBLOCK)
                goto Exit0;
#endif
            
            SetError(nError, 0);
            goto Exit0;
        }
        
        assert(nRetCode <= (int)uLeftBufferSize);
        
        m_pbyDataEnd += nRetCode;
        
        while (true)
        {
            uDataLen = m_pbyDataEnd - m_pbyDataBegin;
            
			uHeaderLen = m_pDecodeHandle(&uUserDataLen, m_pbyDataBegin, uDataLen);
            if (uHeaderLen == 0)
                break;
            
            if (uHeaderLen + uUserDataLen > m_uBufferSize)
            {
                SetError(0, eSocketStreamErr);
                goto Exit0;
            }
            
            if (uDataLen < uHeaderLen + uUserDataLen)
                break;
            
            // 注意,在callback里面,用户可能重新重新发起connect
            // 如果用户调用了connect,那么就会重置m_pbyDataBegin,m_pbyDataEnd
            // 所以....,由于conect也可以是同步阻塞模式的,所以,这里光在callback后检查状态是不够的
            // 成员变量m_nSequence即是用来检测这种情况的
            if (m_pDataProc)
            {
                (*m_pDataProc)(m_pvUsrData, m_pbyDataBegin + uHeaderLen, (size_t)uUserDataLen);
                XY_FAILED_JUMP(m_eState == eConnected);
                XY_FAILED_JUMP(m_nSequence == nSequence);
            }
            
            m_pbyDataBegin += (uHeaderLen + uUserDataLen);
        }
        
        // 如果后面的buffer空间放不下一个完整的包了,则把数据往前挪动
        uDataLen = m_pbyDataEnd - m_pbyDataBegin;
        uLeftBufferSize = (size_t)(m_pbyBuffer + m_uBufferSize - m_pbyDataBegin);
        
		uHeaderLen = m_pDecodeHandle(&uUserDataLen, m_pbyDataBegin, uDataLen);
        if (
            uLeftBufferSize <= MAX_HEADER_LEN ||
            (uHeaderLen > 0 && (m_pbyDataBegin + uHeaderLen + uUserDataLen > m_pbyBuffer + m_uBufferSize))
        )
        {
            memmove(m_pbyBuffer, m_pbyDataBegin, uDataLen);
            m_pbyDataBegin = m_pbyBuffer;
            m_pbyDataEnd = m_pbyDataBegin + uDataLen;
        }
    }
    
Exit0:
    return;
}

void XSampleSocket::ProcessConnect()
{
	int				nRetCode	= 0;
	DWORD			dwTimeNow   = 0;
	int				nError		= 0;
	socklen_t		nErrorLen	= sizeof(nError);
	timeval			timeWait	= {0, 0};
	fd_set			writeSet;

	dwTimeNow = XY_GetTickCount();
	if (m_nSocket == INVALID_SOCKET || dwTimeNow > m_dwConnectEndTime)
    {
        Reset();
        if (m_pConnectProc)
        {
            (*m_pConnectProc)(m_pvUsrData, false);
        }
        goto Exit0;
    }

	FD_ZERO(&writeSet);
	FD_SET(m_nSocket, &writeSet);

	nRetCode = select((int)m_nSocket + 1, NULL, &writeSet, NULL, &timeWait);
	XY_FAILED_JUMP(nRetCode == 1);

	nRetCode = getsockopt((int)m_nSocket, SOL_SOCKET, SO_ERROR, (char*)&nError, &nErrorLen);
	if (nRetCode != 0 || nError != 0)
    {
        Reset();
        if (m_pConnectProc)
        {
            (*m_pConnectProc)(m_pvUsrData, false);
        }
        goto Exit0;
    }

    m_eState = eConnected;
    
	if (m_pConnectProc)
	{
		(*m_pConnectProc)(m_pvUsrData, true);
	}
    
Exit0:
	return;
}

BOOL XSampleSocket::IsAlive()
{
	return IsSocketAlive(m_nSocket); 
}


void XSampleSocket::SetHeaderEncodeHandle(XEncodeHandle* pEncodeHandle, XDecodeHandle* pDecodeHandle)
{
	assert(pEncodeHandle);
	assert(pDecodeHandle);
	m_pEncodeHandle = pEncodeHandle;
	m_pDecodeHandle = pDecodeHandle;
}
