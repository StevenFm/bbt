#pragma once

#include <map>
#include <vector>

class XIniFile
{
public:
    XIniFile();
    virtual ~XIniFile();

    virtual BOOL    Load(const char szFileName[]);
    virtual BOOL    Save(const char szFileName[]);

    virtual void    Clear();

    virtual BOOL    GetNextSection(const char szSection[], char* pszNextSection);
    virtual BOOL    GetNextKey(const char szSection[], const char szKeyName[], char* pszNextKey);
    virtual int     GetSectionCount();
    virtual BOOL    IsSectionExist(const char szSection[]);
    virtual BOOL    IsKeyExist(const char szSection[], const char szKeyName[]);

    virtual BOOL    GetString(const char szSection[], const char szKeyName[], char* pszRetString, unsigned int uSize);
    virtual BOOL    GetInteger(const char szSection[], const char szKeyName[], int* pnValue);
    virtual BOOL    GetInteger2(const char szSection[], const char szKeyName[], int* pnValue1, int* pnValue2);
    virtual int     GetMultiInteger(const char szSection[], const char szKeyName[], int* pnValues, int nCount);
    virtual int     GetMultiLong(const char szSection[], const char szKeyName[], long* pnValues, int nCount);
    virtual BOOL    GetFloat(const char szSection[], const char szKeyName[], float* pfValue);
    virtual BOOL    GetFloat2(const char szSection[], const char szKeyName[], float* pfValue1, float* pfValue2);
    virtual int     GetMultiFloat(const char szSection[], const char szKeyName[], float* pfValues, int nCount);
    virtual BOOL    GetStruct(const char szSection[], const char szKeyName[], void* pStruct, unsigned int uSize);
    virtual BOOL    GetBool(const char szSection[], const char szKeyName[], int* pBool);

    virtual BOOL    AddSection(const char szSection[]);
    virtual BOOL    AddKey(const char szSection[], const char szKeyName[], const char szKeyValue[]);
    virtual BOOL    RemoveSection(const char szSection[]);
    virtual BOOL    RemoveKey(const char szSection[], const char szKeyName[]);

    virtual BOOL    SetString(const char szSection[], const char szKeyName[], const char szString[]);
    virtual BOOL    SetInteger(const char szSection[], const char szKeyName[], int nValue);
    virtual BOOL    SetInteger2(const char szSection[], const char szKeyName[], int nValue1, int nValue2);
    virtual BOOL    SetMultiInteger(const char szSection[], const char szKeyName[], int* pnValues, int nCount);
    virtual BOOL    SetMultiLong(const char szSection[], const char szKeyName[], long* pValues, int nCount);
    virtual BOOL    SetFloat(const char szSection[], const char szKeyName[], float fValue);
    virtual BOOL    SetFloat2(const char szSection[],  const char szKeyName[], float fValue1, float fValue2);
    virtual BOOL    SetMultiFloat(const char szSection[],  const char szKeyName[], float* pfValues, int nCount);
    virtual BOOL    SetStruct(const char szSection[], const char szKeyName[], void* lpStruct, unsigned int uSize);

private:
    struct XStrLess
    {
        bool operator()(const char* pszOne, const char* pszTwo) const
        {	
            return strcmp(pszOne, pszTwo) < 0;
        }
    };

    typedef std::vector<std::pair<char*, char*> >   XKeyList;
    typedef std::map<char*, XKeyList, XStrLess>     XSectionTable;
    typedef std::vector<char*>                      XSectionList;

    XKeyList*           GetKeyList(const char szSection[]);
    const char*         GetKeyValue(const char szSection[], const char szKeyName[]);
    XKeyList::iterator  FindKeyIt(XKeyList* pKeyList, const char szKeyName[]);
    XKeyList*           InsertSection(const char szSection[]);

    XSectionTable   m_SectionTable;
    XSectionList    m_SectionList;
    volatile ULONG  m_ulRefCount;
    char*           m_pszFileBuffer;
    char*           m_pszFileBufferEnd;
};
