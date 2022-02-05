#include <iostream>

#include <map>
#include <vector>

int main(int argc, char** argv, char** environ)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
		args.push_back(argv[i]);

	std::map<std::string, std::string> env;
	while (*environ)
	{
		std::string str = *environ;
		std::string key = str.substr(0, str.find_first_of("="));
		std::string value = str.substr(key.size() + 1);
		env.insert({ key, value });
		++environ;
	}

	for (auto& arg : args)
		std::cout << "Arg: " << arg << std::endl;

	for (auto& [key, value] : env)
		std::cout << "Env: " << key << " -> " << value << std::endl;

    std::cout << "-- END --" << std::endl;

    return 0;
}
