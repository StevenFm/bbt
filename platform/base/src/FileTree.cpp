#include "Base.h"
#include "FileTree.h"

#define IS_DIR_SEPATRATOR(wPos)         (wPos == '\\' || wPos == '/')

#pragma pack(1)
struct XTreeNodeData // 节点名+节点数据(包括子节点)+ 兄弟节点名+兄弟节点数据(包括子节点)
{
    BYTE        byIsFolder;
    union 
    {
        int32_t     nFileIndex;     // 只对文件节点有效
        uint32_t    uDataLen;       // 只对文件夹节点有效, 子项数据的长度
    };
};
#pragma pack()

XFileTreeFolder::XFileTreeFolder()
{
    pParent = NULL;
}

XFileTreeFolder::~XFileTreeFolder()
{
    XLinkTable::iterator it     = m_LinkTable.begin();
    XLinkTable::iterator it_end = m_LinkTable.end();

    for (; it != it_end; ++it)
    {
        char*        pwszNodeName = it->first;
        XVFSTreeNode*   pNode        = it->second;

        if (pNode->bFolder)
        {
            XFileTreeFolder* pFolderNode = (XFileTreeFolder*)pNode;
            XY_DELETE(pFolderNode);
        }
        else
        {
            XTreeFileNode* pFileNode = (XTreeFileNode*)pNode;
            XY_DELETE(pFileNode);
        }

        XY_FREE(pwszNodeName);
    }

    m_LinkTable.clear();
}

BOOL XFileTreeFolder::Save(size_t* puUsedSize, BYTE* pbyBuffer, size_t uBufferSize)
{
    BOOL			            bResult			    = false;
    int			                nRetCode			= false;
    BYTE*                       pbyPos              = pbyBuffer;
    size_t                      uLeftSize           = uBufferSize;
    std::string                strThisNameUtf8;
    size_t                      uNameLen            = 0;
    XTreeFileNode*              pFileNode           = NULL;
    XFileTreeFolder*            pFolderNode         = NULL;

    for (XLinkTable::iterator it = m_LinkTable.begin(); it != m_LinkTable.end(); ++it)
    {
        XTreeNodeData*      pNodeData   = NULL;
        XVFSTreeNode*       pSubNode    = it->second;
        size_t              uSubDataLen = 0;

        XYLOG_FAILED_JUMP(uLeftSize >= sizeof(XTreeNodeData));
        pNodeData = (XTreeNodeData*)pbyPos;
        pbyPos += sizeof(XTreeNodeData);
        uLeftSize -= sizeof(XTreeNodeData);

        pNodeData->byIsFolder = (BYTE)pSubNode->bFolder;

        strThisNameUtf8 = it->first;
        uNameLen = strThisNameUtf8.length() + 1;

        XYLOG_FAILED_JUMP(uLeftSize >= uNameLen);
        memcpy(pbyPos, strThisNameUtf8.c_str(), uNameLen);
        pbyPos += uNameLen;
        uLeftSize -= uNameLen;

        if (pSubNode->bFolder)
        {
            pFolderNode = (XFileTreeFolder*)pSubNode;

            nRetCode = pFolderNode->Save(&uSubDataLen, pbyPos, uLeftSize);
            XYLOG_FAILED_JUMP(nRetCode);

            pNodeData->uDataLen    = (uint32_t)uSubDataLen;

            pbyPos += uSubDataLen;
            uLeftSize -= uSubDataLen;
        }
        else
        {
            pFileNode = (XTreeFileNode*)pSubNode;
            pNodeData->nFileIndex = pFileNode->nFileIndex;
        }
    }

    *puUsedSize = (size_t)(pbyPos - pbyBuffer);

    bResult = true;
Exit0:
    return bResult;
}

