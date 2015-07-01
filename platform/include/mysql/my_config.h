#pragma once

#ifdef _MSC_VER
	#ifdef _M_IX86
	#include "my_config_win_x86.h"
	#endif

	#ifdef _M_X64
	#include "my_config_win_x64.h"
	#endif
#endif

#ifdef __linux
	#ifdef __i386__
	#include "my_config_linux_x86.h"
	#endif

	#ifdef __x86_64__
	#include "my_config_linux_x64.h"
	#endif
#endif

#ifdef __APPLE__
	#ifdef __i386__
	#include "my_config_mac_x86.h"
	#endif

	#ifdef __x86_64__
	#include "my_config_mac_x64.h"
	#endif
#endif
