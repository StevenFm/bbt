#pragma once

#include <string>
#include "lua.hpp"

// 为什么统一用table来表示对象,而且统一都建引用?
// userdata,很难解决一个问题: 
// 脚本中把对象"句柄"保存起来,而C层面的对象已经释放了,再次用这个"句柄"回调回C的时候,就会导致非法内存访问
// 当然,userdata也可以建引用,嗯,但是,无论如何,这个"引用"看来都是必须建的.
// 既然这样,也就没必要同时采用table + userdata两种机制了,省得复杂化.
// 当然,建立了引用不见得就一定要支持动态成员扩展,动态成员扩展为差错带来了麻烦:)
// 所以,可以考虑是否提供一个标志位用来控制动态成员扩展,当然,作用有多大呢?值得怀疑


enum LuaObjectMemberType
{
    eLuaObjectMemberType_Invalid,
    eLuaObjectMemberType_int,
    eLuaObjectMemberType_enum,
    eLuaObjectMemberType_DWORD,
    eLuaObjectMemberType_WORD,
    eLuaObjectMemberType_BYTE,
    eLuaObjectMemberType_time,
    eLuaObjectMemberType_small_bool,
    eLuaObjectMemberType_big_bool,
    eLuaObjectMemberType_float,
    eLuaObjectMemberType_double,
    eLuaObjectMemberType_string,
    eLuaObjectMemberType_std_str,
    eLuaObjectMemberType_function
};

#define LUA_OBJECT_POINTER "__obj_pointer__"

template <typename T>
int LuaFunctionAdpter(lua_State* L)
{
    T*                              pObj       = (T*)lua_touserdata(L, lua_upvalueindex(1));
    typename T::_ObjectMemberInfo*  pMember    = (typename T::_ObjectMemberInfo*)lua_touserdata(L, lua_upvalueindex(2));

    if (pObj == NULL || pMember == NULL || pMember->Function == NULL)
        return 0;

    return (pObj->*pMember->Function)(L);
}

template <typename T> 
int LuaIndex(lua_State* L)
{
    int                             nResult         = 0;
    int                             nTopIndex       = 0;
    T*                              pObj            = NULL;
    const char*                     pszKey          = NULL;
    typename T::_ObjectMemberInfo*  pMemberInfo     = NULL;
    void*                           pvAddr          = NULL;   
    std::string*                    pStdStr         = NULL;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 2);
    
    lua_pushstring(L, LUA_OBJECT_POINTER);
    lua_rawget(L, 1);

    XY_FAILED_JUMP(lua_isuserdata(L, -1));
    pObj    = (T*)lua_touserdata(L, -1);
    XY_FAILED_JUMP(pObj);

    lua_pop(L, 1);  // pop userdata

    pszKey      = lua_tostring(L, 2);
    XY_FAILED_JUMP(pszKey);

    luaL_getmetatable(L, T::_ms_pszMetaTableName);
    XY_FAILED_JUMP(lua_istable(L, -1));

    lua_pushstring(L, pszKey);
    lua_rawget(L, -2);

    XY_FAILED_JUMP(lua_isuserdata(L, -1));
    pMemberInfo = (typename T::_ObjectMemberInfo*)lua_touserdata(L, -1);
    XY_FAILED_JUMP(pMemberInfo);

    lua_settop(L, 2);

    pvAddr = ((BYTE*)pObj) + pMemberInfo->nOffset;

    switch (pMemberInfo->nType)
    {
    case eLuaObjectMemberType_int:
    case eLuaObjectMemberType_enum:
    case eLuaObjectMemberType_DWORD: // 这里DWORD当int是为了避免某些CPU浮点转换错误 
        assert(pMemberInfo->uSize == sizeof(int));
        lua_pushinteger(L, *(int*)pvAddr);
        break;

    case eLuaObjectMemberType_WORD:
        assert(pMemberInfo->uSize == sizeof(WORD));
        lua_pushinteger(L, *(WORD*)pvAddr);
        break;

    case eLuaObjectMemberType_BYTE:
        assert(pMemberInfo->uSize == sizeof(BYTE));
        lua_pushinteger(L, *(BYTE*)pvAddr);
        break;

    case eLuaObjectMemberType_time:
        assert(pMemberInfo->uSize == sizeof(time_t));
        lua_pushnumber(L, (double)*(time_t*)pvAddr);
        break;

    case eLuaObjectMemberType_small_bool:
        assert(pMemberInfo->uSize == sizeof(bool));
        lua_pushboolean(L, *(bool*)pvAddr);
        break;

    case eLuaObjectMemberType_big_bool:
        assert(pMemberInfo->uSize == sizeof(BOOL));
        lua_pushboolean(L, *(BOOL*)pvAddr);
        break;

    case eLuaObjectMemberType_float:
        assert(pMemberInfo->uSize == sizeof(float));
        lua_pushnumber(L, *(float*)pvAddr);
        break;

    case eLuaObjectMemberType_double:
        assert(pMemberInfo->uSize == sizeof(double));
        lua_pushnumber(L, *(double*)pvAddr);
        break;

    case eLuaObjectMemberType_string:
        lua_pushstring(L, (const char*)pvAddr);
        break;

    case eLuaObjectMemberType_std_str:
        pStdStr = (std::string*)pvAddr;
        lua_pushstring(L, pStdStr->c_str());
        break;

    case eLuaObjectMemberType_function:
        lua_pushlightuserdata(L, pObj);
        lua_pushlightuserdata(L, pMemberInfo);
        lua_pushcclosure(L, LuaFunctionAdpter<T>, 2);
        break;

    default:
        lua_pushnil(L);
    }

    nResult = 1;    
