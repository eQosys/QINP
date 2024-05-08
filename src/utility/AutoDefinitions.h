#pragma once

#include "Architecture.h"
#include "Platform.h"

#if defined(QINP_ARCHITECTURE_X86)
    #define QINP_CURRENT_ARCHITECTURE Architecture::x86
#elif defined(QINP_ARCHITECTURE_QIPU)
    #define QINP_CURRENT_ARCHITECTURE Architecture::QIPU
#else
    #define QINP_CURRENT_ARCHITECTURE Architecture::Unknown
    #error "Unknown architecture!"
#endif

#if defined(QINP_PLATFORM_UNIX)
    #define QINP_CURRENT_PLATFORM Platform::Linux
#elif defined(QINP_PLATFORM_WINDOWS)
    #define QINP_CURRENT_PLATFORM Platform::Windows
#elif defined(QINP_PLATFORM_APPLE)
    #define QINP_CURRENT_PLATFORM Platform::MacOS
#else
    #define QINP_CURRENT_PLATFORM Platform::Unknown
    #error "Unknown platform!"
#endif