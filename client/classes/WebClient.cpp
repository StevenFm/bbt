#include "stdafx.h"

//curl库现在在cocos中已经有了，可以不用这种方式了
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	#include "platform/third_party/android/prebuilt/libcurl/include/curl/curl.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	#include "platform/third_party/ios/curl/curl.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	#include "curl.h"
#endif

#include "WebClient.h"

#define MEM_STREAM_DEFAULT_SIZE	(8 * 1024)
#define LUA_WEB_CLIENT_MT		("com.dpull.lib.WebClientMT")
#define LUA_WEB_CLIENT			("com.dpull.lib.WebClient")
#define INT_PTR_BUFFER			(64)

using namespace std;

XWebMemStream::XWebMemStream()
{
	m_uDataLen = 0;
	m_uDataSize = 0;
	m_pszData = NULL;
}

XWebMemStream::~XWebMemStream()
{
	if (m_pszData)
	{
		free(m_pszData);
		m_pszData = NULL;
	}
}

BOOL XWebMemStream::Save(void* pvData, size_t uLen)
{
	BOOL bResult = false;
	size_t uNewDataLen = m_uDataLen + uLen;

	if (uNewDataLen >= m_uDataSize)
	{
		m_uDataSize *= 2;
		m_uDataSize = Max(m_uDataSize, (size_t)MEM_STREAM_DEFAULT_SIZE);
		m_uDataSize = Max(uNewDataLen, m_uDataSize);

		m_pszData = (char*)realloc(m_pszData, m_uDataSize);
	}

	XYLOG_FAILED_JUMP(m_pszData);

	memcpy(m_pszData + m_uDataLen, pvData, uLen);
	m_uDataLen = uNewDataLen;

	bResult = true;
Exit0:
	return bResult;
}

BOOL XWebMemStream::PushResultTable(lua_State* L)
{
	lua_newtable(L);
	lua_pushlstring(L, m_pszData, m_uDataLen);
	lua_setfield(L, -2, "data");
	return true;
}

int XWebMemStream::PushProgressStatus(lua_State* L)
{
	lua_pushnumber(L, 0);
	lua_pushnumber(L, 0);
	return 2;
}

BOOL XWebMemStream::SetProgressStatus(double dwFileSize, double dwDownloadLen)
{
	return true;
}

XWebFileStream::XWebFileStream(const char szFile[])
{
	m_pFile = fopen(szFile, "wb");
	if (!m_pFile)
	{
		Log(eLogDebug, "XWebDataFileStream create file [%s] failed!", szFile);
	}

	m_dwFileSize = 0;
	m_dwDownloadLen = 0;
}

XWebFileStream::~XWebFileStream()
{
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}
BOOL XWebFileStream::Save(void* pvData, size_t uLen)
{
	BOOL bResult = false;
	size_t uResult = 0;

	XY_FAILED_JUMP(m_pFile);

	uResult = fwrite(pvData, uLen, 1, m_pFile);
	XYLOG_FAILED_JUMP(uResult == 1);

	bResult = true;
Exit0:
	return bResult;
}

BOOL XWebFileStream::PushResultTable(lua_State* L)
{
	lua_newtable(L);
	lua_pushboolean(L, m_pFile != NULL);
	lua_setfield(L, -2, "file");
	return true;
}

int XWebFileStream::PushProgressStatus(lua_State* L)
{
	lua_pushnumber(L, m_dwDownloadLen);
	lua_pushnumber(L, m_dwFileSize);
	return 2;
}

BOOL XWebFileStream::SetProgressStatus(double dwFileSize, double dwDownloadLen)
{
	m_dwFileSize = dwFileSize;
	m_dwDownloadLen = dwDownloadLen;
	return true;
}

XWebRequest::XWebRequest()
{
	m_pIndex = NULL;
	m_szError[0] = '\0';
	m_pStream = NULL;
}

XWebRequest::~XWebRequest()
{
	XY_DELETE(m_pStream);
}

XWebClient::XWebClient()
{
	m_pCurlMHandle = NULL;
	m_pEncodeHandle = NULL;
}