Exit0:    
    return nResult;
}


template <typename T>
int LuaNewIndex(lua_State* L)
{
    int                             nResult         = 0;
    int                             nTopIndex       = 0;
    T*                              pObj            = NULL;
    const char*                     pszKey          = NULL;
    const char*                     pszStr          = NULL;
    typename T::_ObjectMemberInfo*  pMemberInfo     = NULL;
    void*                           pvAddr          = NULL;
    size_t                          uStrLen         = 0;

    nTopIndex = lua_gettop(L);
    XY_FAILED_JUMP(nTopIndex == 3);
    
    lua_pushstring(L, LUA_OBJECT_POINTER);
    lua_rawget(L, 1);

    pObj    = (T*)lua_touserdata(L, -1);
    XY_FAILED_JUMP(pObj);

    lua_pop(L, 1);

    pszKey      = lua_tostring(L, 2);
    XY_FAILED_JUMP(pszKey);

    luaL_getmetatable(L, T::_ms_pszMetaTableName);
    XY_FAILED_JUMP(lua_istable(L, -1));

    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    pMemberInfo = (typename T::_ObjectMemberInfo*)lua_touserdata(L, -1);
    
    lua_pop(L, 2);

    if (pMemberInfo == NULL)
    {
        lua_rawset(L, -3);
        goto Exit1;
    }

    if(pMemberInfo->bReadOnly)
    {
        printf("[Lua] Try to write member readonly: %s::%s\n", T::_ms_pszClassName, pMemberInfo->pszName);
        goto Exit0;
    }

    pvAddr = ((BYTE*)pObj) + pMemberInfo->nOffset;

    switch (pMemberInfo->nType)
    {
    case eLuaObjectMemberType_int:
    case eLuaObjectMemberType_enum:
    case eLuaObjectMemberType_DWORD: // 这里DWORD当int是为了避免某些CPU浮点转换错误  
        assert(pMemberInfo->uSize == sizeof(int));
        *(int*)pvAddr = (int)lua_tointeger(L, -1);
        break;

    case eLuaObjectMemberType_WORD:
        assert(pMemberInfo->uSize == sizeof(WORD));
        *(WORD*)pvAddr = (WORD)lua_tointeger(L, -1);
        break;

    case eLuaObjectMemberType_BYTE:
        assert(pMemberInfo->uSize == sizeof(BYTE));
        *(BYTE*)pvAddr = (BYTE)lua_tointeger(L, -1);
        break;

    case eLuaObjectMemberType_time:
        assert(pMemberInfo->uSize == sizeof(time_t));
        *(time_t*)pvAddr = (time_t)lua_tonumber(L, -1);
        break;

    case eLuaObjectMemberType_small_bool:
        assert(pMemberInfo->uSize == sizeof(bool));
        *(bool*)pvAddr = !!lua_toboolean(L, -1);
        break;

    case eLuaObjectMemberType_big_bool:
        assert(pMemberInfo->uSize == sizeof(BOOL));
        *(BOOL*)pvAddr = lua_toboolean(L, -1);
        break;

    case eLuaObjectMemberType_float:
        assert(pMemberInfo->uSize == sizeof(float));
        *(float*)pvAddr = (float)lua_tonumber(L, -1);
        break;

    case eLuaObjectMemberType_double:
        assert(pMemberInfo->uSize == sizeof(double));
        *(double*)pvAddr = lua_tonumber(L, -1);
        break;

    case eLuaObjectMemberType_string:
        XY_FAILED_JUMP(lua_isstring(L, -1));
        pszStr = (const char*)lua_tostring(L, -1);
        XY_FAILED_JUMP(pszStr);
        uStrLen = strlen(pszStr);
        XY_FAILED_JUMP(uStrLen < pMemberInfo->uSize);
        strcpy((char*)pvAddr, pszStr);
        break;

    case eLuaObjectMemberType_std_str:
        XY_FAILED_JUMP(lua_isstring(L, -1));
        pszStr = (const char*)lua_tostring(L, -1);
        XY_FAILED_JUMP(pszStr);
        *(std::string*)pvAddr = pszStr;
        break;

    case eLuaObjectMemberType_function:
        break;

    default:
        goto Exit0;
    }

Exit1:
    nResult = 1;    
Exit0:    
    return nResult;
}

