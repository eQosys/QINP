#include <iostream>

#include <map>
#include <vector>
#include <fstream>
#include <sstream>

#include "Errors/QinpError.h"
#include "Tokenizer.h"
#include "ArgsParser.h"

std::string readTextFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw QinpError("Unable to open file!");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::map<std::string, std::string> getEnv(char** env)
{
	std::map<std::string, std::string> envMap;
	while (*env)
	{
		std::string str = *env;
		std::string key = str.substr(0, str.find_first_of("="));
		std::string value = str.substr(key.size() + 1);
		envMap.insert({ key, value });
		++env;
	}

	return envMap;
}

std::map<std::string, std::string> argNames = 
{
	{ "h", "help" },
	{ "v", "verbose" },
};

int main(int argc, char** argv, char** environ)
{
	try
	{
		auto args = parseArgs(getArgs(argc, argv), argNames);
		auto env = getEnv(environ);

		auto file = args.values[0];
	
		auto code = readTextFile(file);

		auto tokens = tokenize(code, file);

		for (auto& token : tokens)
			std::cout << token << std::endl;
	}
	catch (const QinpError& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
		return 1;
	}

    return 0;
}
