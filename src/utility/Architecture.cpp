#include "Architecture.h"

Architecture architecture_from_string(const std::string& arch_str)
{
    if (arch_str == "x86_64") return Architecture::x86_64;
    if (arch_str == "qipu") return Architecture::QIPU;
    return Architecture::Unknown;
}