template <typename T>
void LuaRegisterClass(lua_State* L)
{
    int         nTopIndex               = lua_gettop(L);
    const char* pszMemberVarPrefix      = "m_";
    size_t      uMemberVarPrefixLen     = strlen(pszMemberVarPrefix);
    const char* pszMemberFuncPrefix     = "Lua";
    size_t      uMemberFuncPrefixLen    = strlen(pszMemberFuncPrefix);

    luaL_newmetatable(L, T::_ms_pszMetaTableName);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, (&LuaIndex<T>));
    lua_rawset(L, -3);

    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, (&LuaNewIndex<T>));
    lua_rawset(L, -3);

    typename T::_ObjectMemberInfo* pMember = T::GetMemberList();

    while (pMember->nType != eLuaObjectMemberType_Invalid)
    {
        const char* pszName  = pMember->pszName;
        size_t      uNameLen = strlen(pszName);

        if (pMember->nType == eLuaObjectMemberType_function)
        {
            if (uNameLen > uMemberFuncPrefixLen)
            {
                // 去掉class成员函数的"Lua"前缀
                int nCmp = memcmp(pszName, pszMemberFuncPrefix, uMemberFuncPrefixLen);
                if (nCmp == 0)
                {
                    pszName += uMemberFuncPrefixLen;
                }
            }
        }
        else
        {
            if (uNameLen > uMemberVarPrefixLen)
            {
                // 去掉class成员变量的"m_"前缀
                int nCmp = memcmp(pszName, pszMemberVarPrefix, uMemberVarPrefixLen);
                if (nCmp == 0)
                {
                    pszName += uMemberVarPrefixLen;
                }
            }
        }

        lua_pushstring(L, pszName);
        lua_pushlightuserdata(L, pMember);
        lua_rawset(L, -3);
        
        pMember++;
    }

    lua_settop(L, nTopIndex);
}

