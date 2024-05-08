#pragma once

#include <map>
#include <vector>
#include <string>

typedef std::map<std::string, std::vector<std::string>> CmdArgMap;

extern const char* CMD_ARG__POSITIONAL;

enum class CmdArgParam
{
    Unused,
    Single,
    Multi
};

struct CmdArgDesc
{
    std::string short_name;
    std::string long_name;
    CmdArgParam param;
};

CmdArgMap parse_cmd_args(int argc, const char** argv, const std::vector<CmdArgDesc>& argDescs);