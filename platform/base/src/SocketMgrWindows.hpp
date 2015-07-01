
#pragma comment(lib, "Ws2_32.lib")

static void WINAPI IOCompletionCallBack(DWORD dwErrorCode, DWORD dwTransferedBytes, LPOVERLAPPED lpOverlapped)
{
    XStreamNode* pNode = CONTAINING_RECORD(lpOverlapped, XStreamNode, WSOVL);

    assert(!pNode->bListener);
    assert(!pNode->bComplete);
    assert(lpOverlapped);

    assert(dwTransferedBytes <= (DWORD)(pNode->pbyBuffer + pNode->uBufferSize - pNode->pbyDataEnd));

    pNode->pbyDataEnd += dwTransferedBytes;

    if (dwTransferedBytes == 0)
    {
        pNode->nUsrErr = eSocketDisconnect;
    }

    pNode->bComplete = true;
}

BOOL XSocketMgr::Setup(int nMaxListen, int nMaxStream)
{
    BOOL    bResult     = false;
    int     nRetCode    = 0;
    WORD    wVersion    = MAKEWORD(2, 2);
    WSADATA wsaData;

    m_pListenNodeArray = new XListenNode[nMaxListen];
    XY_FAILED_JUMP(m_pListenNodeArray);

    for (int i = 0; i < nMaxListen; i++)
    {
        XListenNode* pNode = &m_pListenNodeArray[i];

        pNode->pNext = m_pListenNodeFree;
        m_pListenNodeFree = pNode;
    }

    m_pStreamNodeArray = new XStreamNode[nMaxStream];
    XY_FAILED_JUMP(m_pStreamNodeArray);

    for (int i = 0; i < nMaxStream; i++)
    {
        XStreamNode* pNode = &m_pStreamNodeArray[i];

        pNode->pNext = m_pStreamNodeFree;
        m_pStreamNodeFree = pNode;
    }

    nRetCode = WSAStartup(wVersion, &wsaData);
    XY_FAILED_JUMP(nRetCode == 0);

    bResult = true;
Exit0:
    return bResult;
}

void XSocketMgr::Clear()
{
    XListenNode* pListen = m_ListenHeadNode.pNext;
    while (pListen)
    {
        if (pListen->nSocket != INVALID_SOCKET)
        {
            closesocket(pListen->nSocket);
            pListen->nSocket = INVALID_SOCKET;
            pListen->bUsrClose = true;
        }
        pListen = pListen->pNext;
    }

    XStreamNode* pStream = m_StreamHeadNode.pNext;
    while (pStream)
    {
        if (pStream->nSocket != INVALID_SOCKET)
        {
            closesocket(pStream->nSocket);
            pStream->nSocket = INVALID_SOCKET;
            pStream->bUsrClose = true;
        }
        pStream = pStream->pNext;
    }

    while (m_StreamHeadNode.pNext)
    {
        XStreamNode* pStreamParent = &m_StreamHeadNode;
        XStreamNode* pStreamNode   = pStreamParent->pNext;

        // 等待IO Complete,删除缓冲区
        while (pStreamNode)
        {
            assert(pStreamNode->bUsrClose);
            assert(pStreamNode->nSocket == INVALID_SOCKET);
            if (pStreamNode->bComplete)
            {
                pStreamParent->pNext = pStreamNode->pNext;
                XY_DELETE_ARRAY(pStreamNode->pbyBuffer);
                pStreamNode = pStreamParent->pNext;
            }
            continue;
        }

        Sleep(0);
    }

    WSACleanup();

    XY_DELETE_ARRAY(m_pListenNodeArray);
    XY_DELETE_ARRAY(m_pStreamNodeArray);
}

