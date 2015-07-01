// 文字过滤的通配符版本: Zhao chunfeng, yanrui
// 最后修改时间： 2010/11/16
// 1. 所有过滤不区分大小写
// 2. 通配符'?'表示匹配0个或1个任意字符
// 3. 通配符'*'表示匹配0个或多个任意字符
// 4. 过滤规则,每条最多32字节(含结尾的'\0')
// 5. 过滤处理的文字,一次最多1024字节(含结尾的'\0')
// 6. 过滤规则式,被过滤的文本,均为WideChar

#pragma once
#include <set>

class XTextFilter
{
public:
    XTextFilter();
    ~XTextFilter();

    BOOL LoadFilterFile(const char* pszFilterFile);
    BOOL LoadWhiteListFile(const char* pszWhiteListFile);

    void AddFilterText(const wchar_t* pszFilterText);
    void AddWhiteListText(const wchar_t* pszWhiteListText);

    void ClearAll();

    BOOL Check(const wchar_t* pszText);
    void Replace(wchar_t* pszText);

private:
    BOOL MatchOne(wchar_t* pWCharText, const wchar_t* pWCharFilter);
    BOOL MatchSub(wchar_t* pWCharText, const wchar_t* pWCharFilter);

private:    
    typedef std::set<std::wstring> KFILTER_TABLE;
    KFILTER_TABLE m_FilterList;

    typedef std::set<wchar_t> WHITELIST_TABLE;
    WHITELIST_TABLE m_WhiteList;
};
