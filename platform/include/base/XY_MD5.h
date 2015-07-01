#pragma once

BOOL GetDataMD5(char szMD5[64], const void* pvData, size_t uDataLen);
BOOL GetStringMD5(char szMD5[64], const char szString[]);
BOOL GetFileMD5(char szMD5[64], const char szFileName[]);
