#include "Platform.h"

#include "errors/QinpError.h"

Platform platform_from_string(const std::string& platform_str)
{
    if (platform_str == "linux") return Platform::Linux;
    if (platform_str == "windows") return Platform::Windows;
    if (platform_str == "macos") return Platform::MacOS;
    return Platform::Unknown;
}

std::string platform_to_string(Platform platform)
{
    switch (platform)
    {
    case Platform::Unknown: return "<unknown>";
    case Platform::Linux: return "linux";
    case Platform::Windows: return "windows";
    case Platform::MacOS: return "macos";
    default: throw QinpError("Unknown platform with value " + std::to_string((int)platform));
    }
}