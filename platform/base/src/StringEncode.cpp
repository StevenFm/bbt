#include "Base.h"
#include "StringEncode.h"

int _WideCharToUtf8(char* pszDest, size_t uDestLen, const wchar_t* pszSrc);
int _Utf8ToWideChar(wchar_t* pszDest, int uDestLen, const char* pszSrc);

wchar_t* AllocWideCharFromUTF8(const char* pszString, int nLen/* = -1*/)
{
	wchar_t*	pszResult	= NULL;
	int			nRetCode	= false;
	char*		pszTemp		= NULL;
	int			nWStrLen	= 0;
	wchar_t*	pszWString	= NULL;

    XYLOG_FAILED_JUMP(pszString);

	if (nLen >= 0 && pszString[nLen] != '\0')
	{
		pszTemp = new char[nLen + 1];

		memcpy(pszTemp, pszString, nLen);
		pszTemp[nLen] = '\0';

		pszString = pszTemp;
	}

	nWStrLen = _Utf8ToWideChar(NULL, 0, pszString);
	XY_FAILED_JUMP(nWStrLen != -1);

	pszWString = (wchar_t*)malloc(sizeof(wchar_t) * nWStrLen);

	nRetCode = _Utf8ToWideChar(pszWString, nWStrLen, pszString);
	XY_FAILED_JUMP(nRetCode != -1);

	pszResult = pszWString;
Exit0:
	if (pszResult == NULL)
	{
		XY_FREE(pszWString);
	}
	XY_DELETE_ARRAY(pszTemp);

	return pszResult;
}

char* AllocUTF8FromWideChar(const wchar_t* pszWString, int nLen/* = -1*/)
{
	char*		pszResult	= NULL;
	int			nRetCode	= false;
	wchar_t*	pszTemp		= NULL;
	int			nStrLen		= 0;
	char*		pszString	= NULL;

    XYLOG_FAILED_JUMP(pszWString);

	if (nLen >= 0 && pszWString[nLen] != '\0')
	{
		pszTemp = new wchar_t[nLen + 1];

		memcpy(pszTemp, pszWString, sizeof(wchar_t) * nLen);
		pszTemp[nLen] = '\0';

		pszWString = pszTemp;
	}

	nStrLen = _WideCharToUtf8(NULL, 0, pszWString);
	XY_FAILED_JUMP(nStrLen != -1);

	pszString = (char*)malloc(sizeof(char) * nStrLen);

	nRetCode = _WideCharToUtf8(pszString, nStrLen, pszWString);
	XY_FAILED_JUMP(nRetCode != -1);

	pszResult = pszString;
Exit0:
	if (pszResult == NULL)
	{
		XY_FREE(pszString);
	}
	XY_DELETE_ARRAY(pszTemp);

	return pszResult;
}

void PlatformFreeString(void* pvString)
{
    free(pvString);
}

int _WideCharToUtf8_Count(const wchar_t* pszSrc)
{
	int				nResult		= -1;
	int				nCount		= 0;
	const wchar_t*  pWChar      = pszSrc;

	assert(pszSrc);

	while (true)
	{
		if (*pWChar >= 0x0000 && *pWChar <= 0x007f)
		{
			nCount++;
		}
		else if (*pWChar >= 0x0080 && *pWChar <= 0x07ff)
		{
			nCount += 2;
		}
		else if (*pWChar >= 0x0800 && *pWChar <= 0xffff)
		{
			nCount += 3;
		}
		else
		{
			goto Exit0; // 可能有UCS4，但这里只支持到UCS2
		}

		if (*pWChar == '\0')
			break;

		pWChar++;
	}

	nResult = nCount;
Exit0:
	return nResult;
}

