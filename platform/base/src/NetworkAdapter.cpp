#include "Base.h"

BOOL IsInternalIP(const char szIP[])
{
    BOOL    bResult                      = false;
	int		nRetCode					 = 0;
    char*   pszTokenSave                 = NULL;
	char*   pToks						 = NULL;
    int     nIPNumIndex                  = 0;
    int     nIPNumber[4];
	char	szTempIP[32];

    nRetCode = SafeCopyString(szTempIP, szIP);
	XY_FAILED_JUMP(nRetCode);

	pToks = strtok_r(szTempIP, ".", &pszTokenSave);
	while (pToks)
	{
        XYLOG_FAILED_JUMP(nIPNumIndex < 4);
        nIPNumber[nIPNumIndex++] = atoi(pToks);
        pToks = strtok_r(NULL, ".", &pszTokenSave);
    }

    XY_SUCCESS_JUMP(nIPNumber[0] == 127 && nIPNumber[1] == 0 && nIPNumber[2] == 0 && nIPNumber[3] == 1);
    XY_SUCCESS_JUMP(nIPNumber[0] == 10);

    if (nIPNumber[0] == 172)
    {
        XY_FAILED_JUMP(nIPNumber[1] >= 16 && nIPNumber[1] <= 31);
        goto Exit1;
    }

    XY_FAILED_JUMP(nIPNumber[0] == 192 && nIPNumber[1] == 168);

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

static std::string FormatMacAddr(BYTE byAddr[], int nLen)
{
    std::string strAddr;
    char szTxt[8];
    
    for (int i = 0; i < nLen; i++)
    {
        sprintf(szTxt, "%02x", byAddr[i]);
        
        strAddr += szTxt;
        if (i != nLen - 1)
        {
            strAddr += ":";
        }
    }
    
    return strAddr;
}

#ifdef _MSC_VER

#include <IPHlpApi.h>

BOOL QueryNetworkAdaptersInfo(XNetworkAdapterTable& adapterTable)
{
    BOOL                    bResult         = false;
    DWORD                   dwRet           = 0;
    IP_ADAPTER_INFO*        pAdapter        = NULL;
    ULONG                   uLen            = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO*        pAdapterData    = (IP_ADAPTER_INFO*)malloc(uLen);
    
    adapterTable.clear();

    dwRet = GetAdaptersInfo(pAdapterData, &uLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW) 
    {
        free(pAdapterData);
        pAdapterData = (IP_ADAPTER_INFO*)malloc(uLen);
        dwRet = GetAdaptersInfo(pAdapterData, &uLen);
    }

    if (dwRet != ERROR_SUCCESS)
        goto Exit0;

    pAdapter = pAdapterData;

    while (pAdapter)
    {
        IP_ADDR_STRING* pAddr = &pAdapter->IpAddressList;
        std::string strMacAddr = FormatMacAddr(pAdapter->Address, (int)pAdapter->AddressLength);

        while (pAddr)
        {
            DWORD dwAddrValue = inet_addr(pAddr->IpAddress.String);

            if (dwAddrValue == INADDR_NONE || dwAddrValue == 0)
            {
                pAddr = pAddr->Next;
                continue;
            }

            XNetworkAdapterInfo info;
            
            info.strName        = pAdapter->AdapterName;
            info.strEtherAddr   = strMacAddr;
            info.strIpAddr      = pAddr->IpAddress.String;
            
            adapterTable.push_back(info);
            
            pAddr = pAddr->Next;
        }

        pAdapter = pAdapter->Next;
    }

    bResult = true;
Exit0:
    XY_FREE(pAdapterData);
    return bResult;
}

#endif // end of _MSC_VER


#ifdef __linux 

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

BOOL QueryNetworkAdaptersInfo(XNetworkAdapterTable& adapterTable)
{
    BOOL                    bResult         = false;
    int                     nRetCode        = false;
    int                     nSocket         = -1;
    int                     nAdapterCount   = -1;
    ifreq                   buffer[16];
    ifconf                  ifc;
    
    adapterTable.clear();

    nSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (nSocket < 0)
        goto Exit0;

    ifc.ifc_req = buffer;
    ifc.ifc_len = sizeof(buffer);

    nRetCode = ioctl(nSocket, SIOCGIFCONF, &ifc);
    if (nRetCode != 0)
        goto Exit0;

    nAdapterCount = (int)(ifc.ifc_len / sizeof(ifreq));

    for (int i = 0; i < nAdapterCount; i++)
    {
        ifreq*                  pIfreq          = NULL;
        BYTE*                   pbyMac          = NULL;
        XNetworkAdapterInfo     info;
        sockaddr_in             addr;

        pIfreq = &ifc.ifc_req[i];
        
        info.strName = pIfreq->ifr_name;

        nRetCode = ioctl(nSocket, SIOCGIFHWADDR, pIfreq);
        if (nRetCode != 0)
            continue;

        pbyMac = (BYTE*)&pIfreq->ifr_hwaddr.sa_data[0];
        
        info.strEtherAddr = FormatMacAddr(pbyMac, IFHWADDRLEN);

        nRetCode = ioctl(nSocket, SIOCGIFADDR, pIfreq);
        if (nRetCode != 0) 
            continue;
            
        addr = *((sockaddr_in*)&(pIfreq->ifr_addr));

        info.strIpAddr = inet_ntoa(addr.sin_addr);
        
        adapterTable.push_back(info);
    }

    bResult = true;
Exit0:
    if (nSocket != -1)
    {
        close(nSocket);
        nSocket = -1;
    }
    return bResult;
}
#endif

#ifdef __APPLE__
#include <sys/param.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <sys/sysctl.h> 
#include <net/ethernet.h> 
#include <net/if.h> 
#include <net/if_var.h> 
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void GetIpList(XNetworkAdapterTable& adapterTable, int nSocket, BYTE* pbyPos, BYTE* pbyEnd)
{
    int nRetCode = 0;
    
    while (pbyPos < pbyEnd)
    {
        ifreq* pIfreq       = (ifreq*)pbyPos;
        int    nLen         = Max((int)sizeof(sockaddr), (int)pIfreq->ifr_addr.sa_len);
        ifreq  copyIfreq    = *pIfreq;
        
        pbyPos += sizeof(pIfreq->ifr_name) + nLen;
        
        nRetCode = ioctl(nSocket, SIOCGIFFLAGS, &copyIfreq);
        if (nRetCode != 0)
            continue;
        
        if ((copyIfreq.ifr_flags & IFF_UP) == 0)
            continue;
                
        if (pIfreq->ifr_addr.sa_family != AF_INET)
            continue;
        
        sockaddr_in addr = *((sockaddr_in*)&(pIfreq->ifr_addr));
        XNetworkAdapterInfo info;
            
        info.strName    = pIfreq->ifr_name;
        info.strIpAddr  = inet_ntoa(addr.sin_addr);
            
        adapterTable.push_back(info);
    }
}


static void GetHwList(XNetworkAdapterTable& adapterTable, BYTE* pbyPos, BYTE* pbyEnd)
{
    int nCount = (int)adapterTable.size();
    
    while (pbyPos < pbyEnd)
    {
        ifreq* pIfreq       = (ifreq*)pbyPos;
        int    nLen         = Max((int)sizeof(sockaddr), (int)pIfreq->ifr_addr.sa_len);
        
        pbyPos += sizeof(pIfreq->ifr_name) + nLen;
        
        if (pIfreq->ifr_addr.sa_family != AF_LINK)
            continue;
                
        struct sockaddr_dl* pSDL = (struct sockaddr_dl*)&pIfreq->ifr_addr;
        ether_addr* pEtherAddr = (ether_addr*)LLADDR(pSDL);
        char* pszAddr = ether_ntoa(pEtherAddr);
        
        for (int i = 0; i < nCount; i++)
        {
            if (adapterTable[i].strName == pIfreq->ifr_name)
            {
                adapterTable[i].strEtherAddr = pszAddr;
            }
        }
    }
}

BOOL QueryNetworkAdaptersInfo(XNetworkAdapterTable& adapterTable)
{
    BOOL                    bResult         = false;
    int                     nRetCode        = false;
    int                     nSocket         = -1;
    BYTE*                   pbyPos          = NULL;
    BYTE*                   pbyEnd          = NULL;
    BYTE                    byBuffer[4096];
    ifconf                  ifc;
    
    adapterTable.clear();

    nSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (nSocket < 0)
        goto Exit0;

    ifc.ifc_req = (ifreq*)byBuffer;
    ifc.ifc_len = sizeof(byBuffer);

    nRetCode = ioctl(nSocket, SIOCGIFCONF, &ifc);
    if (nRetCode != 0)
        goto Exit0;
        
    pbyPos = (BYTE*)ifc.ifc_req;
    pbyEnd = pbyPos + ifc.ifc_len;

    GetIpList(adapterTable, nSocket, pbyPos, pbyEnd);
    GetHwList(adapterTable, pbyPos, pbyEnd);

    bResult = true;
Exit0:
    if (nSocket != -1)
    {
        close(nSocket);
        nSocket = -1;
    }
    return bResult;
}
#endif
