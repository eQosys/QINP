#pragma once

#include <map>
#include <vector>
#include <string>

#include "CmdFlags.h"

typedef std::map<std::string, std::vector<std::string>> CmdArgMap;

extern const char* CMD_ARG__POSITIONAL;

enum class CmdArgParam
{
    Unused,
    Single,
    Multi
};

template<typename FlagEnum>
struct CmdArgDesc
{
    CmdArgDesc(const std::string& short_name, const std::string& long_name, CmdArgParam param)
        : short_name(short_name), long_name(long_name), param(param), modify_flags(false)
    {}
    CmdArgDesc(const std::string& short_name, const std::string& long_name, CmdArgParam param, FlagEnum flag_to_set)
        : short_name(short_name), long_name(long_name), param(param), modify_flags(true), flag_to_set(flag_to_set)
    {}
    std::string short_name;
    std::string long_name;
    CmdArgParam param;
    bool modify_flags;
    FlagEnum flag_to_set;
};

void parse_cmd_args(int argc, const char** argv, const std::vector<CmdArgDesc<CmdFlag>>& argDescs, CmdArgMap& args_out, CmdFlags& flags_out);