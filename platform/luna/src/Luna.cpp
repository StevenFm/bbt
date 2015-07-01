#include "base/Base.h"
#include "Luna.h"
#include "LuaTabFile.h"
#include "LuaFunction.h"

#define LUA_THIS_FILE       "this"
#define LUA_FILE_METATABLE  "_file_metatable_"

// map<filename, modify_time>
typedef std::map<std::string, time_t> XScriptTable;
typedef std::map<lua_State*, XScriptTable> XLuaScriptTable;
static XLuaScriptTable g_ScriptTable;
static XMutex g_ScriptTableLock;

XScriptTable* GetScriptTable(lua_State* L, BOOL bCreateIfNotExist = false)
{
	XScriptTable* pResult = NULL;

	g_ScriptTableLock.Lock();
	XLuaScriptTable::iterator it = g_ScriptTable.find(L);
	if (it != g_ScriptTable.end())
	{
		pResult = &it->second;
	}
	else if (bCreateIfNotExist)
	{
		std::pair<XLuaScriptTable::iterator, bool> itInsert = g_ScriptTable.insert(make_pair(L, XScriptTable()));
		if (itInsert.second)
			pResult = &itInsert.first->second;
	}
	g_ScriptTableLock.Unlock();

	return pResult;
}

static int LuaImport(lua_State* L)
{
    BOOL        bResult       = false;
    int			nRetCode      = false;
    int         nTopIndex     = 0;
    const char* pszFileName   = NULL;
	std::string	strEnvName;
	std::string	strRelativePath;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 1);
    XY_FAILED_JUMP(lua_isstring(L, 1));

    pszFileName = lua_tostring(L, 1);
    XY_FAILED_JUMP(pszFileName);

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", pszFileName);
	XY_FAILED_JUMP(nRetCode);

	strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;

    lua_getglobal(L, strEnvName.c_str());
    if (lua_istable(L, -1))
        goto Exit1;

    lua_remove(L, -1);

    nRetCode = Lua_LoadScript(L, pszFileName);
    XY_FAILED_JUMP(nRetCode);

    lua_getglobal(L, strEnvName.c_str());

Exit1:
    bResult = true;
Exit0:
    if (!bResult)
    {
        lua_pushnil(L);
    }
    return 1;
}

// LoadScript与Import类似,区别在于Import不会重复加载
// 而LoadScript会强制加载,不管之前是否已经加载过了
static int LuaLoadScript(lua_State* L)
{
    BOOL        bResult       = false;
    int			nRetCode      = false;
    int         nTopIndex     = 0;
    const char* pszFileName   = NULL;
	std::string	strEnvName;
	std::string	strRelativePath;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 1);
    XY_FAILED_JUMP(lua_isstring(L, 1));

    pszFileName = lua_tostring(L, 1);
    XY_FAILED_JUMP(pszFileName);

    nRetCode = Lua_LoadScript(L, pszFileName);
    XY_FAILED_JUMP(nRetCode);

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", pszFileName);
	XY_FAILED_JUMP(nRetCode);

	strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;

    lua_getglobal(L, strEnvName.c_str());

    bResult = true;
Exit0:
    if (!bResult)
    {
        lua_pushnil(L);
    }
    return 1;
}

static int LuaFreeScript(lua_State* L)
{
    int         nTopIndex     = 0;
    const char* pszFileName   = NULL;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 1);
    XY_FAILED_JUMP(lua_isstring(L, 1));

    pszFileName = lua_tostring(L, 1);
    XY_FAILED_JUMP(pszFileName);

    Lua_FreeScript(L, pszFileName);
Exit0:
    return 0;
}

static int LuaGetScriptList(lua_State* L)
{
    lua_newtable(L);

	XScriptTable* pScripts = GetScriptTable(L);
	if (!pScripts)
		return 1;

	for (XScriptTable::iterator it = pScripts->begin(); it != pScripts->end(); ++it)
	{
		lua_pushstring(L, it->first.c_str());
		lua_pushnumber(L, (double)it->second);

		lua_settable(L, -3);
	}

    return 1;
}


static int  IndexForFileEnv(lua_State* L)
{
    BOOL        bResult       = false;
    int         nTopIndex     = 0;
    const char* pszKey        = NULL;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 2);

    pszKey = lua_tostring(L, 2);
    XY_FAILED_JUMP(pszKey);

    lua_getglobal(L, pszKey);

    bResult = true;
