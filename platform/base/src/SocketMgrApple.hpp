BOOL XSocketMgr::Setup(int nMaxListen, int nMaxStream)
{
    BOOL			bResult		= false;
    sig_t			pHandler	= NULL;

    pHandler = signal(SIGPIPE, SIG_IGN);
    XY_FAILED_JUMP(pHandler != SIG_ERR);

	m_nEventCount = nMaxListen + nMaxStream;
	m_pEventTable = (struct kevent*)malloc(sizeof(struct kevent) * m_nEventCount);
	XY_FAILED_JUMP(m_pEventTable);

	m_nKqueue = kqueue();
	XY_FAILED_JUMP(m_nKqueue != -1);
    
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
            close(pListen->nSocket);
            pListen->nSocket = INVALID_SOCKET;
        }
        pListen = pListen->pNext;
    }

    XStreamNode* pStream = m_StreamHeadNode.pNext;
    while (pStream)
    {
        if (pStream->nSocket != INVALID_SOCKET)
        {
            close(pStream->nSocket);
            pStream->nSocket = INVALID_SOCKET;
        }
        XY_DELETE_ARRAY(pStream->pbyBuffer);
        pStream = pStream->pNext;
    }

	XY_FREE(m_pEventTable);
	XY_CLOSE_FD(m_nKqueue);

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
	struct kevent		event;

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
    XY_FAILED_JUMP(nRetCode != SOCKET_ERROR);

    nRetCode = listen(nSocket, 8);
    XY_FAILED_JUMP(nRetCode != SOCKET_ERROR);

    XY_FAILED_JUMP(m_pListenNodeFree);
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

    nRetCode = SetSocketNoneBlock(nSocket);
    XY_FAILED_JUMP(nRetCode);

	EV_SET(&event, nSocket, EVFILT_READ, EV_ADD, 0, 0, pNode);

	nRetCode = kevent(m_nKqueue, &event, 1, NULL, 0, NULL);
	XY_FAILED_JUMP(nRetCode != -1);

    pResult = pNode;
Exit0:
    if (pResult == NULL)
    { 
        if (pNode)
        {
            pNode->pNext = m_pListenNodeFree;
            m_pListenNodeFree = pNode;
            pNode = NULL;
        }

		XY_CLOSE_FD(nSocket);
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

    pNode->bUsrClose = true;
}

int XSocketMgr::Query()
{
    int			nCount		= 0;
    time_t		uTimeNow    = time(NULL);
	timespec*	pTimeWait	= NULL;
	timespec	timeWait;

	assert(m_nKqueue != -1);

	if (m_nQueryTimeout >= 0)
	{
		timeWait.tv_sec = m_nQueryTimeout / 1000;
		timeWait.tv_nsec = (m_nQueryTimeout % 1000) * 1000000;
		pTimeWait = &timeWait;
	}

	nCount = kevent(m_nKqueue, NULL, 0, m_pEventTable, m_nEventCount, pTimeWait);

	for (int i = 0; i < nCount; i++)
    {
		XSocketNode*	pNode	= NULL;
		struct kevent*	pEvent	= &m_pEventTable[i];

		assert(pEvent->filter == EVFILT_READ);
		assert(!(pEvent->flags & EV_ERROR));

		pNode = (XSocketNode*)pEvent->udata;
        if (pNode->bUsrClose)
            continue;

        if (pNode->bListener)
        {
            ProcessNewSocket((XListenNode*)pNode, uTimeNow);
            continue;
        }

        ProcessReceive((XStreamNode*)pNode, uTimeNow);
    }

    if (uTimeNow > m_uNextCycleProcess)
    {
        ProcessCycle(uTimeNow);
        m_uNextCycleProcess = uTimeNow;
    }

	return nCount;
}