XWebClient::~XWebClient()
{
	Clear();
}

BOOL XWebClient::Setup()
{
	BOOL bResult = false;

	curl_global_init(CURL_GLOBAL_ALL);

	m_pCurlMHandle = curl_multi_init();
	XYLOG_FAILED_JUMP(m_pCurlMHandle);

	m_pEncodeHandle = curl_easy_init();
	XYLOG_FAILED_JUMP(m_pEncodeHandle);

	bResult = true;
Exit0:
	return bResult;
}

void XWebClient::Clear()
{
	for (XWebRequestTable::iterator it = m_WebRequestTable.begin(); it != m_WebRequestTable.end(); ++it)
	{
		CURL* pHandle = it->first;
		XWebRequest* pData = it->second;
		curl_easy_cleanup(pHandle);
		XY_DELETE(pData);
		curl_multi_remove_handle(m_pCurlMHandle, pHandle);
	}
	m_WebRequestTable.clear();

	if (m_pCurlMHandle)
	{
		curl_multi_cleanup(m_pCurlMHandle);
		m_pCurlMHandle = NULL;
	}

	if (m_pEncodeHandle)
	{
		curl_easy_cleanup(m_pEncodeHandle);
		m_pEncodeHandle = NULL;
	}

	curl_global_cleanup();
}

void XWebClient::Query(XWebDataList* pWebDataList)
{
	CURLMsg* pCurMsg = NULL;
	CURLMcode euRetCode = CURLM_LAST;
	int nRunningHandleCount = 0;
	int nLeftMessageCount = 0;

	euRetCode = curl_multi_perform(m_pCurlMHandle, &nRunningHandleCount);
	if (euRetCode != CURLM_OK && euRetCode != CURLM_CALL_MULTI_PERFORM)
	{
		Log(eLogDebug, "curl_multi_perform failed, error code:%d", euRetCode);
		goto Exit0;
	}

	while (true)
	{
		pCurMsg = curl_multi_info_read(m_pCurlMHandle, &nLeftMessageCount);
		if (!pCurMsg)
			break;

		if (pCurMsg->msg != CURLMSG_DONE)
			continue;

		XWebRequestTable::iterator it = m_WebRequestTable.find(pCurMsg->easy_handle);
		if (it != m_WebRequestTable.end())
		{
			curl_easy_cleanup(it->first);
			pWebDataList->push_back(it->second);
			m_WebRequestTable.erase(it);
			continue;
		}

		curl_multi_remove_handle(m_pCurlMHandle, pCurMsg->easy_handle);
	}

Exit0:
	return;
}

size_t XWebClient::OnWebDataCallback(void* pvData, size_t uBlock, size_t uCount, void* pvArg)
{
	XWebRequest* pWebData = (XWebRequest*)pvArg;
	assert(pWebData);

	size_t uLen = uBlock * uCount;
	if (pWebData->m_szError[0] != '\0')
		return uLen;

	if (!pWebData->m_pStream->Save(pvData, uLen))
		strcpy(pWebData->m_szError, "XStream error!");

	return uLen;
}

int XWebClient::OnStreamProgress(void* pvArg, double dwTotalToDownload, double dwNowDownloaded, double dwTotalToUpload, double dwNowUpLoaded)
{
	XWebRequest* pWebData = (XWebRequest*)pvArg;
	assert(pWebData);
	pWebData->m_pStream->SetProgressStatus(dwTotalToDownload, dwNowDownloaded);

	return 0;
}

