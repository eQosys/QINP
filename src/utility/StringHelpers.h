#pragma once

#include <set>
#include <string>

bool startswith(std::string str, std::string start);
bool endswith(std::string str, std::string end);

std::string join(const std::set<std::string>& elements, const std::string& delimiter);

std::string replace_all(std::string str, const std::string& from, const std::string& to);