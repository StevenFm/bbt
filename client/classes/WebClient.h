//pragma once

class XWebStream
{
public:
	virtual ~XWebStream() {}
	virtual BOOL Save(void* pvData, size_t uLen) = 0;
	virtual BOOL PushResultTable(lua_State* L) = 0;
	virtual int PushProgressStatus(lua_State* L) = 0;
	virtual BOOL SetProgressStatus(double dwFileSize, double dwDownloadLen) = 0;
};

class XWebMemStream :public XWebStream
{
public:
	XWebMemStream();
	virtual ~XWebMemStream();

	virtual BOOL Save(void* pvData, size_t uLen);
	virtual BOOL PushResultTable(lua_State* L);
	virtual int PushProgressStatus(lua_State* L);
	virtual BOOL SetProgressStatus(double dwFileSize, double dwDownloadLen);

private:
	char*  m_pszData;
	size_t m_uDataSize;
	size_t m_uDataLen;
};

class XWebFileStream :public XWebStream
{
public:
	XWebFileStream(const char szFile[]);
	virtual ~XWebFileStream();

	virtual BOOL Save(void* pvData, size_t uLen);
	virtual BOOL PushResultTable(lua_State* L);
	virtual int PushProgressStatus(lua_State* L);
	virtual BOOL SetProgressStatus(double dwFileSize, double dwDownloadLen);

private:
	FILE* m_pFile;
	double m_dwFileSize;
	double m_dwDownloadLen;
};

class XWebRequest
{
public:
	XWebRequest();
	virtual ~XWebRequest();

public:
	void*			m_pIndex;
	XWebStream*		m_pStream;
	char			m_szError[CURL_ERROR_SIZE];
};

typedef std::map<CURL*, XWebRequest*> XWebRequestTable;
typedef std::list<XWebRequest*> XWebDataList;

class XWebClient
{
public:
	XWebClient();
	virtual ~XWebClient();

public:
	BOOL Setup();
	void Clear();
	
	// XY_DELETE 每个XWebData*
	void Query(XWebDataList* pRetWebData);

	void* DownloadData(const char szUrl[]);
	void* DownloadFile(const char szUrl[], const char szFile[]);

	CURL* GetEncodeHandle(){ return m_pEncodeHandle; }
	XWebStream* GetWebStream(void* pIndex);

private:
	static size_t OnWebDataCallback(void* pvData, size_t uBlock, size_t uCount, void* pvArg);
	static int OnStreamProgress(void* pvArg, double dwTotalToDownload, double dwNowDownloaded, double dwTotalToUpload, double dwNowUpLoaded);

private:
	XWebRequestTable	m_WebRequestTable;
	CURLM*				m_pCurlMHandle;
	CURL*				m_pEncodeHandle;
};

extern int luaopen_base_webclient(lua_State * L);
