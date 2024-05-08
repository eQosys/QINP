#include "Platform.h"

Platform platform_from_string(const std::string& platform_str)
{
    if (platform_str == "linux") return Platform::Linux;
    if (platform_str == "windows") return Platform::Windows;
    if (platform_str == "macos") return Platform::MacOS;
    return Platform::Unknown;
}