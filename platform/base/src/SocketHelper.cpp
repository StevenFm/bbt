#include "Base.h"
#include "SocketHelper.h"

#if defined(__linux) || defined(__APPLE__)
BOOL SetSocketNoneBlock(SOCKET nSocket)
{
    BOOL    bResult  = false;
    int     nRetCode = 0;
    int     nOption  = 0;

    nOption = fcntl(nSocket, F_GETFL, 0);

    nRetCode = fcntl(nSocket, F_SETFL, nOption | O_NONBLOCK);
    XYLOG_FAILED_JUMP(nRetCode == 0);

    bResult = true;
Exit0:
    return bResult;
}
#endif

#ifdef _MSC_VER
BOOL SetSocketNoneBlock(SOCKET nSocket)
{
    BOOL    bResult  = false;
    int     nRetCode = 0;
    u_long  ulOption = 1;

    nRetCode = ioctlsocket(nSocket, FIONBIO, &ulOption);
    XY_FAILED_JUMP(nRetCode == 0);

    bResult = true;
Exit0:
    return bResult;
}
#endif

SOCKET  ConnectSocket(const char cszIP[], int nPort, int ntimeout, size_t uRecvBufferSize)
{
    SOCKET              nResult         = INVALID_SOCKET;
    int                 nRetCode        = false;
    SOCKET              nSocket         = INVALID_SOCKET; 
    hostent*            pHost           = NULL;
    timeval             timeoutValue    = {ntimeout / 1000, 1000 * (ntimeout % 1000)};
	int					nError			= 0;
	socklen_t			nSockLen		= sizeof(nError);
    fd_set              writeSet;
    sockaddr_in         serverAddr;

    assert(cszIP);

    pHost = gethostbyname(cszIP);
    XY_FAILED_JUMP(pHost);

    nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    XY_FAILED_JUMP(nSocket != INVALID_SOCKET);

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_addr.s_addr  = *(unsigned long*)pHost->h_addr_list[0];
    serverAddr.sin_port         = htons(nPort);

    nRetCode = SetSocketNoneBlock(nSocket);
    XY_FAILED_JUMP(nRetCode);

    if (uRecvBufferSize > 0)
    {
        int nBufSize = (int)uRecvBufferSize; // 为了兼容性,用一个int做参数,而不是size_t

        nRetCode = setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nBufSize, sizeof(nBufSize));
        XY_FAILED_JUMP(nRetCode == 0);
    }

    nRetCode = connect(nSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
    if (nRetCode == SOCKET_ERROR)
    {
#ifdef _MSC_VER
        nRetCode = GetSocketError();
        XY_FAILED_JUMP(nRetCode == WSAEWOULDBLOCK);
#endif

#if defined(__linux) || defined(__APPLE__)
        nRetCode = GetSocketError();
        XY_FAILED_JUMP(nRetCode == EINPROGRESS);
#endif
    }

    FD_ZERO(&writeSet);
    FD_SET(nSocket, &writeSet);

    nRetCode = select((int)nSocket + 1, NULL, &writeSet, NULL, &timeoutValue);
    XY_FAILED_JUMP(nRetCode == 1);

	nRetCode = getsockopt(nSocket, SOL_SOCKET, SO_ERROR, (char*)&nError, &nSockLen);
	XY_FAILED_JUMP(nRetCode == 0 && nError == 0);

    nResult = nSocket;
Exit0:
    if (nResult == INVALID_SOCKET)
    {
        if (nSocket != INVALID_SOCKET)
        {
            CloseSocketHandle(nSocket);
            nSocket = INVALID_SOCKET;
        }
    }
    return nResult;
}

BOOL IsSocketAlive(SOCKET nSocket)
{
	BOOL    bResult = false;
	int     nRetCode = 0;
	volatile int nData = false; // no use


	XY_FAILED_JUMP(nSocket != INVALID_SOCKET);

	nRetCode = send(nSocket, (char *)&nData, 0, 0);
	XY_FAILED_JUMP(nRetCode != -1);

	bResult = true;
Exit0:
	return bResult;
}