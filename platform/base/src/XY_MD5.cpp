#include "Base.h"
#include "Md5.h"

BOOL GetDataMD5(char szMD5[64], const void* pvData, size_t uDataLen)
{
    BOOL            bResult         = false;
    md5_state_t     md5_state;
    BYTE            byMD5Value[16];

    szMD5[0] = '\0';
    XY_FAILED_JUMP(uDataLen > 0);

    md5_init(&md5_state);

    md5_append(&md5_state, (unsigned char*)pvData, (int)uDataLen);

    md5_finish(&md5_state, byMD5Value);

    sprintf(
        szMD5,
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        byMD5Value[ 0], byMD5Value[ 1], byMD5Value[ 2], byMD5Value[ 3],
        byMD5Value[ 4], byMD5Value[ 5], byMD5Value[ 6], byMD5Value[ 7],
        byMD5Value[ 8], byMD5Value[ 9], byMD5Value[10], byMD5Value[11],
        byMD5Value[12], byMD5Value[13], byMD5Value[14], byMD5Value[15]
    );

    bResult = true;
Exit0:
    return bResult;
}

BOOL GetStringMD5(char szMD5[64], const char szString[])
{
    return GetDataMD5(szMD5, szString, strlen(szString));
}

BOOL GetFileMD5(char szMD5[64], const char szFileName[])
{
    BOOL            bResult         = false;
    BOOL            bRetCode        = false;
    size_t          uSize           = 0;
    BYTE*           pbyBuffer       = NULL;

    szMD5[0] = '\0';

    pbyBuffer = g_pFileHelper->ReadFileData((size_t*)&uSize, szFileName);
    XY_FAILED_JUMP(pbyBuffer);

    bRetCode = GetDataMD5(szMD5, pbyBuffer, uSize);
    XY_FAILED_JUMP(bRetCode);

    bResult = true;
Exit0:
    XY_DELETE_ARRAY(pbyBuffer);
    return bResult;
}

