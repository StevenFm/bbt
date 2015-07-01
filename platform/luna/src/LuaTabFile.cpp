#include "base/Base.h"
#include "Luna.h"
#include "LuaTabFile.h"


enum XTabValueType
{
	eTabValue_Invalid,
	eTabValue_String,
	eTabValue_Int,
	eTabValue_UInt,
	eTabValue_Double
};
typedef std::map<std::string, XTabValueType> XColumnTable;

static BOOL PushTabValueToLua(lua_State* L, XTabValueType eValueType, XTabFile& tab, int nLine, int nCol)
{
	BOOL	bResult		= false;
	int		nRetCode	= 0;

	switch (eValueType)
	{
	case eTabValue_String:
		{
			const char* pszValue = NULL;
			pszValue = tab.GetString(nLine, nCol);
			XY_FAILED_JUMP(pszValue);
			lua_pushstring(L, pszValue);
		}
		break;

	case eTabValue_Int:
		{
			int32_t nValue = 0;
			nRetCode = tab.GetInt32(nLine, nCol, &nValue);
			XY_FAILED_JUMP(nRetCode);
			lua_pushinteger(L, nValue);
		}
		break;

	case eTabValue_UInt:
		{
			uint32_t uValue = 0;
			nRetCode = tab.GetUInt32(nLine, nCol, &uValue);
			XY_FAILED_JUMP(nRetCode);
			lua_pushinteger(L, (int)uValue);
		}
		break;

	case eTabValue_Double:
		{
			double dValue = 0.0;
			nRetCode = tab.GetDouble(nLine, nCol, &dValue);
			XY_FAILED_JUMP(nRetCode);
			lua_pushnumber(L, dValue);
		}
		break;

	default:
		goto Exit0;
	}

	bResult = true;
Exit0:
	return bResult;
}

static XTabValueType TabValueTypeFromChar(char cType)
{
	XTabValueType eResult = eTabValue_String;

	switch (cType)
	{
	case 's':
		eResult = eTabValue_String;
		break;

	case 'd':
		eResult = eTabValue_Int;
		break;

	case 'u':
		eResult = eTabValue_UInt;
		break;

	case 'f':
		eResult = eTabValue_Double;
		break;

	default:
		eResult = eTabValue_Invalid;
		break;
	}

	return eResult;
}

static BOOL ParseColumnTable(XColumnTable& columnTab, lua_State* L, int nIndex)
{
	BOOL			bResult		= false;
	const char*		pszValue	= NULL;
	const char*		pszPos		= NULL;
	XTabValueType	eValueType;
	std::string		strKey;

	columnTab.clear();

	lua_pushvalue(L, nIndex);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		char cType = 's';

		pszValue = lua_tostring(L, -1);
		XY_FAILED_JUMP(pszValue);

		pszPos = strrchr(pszValue, ':');
		if (pszPos == NULL)
		{
			strKey = pszValue;
		}
		else
		{
			strKey.assign(pszValue, pszPos);
			cType = *(++pszPos);
		}

		eValueType = TabValueTypeFromChar(cType);

		columnTab[strKey] = eValueType;

		lua_pop(L, 1);
	}
	lua_remove(L, -1);

	bResult = true;
Exit0:
	return bResult;
}

static void GetTableColumnTypes(std::vector<XTabValueType>& columnType, XColumnTable& columnTab, XTabFile& tab)
{
	int  nColumnCount	= tab.GetColumnCount();
	BOOL bLoadAllColumn	= false;
	XTabValueType eDefaultType = eTabValue_String;
	XColumnTable::iterator it = columnTab.find("...");
	if (it != columnTab.end())
	{
		bLoadAllColumn = true;
		eDefaultType = it->second;
	}

	columnType.clear();
	columnType.resize(nColumnCount, eDefaultType);

	if (columnTab.empty())
		return;

	for (int nIndex = 1; nIndex <= nColumnCount; nIndex++)
	{
		const char* pszColName = tab.GetString(1, nIndex);

		XColumnTable::iterator it = columnTab.find(pszColName);
		if (it != columnTab.end())
			columnType[nIndex - 1] = it->second;
		else
			columnType[nIndex - 1] = bLoadAllColumn ? eDefaultType : eTabValue_Invalid;
	}
}