BOOL XFileTreeFolder::Load(BYTE* pbyData, size_t uDataLen)
{
    BOOL	                bResult			    = false;
    int			            nRetCode			= false;
    std::string            wstrThisName;
    BYTE*                   pbyPos              = pbyData;   
    XFileTreeFolder*        pSubFolderNode      = NULL;
    XTreeFileNode*          pSubFileNode        = NULL;
    BYTE*                   pbyEnd              = pbyData + uDataLen;
    char*                pwszMyName          = NULL;
    XLinkTable::iterator    it;

    while (pbyPos < pbyEnd)
    {
        XTreeNodeData*  pThisTreeNodeData   = (XTreeNodeData*)pbyPos;
        BYTE*           pbyNameEnd          = pbyPos + sizeof(XTreeNodeData);
        size_t          uUtf8NameLen        = 0;

        pbyPos += sizeof(XTreeNodeData);

        while (pbyNameEnd < pbyEnd && *pbyNameEnd != '\0')
        {
            pbyNameEnd++;
        }
        XYLOG_FAILED_JUMP(pbyNameEnd < pbyEnd);
        pbyNameEnd++;
        uUtf8NameLen = (size_t)(pbyNameEnd - pbyPos);

        wstrThisName = (char*)pbyPos;

        pbyPos = pbyNameEnd;        

        it = m_LinkTable.find(&wstrThisName[0]);
        XYLOG_FAILED_JUMP(it == m_LinkTable.end());

        if (pThisTreeNodeData->byIsFolder)
        {
            XYLOG_FAILED_JUMP(pThisTreeNodeData->uDataLen <= (size_t)(pbyEnd - pbyPos));

            pSubFolderNode    = new XFileTreeFolder;

            pwszMyName        = strdup(wstrThisName.c_str());
            XYLOG_FAILED_JUMP(pwszMyName);

            pSubFolderNode->bFolder     = true;
            pSubFolderNode->pParent     = this;
            pSubFolderNode->pwszName    = pwszMyName;

            m_LinkTable[pwszMyName] = pSubFolderNode;

            nRetCode = pSubFolderNode->Load(pbyPos, pThisTreeNodeData->uDataLen);
            XYLOG_FAILED_JUMP(nRetCode);

            pbyPos += pThisTreeNodeData->uDataLen;
        }
        else
        {
            pSubFileNode      = new XTreeFileNode;

            pwszMyName        = strdup(wstrThisName.c_str());
            XYLOG_FAILED_JUMP(pwszMyName);

            pSubFileNode->bFolder   = false;
            pSubFileNode->pParent   = this;
            pSubFileNode->pwszName  = pwszMyName;

            pSubFileNode->nFileIndex = pThisTreeNodeData->nFileIndex;
            m_LinkTable[pwszMyName]  = pSubFileNode;
        }
    }

    XYLOG_FAILED_JUMP(pbyPos == pbyEnd);

    bResult = true;
Exit0:
    return bResult;
}

XTreeFileNode* XFileTreeFolder::GetFileNode(const char wszFileName[], BOOL bCreateIfNotExist)
{
    XTreeFileNode*              pResult                 = NULL;
    int 					    nRetCode			    = false;
    XFileTreeFolder*            pCurrentFolderNode      = NULL;
    char*                    pwszString				= NULL;
    char*                    pwszNextString			= NULL;
    char*                    pwszNodeName			= NULL;
    XFileTreeFolder*            pNewFolder              = NULL;
    XTreeFileNode*              pNewFile                = NULL;
    char					    wszFileNameCopy[XY_MAX_PATH];
    XLinkTable::iterator        it;

    nRetCode = SafeCopyString(wszFileNameCopy, wszFileName);
    XYLOG_FAILED_JUMP(nRetCode);

    pCurrentFolderNode = this;
    pwszString = wszFileNameCopy;
    pwszNextString = wszFileNameCopy;

    while (true)
    {
        char				  wPos          = *pwszNextString;

        if (wPos == '\0')
            break;

        if (!IS_DIR_SEPATRATOR(wPos))
        {
            pwszNextString++;
            continue;
        }

        *pwszNextString++ = '\0';

        it = pCurrentFolderNode->m_LinkTable.find(pwszString);
        if (it != pCurrentFolderNode->m_LinkTable.end())
        {
            XY_FAILED_JUMP(it->second->bFolder);
            pCurrentFolderNode = (XFileTreeFolder*)it->second;
            pwszString = pwszNextString;
            continue;
        }

        XY_FAILED_JUMP(bCreateIfNotExist);

        pwszNodeName = strdup(pwszString);
        XYLOG_FAILED_JUMP(pwszNodeName);

        pNewFolder = new XFileTreeFolder;

        pNewFolder->bFolder    = true;
        pNewFolder->pParent    = pCurrentFolderNode;
        pNewFolder->pwszName   = pwszNodeName;

        pCurrentFolderNode->m_LinkTable[pwszNodeName] = pNewFolder;
        pCurrentFolderNode = pNewFolder;
        pwszString = pwszNextString;
    }

    it = pCurrentFolderNode->m_LinkTable.find(pwszString);
    if (it == pCurrentFolderNode->m_LinkTable.end())
    {
        XY_FAILED_JUMP(bCreateIfNotExist);

        pwszNodeName = strdup(pwszString);
        XYLOG_FAILED_JUMP(pwszNodeName);

        pNewFile = new XTreeFileNode;

        pNewFile->bFolder     = false;
        pNewFile->pParent     = pCurrentFolderNode;
        pNewFile->pwszName    = pwszNodeName;
        pNewFile->nFileIndex = INVALID_FILE_INDEX;

        pCurrentFolderNode->m_LinkTable[pwszNodeName] = pNewFile;

        pResult = pNewFile;
    }
    else
    {
        XY_FAILED_JUMP(!it->second->bFolder);
        pResult = (XTreeFileNode*)it->second;
    }
    
Exit0:
    return pResult;
}

