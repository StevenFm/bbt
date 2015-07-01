#pragma  once

class XTabFile
{
public:
	XTabFile();
	virtual ~XTabFile();

    virtual BOOL        Load(const char cszFileName[]);
	virtual BOOL	    Save(const char cszFileName[]);

	virtual void	    Clear();

	virtual int		    GetLineCount();
	virtual int		    GetColumnCount();

    virtual int         FindLine(const char cszLine[]);
    virtual int         FindColumn(const char cszCol[]);

    virtual BOOL        InsertLine(int nLine);
    virtual BOOL        AddLine(int nAddLineCount);
    virtual BOOL        DelLine(int nLine);

    virtual BOOL        InsertColumn(int nColumn);
    virtual BOOL        AddColumn(int nAddColumnCount);
    virtual BOOL        DelColumn(int nColumn);

    // 行列均为数字以进行检索
    virtual const char* GetString(int nLine, int nCol);
    virtual BOOL        GetString(int nLine, int nCol, char* pszBuffer, size_t uBufferSize);
    virtual BOOL        SetString(int nLine, int nCol, const char cszValue[]);

    virtual BOOL        GetInt16(int nLine, int nCol, int16_t* pnRetValue);
    virtual BOOL        SetInt16(int nLine, int nCol, int16_t  nSetValue);

    virtual BOOL        GetUInt16(int nLine, int nCol, uint16_t* puRetValue);
    virtual BOOL        SetUInt16(int nLine, int nCol, uint16_t  uSetValue);

    virtual BOOL        GetInt32(int nLine, int nCol, int32_t* pnRetValue);
    virtual BOOL        SetInt32(int nLine, int nCol, int32_t  nSetValue);

    virtual BOOL        GetUInt32(int nLine, int nCol, uint32_t* puRetValue);
    virtual BOOL        SetUInt32(int nLine, int nCol, uint32_t  uSetValue);
    
    virtual BOOL        GetInt64(int nLine, int nCol, int64_t* pnRetValue);
    virtual BOOL        SetInt64(int nLine, int nCol, int64_t  nSetValue);

    virtual BOOL        GetUInt64(int nLine, int nCol, uint64_t* puRetValue);
    virtual BOOL        SetUInt64(int nLine, int nCol, uint64_t  uSetValue);

    virtual BOOL        GetFloat(int nLine, int nCol, float* pfRetValue);
    virtual BOOL        SetFloat(int nLine, int nCol, float  fSetValue);

    virtual BOOL        GetDouble(int nLine, int nCol, double* pfRetValue);
    virtual BOOL        SetDouble(int nLine, int nCol, double  fSetValue);

    virtual BOOL        GetInt(int nLine, int nCol, int* pnValue);
    virtual BOOL        SetInt(int nLine, int nCol, int nValue);

    virtual BOOL        GetDword(int nLine, int nCol, DWORD* pdwValue);
    virtual BOOL        SetDword(int nLine, int nCol, DWORD dwValue);
    
    // 行为数字列为字串以进行检索
    virtual const char* GetString(int nLine, const char cszCol[]);
    virtual BOOL        GetString(int nLine, const char cszCol[], char* pszBuffer, size_t uBufferSize);
    virtual BOOL        SetString(int nLine, const char cszCol[], const char cszValue[]);

    virtual BOOL        GetInt16(int nLine, const char cszCol[], int16_t* pnRetValue);
    virtual BOOL        SetInt16(int nLine, const char cszCol[], int16_t  nSetValue);

    virtual BOOL        GetUInt16(int nLine, const char cszCol[], uint16_t* puRetValue);
    virtual BOOL        SetUInt16(int nLine, const char cszCol[], uint16_t  uSetValue);

    virtual BOOL        GetInt32(int nLine, const char cszCol[], int32_t* pnRetValue);
    virtual BOOL        SetInt32(int nLine, const char cszCol[], int32_t  nSetValue);

    virtual BOOL        GetUInt32(int nLine, const char cszCol[], uint32_t* puRetValue);
    virtual BOOL        SetUInt32(int nLine, const char cszCol[], uint32_t  uSetValue);
    
    virtual BOOL        GetInt64(int nLine, const char cszCol[], int64_t* pnRetValue);
    virtual BOOL        SetInt64(int nLine, const char cszCol[], int64_t  nSetValue);

