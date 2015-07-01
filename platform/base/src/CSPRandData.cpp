#include "Base.h"

#ifdef _MSC_VER
#include <WinCrypt.h>

static HCRYPTPROV   gs_hRandProvider = NULL;
static XMutex       gs_ProvLock;

BOOL CSPRandData(void* pvBuffer, size_t uBufferLen)
{
	BOOL bResult    = false;
	BOOL bRetCode   = false;

	if (gs_hRandProvider == NULL)
	{
		gs_ProvLock.Lock();
		if (gs_hRandProvider == NULL)
		{
			CryptAcquireContext(&gs_hRandProvider, NULL, NULL, PROV_RSA_FULL, 0);
		}
		gs_ProvLock.Unlock();
		XY_FAILED_JUMP(gs_hRandProvider != NULL);
	}

	bRetCode = CryptGenRandom(gs_hRandProvider, (DWORD)uBufferLen, (BYTE*)pvBuffer);
	XY_FAILED_JUMP(bRetCode);

	bResult = true;
Exit0:
	return bResult;
}
#endif

#if defined(__linux) || defined(__APPLE__)
static int      gs_RandFile = -1;
static XMutex   gs_FileLock;

BOOL CSPRandData(void* pvBuffer, size_t uBufferLen)
{
    BOOL    bResult     = false;
    int     nRetCode    = 0;

    if (gs_RandFile == -1)
    {
        gs_FileLock.Lock();
        if (gs_RandFile == -1)
        {
            gs_RandFile = open("/dev/urandom", O_RDONLY);
        }
        gs_FileLock.Unlock();
        XY_FAILED_JUMP(gs_RandFile != -1);
    }

    nRetCode = read(gs_RandFile, (unsigned char*)pvBuffer, (int)uBufferLen);
    XY_FAILED_JUMP(nRetCode == (int)uBufferLen);

    bResult = true;
Exit0:
    return bResult;
}
#endif