int LuaLoadTabFile(lua_State* L)
{
	int				nResult			= 0;
	int				nRetCode		= false;
	int				nArgCount		= lua_gettop(L);
	int				nLineCount		= 0;
	int				nColumnCount	= 0;
	const char*		pszFileName		= NULL;
	const char*		pszIndex		= NULL;
	int				nIndexCol		= -1;
	const char*		pszValue		= NULL;
	XColumnTable	columnTab;
	XTabFile		tab;
	std::vector<XTabValueType> columnTypes;

	// 解析传入参数：
	XY_FAILED_JUMP(nArgCount >= 1);
	pszFileName = lua_tostring(L, 1);
	XY_FAILED_JUMP(pszFileName);
	
	for (int i = 2; i <= nArgCount; i++)
	{
		int nType = lua_type(L, i);
		if (nType == LUA_TSTRING)
		{
			pszIndex = lua_tostring(L, i);
			XY_FAILED_JUMP(pszIndex);
		}
		else
		{
			XY_FAILED_JUMP(nType == LUA_TTABLE);
			nRetCode = ParseColumnTable(columnTab, L, i);
		}
	}	

	nRetCode = tab.Load(pszFileName);
	XY_FAILED_JUMP(nRetCode);

	GetTableColumnTypes(columnTypes, columnTab, tab);

	if (pszIndex)
	{
		nIndexCol = tab.FindColumn(pszIndex);
		XY_FAILED_JUMP(nIndexCol > 0);
	}

	// 生成返回的lua table
	nLineCount = tab.GetLineCount();
	nColumnCount = tab.GetColumnCount();

	lua_newtable(L);
	for (int nLine = 2; nLine <= nLineCount; nLine++)
	{
		if (pszIndex == NULL)
		{
			lua_pushinteger(L, nLine);
		}
		else
		{
			XTabValueType eType = columnTypes[nIndexCol - 1];
			XY_FAILED_JUMP(eType != eTabValue_Invalid);

			nRetCode = PushTabValueToLua(L, eType, tab, nLine, nIndexCol);
			if (!nRetCode)
				continue;
		}

		lua_newtable(L);
		for (int nCol = 1; nCol <= nColumnCount; nCol++)
		{
			XTabValueType eType = columnTypes[nCol - 1];
			if (eType == eTabValue_Invalid)
				continue;

			pszValue = tab.GetString(1, nCol);
			XY_FAILED_JUMP(pszValue);

			lua_pushstring(L, pszValue);

			nRetCode = PushTabValueToLua(L, eType, tab, nLine, nCol);
			if (!nRetCode)
			{
				lua_pushnil(L);
			}

			lua_settable(L, -3);
		}
		lua_settable(L, -3);
	}

	nResult = 1;
Exit0:
	return nResult;
}

int LuaLoadTabData(lua_State* L)
{
	int				nResult			= 0;
	int				nRetCode		= false;
	int				nArgCount		= lua_gettop(L);
	int				nLineCount		= 0;
	int				nColumnCount	= 0;
	const char*		pszFileName		= NULL;
	const char*		pszType			= NULL;
	XTabValueType	eValueType		= eTabValue_String;
	XColumnTable	columnTab;
	XTabFile		tab;

	// 解析传入参数：
	XY_FAILED_JUMP(nArgCount >= 1);
	pszFileName = lua_tostring(L, 1);
	XY_FAILED_JUMP(pszFileName);
	
	if (nArgCount >= 2)
	{
		pszType = lua_tostring(L, 2);
		XY_FAILED_JUMP(pszType);
	}

	nRetCode = tab.Load(pszFileName);
	XY_FAILED_JUMP(nRetCode);

	if (pszType != NULL)
	{
		eValueType = TabValueTypeFromChar(pszType[0]);
		XY_FAILED_JUMP(eValueType != eTabValue_Invalid);
	}

	// 生成返回的lua table
	lua_newtable(L);
	nLineCount = tab.GetLineCount();
	nColumnCount = tab.GetColumnCount();

	for (int nLine = 1; nLine <= nLineCount; nLine++)
	{
		lua_pushinteger(L, nLine);

		lua_newtable(L);
		for (int nCol = 1; nCol <= nColumnCount; nCol++)
		{
			lua_pushinteger(L, nCol);

			nRetCode = PushTabValueToLua(L, eValueType, tab, nLine, nCol);
			if (!nRetCode)
			{
				lua_pushnil(L);
			}

			lua_settable(L, -3);
		}
		lua_settable(L, -3);
	}

	nResult = 1;
Exit0:
	return nResult;
}