struct XLuaObjRef
{
    XLuaObjRef()
    {
        m_pLuaState = NULL;
        m_nLuaRef = LUA_NOREF;
    }
    // 如果出现这个断言,表明这个对象具有lua对象引用但是未进行释放
    // 请在合适的地方(如删除对象的时候)加上"LUA_CLEAR_REF(L);"
    ~XLuaObjRef()
    {
        Clear();
    }
    void Clear()
    {
        int nTop = 0;

        if (m_pLuaState)
        {
            nTop = lua_gettop(m_pLuaState);
        }

        XY_FAILED_JUMP(m_nLuaRef != LUA_NOREF);

        lua_rawgeti(m_pLuaState, LUA_REGISTRYINDEX, m_nLuaRef);
        XY_FAILED_JUMP(lua_istable(m_pLuaState, -1));

        lua_pushstring(m_pLuaState, LUA_OBJECT_POINTER);
        lua_pushnil(m_pLuaState);
        lua_rawset(m_pLuaState, -3);

        luaL_unref(m_pLuaState, LUA_REGISTRYINDEX, m_nLuaRef);

Exit0:
        if (m_pLuaState)
        {
            lua_settop(m_pLuaState, nTop);
        }

        m_pLuaState = NULL;
        m_nLuaRef = LUA_NOREF;
    }
    lua_State*  m_pLuaState;
    int         m_nLuaRef;
};

#define DECLARE_LUA_CLASS(ClassName)    \
    typedef int (ClassName::*LUA_MEMBER_FUNCTION)(lua_State*);    \
    struct _ObjectMemberInfo    \
    {   \
        int                     nType;      \
        const char*             pszName;    \
        int                     nOffset;    \
        size_t                  uSize;      \
        BOOL                    bReadOnly;  \
        LUA_MEMBER_FUNCTION     Function;   \
    };  \
    XLuaObjRef  _m_LuaRef;  \
    static const char* _ms_pszClassName;    \
    static const char* _ms_pszMetaTableName;    \
    static ClassName::_ObjectMemberInfo* GetMemberList();   \
    \
    virtual void LuaPushThis(lua_State* L)  \
    {   \
        if (_m_LuaRef.m_nLuaRef != LUA_NOREF)   \
        {   \
            assert(_m_LuaRef.m_pLuaState == L); \
            lua_rawgeti(L, LUA_REGISTRYINDEX, _m_LuaRef.m_nLuaRef); \
            return; \
        }   \
        \
        lua_newtable(L);    \
        lua_pushstring(L, LUA_OBJECT_POINTER);  \
        lua_pushlightuserdata(L, this); \
        lua_settable(L, -3);    \
        \
        luaL_getmetatable(L, _ms_pszMetaTableName); \
        if (lua_isnil(L, -1)) \
        {   \
            lua_remove(L, -1);  \
            LuaRegisterClass<ClassName>(L); \
            luaL_getmetatable(L, _ms_pszMetaTableName); \
        }   \
        lua_setmetatable(L, -2);    \
        \
        lua_pushvalue(L, -1);   \
        _m_LuaRef.m_pLuaState = L;  \
        _m_LuaRef.m_nLuaRef = luaL_ref(L, LUA_REGISTRYINDEX);  \
    }

#define LUA_CLEAR_REF() _m_LuaRef.Clear()

#define IMPL_LUA_CLASS_BEGIN(ClassName) \
    const char* ClassName::_ms_pszClassName = #ClassName;   \
    const char* ClassName::_ms_pszMetaTableName = #ClassName"_metatable_";  \
ClassName::_ObjectMemberInfo* ClassName::GetMemberList()    \
{   \
    typedef ClassName   __ThisClass__;  \
    static _ObjectMemberInfo _ms_MemberList[] =  \
    {

#define IMPL_LUA_CLASS_END()    \
        {eLuaObjectMemberType_Invalid, NULL, 0, 0, false, NULL},    \
    };  \
    return _ms_MemberList;  \
}

