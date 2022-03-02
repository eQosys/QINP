#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

struct Args
{
	std::vector<std::string> values;
	std::map<std::string, std::vector<std::string>> options;

	bool hasOption(const std::string option);
	const std::vector<std::string>& getOption(const std::string option);
};

std::vector<std::string> getArgs(int argc, char** argv);

Args parseArgs(const std::vector<std::string>& argsVec, const std::map<std::string, std::string>& shortToLongNames);