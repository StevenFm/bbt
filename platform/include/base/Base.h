#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <list>
#include <vector>
#include <map>
#include <algorithm>

// OS和CPU平台宏使用说明:
// MSVC环境: 用_MSC_VER标识VC编译器和Windows系统,用_M_IX86/_M_X64区分32位/64位
// GCC环境: 用__linux标识操作系统(GCC内建宏),用__i386__/__x86_64__区分32位/64位

#ifdef _MSC_VER
#include "PlatformWindows.h"
#endif

#ifdef __linux
#include "PlatformLinux.h"
#endif

#ifdef __APPLE__
#include "PlatformApple.h"
#endif

#include "Thread.h"
#include "Log.h"
#include "XY_Time.h"
#include "ToolMacros.h"
#include "ToolFunctions.h"
#include "Mutex.h"
#include "InterlockedVar.h"
#include "StringEncode.h"
#include "TabFile.h"
#include "IniFile.h"
#include "Random.h"
#include "SocketMgr.h"
#include "SampleSocket.h"
#include "FileHelper.h"
#include "WinGetOpt.h"
#include "ValueCoder.h"
#include "NetworkAdapter.h"
#include "XY_MD5.h"
#include "TextMatch.h"
#include "TextFilter.h"
#include "FilePackage.h"
#include "FilePackageMulti.h"

#define   XY_MAX_PATH     512