void* XWebClient::DownloadData(const char szUrl[])
{
	void* pResult = 0;
	CURLMcode euRetCode = CURLM_LAST;
	XWebRequest* pWebData = new XWebRequest;
	CURL* pHandle = curl_easy_init();

	XYLOG_FAILED_JUMP(pWebData);
	XYLOG_FAILED_JUMP(pHandle);

	pWebData->m_pIndex = pHandle;

	pWebData->m_pStream = new XWebMemStream;
	XYLOG_FAILED_JUMP(pWebData->m_pStream);

	curl_easy_setopt(pHandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pHandle, CURLOPT_WRITEFUNCTION, XWebClient::OnWebDataCallback);
	curl_easy_setopt(pHandle, CURLOPT_WRITEDATA, pWebData);
	curl_easy_setopt(pHandle, CURLOPT_ERRORBUFFER, pWebData->m_szError);
	curl_easy_setopt(pHandle, CURLOPT_CONNECTTIMEOUT_MS, 5000);
	curl_easy_setopt(pHandle, CURLOPT_URL, szUrl);

	euRetCode = curl_multi_add_handle(m_pCurlMHandle, pHandle);
	XYLOG_FAILED_JUMP(euRetCode == CURLM_OK);

	m_WebRequestTable[pHandle] = pWebData;

	pResult = pWebData->m_pIndex;
Exit0:
	if (!pResult)
	{
		XY_DELETE(pWebData);
		if (pHandle)
		{
			curl_easy_cleanup(pHandle);
			pHandle = NULL;
		}
	}
	return pResult;
}

