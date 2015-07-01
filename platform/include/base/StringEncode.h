#pragma once

// nLen表示输入字符串长度,不包含结尾的'\0'
// 返回的字符串,调用者有义务释放: XY_FREE_STRING(pszString)
wchar_t*	AllocWideCharFromUTF8(const char* pszString, int nLen = -1);
char*		AllocUTF8FromWideChar(const wchar_t* pszWString, int nLen = -1);
void		PlatformFreeString(void* pvString);

// 检查字符串是否具有Utf8的Bom头(3个字节)
BOOL		HasUtf8BomHeader(const char* pszText, int nLen = -1);

// 检查UTF8返回结构
// nBomLen          : 0 - 无bom头， 3 - 有3个字节bom头
// nWideCharLen     : 可以转化为多少个WideChar
// nValidUTF8Bytes  : 有效的utf-8字节(不包含BOM头和'\0')
// nStrLen          : 字符串长，不包括结尾'\0'，包括bom头
struct XUTF8Info 
{
    int nBomLen;
    int nWideCharLen;
    int nValidUTF8Bytes;
    int nStrLen;
};

void ParseUTF8(XUTF8Info* pParam, const char* pszUTF8String, int nLen = -1);

const char* GetText(const char szString[]);
