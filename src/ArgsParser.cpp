#include "ArgsParser.h"

#include "Errors/QinpError.h"


bool Args::hasOption(const std::string option)
{
	return options.find(option) != options.end();
}
const std::vector<std::string>& Args::getOption(const std::string option)
{
	return options.at(option);
}

std::vector<std::string> getArgs(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 1; i < argc; ++i)
		args.push_back(argv[i]);

	return args;
}

Args parseArgs(const std::vector<std::string>& argsVec, const std::map<std::string, std::string>& shortToLongNames)
{
	Args args;

	bool nextIsPlain = false;

	for (std::string arg : argsVec)
	{
		if (arg == "--")
		{
			nextIsPlain = true;
			continue;
		}

		if (nextIsPlain)
		{
			args.values.push_back(arg);
			nextIsPlain = false;
			continue;
		}

		if (arg.find("--") == 0)
		{
			arg = arg.substr(2);
			auto pos = arg.find("=");
			auto key = arg.substr(0, pos);
			if (pos == std::string::npos)
				args.options[key].push_back("");
			else
				args.options[key].push_back(arg.substr(pos + 1));
			continue;
		}

		if (arg.find("-") == 0)
		{
			arg = arg.substr(1);
			auto pos = arg.find("=");
			auto key = arg.substr(0, pos);
			auto it = shortToLongNames.find(key);
			if (it == shortToLongNames.end())
				THROW_QINP_ERROR("Unknown option: " + key);
			key = it->second;

			if (pos == std::string::npos)
				args.options[key].push_back("true");
			else
				args.options[key].push_back(arg.substr(pos + 1));
			continue;
		}

		args.values.push_back(arg);
	}
	
	std::set<std::string> longNames;
	for (auto& stl : shortToLongNames)
		longNames.insert(stl.second);

	for (auto& opt : args.options)
		if (longNames.find(opt.first) == longNames.end())
			THROW_QINP_ERROR("Unknown option: " + opt.first);

	return args;
}