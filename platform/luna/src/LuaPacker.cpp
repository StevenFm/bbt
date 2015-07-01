#include "base/Base.h"
#include "Luna.h"
#include "LuaPacker.h"
#include "base/minilzo.h"

//  warning C4800: “int”: 将值强制为布尔值“true”或“false”(性能警告)
#pragma warning(disable:4800)
#define LZO_COMPRESS_BUFFER_SIZE(InputSize) (InputSize + (InputSize / 16) + 64 + 3)

enum KLuaValueDef
{
	eLuaPackTypeBegin = 245,
    eLuaPackNumber,
    eLuaPackInteger,
    eLuaPackBoolTrue,
    eLuaPackBoolFalse,    
    eLuaPackLuaString, 		// 可能是字符串,其实也可能是二进制数据
    eLuaPackIndexString,	// 以索引的方式存储的字符串,表示该字符串与之前遇到的第几个字符串一样
    eLuaPackNill,
    eLuaPackTable,
};

XLuaPaker::XLuaPaker(size_t uBufferSize)
{
    m_pbyBuffer     = new BYTE[uBufferSize];
    m_uBufferSize   = uBufferSize;
    m_pbyPos        = m_pbyBuffer;
    m_pbyEnd        = m_pbyBuffer + uBufferSize;
    m_bIsOverflow   = false;
    m_nCallStack    = 0;
    m_StringTable.reserve(256);
}

XLuaPaker::~XLuaPaker()
{
    XY_DELETE_ARRAY(m_pbyBuffer);
}

BOOL XLuaPaker::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL     bResult            = false;
    int      nRetCode           = 0;
    BYTE*    pbyLzoWorkMem      = NULL;
    lzo_uint uOutSize           = uBufferSize;
    DWORD    uSrcBufferSize     = (DWORD)(m_pbyPos - m_pbyBuffer);
    static size_t uMaxSrcSize   = 0;
    static size_t uMaxDstSize   = 0;
    
    XYLOG_FAILED_JUMP(!m_bIsOverflow);
    XYLOG_FAILED_JUMP(uBufferSize >= LZO_COMPRESS_BUFFER_SIZE(uSrcBufferSize));
   
    pbyLzoWorkMem = new BYTE[LZO1X_1_MEM_COMPRESS];

    nRetCode = lzo1x_1_compress(
        m_pbyBuffer, uSrcBufferSize, 
        pbyBuffer, &uOutSize, pbyLzoWorkMem
    );
    if (nRetCode != LZO_E_OK)
    {
        Log(eLogDebug, "[XLuaPaker::Save]lzo1x_1_compress failed. err:%u, data size:%u, out size:%u", nRetCode, uBufferSize, uOutSize);
        goto Exit0;
    }

    if ((uSrcBufferSize > uMaxSrcSize && uSrcBufferSize > 1024 * 10) || (uOutSize > uMaxDstSize && uOutSize > 1024 * 10))
    {
        uMaxSrcSize = Max(uMaxSrcSize, (size_t)uSrcBufferSize);
        uMaxDstSize = Max(uMaxDstSize, (size_t)uOutSize);
        Log(eLogInfo, "[Stats]lzo1x_1_compress. max data size:%u, max out size:%u", uMaxSrcSize, uMaxDstSize);
    }

    *puUsedSize = uOutSize;
    
    bResult = true;
Exit0:
    XY_DELETE_ARRAY(pbyLzoWorkMem);
    return bResult;
}

BOOL XLuaPaker::PushNumber(double fValue)
{
    BOOL 	bResult = false;
    int64_t	n64 	= (int64_t)fValue;
    double	f64     = (double)n64;
    
    if (f64 == fValue && n64 >= 0 && n64 <= eLuaPackTypeBegin)
    {	
		XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
		*m_pbyPos++ = (BYTE)n64;
		    
    	goto Exit1;
    }
    
    if (f64 == fValue)
    {
    	size_t uEncodeLen = 0;
    	 
    	assert(n64 < 0 || n64 > eLuaPackTypeBegin);
    	// 这段数值空间被编码到上面了,所以前挪点
		if (n64 > eLuaPackTypeBegin) 
		{
			n64 -= eLuaPackTypeBegin + 1;
		}
		
		XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
		*m_pbyPos++ = eLuaPackInteger;
				
		uEncodeLen = EncodeS64(m_pbyPos, (size_t)(m_pbyEnd - m_pbyPos), n64);
		XYLOG_FAILED_JUMP(uEncodeLen > 0);
		m_pbyPos  += uEncodeLen;
		    
    	goto Exit1;
    }    
    
	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
    *m_pbyPos++ = eLuaPackNumber;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(double));
    memcpy(m_pbyPos, (void*)&fValue, sizeof(double));
    m_pbyPos   += sizeof(double);

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    return bResult;
}

