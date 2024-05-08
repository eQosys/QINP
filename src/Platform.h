#pragma once

#include <string>

enum class Platform
{
    Unknown,
    Linux,
    Windows,
    MacOS
};

Platform platform_from_string(const std::string& platform_str);