void* XWebClient::DownloadFile(const char szUrl[], const char szFile[])
{
	void* pResult = 0;
	CURLMcode euRetCode = CURLM_LAST;
	XWebRequest* pWebData = new XWebRequest;
	CURL* pHandle = curl_easy_init();

	XYLOG_FAILED_JUMP(pWebData);
	XYLOG_FAILED_JUMP(pHandle);

	pWebData->m_pIndex = pHandle;

	pWebData->m_pStream = new XWebFileStream(szFile);
	XYLOG_FAILED_JUMP(pWebData->m_pStream);

	curl_easy_setopt(pHandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pHandle, CURLOPT_WRITEFUNCTION, XWebClient::OnWebDataCallback);
	curl_easy_setopt(pHandle, CURLOPT_WRITEDATA, pWebData);
	curl_easy_setopt(pHandle, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(pHandle, CURLOPT_PROGRESSFUNCTION, XWebClient::OnStreamProgress);
	curl_easy_setopt(pHandle, CURLOPT_PROGRESSDATA, pWebData);
	curl_easy_setopt(pHandle, CURLOPT_ERRORBUFFER, pWebData->m_szError);
	curl_easy_setopt(pHandle, CURLOPT_CONNECTTIMEOUT_MS, 5000);
	curl_easy_setopt(pHandle, CURLOPT_URL, szUrl);

	euRetCode = curl_multi_add_handle(m_pCurlMHandle, pHandle);
	XYLOG_FAILED_JUMP(euRetCode == CURLM_OK);

	m_WebRequestTable[pHandle] = pWebData;

	pResult = pWebData->m_pIndex;
Exit0:
	if (!pResult)
	{
		XY_DELETE(pWebData);
		if (pHandle)
		{
			curl_easy_cleanup(pHandle);
			pHandle = NULL;
		}
	}
	return pResult;
}

XWebStream* XWebClient::GetWebStream(void* pIndex)
{
	XWebRequestTable::iterator it = m_WebRequestTable.find(pIndex);
	if (it != m_WebRequestTable.end())
		return it->second->m_pStream;
	return NULL;
}

int LuaCreate(lua_State* L)
{
	BOOL bResult = false;
	BOOL bRetCode = false;
	XWebClient* pClient = new XWebClient;
	XWebClient** ppUserValue = NULL;

	bRetCode = pClient->Setup();
	XYLOG_FAILED_JUMP(bRetCode);

	ppUserValue = (XWebClient**)lua_newuserdata(L, sizeof(pClient));
	*ppUserValue = pClient;

	luaL_getmetatable(L, LUA_WEB_CLIENT_MT);
	lua_setmetatable(L, -2);

Exit0:
	return 1;
}

XWebClient* GetWebClient(lua_State* L, int nIndex)
{
	XWebClient** ppUserValue = (XWebClient**)luaL_checkudata(L, nIndex, LUA_WEB_CLIENT_MT);
	if (ppUserValue)
	{
		return *ppUserValue;
	}
	return NULL;
}

int LuaDestory(lua_State* L)
{
	XWebClient* pClient = GetWebClient(L, 1);
	XY_DELETE(pClient);
	return 0;
}

int LuaQuery(lua_State* L)
{
	XWebClient* pClient = GetWebClient(L, 1);
	XWebDataList DataList;

	if (!pClient)
		return luaL_error(L, "self invalid");

	pClient->Query(&DataList);

	lua_newtable(L);
	for (XWebDataList::iterator it = DataList.begin(); it != DataList.end(); ++it)
	{
		XWebRequest* pData = *it;
		lua_pushlightuserdata(L, pData->m_pIndex);

		if (pData->m_szError[0] != '\0')
		{
			lua_newtable(L);
			lua_pushstring(L, pData->m_szError);
			lua_setfield(L, -2, "error");
		}
		else
		{
			pData->m_pStream->PushResultTable(L);
		}
		lua_settable(L, -3);
		XY_DELETE(pData);
	}

	return 1;
}

int LuaDownloadData(lua_State* L)
{
	XWebClient* pClient = GetWebClient(L, 1);
	const char* pszUrl = lua_tostring(L, 2);

	if (!pClient)
		return luaL_error(L, "self invalid");

	if (!pszUrl)
		return luaL_error(L, "url invalid");

	void* pIndex = pClient->DownloadData(pszUrl);
	if (pIndex)
		lua_pushlightuserdata(L, pIndex);
	else
		lua_pushnil(L);

	return 1;
}

int LuaDownloadFile(lua_State* L)
{
	XWebClient* pClient = GetWebClient(L, 1);
	const char* pszUrl = lua_tostring(L, 2);
	const char* pszFile = lua_tostring(L, 3);

	if (!pClient)
		return luaL_error(L, "self invalid");

	if (!pszUrl)
		return luaL_error(L, "url invalid");

	if (!pszFile)
		return luaL_error(L, "file invalid");

	void* pIndex = pClient->DownloadFile(pszUrl, pszFile);
	if (pIndex)
		lua_pushlightuserdata(L, pIndex);
	else
		lua_pushnil(L);

	return 1;
}

int LuaUrlEncoding(lua_State* L)
{
	XWebClient* pClient = GetWebClient(L, 1);
	size_t uLen = 0;
	const char* pszUrl = lua_tolstring(L, 2, &uLen);
	char* pszResult = NULL;

	if (!pClient)
		return luaL_error(L, "self invalid");

	if (!pszUrl)
		return luaL_error(L, "url invalid");

	pszResult = curl_easy_escape(pClient->GetEncodeHandle(), pszUrl, uLen);
	if (pszResult)
	{
		lua_pushstring(L, pszResult);
		curl_free(pszResult);
	}
	return 1;
}

int LuaQueryProgressStatus(lua_State* L)
{
	int nResult = 0;
	XWebClient* pClient = GetWebClient(L, 1);
	void* pIndex = lua_touserdata(L, 2);
	XWebStream* pStream = NULL;

	if (!pClient)
		return luaL_error(L, "self invalid");

	if (!pIndex)
		return luaL_error(L, "index invalid");

	pStream = pClient->GetWebStream(pIndex);
	if (!pStream)
		return luaL_error(L, "index not exist");

	nResult = pStream->PushProgressStatus(L);
	return nResult;
}


luaL_Reg WebClientCreateFuns[] = {
	{ "Create", LuaCreate },
	{ NULL, NULL }
};

luaL_Reg WebClientFuns[] = {
	{ "__gc", LuaDestory },
	{ "Query", LuaQuery },
	{ "DownloadData", LuaDownloadData },
	{ "DownloadFile", LuaDownloadFile },
	{ "UrlEncoding", LuaUrlEncoding },
	{ "QueryProgressStatus", LuaQueryProgressStatus },
	{ NULL, NULL }
};

int luaopen_base_webclient(lua_State * L)
{
	int nRetCode = false;

	nRetCode = luaL_newmetatable(L, LUA_WEB_CLIENT_MT);
	XYLOG_FAILED_JUMP(nRetCode);

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, WebClientFuns);

	luaL_register(L, LUA_WEB_CLIENT, WebClientCreateFuns);

Exit0:
	return 1;
}