int _WideCharToUtf8(char* pszDest, size_t uDestLen, const wchar_t* pszSrc)
{
	int				nResult     = -1;
	const wchar_t*  pWChar      = pszSrc;
	char*           pUtf8       = pszDest;
	char*           pszDestEnd  = pszDest + uDestLen;

	assert(pszSrc);

	if (pszDest == NULL)
	{
		return _WideCharToUtf8_Count(pszSrc);
	}

	while (true)
	{
		if (*pWChar >= 0x0000 && *pWChar <= 0x007f)
		{
			XY_FAILED_JUMP(pUtf8 < pszDestEnd);

			*pUtf8++ = (char)*pWChar;
		}
		else if (*pWChar >= 0x0080 && *pWChar <= 0x07ff)
		{
			XY_FAILED_JUMP(pUtf8 + 1 < pszDestEnd);

			*pUtf8++ = 0xc0 | *pWChar >> 6;
			*pUtf8++ = 0x80 | (*pWChar & 0x003f);
		}
		else if (*pWChar >= 0x0800 && *pWChar <= 0xffff)
		{
			XY_FAILED_JUMP(pUtf8 + 2 < pszDestEnd);

			*pUtf8++ = 0xe0 | *pWChar >> 12;
			*pUtf8++ = 0x80 | ((*pWChar >> 6) & 0x003f);
			*pUtf8++ = 0x80 | (*pWChar & 0x003f);
		}
		else
		{
			goto Exit0; // 可能有UCS4，但这里只支持到UCS2
		}

		if (*pWChar == '\0')
			break;

		pWChar++;
	}

	nResult = pUtf8 - pszDest;
Exit0:
	return nResult;
}

enum _XUtf8ToWCharState
{
	eksEmpty,
	eksDouble,
	eksThreeFirst,
	eksThreeSecond,
};

inline int _ParseUtf8Byte(wchar_t* pOut, DWORD dwValue)
{
	int nPrefixCount = 0;

	assert(pOut);

	while ((dwValue << nPrefixCount) & 0x80)
	{
		nPrefixCount++;
	}

	*pOut = (wchar_t)(dwValue & (0xff >> nPrefixCount));

	return nPrefixCount;
}

int _Utf8ToWideChar_Count(const char* pszSrc)
{
	int         		nResult     = -1;
	int					nCount		= 0;
	const char* 		pszUtf8     = pszSrc;
	wchar_t     		nWideChar   = 0;
	_XUtf8ToWCharState	nState		= eksEmpty;

	assert(pszSrc);

	while (true)
	{
		int     nPrefixCount    = 0;
		wchar_t nWChar          = 0;

		nPrefixCount = _ParseUtf8Byte(&nWChar, (DWORD)*pszUtf8);

		switch (nState)
		{
		case eksEmpty:
			switch (nPrefixCount)
			{
			case 0:
				nCount++;
				break;

			case 2:
				nWideChar = (nWChar << 6);
				nState = eksDouble;
				break;

			case 3:
				nWideChar = (nWChar << 12);
				nState = eksThreeFirst;
				break;

			default:
				goto Exit0;
			}
			break;

		case eksDouble:
			XY_FAILED_JUMP(nPrefixCount == 1);
			nCount++;
			nState = eksEmpty;
			break;

		case eksThreeFirst:
			XY_FAILED_JUMP(nPrefixCount == 1);
			nWideChar |= (nWChar << 6);
			nState = eksThreeSecond;
			break;

		case eksThreeSecond:
			XY_FAILED_JUMP(nPrefixCount == 1);
			nCount++;
			nState = eksEmpty;
			break;

		default:
			assert(false);
		}

		if (*pszUtf8 == '\0')
			break;

		pszUtf8++;
	}

	XY_FAILED_JUMP(nState == eksEmpty);

	nResult = nCount;
Exit0:
	return nResult;
}