BOOL XLuaPaker::PushBool(BOOL bValue)
{
    BOOL bResult = false;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
    *m_pbyPos++ = bValue ? eLuaPackBoolTrue : eLuaPackBoolFalse;
    
    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    return bResult;
}

BOOL XLuaPaker::PushString(const char cszValue[])
{
    return PushLString(cszValue, strlen(cszValue));
}

BOOL XLuaPaker::PushNill()
{
    BOOL bResult = false;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
    *m_pbyPos++ = eLuaPackNill;

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    return bResult;
}

BYTE* XLuaPaker::NewTable()
{
    BYTE* pbyResult = NULL;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
    *m_pbyPos++ = eLuaPackTable;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(DWORD));
    memset(m_pbyPos, 0, sizeof(DWORD));
    m_pbyPos   += sizeof(DWORD);

    pbyResult = m_pbyPos;
Exit0:
    if (!pbyResult)
    {
        m_bIsOverflow = true;
    }
    return pbyResult;
}

void XLuaPaker::SetTable(BYTE* pbyTable)
{
    BYTE* pbyType   = pbyTable - sizeof(DWORD) - sizeof(BYTE);
    BYTE* pbyLength = pbyTable - sizeof(DWORD);
    DWORD dwLen     = 0;

    if (!m_bIsOverflow)
    {
        assert(*pbyType == eLuaPackTable);
        dwLen = (DWORD)(m_pbyPos - pbyTable);

        memcpy(pbyLength, (void*)&dwLen, sizeof(dwLen));
    }
}

BOOL XLuaPaker::PushValue(lua_State* L, int nValueIndex, int nCount)
{
    BOOL    bResult     = false;
    BOOL    bRetCode    = false;

    assert(L);

    m_nCallStack++; // 检测递归深度        
    XYLOG_FAILED_JUMP(m_nCallStack < 65536);

    for (int i = 0; i < nCount; i++)
    {
        int     nIndex      = nValueIndex + i;
        int     nType       = lua_type(L, nIndex);

        switch (nType)
        {
        case LUA_TNUMBER:
            {
                double fValue = lua_tonumber(L, nIndex);
                bRetCode = PushNumber(fValue);
            }
            break;

        case LUA_TBOOLEAN:
            {
                BOOL bValue = lua_toboolean(L, nIndex);
                bRetCode = PushBool(bValue);
            }
            break;

        case LUA_TSTRING:
            {
                size_t uLen = 0;
                const char* pszValue = lua_tolstring(L, nIndex, &uLen);
				XYLOG_FAILED_JUMP(pszValue);
                bRetCode = PushLString(pszValue, uLen);
            }
            break;

        case LUA_TNIL:
            {
                bRetCode = PushNill();
            }
            break;

        case LUA_TTABLE:
            bRetCode = PushTable(L, nIndex);
            break;

        default:
            {
                bRetCode = PushNill();
            }
            break;
        }

		XYLOG_FAILED_JUMP(bRetCode);

    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    m_nCallStack--;
    return bResult;
}

void  XLuaPaker::Reset()
{
    m_pbyPos        = m_pbyBuffer;
    m_pbyEnd        = m_pbyBuffer + m_uBufferSize;
    m_bIsOverflow   = false;
    m_nCallStack    = 0;
    m_StringTable.clear();
}

BOOL XLuaPaker::PushTable(lua_State* L, int nIndex)
{
    BOOL    bResult    = false;
    BOOL    bRetCode   = false;
    BYTE*   pbyTable   = NULL;

    assert(lua_istable(L, nIndex));

    pbyTable = NewTable();
	XYLOG_FAILED_JUMP(pbyTable);

    lua_pushnil(L);

    if (nIndex < 0) // pushnil会改变相对索引
        nIndex--;

    while (lua_next(L, nIndex))
    {
        int nTopIndex = lua_gettop(L);

        bRetCode = PushValue(L, nTopIndex - 1);
		XYLOG_FAILED_JUMP(bRetCode);

        bRetCode = PushValue(L, nTopIndex);
		XYLOG_FAILED_JUMP(bRetCode);

        SetTable(pbyTable);

        lua_pop(L, 1);
    }

    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    return bResult;
}

BOOL XLuaPaker::PushLString(const char* pszValue, size_t uLen)
{
	BOOL				bResult		= false;
	int					nIndex		= 0;
	size_t				uEncodeLen	= 0;
	XCachedStringInfo	inf;

	nIndex = FindString(pszValue, uLen);
	if (nIndex >= 0)
	{
		XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
		*m_pbyPos++ = eLuaPackIndexString;

		uEncodeLen = EncodeU64(m_pbyPos, (size_t)(m_pbyEnd - m_pbyPos), nIndex);
		XYLOG_FAILED_JUMP(uEncodeLen > 0);
		m_pbyPos += uEncodeLen;

		goto Exit1;
	}

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= sizeof(BYTE));
    *m_pbyPos++ = eLuaPackLuaString;

	uEncodeLen = EncodeU64(m_pbyPos, (size_t)(m_pbyEnd - m_pbyPos), uLen);
	XYLOG_FAILED_JUMP(uEncodeLen > 0);
	m_pbyPos += uEncodeLen;

	XYLOG_FAILED_JUMP((size_t)(m_pbyEnd - m_pbyPos) >= uLen);
	inf.pbyPos = m_pbyPos;
	inf.uLen = uLen;
    m_StringTable.push_back(inf);
    memcpy(m_pbyPos, pszValue, uLen);
    m_pbyPos += uLen;

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        m_bIsOverflow = true;
    }
    return bResult;
}

