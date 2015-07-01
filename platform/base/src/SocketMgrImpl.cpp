#include "Base.h"
#include "SocketMgrImpl.h"

ISocketMgr* CreateSocketMgr(int nMaxListen, int nMaxStream)
{
    ISocketMgr*  piResult   = NULL;
    int          nRetCode   = 0;
    XSocketMgr*  pSocketMgr = NULL;

    pSocketMgr = new XSocketMgr;
    XY_FAILED_JUMP(pSocketMgr);

    nRetCode = pSocketMgr->Setup(nMaxListen, nMaxStream);
    XY_FAILED_JUMP(nRetCode);

    piResult = pSocketMgr;
Exit0:
    if (piResult == NULL)
    {
		XY_DELETE(pSocketMgr);
    }
    return piResult;
}

XSocketMgr::XSocketMgr()
{
    m_ulRefCount            = 1;

#ifdef __linux
    m_nEpoll                = -1;
    m_pEventTable           = NULL;
#endif

#ifdef __APPLE__
	m_nKqueue				= -1;
	m_pEventTable			= NULL;
#endif

    m_pListenNodeArray      = NULL;
    m_pListenNodeFree       = NULL;
    m_ListenHeadNode.pNext  = NULL;
    
    m_pStreamNodeArray      = NULL;
    m_pStreamNodeFree       = NULL;
    m_StreamHeadNode.pNext  = NULL;

    m_uNextCycleProcess     = 0;
	m_nQueryTimeout			= 0;
}

XSocketMgr::~XSocketMgr()
{
	Clear();
}

u_long STDMETHODCALLTYPE XSocketMgr::Release(void)
{
    u_long ulRefCount = XY_InterlockedDecrement(&m_ulRefCount);

    if (ulRefCount == 0)
    {
        delete this;
    }

    return ulRefCount;
}

XSocket* XSocketMgr::Connect(const char cszIP[], int nPort, int nTimeout, size_t uMaxPackLen, size_t uRecvBufferSize)
{
    XSocket*            pResult         = NULL;
    SOCKET              nSocket         = INVALID_SOCKET;
    time_t              uTimeNow        = time(NULL);

    nSocket = ConnectSocket(cszIP, nPort, nTimeout, uRecvBufferSize);
    XY_FAILED_JUMP(nSocket != INVALID_SOCKET);

    pResult = CreateStream(nSocket, cszIP, nPort, uTimeNow, uMaxPackLen);
Exit0:
    return pResult;
}

void XSocketMgr::SetAcceptCallback(XSocket* pSocket, XSocketAcceptCallback* pAcceptProc)
{
    XListenNode* pNode = (XListenNode*)pSocket;

    assert(pNode->bListener);
    assert(!pNode->bUsrClose);

    pNode->pCallback    = pAcceptProc;
}

void XSocketMgr::SetStreamDataCallback(XSocket* pSocket, XSocketDataCallback* pDataProc)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;

    assert(!pNode->bListener);
    assert(!pNode->bUsrClose);

    pNode->pDataProc  = pDataProc;
}

void XSocketMgr::SetStreamErrorCallback(XSocket* pSocket, XSocketErrorCallback pErrProc)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;

    assert(!pNode->bListener);
    assert(!pNode->bUsrClose);

    pNode->pErrProc   = pErrProc;
}

void XSocketMgr::SetSendRetryCount(XSocket* pSocket, int nCount, int nSleep)
{
	XSocketNode* pNode = (XSocketNode*)pSocket;

	pNode->nSendRetryCount = nCount;
	pNode->nSendRetrySleep = nSleep;
}

void XSocketMgr::SetUsrData(XSocket* pSocket, void* pvUsrData)
{
    XSocketNode* pNode = (XSocketNode*)pSocket;

    assert(pNode);
    assert(!pNode->bUsrClose);

    pNode->pvUsrData = pvUsrData;
}

void* XSocketMgr::GetUsrData(XSocket* pSocket)
{
    XSocketNode* pNode = (XSocketNode*)pSocket;

    assert(pNode);
    assert(!pNode->bUsrClose);

    return pNode->pvUsrData;
}

void XSocketMgr::SetStreamTimeout(XSocket* pSocket, int nSeconds)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;

    assert(!pNode->bListener);
    assert(!pNode->bUsrClose);

    pNode->nTimeoutSeconds = nSeconds;
}

void XSocketMgr::SetLinger(XSocket* pSocket, BOOL bEnable, int nSeconds)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;
    linger       param;

    assert(!pNode->bListener);
    assert(!pNode->bUsrClose);

    param.l_onoff  = (u_short)bEnable;
    param.l_linger = (u_short)nSeconds;
		
    setsockopt(pNode->nSocket, SOL_SOCKET, SO_LINGER, (char*)&param, sizeof(param));   
}

const char*  XSocketMgr::GetStreamRemoteIP(XSocket* pSocket)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;

    assert(pSocket);
    assert(!pNode->bListener);

    return pNode->szRemoteIP;
}

int XSocketMgr::GetStreamRemotePort(XSocket* pSocket)
{
    XStreamNode* pNode = (XStreamNode*)pSocket;

    assert(pSocket);
    assert(!pNode->bListener);

    return pNode->nRemotePort;
}

BOOL XSocketMgr::Send(XSocket* pSocket, void* pvData, size_t uDataLen)
{
    BOOL                bResult    = false;
    int                 nRetCode   = 0;
    XStreamNode*        pNode      = (XStreamNode*)pSocket;
    size_t              uHeaderLen = 0;
    BYTE                byHeader[MAX_HEADER_LEN];

    assert(!pNode->bListener);
    assert(!pNode->bUsrClose);

    XYLOG_FAILED_JUMP(pNode->nSysErr == 0);
	XYLOG_FAILED_JUMP(pNode->nUsrErr == 0);

    uHeaderLen = EncodeU64(byHeader, sizeof(byHeader), uDataLen);
	XY_FAILED_JUMP(uHeaderLen > 0);

    nRetCode = StreamSend(pNode, byHeader, uHeaderLen);
    XY_FAILED_JUMP(nRetCode);

    nRetCode = StreamSend(pNode, (BYTE*)pvData, uDataLen);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void XSocketMgr::SetQueryTimeout(int nTimeout)
{
	m_nQueryTimeout = nTimeout;
}

BOOL XSocketMgr::StreamSend(XStreamNode* pNode, BYTE* pbyData, size_t uDataLen)
{
    int     nRetCode = 0;
	int		nCount	 = 0;

    while (uDataLen > 0)
    {
		size_t uTrySend = Min(MAX_SIZE_PER_SEND, uDataLen);
        nRetCode = send(pNode->nSocket, (const char*)pbyData, uTrySend, 0);
        if (nRetCode == -1)
        {
            int nErr = GetSocketError();
            if (nErr == EINTR)
                continue;

			if (nErr == EAGAIN && nCount < pNode->nSendRetryCount)
			{
				nCount++;
				ThreadSleep(pNode->nSendRetrySleep);
				continue;
			}

            pNode->nSysErr = nErr;
            return false;
        }

        pbyData  += nRetCode;
        uDataLen -= nRetCode;
    }
    return true;
}

#ifdef _MSC_VER
#include "SocketMgrWindows.hpp"
#endif

#ifdef __linux
#include "SocketMgrLinux.hpp"
#endif

#ifdef __APPLE__
#include "SocketMgrApple.hpp"
#endif

