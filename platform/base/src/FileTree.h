#pragma once

#include <map>

class XFileTreeFolder;

struct XVFSTreeNode
{
    BOOL               bFolder;
    XFileTreeFolder*   pParent;
    char*           pwszName;
};

struct XTreeFileNode : XVFSTreeNode
{
    int nFileIndex;
};

class XFileTreeFolder : protected XVFSTreeNode
{
public:
    XFileTreeFolder();
    ~XFileTreeFolder();

    BOOL Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize);
    BOOL Load(BYTE* pbyData, size_t uDataLen);

    XTreeFileNode*   GetFileNode(const char wszFilePath[], BOOL bCreateIfNotExist);
    int              DelFileNode(const char wszFilePath[]);

    // wszFilePath末尾不要加斜杠
    XFileTreeFolder* GetFolder(const char wszFilePath[]);

    void             Print(const char wszPrefix[] = "");

private:
    struct XNameLess
    {
	    bool operator()(const char* pwszX, const char* pwszY) const
	    {
		    return strcmp(pwszX, pwszY) < 0;
	    }
    };
    typedef std::map<char*, XVFSTreeNode*, XNameLess> XLinkTable;

    XLinkTable m_LinkTable;
};