int XSocketMgr::ProcessNewSocket(XListenNode* pListen, time_t uTimeNow)
{
	int			nCount		= 0;
    int         nRetCode    = 0;
    sockaddr_in remoteAddr;

    while (!pListen->bUsrClose)
    {
        SOCKET              nSocket     = INVALID_SOCKET;
        socklen_t           nAddrLen    = (socklen_t)sizeof(sockaddr_in);
        XStreamNode*        pNode       = NULL;

        memset(&remoteAddr, 0, sizeof(sockaddr_in));
        
        nSocket = accept(pListen->nSocket, (sockaddr*)&remoteAddr, &nAddrLen);
        if (nSocket == INVALID_SOCKET)
        {
            if (errno == EINTR)
                continue;

            break;
        }

		nCount++;

        nRetCode = SetSocketNoneBlock(nSocket);
        if (!nRetCode)
        {
            close(nSocket);
            continue;
        }

        pNode = CreateStream(
            nSocket, inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), uTimeNow, pListen->uMaxRecvPackSize
        );
        if (pNode == NULL)
        {
            close(nSocket);
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
    int nSocket, const char cszRemoteIP[], int nRemotePort, time_t uTimeNow, size_t uMaxPackLen
)
{
    XStreamNode*        pResult     = NULL;
    int                 nRetCode    = 0;
    XStreamNode*        pNode       = NULL;
    BYTE*               pbyBuffer   = NULL;
	struct kevent		event;

    XY_FAILED_JUMP(m_pStreamNodeFree);
    pNode = m_pStreamNodeFree;
    m_pStreamNodeFree = m_pStreamNodeFree->pNext;

    pbyBuffer = new BYTE[uMaxPackLen];
    XY_FAILED_JUMP(pbyBuffer);

	EV_SET(&event, nSocket, EVFILT_READ, EV_ADD, 0, 0, pNode);
	nRetCode = kevent(m_nKqueue, &event, 1, NULL, 0, NULL);
	XY_FAILED_JUMP(nRetCode != -1);

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
    pNode->uLastPackTime    = uTimeNow;
    pNode->pDataProc        = NULL;
    pNode->pErrProc         = NULL;
    pNode->pvUsrData        = NULL;
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

BOOL XSocketMgr::ProcessReceive(XStreamNode* pNode, time_t uTimeNow)
{
    BOOL		bResult             = false;
    int			nRetCode            = 0;
    size_t		uLeftBufferSize     = 0;
    size_t		uDataLen            = 0;
    size_t		uHeaderLen          = 0;
    uint64_t	uUserDataLen        = 0;
	size_t		uCount				= 0;

    XY_FAILED_JUMP(pNode->nSysErr == 0);
    XY_FAILED_JUMP(pNode->nUsrErr == 0);

    while (!pNode->bUsrClose)
    {    
        uLeftBufferSize = (size_t)(pNode->pbyBuffer + pNode->uBufferSize - pNode->pbyDataEnd);
        assert(uLeftBufferSize > 0); // 理论上不可能出现这种情况,除非是后面的缓冲区处理写错了:)

        nRetCode = recv(pNode->nSocket, (char*)pNode->pbyDataEnd, Min(uLeftBufferSize, MAX_SIZE_PER_RECV), 0);
        if (nRetCode == 0)
        {
            pNode->nUsrErr = eSocketDisconnect;
            goto Exit0;
        }

        if (nRetCode < 0)
        {
            int nError = errno;

            if (nError == EINTR)
                continue;

            if (nError == EAGAIN || nError == EWOULDBLOCK)
                goto Exit1;

            pNode->nSysErr = nError;

            goto Exit0;
        }

		uCount += nRetCode;

        assert(nRetCode <= (int)uLeftBufferSize);

        pNode->pbyDataEnd += nRetCode;

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

            pNode->uLastPackTime = uTimeNow;

            // call back
            if (pNode->pDataProc)
            {
                (*pNode->pDataProc)(pNode, pNode->pvUsrData, pNode->pbyDataBegin + uHeaderLen, (size_t)uUserDataLen);
            }

            pNode->pbyDataBegin += (uHeaderLen + uUserDataLen);
        }

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
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void XSocketMgr::ProcessCycle(time_t uTimeNow)
{
    XListenNode* pListenParent = &m_ListenHeadNode;
    XListenNode* pListenNode   = pListenParent->pNext;

    while (pListenNode)
    {
        if (!pListenNode->bUsrClose)
        {
            pListenParent = pListenNode;
            pListenNode = pListenNode->pNext;
            continue;
        }

        close(pListenNode->nSocket);
        pListenNode->nSocket = INVALID_SOCKET;

        pListenParent->pNext = pListenNode->pNext;
        pListenNode->pNext = m_pListenNodeFree;
        m_pListenNodeFree = pListenNode;
        pListenNode = pListenParent->pNext;
    }

    XStreamNode* pStreamParent = &m_StreamHeadNode;
    XStreamNode* pStreamNode   = pStreamParent->pNext;

    while (pStreamNode)
    {
        if (pStreamNode->bUsrClose)
        {
            pStreamParent->pNext = pStreamNode->pNext;
            CloseSocketHandle(pStreamNode->nSocket);
            pStreamNode->nSocket = INVALID_SOCKET;
            XY_DELETE_ARRAY(pStreamNode->pbyBuffer);
            pStreamNode->pNext = m_pStreamNodeFree;
            m_pStreamNodeFree = pStreamNode;
            pStreamNode = pStreamParent->pNext;
            continue;
        }

        if (pStreamNode->nTimeoutSeconds > 0 && !pStreamNode->bErrNotified)
        {
            if (uTimeNow > pStreamNode->uLastPackTime + pStreamNode->nTimeoutSeconds)
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