int _Utf8ToWideChar(wchar_t* pszDest, int uDestLen, const char* pszSrc)
{
	int        			nResult     = -1;
	const char*       	pszUtf8     = pszSrc;
	wchar_t*    		pWChar      = pszDest;
	wchar_t*    		pWCharEnd   = pszDest + uDestLen;
	wchar_t     		nWideChar   = 0;
	_XUtf8ToWCharState	nState		= eksEmpty;

	assert(pszSrc);

    if (pszDest == NULL)
    {
        return _Utf8ToWideChar_Count(pszSrc);
    }

	while (true)
	{
		int     nPrefixCount    = 0;
		wchar_t nWChar          = 0;

		XY_FAILED_JUMP(pWChar < pWCharEnd);

		nPrefixCount = _ParseUtf8Byte(&nWChar, (DWORD)*pszUtf8);

		switch (nState)
		{
		case eksEmpty:
			switch (nPrefixCount)
			{
			case 0:
				*pWChar++ = nWChar;
				break;

			case 2:
				nWideChar = (nWChar << 6);
				nState = eksDouble;
				break;

			case 3:
				nWideChar = (nWChar << 12);
				nState = eksThreeFirst;
				break;

			default:
				goto Exit0;
			}
			break;

		case eksDouble:
			XY_FAILED_JUMP(nPrefixCount == 1);
			*pWChar++ = nWideChar | nWChar;
			nState = eksEmpty;
			break;

		case eksThreeFirst:
			XY_FAILED_JUMP(nPrefixCount == 1);
			nWideChar |= (nWChar << 6);
			nState = eksThreeSecond;
			break;

		case eksThreeSecond:
			XY_FAILED_JUMP(nPrefixCount == 1);
			*pWChar++ = nWideChar | nWChar;
			nState = eksEmpty;
			break;

		default:
			assert(false);
		}

		if (*pszUtf8 == '\0')
			break;

		pszUtf8++;
	}

	XY_FAILED_JUMP(nState == eksEmpty);

	nResult = pWChar - pszDest;
Exit0:
	return nResult;
}

BOOL HasUtf8BomHeader(const char* pszText, int nLen)
{
	if (nLen < 0)
	{
		nLen = (int)strlen(pszText);
	}

    if (nLen >= 3 && pszText[0] == (char)0xEF && pszText[1] == (char)0xBB && pszText[2] == (char)0xBF)
		return true;

	return false;
}

void ParseUTF8(XUTF8Info* pParam, const char* pszUTF8String, int nLen/* = -1*/)
{
    const char* pszPos          = pszUTF8String;
    const char* pszEnd          = pszUTF8String + nLen;
    int         nBomLen         = 0;
    int         nWideCharLen    = 0;

    if (nLen < 0)
    {
        nLen    = (int)strlen(pszUTF8String);
        pszEnd  = pszUTF8String + nLen;
    }

    if (HasUtf8BomHeader(pszUTF8String, nLen))
    {
        nBomLen = 3;
        pszPos += 3;
    }

    while (pszPos != pszEnd)
    {
        // 1 bytes(ascii)
        if (*pszPos > 0)
        {
            pszPos++;
            nWideCharLen++;
            continue;
        }

        // err : 4字节以上的现在还用不到
        XY_FAILED_JUMP(*pszPos < -8);

        // 4 bytes
        if (*pszPos >= -16)
        {
            XY_FAILED_JUMP(pszEnd - pszPos >= 4);
            XY_FAILED_JUMP(pszPos[1] < -64);
            XY_FAILED_JUMP(pszPos[2] < -64);
            XY_FAILED_JUMP(pszPos[3] < -64);

            pszPos += 4;
            nWideCharLen++;
            continue;
        }

        // 3 bytes
        if (*pszPos >= -32)
        {
            XY_FAILED_JUMP(pszEnd - pszPos >= 3);
            XY_FAILED_JUMP(pszPos[1] < -64);
            XY_FAILED_JUMP(pszPos[2] < -64);

            pszPos += 3;
            nWideCharLen++;
            continue;
        }

        // 2 bytes
        if (*pszPos >= -64)
        {
            XY_FAILED_JUMP(pszEnd - pszPos >= 2);
            XY_FAILED_JUMP(pszPos[1] < -64);

            pszPos += 2;
            nWideCharLen++;
            continue;
        }

        // err : 10xx,xxxx can't be the first
        XY_FAILED_JUMP(false);
    }

Exit0:
    pParam->nBomLen         = nBomLen;
    pParam->nWideCharLen    = nWideCharLen;
    pParam->nValidUTF8Bytes = (int)(pszPos - pszUTF8String) - nBomLen;
    pParam->nStrLen         = nLen;

    return;
}

const char* GetText(const char szString[])
{
    return szString;
}
