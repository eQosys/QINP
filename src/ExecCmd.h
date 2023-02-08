#pragma once

#include <string>

typedef std::pair<int, std::string> ExecCmdResult;

ExecCmdResult execCmd(const std::string& command, bool grabOutput = true);