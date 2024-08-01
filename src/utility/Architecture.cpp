#include "Architecture.h"

#include "errors/QinpError.h"

Architecture architecture_from_string(const std::string& arch_str)
{
    if (arch_str == "x86_64") return Architecture::x86_64;
    if (arch_str == "qipu") return Architecture::QIPU;
    return Architecture::Unknown;
}

std::string architecture_to_string(Architecture arch)
{
    switch (arch)
    {
    case Architecture::Unknown: return "<unknown>";
    case Architecture::x86_64: return "x86_64";
    case Architecture::QIPU: return "qipu";
    default: throw QinpError("Unknown architecture with value " + std::to_string((int)arch));
    }
}