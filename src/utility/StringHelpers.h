#pragma once

#include <set>
#include <string>

bool startswith(std::string str, std::string start);
bool endswith(std::string str, std::string end);

std::string join(const std::set<std::string>& elements, const std::string& delimiter);