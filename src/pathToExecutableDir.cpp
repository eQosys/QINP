#include "pathToExecutableDir.h"

#include <limits.h>
#include <filesystem>
#include <unistd.h>

#if defined(QINP_PLATFORM_WINDOWS)
std::string pathToExecutableDir()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::filesystem::path p(path);
    return p.parent_path().string() + "\\";
}

#elif defined(QINP_PLATFORM_UNIX)
std::string pathToExecutableDir()
{
    char path[PATH_MAX];
    uint64_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
    path[len] = '\0';
    std::filesystem::path p(path);
    return p.parent_path().string() + "/";
}

#endif