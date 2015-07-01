#pragma once

// 文本的通配符匹配算法
// Match和Replace只有微小的不同,算法本身是一样的
// 使用Replace的时候注意,可能需要循环调用Replace,直到没有Replace发生为止

template <typename CharType>
BOOL TextMatch(const CharType* pszText, const CharType* pszFilter)
{
    assert(pszText);
    assert(pszFilter);

    if (*pszFilter == '?')
    {
        if (TextMatch(pszText, pszFilter + 1))
        {
            return true;
        }

        return TextMatch(pszText + 1, pszFilter + 1);
    }

    if (*pszFilter == '*')
    {
        do 
        {
            if (TextMatch(pszText, pszFilter + 1))
            {
                return true;
            }
        } while (*pszText++ != '\0');

        return false;
    }

    if (*pszFilter == *pszText)
    {
        if (*pszFilter == '\0')
            return true;

        if (TextMatch(pszText + 1, pszFilter + 1))
        {
            return true;
        }
    }

    return false;
}

template <typename CharType>
BOOL TextReplace(CharType* pszText, const CharType* pszFilter)
{
    assert(pszText);
    assert(pszFilter);

    if (*pszFilter == '?')
    {
        if (TextReplace(pszText, pszFilter + 1))
        {
            return true;
        }

        return TextReplace(pszText + 1, pszFilter + 1);
    }

    if (*pszFilter == '*')
    {
        do 
        {
            if (TextReplace(pszText, pszFilter + 1))
            {
                return true;
            }
        } while (*pszText++ != '\0');

        return false;
    }

    if (*pszFilter == *pszText)
    {
        if (*pszFilter == '\0')
            return true;

        if (TextReplace(pszText + 1, pszFilter + 1))
        {
            *pszText = '*';
            return true;
        }
    }

    return false;
}