Exit0:
    if (!bResult)
    {
        lua_pushnil(L);
    }
    return 1;
}

BOOL Lua_SetupEnv(lua_State* L)
{
    BOOL        bResult     = false;
    int         nTopIndex   = lua_gettop(L);

    // 在某些编译器下,枚举类型的size可能与int不一样,在Index模板函数中,对此做了假定
    assert(sizeof(LuaObjectMemberType) == sizeof(int));

    // 在成员变量数据访问时,假定了int和DWORD是一样的size,参见Index函数
    assert(sizeof(int) == sizeof(DWORD));

    luaL_openlibs(L);

    lua_register(L, "Import", LuaImport);
    lua_register(L, "LoadScript", LuaLoadScript);
    lua_register(L, "FreeScript", LuaFreeScript);
	lua_register(L, "LoadTabFile", LuaLoadTabFile);
	lua_register(L, "LoadTabData", LuaLoadTabData);
    lua_register(L, "GetScriptList", LuaGetScriptList);
    
    RegisterHelperFunctions(L);

    // 文件环境metatable
    luaL_newmetatable(L, LUA_FILE_METATABLE);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, IndexForFileEnv);
    lua_settable(L, -3);

    bResult = true;
//Exit0:
    lua_settop(L, nTopIndex);
    return bResult;
}

static BOOL IsLuaFile(const char* pszFileName)
{
    int             nLen        = (int)strlen(pszFileName);
    const char*     pszExtName  = pszFileName + nLen;

    while (pszExtName >= pszFileName)
    {
        if (*pszExtName == '.')
        {
            int nCmp = STR_CASE_CMP(pszExtName, ".lua");

            return (nCmp == 0);
        }
        pszExtName--;
    }

    return false;
}

int Lua_SearchScripts(lua_State* L, const char szDir[])
{
	int			nResult		= 0;
	int			nRetCode	= 0;
	std::list<std::string>	fileList;

	nRetCode = g_pFileHelper->GetFileList(fileList, szDir, true);
	XY_FAILED_JUMP(nRetCode);

	for (std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
        const char* pszFileName = it->c_str();

		if (!IsLuaFile(pszFileName))
			continue;
        
        nRetCode = Lua_IsScriptLoaded(L, pszFileName);
        if (!nRetCode)
        {
			Lua_LoadScript(L, pszFileName);
        }        
	}

	nResult = g_ScriptTable.size();
Exit0:
	return nResult;
}

void  Lua_ReloadModifiedFiles(lua_State* L)
{
	BOOL bRetCode = false;

	XScriptTable* pScripts = GetScriptTable(L);
	if (!pScripts)
		return;

	for (XScriptTable::iterator it = pScripts->begin(); it != pScripts->end(); ++it)
	{
		const char*     pszFilePath = it->first.c_str();
		time_t			nLastModify = it->second;
		time_t			nModifyTime = g_pFileHelper->GetFileModifyTime(pszFilePath);
		if (nModifyTime != nLastModify)
		{
			bRetCode = Lua_LoadScript(L, pszFilePath);
			if (bRetCode)
			{
				Log(eLogDebug, "[luna]reload [%s] succeed!", pszFilePath);
			}
		}
	}
}

void  Lua_ExportConst(lua_State* L, const char cszName[], XLuaConstValue* pExport, int nCount)
{
    int nTopIndex = lua_gettop(L);

    lua_newtable(L);

    for (int i = 0; i < nCount; i++)
    {
        lua_pushstring(L, pExport[i].pszName);
        lua_pushnumber(L, pExport[i].Value);
        lua_settable(L, -3);
    }

    lua_setglobal(L, cszName);

    lua_settop(L, nTopIndex);
}

