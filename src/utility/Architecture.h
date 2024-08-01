#pragma once

#include <string>

enum class Architecture
{
    Unknown,
    x86_64,
    QIPU
};

Architecture architecture_from_string(const std::string& arch_str);
std::string architecture_to_string(Architecture arch);