XSocket* XSocketMgr::Listen(const char cszIP[], int nPort, size_t uMaxRecvPackSize, size_t uRecvBufferSize)
{
    XSocket*            pResult         = NULL;
    int                 nRetCode        = false;
    int                 nOne            = 1;
    unsigned long       ulAddress       = INADDR_ANY;
    SOCKET              nSocket         = INVALID_SOCKET;
    XListenNode*        pNode           = NULL;    
    sockaddr_in         localAddr;

    assert(cszIP);

    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    XY_FAILED_JUMP(nSocket != INVALID_SOCKET);

    if (cszIP[0] != '\0')
    {
        ulAddress = inet_addr(cszIP);
        if (ulAddress == INADDR_NONE)
            ulAddress = INADDR_ANY;
    }

	nRetCode = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nOne, sizeof(nOne));
    XY_FAILED_JUMP(nRetCode != SOCKET_ERROR);

    if (uRecvBufferSize > 0)
    {
        int nBufSize = (int)uRecvBufferSize; // 为了兼容性,用一个int做参数,而不是size_t

        nRetCode = setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, sizeof(nBufSize));
        XY_FAILED_JUMP(nRetCode == 0);
    }

	memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family        = AF_INET;
    localAddr.sin_addr.s_addr   = ulAddress;
    localAddr.sin_port          = htons(nPort);

    nRetCode = bind(nSocket, (sockaddr*)&localAddr, sizeof(localAddr));
    XYLOG_FAILED_JUMP(nRetCode != SOCKET_ERROR);                                      

    nRetCode = listen(nSocket, 8);
    XYLOG_FAILED_JUMP(nRetCode != SOCKET_ERROR);

    nRetCode = SetSocketNoneBlock(nSocket);
    XYLOG_FAILED_JUMP(nRetCode);

    XYLOG_FAILED_JUMP(m_pListenNodeFree);
    pNode = m_pListenNodeFree;
    m_pListenNodeFree = m_pListenNodeFree->pNext;

    pNode->bListener    = true;
    pNode->nSocket      = nSocket;
    pNode->bUsrClose    = false;
    pNode->pCallback    = NULL;
    pNode->pvUsrData    = NULL;
	pNode->nSendRetryCount	 = 0;
	pNode->nSendRetrySleep	 = 0;
    pNode->uMaxRecvPackSize  = uMaxRecvPackSize;

    pNode->pNext = m_ListenHeadNode.pNext;
    m_ListenHeadNode.pNext = pNode;

    pResult = pNode;
Exit0:
    if (pResult == NULL)
    {    
        if (nSocket != INVALID_SOCKET)
        {
            closesocket(nSocket);
            nSocket = INVALID_SOCKET;
        }
    }
    return pResult;
}

BOOL XSocketMgr::SetSendBufferSize(XSocket* pSocket, size_t uBufferSize)
{
    int          nOptRet = 0;
    XSocketNode* pNode   = (XSocketNode*)pSocket;

    assert(!pNode->bUsrClose);
    assert(uBufferSize > 0);

    nOptRet = setsockopt(pNode->nSocket, SOL_SOCKET, SO_SNDBUF, (char*)&uBufferSize, sizeof(uBufferSize));
    return (nOptRet == 0);
}

BOOL XSocketMgr::SetRecvBufferSize(XSocket* pSocket, size_t uBufferSize)
{
    int          nOptRet = 0;
    XSocketNode* pNode   = (XSocketNode*)pSocket;

    assert(!pNode->bUsrClose);
    assert(uBufferSize > 0);

    nOptRet = setsockopt(pNode->nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&uBufferSize, sizeof(uBufferSize));
    return (nOptRet == 0);
}

void XSocketMgr::CloseSocket(XSocket* pSocket)
{
    XSocketNode* pNode = (XSocketNode*)pSocket;

    if (pNode == NULL)
        return;

    assert(!pNode->bUsrClose);
    assert(pNode->nSocket != INVALID_SOCKET);

    // 在这里就关闭socket是为了使已投递的异步操作被取消
    // socket的缓冲区还不能释放,因为异步线程可能还在访问,缓冲区内存是在回收的时候释放的
    CloseSocketHandle(pNode->nSocket);
    pNode->nSocket = INVALID_SOCKET;

    pNode->bUsrClose = true;
}

int XSocketMgr::Query()
{
	int		nRetCode	= 0;
    time_t  nTimeNow    = time(NULL);
	int		nIOCount	= 0;

    XListenNode* pListenNode = m_ListenHeadNode.pNext;
    while (pListenNode)
    {
        if (!pListenNode->bUsrClose)
        {
            nRetCode = ProcessNewSocket(pListenNode, nTimeNow);
			if (nRetCode > 0)
				nIOCount++;
        }
        pListenNode = pListenNode->pNext;
    }

    XStreamNode* pStreamNode = m_StreamHeadNode.pNext;
    while (pStreamNode)
    {
        if (pStreamNode->bComplete && !pStreamNode->bUsrClose)
        {
            ProcessReceive(pStreamNode, nTimeNow);
			nIOCount++;
        }
        pStreamNode = pStreamNode->pNext;
    }

    if (nTimeNow > m_uNextCycleProcess)
    {
        ProcessCycle(nTimeNow);
        m_uNextCycleProcess = nTimeNow;
    }

	// Windows由于IOCP机制问题,这里只是近似的模拟epoll,kqueue的阻塞
	if (nIOCount == 0)
	{
		ThreadSleep(m_nQueryTimeout);
	}
	return nIOCount;
}

