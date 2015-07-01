#include "Base.h"

#include <set>

XTextFilter::XTextFilter()
{

}

XTextFilter::~XTextFilter()
{
    ClearAll();
}

BOOL _CheckFilterOnlyStar(const wchar_t* pszText)
{
    while (*pszText)
    {
        if (*pszText != '*')
            return false;

        pszText++;
    }
    return true;
}

BOOL XTextFilter::LoadFilterFile(const char* pszFilterFile)
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    size_t          uFileSize       = 0;
    char*           pszBuffer       = NULL;
    char*           pszPos          = NULL;
    int             nLine           = 0;
    wchar_t*        pszFilterText   = NULL;

    assert(pszFilterFile);

    pszBuffer = (char*)g_pFileHelper->ReadFileData(&uFileSize, pszFilterFile, 1);
    XYLOG_FAILED_JUMP(pszBuffer);
    pszBuffer[uFileSize] = '\0';

    for (pszPos = strtok(pszBuffer, "\r\n"); pszPos; pszPos = strtok(NULL, "\r\n"))
    {
        nLine++;

        XY_FREE(pszFilterText);

        pszFilterText = AllocWideCharFromUTF8(pszPos);
        if (pszFilterText == NULL)
        {
            Log(eLogError, "Text filter error(%ls : %d)", pszFilterFile, nLine);
            goto Exit0;
        }

        bRetCode = _CheckFilterOnlyStar(pszFilterText);
        if (bRetCode)
        {
            Log(eLogError, "Text filter error(%ls : %d)", pszFilterFile, nLine);
            goto Exit0;
        }

        AddFilterText(pszFilterText);
    }

    bResult = true;
Exit0:
    XY_FREE(pszFilterText);
    XY_DELETE_ARRAY(pszBuffer);
    return bResult;
}

BOOL XTextFilter::LoadWhiteListFile(const char* pszWhiteListFile)
{
    BOOL            bResult             = false;
    size_t          uFileSize           = 0;
    char*           pszBuffer           = NULL;
    char*           pszPos              = NULL;
    int             nLine               = 0;
    wchar_t*        pszWhiteListText    = NULL;

    assert(pszWhiteListFile);

    pszBuffer = (char*)g_pFileHelper->ReadFileData(&uFileSize, pszWhiteListFile, 1);
    XYLOG_FAILED_JUMP(pszBuffer);
    pszBuffer[uFileSize] = '\0';

    for (pszPos = strtok(pszBuffer, "\r\n"); pszPos; pszPos = strtok(NULL, "\r\n"))
    {
        nLine++;

        XY_FREE_STRING(pszWhiteListText);

        pszWhiteListText = AllocWideCharFromUTF8(pszPos);
        if (pszWhiteListText == NULL)
        {
            Log(eLogError, "Text whitelist error(%s : %d)\n", pszWhiteListFile, nLine);
            goto Exit0;
        }

        AddWhiteListText(pszWhiteListText);
    }

    bResult = true;
Exit0:
    XY_FREE_STRING(pszWhiteListText);
    XY_DELETE_ARRAY(pszBuffer);
    return bResult;
}

void XTextFilter::AddFilterText(const wchar_t* pszFilterText)
{
    assert(pszFilterText);

    if (pszFilterText[0] != '\0')
        m_FilterList.insert(std::wstring(pszFilterText));
}

void XTextFilter::AddWhiteListText(const wchar_t* pszWhiteListText)
{
    assert(pszWhiteListText);

    for (int i = 0; pszWhiteListText[i] != '\0'; i++)
    {
        if (pszWhiteListText[i] == '\n' || pszWhiteListText[i] == '\r' ||
            pszWhiteListText[i] == '\t' || pszWhiteListText[i] == ' '
            )
            continue;

        m_WhiteList.insert(pszWhiteListText[i]);
    }
}

void XTextFilter::ClearAll()
{
    m_FilterList.clear();
    m_WhiteList.clear();
}

BOOL XTextFilter::Check(const wchar_t* pszText)
{
    BOOL    bResult   = false;
    BOOL    bRetCode  = false;

    XY_FAILED_JUMP(pszText);
    XY_SUCCESS_JUMP(pszText[0] == '\0');

    if (!m_WhiteList.empty())
    {
        for (int i = 0; pszText[i] != '\0'; i++)
        {
            WHITELIST_TABLE::iterator it = m_WhiteList.find(pszText[i]);

            if (it == m_WhiteList.end())
                goto Exit0;
        }
    }

    for (KFILTER_TABLE::iterator it = m_FilterList.begin(); it != m_FilterList.end(); ++it)
    {
        bRetCode = TextMatch(pszText, it->c_str());
        if (bRetCode)
            goto Exit0;
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

void XTextFilter::Replace(wchar_t* pszText)
{
    XY_FAILED_JUMP(pszText);
    XY_SUCCESS_JUMP(pszText[0] == '\0');

    if (!m_WhiteList.empty())
    {
        for (int i = 0; pszText[i] != '\0'; i++)
        {
            WHITELIST_TABLE::iterator it= m_WhiteList.find(pszText[i]);

            if (it == m_WhiteList.end())
                pszText[i] = '*';
        }
    }

    for (KFILTER_TABLE::iterator it = m_FilterList.begin(); it != m_FilterList.end(); ++it)
    {
        while (TextReplace(pszText, it->c_str()))
        {
            ;
        }
    }

Exit1:
Exit0:
    return;
}