BOOL Lua_LoadScript(lua_State* L, const char cszFileName[])
{
    BOOL            bResult         = false;
    int             nRetCode        = 0;
    int             nTopIndex       = lua_gettop(L);
    BYTE*           pbyBuffer       = NULL;
    size_t          uFileSize       = 0;
    BOOL            bByteCode       = false;
    BYTE*           pbyData         = NULL;
    size_t          uDataLen        = 0;
	time_t			nFileModifyTime = 0;
	XScriptTable*	pScripts		= NULL;
	std::string		strEnvName;
	std::string		strRelativePath;

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", cszFileName);
	XY_FAILED_JUMP(nRetCode);

	nFileModifyTime = g_pFileHelper->GetFileModifyTime(strRelativePath.c_str());

	pScripts = GetScriptTable(L, true);
	if (pScripts)
	{
		(*pScripts)[strRelativePath] = nFileModifyTime;
	}
    
	strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;

    lua_getglobal(L, strEnvName.c_str());
    nRetCode = lua_istable(L, -1);
    if (!nRetCode)
    {
        lua_pop(L, 1);

        lua_newtable(L);    // file env

        luaL_getmetatable(L, LUA_FILE_METATABLE);
        XY_FAILED_JUMP(lua_istable(L, -1));
        lua_setmetatable(L, -2); 

        lua_setglobal(L, strEnvName.c_str());
    }

    lua_settop(L, nTopIndex);

	pbyBuffer = g_pFileHelper->ReadFileData(&uFileSize, cszFileName);
    XY_FAILED_JUMP(pbyBuffer);

    if (uFileSize >= sizeof(LUA_SIGNATURE))
    {
        nRetCode = memcmp(pbyBuffer, LUA_SIGNATURE, sizeof(LUA_SIGNATURE));
        if (nRetCode == 0)
        {
            bByteCode = true;
        }
    }

	pbyData     = pbyBuffer;
	uDataLen    = uFileSize;
	if ((!bByteCode) && HasUtf8BomHeader((const char*)pbyBuffer, (int)uFileSize))
	{
		pbyData     = pbyBuffer + 3;
		uDataLen    = uFileSize - 3;
	}

    nRetCode = luaL_loadbuffer(L, (const char*)pbyData, uDataLen, strRelativePath.c_str());
    if (nRetCode != 0)
    {
        const char* pszErr = lua_tostring(L, -1);
        if (pszErr)
        {
            Log(eLogError, "[Lua] luaL_loadbuffer error: %s", pszErr);
        }
        goto Exit0;
    }

    lua_getglobal(L, strEnvName.c_str());
    XY_FAILED_JUMP(lua_istable(L, -1));
    lua_setfenv(L, -2);

    // pcall
    nRetCode = lua_pcall(L, 0, 0, 0);
    if (nRetCode != 0)
    {
        const char* pszErr = lua_tostring(L, -1);
        if (pszErr)
        {
            Log(eLogError, "[Lua] lua_pcall error: %s", pszErr);
        }
        goto Exit0;
    }

    bResult = true;
Exit0:
    XY_DELETE_ARRAY(pbyBuffer);
    lua_settop(L, nTopIndex);
    return bResult;
}

BOOL  Lua_ExecuteString(lua_State* L, const char cszEnvName[], const char cszContent[])
{
    BOOL            bResult         = false;
    int             nRetCode        = 0;
    int             nTopIndex       = lua_gettop(L);
    size_t          uStrLen         = strlen(cszContent);

    lua_getglobal(L, cszEnvName);
    nRetCode = lua_istable(L, -1);
    if (!nRetCode)
    {
        lua_pop(L, 1);

        lua_newtable(L);    // file env

        luaL_getmetatable(L, LUA_FILE_METATABLE);
        XY_FAILED_JUMP(lua_istable(L, -1));
        lua_setmetatable(L, -2); 

        lua_setglobal(L, cszEnvName);
    }

    lua_settop(L, nTopIndex);

    nRetCode = luaL_loadbuffer(L, cszContent, uStrLen, cszEnvName);
    if (nRetCode != 0)
    {
        const char* pszErr = lua_tostring(L, -1);
        if (pszErr)
        {
            Log(eLogError, "[Lua] Lua_ExecuteString error: %s", pszErr);
        }
        goto Exit0;
    }

    lua_getglobal(L, cszEnvName);
    XY_FAILED_JUMP(lua_istable(L, -1));
    lua_setfenv(L, -2);

    // pcall
    nRetCode = lua_pcall(L, 0, 0, 0);
    if (nRetCode != 0)
    {
        const char* pszErr = lua_tostring(L, -1);
        if (pszErr)
        {
            Log(eLogError, "[Lua] lua_pcall error: %s", pszErr);
        }
        goto Exit0;
    }

    bResult = true;
Exit0:
    lua_settop(L, nTopIndex);
    return bResult;
}