int XSocketMgr::ProcessNewSocket(XListenNode* pListen, time_t nTimeNow)
{
	int			nCount      = 0;
    int         nRetCode    = 0;
    sockaddr_in remoteAddr;

    while (!pListen->bUsrClose)
    {
        SOCKET         nSocket     = INVALID_SOCKET;
        int            nAddrLen    = sizeof(sockaddr_in);
        XStreamNode*   pNode       = NULL;

        memset(&remoteAddr, 0, sizeof(sockaddr_in));
        
        nSocket = accept(pListen->nSocket, (sockaddr*)&remoteAddr, &nAddrLen);
        if (nSocket == INVALID_SOCKET)
            break;

		nCount++;

        nRetCode = SetSocketNoneBlock(nSocket);
        if (!nRetCode)
        {
            closesocket(nSocket);
            continue;
        }

        pNode = CreateStream(
            nSocket, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), nTimeNow, pListen->uMaxRecvPackSize
        );
        if (pNode == NULL)
        {
            closesocket(nSocket);
            continue;
        }

		pNode->nSendRetryCount = pListen->nSendRetryCount;
		pNode->nSendRetrySleep = pListen->nSendRetrySleep;

        if (pListen->pCallback)
        {
            (*pListen->pCallback)(pListen, pListen->pvUsrData, pNode);
        }
    }

	return nCount;
}

XStreamNode* XSocketMgr::CreateStream(
    SOCKET nSocket, const char cszRemoteIP[], int nRemotePort, time_t nTimeNow, size_t uMaxPackLen
)
{
    XStreamNode*        pResult     = NULL;
    int                 nRetCode    = 0;
    int                 nSocketOpt  = 0;
    XStreamNode*        pNode       = NULL;
    BYTE*               pbyBuffer   = NULL;

    XY_FAILED_JUMP(m_pStreamNodeFree);
    pNode = m_pStreamNodeFree;
    m_pStreamNodeFree = m_pStreamNodeFree->pNext;

    pbyBuffer = new BYTE[uMaxPackLen];
    XY_FAILED_JUMP(pbyBuffer);

    nRetCode = BindIoCompletionCallback((HANDLE)nSocket, IOCompletionCallBack, 0);
    XY_FAILED_JUMP(nRetCode);

    nRetCode = SafeCopyString(pNode->szRemoteIP, sizeof(pNode->szRemoteIP), cszRemoteIP);
    XY_FAILED_JUMP(nRetCode);

    pNode->bListener        = false;
    pNode->nSocket          = nSocket;
    pNode->bUsrClose        = false;
    pNode->pbyBuffer        = pbyBuffer;
    pNode->uBufferSize      = uMaxPackLen;
    pNode->pbyDataBegin     = pbyBuffer;
    pNode->pbyDataEnd       = pbyBuffer;
    pNode->nRemotePort      = nRemotePort;
    pNode->bErrNotified     = false;
    pNode->nSysErr          = 0;
    pNode->nUsrErr          = 0;
    pNode->nTimeoutSeconds  = -1;
    pNode->uLastPackTime    = nTimeNow;
    pNode->pDataProc        = NULL;
    pNode->pErrProc         = NULL;
    pNode->pvUsrData        = NULL;
    pNode->bComplete        = true;
	pNode->nSendRetryCount	= 0;
	pNode->nSendRetrySleep	= 0;

    pNode->pNext = m_StreamHeadNode.pNext;
    m_StreamHeadNode.pNext = pNode;

    pResult = pNode;
Exit0:
    if (pResult == NULL)
    {
        XY_DELETE_ARRAY(pbyBuffer);

        if (pNode)
        {
            pNode->pNext = m_pStreamNodeFree;
            m_pStreamNodeFree = pNode;
            pNode = NULL;
        }
    }
    return pResult;
}

