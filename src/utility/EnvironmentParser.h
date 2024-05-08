#pragma once

#include <map>
#include <string>

typedef std::map<std::string, std::string> EnvironmentMap;

EnvironmentMap parse_environment(const char** env);