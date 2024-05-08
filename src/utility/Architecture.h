#pragma once

#include <string>

enum class Architecture
{
    Unknown,
    x86,
    QIPU
};

Architecture architecture_from_string(const std::string& arch_str);