int XLuaPaker::FindString(const void* pvData, size_t uDataLen)
{
	int nCount = (int)m_StringTable.size();
	
	for (int i = 0; i < nCount; i++)
	{
		XCachedStringInfo& inf = m_StringTable[i];
		
		if (inf.uLen != uDataLen)
			continue;
		
		int nCmp = memcmp(inf.pbyPos, pvData, uDataLen);
		if (nCmp == 0)
			return i;
	}
	
	return -1;
}

XLuaUnpaker::XLuaUnpaker(size_t uBufferSize)
{
    m_pbyBuffer     = new BYTE[uBufferSize];
    m_uBufferSize   = uBufferSize;
    m_StringTable.reserve(256);
}

XLuaUnpaker::~XLuaUnpaker()
{
    XY_DELETE_ARRAY(m_pbyBuffer);
}

int XLuaUnpaker::Expand(lua_State* L, const BYTE* pbyData, size_t uDataLen)
{
    int             nCount      = 0;
    int             nRetCode    = 0;
    lzo_uint        uOutSize    = m_uBufferSize;
    const BYTE*     pbyPos      = m_pbyBuffer;
    const BYTE*     pbyEnd      = 0;
    static size_t   uMaxSrcSize = 0;
    static size_t   uMaxDstSize = 0;

    nRetCode = lzo1x_decompress_safe(
        pbyData, uDataLen,
        m_pbyBuffer, &uOutSize, NULL
    );

    if (nRetCode != LZO_E_OK)
    {
        Log(eLogDebug, "[XLuaUnpaker::Expand]lzo1x_decompress_safe failed. err:%u, data size:%u, out size:%u", nRetCode, uDataLen, uOutSize);
        goto Exit0;
    }

    if ((uDataLen > uMaxSrcSize && uDataLen > 1024 * 10) || (uOutSize > uMaxDstSize && uOutSize > 1024 * 10))
    {
        uMaxSrcSize = Max(uMaxSrcSize, (size_t)uDataLen);
        uMaxDstSize = Max(uMaxDstSize, (size_t)uOutSize);
        Log(eLogInfo, "[Stats]lzo1x_decompress_safe. max data size:%u, max out size:%u", uMaxSrcSize, uMaxDstSize);
    }

    pbyEnd = m_pbyBuffer + uOutSize; 

    while (pbyPos < pbyEnd)
    {
        pbyPos = ExpandValue(L, pbyPos, size_t(pbyEnd - pbyPos));
        XYLOG_FAILED_JUMP(pbyPos);
        nCount++;
    }

Exit0:
    return nCount;
}

void XLuaUnpaker::Reset()
{
	m_StringTable.clear();
}

