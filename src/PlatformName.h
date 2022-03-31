#pragma once

#if defined(QINP_PLATFORM_WINDOWS)
	#define QINP_PLATFORM "windows"
#elif defined(QINP_PLATFORM_UNIX)
	#define QINP_PLATFORM "linux"
#elif defined(QINP_PLATFORM_APPLE)
	#define QINP_PLATFORM "macos"
#else
	#define QINP_PLATFORM "unknown"
	#error "Unknown platform!"
#endif