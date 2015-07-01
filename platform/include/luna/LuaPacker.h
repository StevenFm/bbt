#pragma once

#include <vector>

// 下面一系列函数用于在内存中填充一个用于传输/存储Lua变量的缓冲区
// 返回值均表示打包参数存储区之后的地址,出错则返回NULL

struct XCachedStringInfo
{
	const BYTE*	pbyPos;
	size_t	    uLen;
};

struct XLuaPaker
{
    XLuaPaker(size_t uBufferSize = 1024 * 64);
    ~XLuaPaker();

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize); // Compress

    BOOL PushNumber(double fValue);
    BOOL PushBool(BOOL bValue);
    BOOL PushString(const char cszValue[]);
    BOOL PushNill();

    BYTE* NewTable();
    void  SetTable(BYTE* pbyTable);

    BOOL  PushValue(lua_State* L, int nValueIndex, int nCount = 1);

    // 可以在push的过程中不检查返回值,最后才判断是否发生了溢出
    BOOL  IsOverflow() { return m_bIsOverflow; }

    void  Reset();

private:
    BOOL  PushTable(lua_State* L, int nIndex);
    BOOL  PushLString(const char* pszValue, size_t uLen);
    int   FindString(const void* pvData, size_t uLen);

    BYTE*   m_pbyBuffer;
    size_t  m_uBufferSize;
    BYTE*   m_pbyPos;
    BYTE*   m_pbyEnd;
    BOOL    m_bIsOverflow;
    int     m_nCallStack;
    
	std::vector<XCachedStringInfo> m_StringTable;
};

struct XLuaUnpaker
{
    XLuaUnpaker(size_t uBufferSize = 1024 * 64);
    ~XLuaUnpaker();

    // 返回展开值个数
    int  Expand(lua_State* L, const BYTE* pbyData, size_t uDataLen);
	void  Reset();
private:
    const BYTE* ExpandValue(lua_State* L, const BYTE* pbyData, size_t uDataLen);
    BOOL  ExpandTable(lua_State* L, const BYTE* pbyTable, size_t uTableLen);

    BYTE*	m_pbyBuffer;
    size_t  m_uBufferSize;
    std::vector<XCachedStringInfo> m_StringTable;
};