    virtual BOOL        GetUInt64(int nLine, const char cszCol[], uint64_t* puRetValue);
    virtual BOOL        SetUInt64(int nLine, const char cszCol[], uint64_t  uSetValue);

    virtual BOOL        GetFloat(int nLine, const char cszCol[], float* pfRetValue);
    virtual BOOL        SetFloat(int nLine, const char cszCol[], float  fSetValue);

    virtual BOOL        GetDouble(int nLine, const char cszCol[], double* pfRetValue);
    virtual BOOL        SetDouble(int nLine, const char cszCol[], double  fSetValue);

    virtual BOOL        GetInt(int nLine, const char cszCol[], int* pnValue);
    virtual BOOL        SetInt(int nLine, const char cszCol[], int nValue);

    virtual BOOL        GetDword(int nLine, const char cszCol[], DWORD* pdwValue);
    virtual BOOL        SetDword(int nLine, const char cszCol[], DWORD dwValue);

    // 行列均为字串以进行检索
    virtual const char* GetString(const char cszLine[], const char cszCol[]);
    virtual BOOL        GetString(const char cszLine[], const char cszCol[], char* pszBuffer, size_t uBufferSize);
    virtual BOOL        SetString(const char cszLine[], const char cszCol[], const char cszValue[]);

    virtual BOOL        GetInt16(const char cszLine[], const char cszCol[], int16_t* pnRetValue);
    virtual BOOL        SetInt16(const char cszLine[], const char cszCol[], int16_t  nSetValue);

    virtual BOOL        GetUInt16(const char cszLine[], const char cszCol[], uint16_t* puRetValue);
    virtual BOOL        SetUInt16(const char cszLine[], const char cszCol[], uint16_t  uSetValue);

    virtual BOOL        GetInt32(const char cszLine[], const char cszCol[], int32_t* pnRetValue);
    virtual BOOL        SetInt32(const char cszLine[], const char cszCol[], int32_t  nSetValue);

    virtual BOOL        GetUInt32(const char cszLine[], const char cszCol[], uint32_t* puRetValue);
    virtual BOOL        SetUInt32(const char cszLine[], const char cszCol[], uint32_t  uSetValue);
    
    virtual BOOL        GetInt64(const char cszLine[], const char cszCol[], int64_t* pnRetValue);
    virtual BOOL        SetInt64(const char cszLine[], const char cszCol[], int64_t  nSetValue);

    virtual BOOL        GetUInt64(const char cszLine[], const char cszCol[], uint64_t* puRetValue);
    virtual BOOL        SetUInt64(const char cszLine[], const char cszCol[], uint64_t  uSetValue);

    virtual BOOL        GetFloat(const char cszLine[], const char cszCol[], float* pfRetValue);
    virtual BOOL        SetFloat(const char cszLine[], const char cszCol[], float  fSetValue);

    virtual BOOL        GetDouble(const char cszLine[], const char cszCol[], double* pfRetValue);
    virtual BOOL        SetDouble(const char cszLine[], const char cszCol[], double  fSetValue);

    virtual BOOL        GetInt(const char cszLine[], const char cszCol[], int* pnValue);
    virtual BOOL        SetInt(const char cszLine[], const char cszCol[], int nValue);

    virtual BOOL        GetDword(const char cszLine[], const char cszCol[], DWORD* pdwValue);
    virtual BOOL        SetDword(const char cszLine[], const char cszCol[], DWORD dwValue);

private:
    const char*         GetCell(int nLine, int nCol);
    BOOL                SetCell(int nLine, int nCol, const char cszValue[]);

    void                BuildIndex();
    
private:
    // 为了优化,用NULL指针表示了空字符串""
    typedef std::vector<char*>      XTabLine;
    typedef std::vector<XTabLine*>  XLineTable;

	struct XStringLess
	{
		bool operator()(const char* pszX, const char* pszY) const
		{
			return strcmp(pszX, pszY) < 0;
		}
	};


	typedef std::map<const char*, int, XStringLess> XNameTable;
	XNameTable       m_LineIndex;
    XNameTable       m_ColIndex;

    XLineTable       m_LineTable;

    int              m_nLineCount;
    int              m_nColumnCount;

    volatile u_long  m_ulRefCount;
};