int XFileTreeFolder::DelFileNode(const char wszFileName[])
{
    int                 nFileIndex   = INVALID_FILE_INDEX;
    XTreeFileNode*      pFileNode    = GetFileNode(wszFileName, false);
    XFileTreeFolder*    pParent      = NULL;
    XFileTreeFolder*    pFolder      = NULL;

    XYLOG_FAILED_JUMP(pFileNode);

    nFileIndex = pFileNode->nFileIndex;
    pFolder = pFileNode->pParent;
    
    pFolder->m_LinkTable.erase(pFileNode->pwszName);
    XY_FREE(pFileNode->pwszName);
    XY_DELETE(pFileNode);

    while (pFolder->m_LinkTable.empty())
    {
        pParent = pFolder->pParent;
        if (pParent == NULL)
            break;

        pParent->m_LinkTable.erase(pFolder->pwszName);
        XY_FREE(pFolder->pwszName);
        XY_DELETE(pFolder);

        pFolder = pParent;
    }

Exit0:
    return nFileIndex;
}

XFileTreeFolder* XFileTreeFolder::GetFolder(const char wszFolderPath[])
{
    XFileTreeFolder*        pResult             = NULL;
    int                     nRetCode		    = false;
    char*                pwszString          = NULL;
    char*                pwszNextString      = NULL;
    char                 wPos                = '\0';
    XFileTreeFolder*        pCurrentFolderNode  = NULL;
    char				    wszFileNameCopy[XY_MAX_PATH];
    XLinkTable::iterator    it;

    assert(wszFolderPath);

    nRetCode = SafeCopyString(wszFileNameCopy, wszFolderPath);
    XYLOG_FAILED_JUMP(nRetCode);

    pCurrentFolderNode = this;
    pwszString = wszFileNameCopy;
    pwszNextString = wszFileNameCopy;
    wPos = *pwszNextString;

    while (wPos != '\0')
    {
        wPos = *pwszNextString;

        if (!IS_DIR_SEPATRATOR(wPos) && wPos != '\0')
        {
            pwszNextString++;
            continue;
        }

        *pwszNextString++ = '\0';

        it = pCurrentFolderNode->m_LinkTable.find(pwszString);
        if (it != pCurrentFolderNode->m_LinkTable.end())
        {
            XYLOG_FAILED_JUMP(it->second->bFolder);
            pCurrentFolderNode = (XFileTreeFolder*)it->second;
            pwszString = pwszNextString;
        }
    }

    pResult = pCurrentFolderNode;
Exit0:
    return pResult;
}

void XFileTreeFolder::Print(const char wszPrefix[])
{
    int                 nPrintSubNodeCount   = 0;
    std::string        strSubNodePrefix     = std::string(wszPrefix) + "│  ";
    std::string        strLastSubNodePrefix = std::string(wszPrefix) + "    ";

    for(XLinkTable::iterator it = m_LinkTable.begin(); it != m_LinkTable.end(); ++it)
    {
        XVFSTreeNode*   pSubNode            = it->second;
        const char*  pwszSubNodePrefix   = NULL;

        nPrintSubNodeCount++;
        if (nPrintSubNodeCount == (int)m_LinkTable.size())
        {
            printf("%s└─%s\n", wszPrefix, pSubNode->pwszName);
            pwszSubNodePrefix = strLastSubNodePrefix.c_str();
        }
        else
        {
            printf("%s├─%s\n", wszPrefix, pSubNode->pwszName);
            pwszSubNodePrefix = strSubNodePrefix.c_str();      
        }

        if (it->second->bFolder)
        {
            XFileTreeFolder* pSubFolder = (XFileTreeFolder*)it->second;
            pSubFolder->Print(pwszSubNodePrefix);
        }
    }
}