const BYTE* XLuaUnpaker::ExpandValue(lua_State* L, const BYTE* pbyData, size_t uDataLen)
{
    const BYTE*     pbyResult   = NULL;
    BOOL            bRetCode    = false;
    const BYTE*     pbyPos      = pbyData;
    const BYTE*     pbyEnd      = pbyData + uDataLen;
    int             nType       = 0;

	XYLOG_FAILED_JUMP((size_t)(pbyEnd - pbyPos) >= sizeof(BYTE));
    nType = *pbyPos++;
    
	if (nType <= eLuaPackTypeBegin)
	{
		lua_pushnumber(L, nType);
		goto Exit1;
	}

    switch (nType)
    {
    case eLuaPackNumber:
        {
            double fValue = 0.0f;
			XYLOG_FAILED_JUMP((size_t)(pbyEnd - pbyPos) >= sizeof(double));
            memcpy((void*)&fValue, pbyPos, sizeof(double));
            pbyPos += sizeof(double);
            lua_pushnumber(L, fValue);
        }
        break;
    	
    case eLuaPackInteger:
        {
        	size_t  uEncodeLen 	= 0;
            int64_t nValue   	= 0;
            double 	f64 		= 0;
            
            uEncodeLen = DecodeS64(&nValue, pbyPos, (size_t)(pbyEnd - pbyPos));
			XYLOG_FAILED_JUMP(uEncodeLen > 0);
            pbyPos += uEncodeLen;
            
            if (nValue >= 0)
            {
            	nValue += eLuaPackTypeBegin + 1;
            }
            
            f64 = (double)nValue;
            
            lua_pushnumber(L, f64);
        }
        break;    	
        
    case eLuaPackBoolTrue:
        lua_pushboolean(L, true);
        break;
        
    case eLuaPackBoolFalse:
        lua_pushboolean(L, false);
        break;        

    case eLuaPackLuaString:
        {
			size_t		uCodeLen = 0;
			uint64_t	uStrLen  = 0;
			XCachedStringInfo	inf;

			uCodeLen = DecodeU64(&uStrLen, pbyPos, (size_t)(pbyEnd - pbyPos));
			XYLOG_FAILED_JUMP(uCodeLen > 0);
			pbyPos += uCodeLen;

			XYLOG_FAILED_JUMP((size_t)(pbyEnd - pbyPos) >= uStrLen);
			inf.pbyPos = pbyPos;
			inf.uLen = (size_t)uStrLen;
            m_StringTable.push_back(inf);
            lua_pushlstring(L, (const char*)pbyPos, (size_t)uStrLen);
            pbyPos += uStrLen;
        }
        break;
        
    case eLuaPackIndexString:
		{
			uint64_t			uCachedIndex	= 0;
			size_t				uCodeLen		= 0;
			XCachedStringInfo	inf;

			uCodeLen = DecodeU64(&uCachedIndex, pbyPos, (size_t)(pbyEnd - pbyPos));
			XYLOG_FAILED_JUMP(uCodeLen > 0);
			pbyPos += uCodeLen;
			
			XYLOG_FAILED_JUMP(uCachedIndex < m_StringTable.size());
			inf = m_StringTable[(unsigned)uCachedIndex];

            lua_pushlstring(L, (const char*)inf.pbyPos, inf.uLen);
		}
    	break;

    case eLuaPackNill:
        lua_pushnil(L);
        break;

    case eLuaPackTable:
        {
            DWORD           dwSize      = 0;

			XYLOG_FAILED_JUMP((size_t)(pbyEnd - pbyPos) >= sizeof(DWORD));
            memcpy((void*)&dwSize, pbyPos, sizeof(DWORD));
            pbyPos   += sizeof(DWORD);

			XYLOG_FAILED_JUMP((size_t)(pbyEnd - pbyPos) >= dwSize);
            bRetCode = ExpandTable(L, pbyPos, dwSize);
			XYLOG_FAILED_JUMP(bRetCode);
            pbyPos += dwSize;
        }
        break;

    default:        
        goto Exit0;
    }

Exit1:
    pbyResult = pbyPos;
Exit0:
    return pbyResult;
}

BOOL XLuaUnpaker::ExpandTable(lua_State* L, const BYTE* pbyTable, size_t uTableLen)
{
    BOOL        bResult   = false;
    const BYTE* pbyOffset = pbyTable;
    const BYTE* pbyTabEnd = pbyTable + uTableLen;

    lua_newtable(L);

    while (pbyOffset < pbyTabEnd)
    {
        pbyOffset = ExpandValue(L, pbyOffset, (size_t)(pbyTabEnd - pbyOffset));
		XYLOG_FAILED_JUMP(pbyOffset);

        pbyOffset = ExpandValue(L, pbyOffset, (size_t)(pbyTabEnd - pbyOffset));
		XYLOG_FAILED_JUMP(pbyOffset);

        lua_settable(L, -3);
    }

    bResult = true;
Exit0:
    return bResult;
}