void Lua_FreeScript(lua_State* L, const char cszFileName[])
{
    int     nTopIndex   = lua_gettop(L);
	int		nRetCode	= 0;
	std::string strEnvName;
	std::string	strRelativePath;

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", cszFileName);
	XY_FAILED_JUMP(nRetCode);

	strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;

    lua_pushnil(L);
    lua_setglobal(L, strEnvName.c_str());

Exit0:
    lua_settop(L, nTopIndex);
    return;
}

BOOL Lua_IsScriptLoaded(lua_State* L, const char cszFileName[])
{
    BOOL            bResult         = false;
    int             nRetCode        = 0;
    int             nTopIndex       = lua_gettop(L);
   	std::string     strEnvName;
	std::string		strRelativePath;

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", cszFileName);
	XY_FAILED_JUMP(nRetCode);

    strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;
    
    lua_getglobal(L, strEnvName.c_str());
    
    nRetCode = lua_istable(L, -1);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    lua_settop(L, nTopIndex);   
    return bResult;
}

BOOL Lua_GetFunction(lua_State* L, const char cszFileName[], const char cszFunction[])
{
    BOOL    bResult        = false;
    int     nRetCode       = false;
    int     nTopIndex      = lua_gettop(L);
	std::string strEnvName;
	std::string	strRelativePath;
	XScriptTable::iterator it;

	nRetCode = g_pFileHelper->GetRelativePath(strRelativePath, "", cszFileName);
	XY_FAILED_JUMP(nRetCode);

	strEnvName  = "__FILE__:";
	strEnvName += strRelativePath;

    lua_getglobal(L, strEnvName.c_str());

    nRetCode = lua_istable(L, -1);
    if (!nRetCode)
    {
        nRetCode = Lua_LoadScript(L, strRelativePath.c_str());
        XYLOG_FAILED_JUMP(nRetCode);

		lua_getglobal(L, strEnvName.c_str());
        nRetCode = lua_istable(L, -1);
        XYLOG_FAILED_JUMP(nRetCode);
    }

    lua_getfield(L, -1, cszFunction);

    nRetCode = lua_isfunction(L, -1);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        lua_settop(L, nTopIndex);
    }
    return bResult;
}

BOOL Lua_GetTableFunction(lua_State* L, const char cszTable[], const char cszFunction[])
{
    BOOL    bResult        = false;
    int     nRetCode       = false;
    int     nTopIndex      = 0;

    nTopIndex = lua_gettop(L);

    lua_getglobal(L, cszTable);

    nRetCode = lua_istable(L, -1);
    XY_FAILED_JUMP(nRetCode);

    lua_getfield(L, -1, cszFunction);

    nRetCode = lua_isfunction(L, -1);
    XY_FAILED_JUMP(nRetCode);

    bResult = true;
Exit0:
    if (!bResult)
    {
        lua_settop(L, nTopIndex);
    }
    return bResult;
}

BOOL Lua_XCall(XLuaSafeStack& luaStackSafe, int nArgs, int nResults)
{
    BOOL        bResult    = false;
    int         nRetCode   = 0;
    lua_State*  L          = luaStackSafe.m_pLua;
    int         nFuncIdx   = lua_gettop(L) - nArgs;

    assert(luaStackSafe.m_nCount == 0);
    luaStackSafe.m_nCount++;

    XYLOG_FAILED_JUMP(nFuncIdx > 0);

    lua_getglobal(L, "debug");
    XYLOG_FAILED_JUMP(lua_istable(L, -1));

    lua_getfield(L, -1, "traceback");
    XYLOG_FAILED_JUMP(lua_isfunction(L, -1));

    lua_remove(L, -2); // remove 'debug'
    lua_insert(L, nFuncIdx);
    nRetCode = lua_pcall(L, nArgs, nResults, nFuncIdx);
    if (nRetCode != 0)
    {
        const char* pszErrInfo = lua_tostring(L, -1);
        Log(eLogError, "[Lua] %s", pszErrInfo);
        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