BOOL XSocketMgr::ProcessReceive(XStreamNode* pNode, time_t nTimeNow)
{
    BOOL		bResult             = false;
    int			nRetCode            = 0;
    size_t		uLeftBufferSize     = 0;
    size_t		uDataLen            = 0;
    size_t		uHeaderLen          = 0;
    uint64_t	uUserDataLen        = 0;
    DWORD		dwRecvBytes         = 0;
    DWORD		dwFlags             = 0;

    assert(pNode->bComplete);

    XY_FAILED_JUMP(pNode->nSysErr == 0);
    XY_FAILED_JUMP(pNode->nUsrErr == 0);

    while (!pNode->bUsrClose)
    {
        uDataLen = pNode->pbyDataEnd - pNode->pbyDataBegin;

        uHeaderLen = DecodeU64(&uUserDataLen, pNode->pbyDataBegin, uDataLen);
        if (uHeaderLen == 0)
            break;

        if (uHeaderLen + uUserDataLen > pNode->uBufferSize)
        {
            pNode->nUsrErr = eSocketStreamErr;
            goto Exit0;
        }

        if (uDataLen < uHeaderLen + uUserDataLen)
            break;

        pNode->uLastPackTime = nTimeNow;

        // call back
        if (pNode->pDataProc)
        {
            (*pNode->pDataProc)(pNode, pNode->pvUsrData, pNode->pbyDataBegin + uHeaderLen, (size_t)uUserDataLen);
        }

        pNode->pbyDataBegin += (uHeaderLen + uUserDataLen);
    }

    XY_FAILED_JUMP(!pNode->bUsrClose);

    // 如果后面的buffer空间放不下一个完整的包了,则把数据往前挪动
    uDataLen = pNode->pbyDataEnd - pNode->pbyDataBegin;
    uLeftBufferSize = (size_t)(pNode->pbyBuffer + pNode->uBufferSize - pNode->pbyDataBegin);

    uHeaderLen = DecodeU64(&uUserDataLen, pNode->pbyDataBegin, uDataLen);
    if (
        uLeftBufferSize <= MAX_HEADER_LEN ||
        (uHeaderLen > 0 && (pNode->pbyDataBegin + uHeaderLen + uUserDataLen > pNode->pbyBuffer + pNode->uBufferSize))
    )
    {
        memmove(pNode->pbyBuffer, pNode->pbyDataBegin, uDataLen);
        pNode->pbyDataBegin = pNode->pbyBuffer;
        pNode->pbyDataEnd = pNode->pbyDataBegin + uDataLen;
    }

    uLeftBufferSize = (size_t)(pNode->pbyBuffer + pNode->uBufferSize - pNode->pbyDataEnd);
    assert(uLeftBufferSize > 0);

    pNode->bComplete = false;

    pNode->wsBuf.len = (u_long)uLeftBufferSize;
    pNode->wsBuf.buf = (CHAR*)pNode->pbyDataEnd;

    memset(&pNode->WSOVL, 0, sizeof(pNode->WSOVL));

    nRetCode = WSARecv(pNode->nSocket, &pNode->wsBuf, 1, &dwRecvBytes, &dwFlags, &pNode->WSOVL, NULL);
    if (nRetCode == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr != WSA_IO_PENDING)
        {
            pNode->bComplete = true;
            pNode->nSysErr = nErr;
            goto Exit0;
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void XSocketMgr::ProcessCycle(time_t nTimeNow)
{
    XListenNode* pListenParent = &m_ListenHeadNode;
    XListenNode* pListenNode   = pListenParent->pNext;

    while (pListenNode)
    {
        if (pListenNode->bUsrClose)
        {
            assert(pListenNode->nSocket == INVALID_SOCKET);
            pListenParent->pNext = pListenNode->pNext;
            pListenNode->pNext = m_pListenNodeFree;
            m_pListenNodeFree = pListenNode;
            pListenNode = pListenParent->pNext;
            continue;
        }

        pListenParent = pListenNode;
        pListenNode = pListenNode->pNext;
    }

    XStreamNode* pStreamParent = &m_StreamHeadNode;
    XStreamNode* pStreamNode   = pStreamParent->pNext;

    while (pStreamNode)
    {
        if (pStreamNode->bUsrClose)
        {
            assert(pStreamNode->nSocket == INVALID_SOCKET);
            if (pStreamNode->bComplete)
            {
                pStreamParent->pNext = pStreamNode->pNext;
                XY_DELETE_ARRAY(pStreamNode->pbyBuffer);
                pStreamNode->pNext = m_pStreamNodeFree;
                m_pStreamNodeFree = pStreamNode;
                pStreamNode = pStreamParent->pNext;
            }
            continue;
        }

        if (pStreamNode->nTimeoutSeconds > 0 && !pStreamNode->bErrNotified)
        {
            if (nTimeNow > pStreamNode->uLastPackTime + pStreamNode->nTimeoutSeconds)
            {
                pStreamNode->nUsrErr = eSocketTimeout;
            }
        }

        if ((pStreamNode->nSysErr || pStreamNode->nUsrErr) && !pStreamNode->bErrNotified)
        {
            if (pStreamNode->pErrProc)
            {
                (*pStreamNode->pErrProc)(pStreamNode, pStreamNode->pvUsrData, pStreamNode->nSysErr, pStreamNode->nUsrErr);

                pStreamNode->bErrNotified = true;
            }
        }

        pStreamParent = pStreamNode;
        pStreamNode = pStreamNode->pNext;
    }
}