#define EXPORT_LUA_MEMBER(Type, Member, bReadOnly)  \
    {Type, #Member, offsetof(__ThisClass__, Member), sizeof(((__ThisClass__*)NULL)->Member), bReadOnly, NULL},

#define EXPORT_LUA_INT(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_int, Member, false)
#define EXPORT_LUA_INT_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_int, Member, true)

#define EXPORT_LUA_ENUM(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_enum, Member, false)
#define EXPORT_LUA_ENUM_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_enum, Member, true)

#define EXPORT_LUA_DWORD(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_DWORD, Member, false)
#define EXPORT_LUA_DWORD_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_DWORD, Member, true)

#define EXPORT_LUA_WORD(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_WORD, Member, false)
#define EXPORT_LUA_WORD_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_WORD, Member, true)

#define EXPORT_LUA_BYTE(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_BYTE, Member, false)
#define EXPORT_LUA_BYTE_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_BYTE, Member, true)

#define EXPORT_LUA_TIME(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_time, Member, false)
#define EXPORT_LUA_TIME_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_time, Member, true)

#define EXPORT_LUA_SMALL_BOOL(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_small_bool, Member, false)
#define EXPORT_LUA_SMALL_BOOL_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_small_bool, Member, true)

#define EXPORT_LUA_BIG_BOOL(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_big_bool, Member, false)
#define EXPORT_LUA_BIG_BOOL_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_big_bool, Member, true)

#define EXPORT_LUA_FLOAT(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_float, Member, false)
#define EXPORT_LUA_FLOAT_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_float, Member, true)

#define EXPORT_LUA_DOUBLE(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_double, Member, false)
#define EXPORT_LUA_DOUBLE_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_double, Member, true)

#define EXPORT_LUA_STRING(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_string, Member, false)
#define EXPORT_LUA_STRING_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_string, Member, true)

#define EXPORT_LUA_STD_STR(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_std_str, Member, false)
#define EXPORT_LUA_STD_STR_R(Member)   EXPORT_LUA_MEMBER(eLuaObjectMemberType_std_str, Member, true)

#define EXPORT_LUA_FUNCTION(Member) \
    {eLuaObjectMemberType_function, #Member, 0, 0, true, &__ThisClass__::Member},

template <typename T>
void Lua_PushObject(lua_State* L, T* pObj)
{
    if (pObj == NULL)
    {
        lua_pushnil(L);
        return;
    }
    pObj->LuaPushThis(L);
}

template <typename T>
T* Lua_ToObject(lua_State* L, int idx)
{
    T* pObj = NULL;
    if (lua_istable(L, idx))
    {
        lua_getfield(L, idx, LUA_OBJECT_POINTER);
        pObj = (T*)lua_touserdata(L, -1);
        lua_pop(L, 1);
    }
    return pObj;
}

template <typename T>
BOOL Lua_GetObjFunction(lua_State* L, T* pObj, const char cszFunc[])
{
    Lua_PushObject(L, pObj);
    lua_pushstring(L, cszFunc);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    return (lua_type(L, -1) == LUA_TFUNCTION);
}

struct XLuaConstValue
{
    const char* pszName;
    lua_Number  Value;
};

struct XLuaSafeStack
{
    XLuaSafeStack(lua_State* L) : m_pLua(L), m_nCount(0) 
    {
        m_nTop = lua_gettop(L);
    }

    ~XLuaSafeStack()
    {
        lua_settop(m_pLua, m_nTop);
    }

    lua_State* m_pLua;
    int        m_nCount;
    int        m_nTop;
};

BOOL  Lua_SetupEnv(lua_State* L);

// 搜索并加载脚本文件数
int   Lua_SearchScripts(lua_State* L, const char szDir[]);
void  Lua_ReloadModifiedFiles(lua_State* L);
void  Lua_ExportConst(lua_State* L, const char cszName[], XLuaConstValue* pExport, int nCount);
BOOL  Lua_LoadScript(lua_State* L, const char cszFileName[]);
BOOL  Lua_ExecuteString(lua_State* L, const char cszEnvName[], const char cszContent[]);
void  Lua_FreeScript(lua_State* L, const char cszFileName[]);
BOOL  Lua_IsScriptLoaded(lua_State* L, const char cszFileName[]);
BOOL  Lua_GetFunction(lua_State* L, const char cszFileName[], const char cszFunction[]);
BOOL  Lua_GetTableFunction(lua_State* L, const char cszTable[], const char cszFunction[]);

// luaSafeStack每次调用必须构造一个新的,不得复用!
BOOL  Lua_XCall(XLuaSafeStack& luaSafeStack, int nArgs, int nResults);
