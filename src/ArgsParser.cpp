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

Args parseArgs(const std::vector<std::string>& argsVec, const std::map<std::string, OptionInfo>& shortNameToOptionInfo)
{
	const std::map<std::string, OptionInfo> longNameToOptionInfo = [&shortNameToOptionInfo]()
	{
		std::map<std::string, OptionInfo> longNameToOptionInfo;
		for (auto& [shortName, optionInfo] : shortNameToOptionInfo)
			longNameToOptionInfo.insert({ optionInfo.longName, optionInfo });
		return longNameToOptionInfo;
	}();

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
			auto it = longNameToOptionInfo.find(key);
			if (it == shortNameToOptionInfo.end())
				THROW_QINP_ERROR("Unknown option: " + key);
			switch(it->second.type)
			{
			case OptionInfo::Type::NoValue:
				if (pos != std::string::npos)
					THROW_QINP_ERROR("Option " + key + " does not take value!");
				args.options.insert({ key, {} });
				break;
			case OptionInfo::Type::Multi:
				if (pos == std::string::npos)
					THROW_QINP_ERROR("Option " + key + " requires value!");
				args.options[key].push_back(arg.substr(pos + 1));
				break;
			case OptionInfo::Type::Single:
				if (pos == std::string::npos)
					THROW_QINP_ERROR("Option " + key + " requires value!");
				if(args.hasOption(key))
					THROW_QINP_ERROR("Option " + key + " already specified!");
				args.options.insert({ key, { arg.substr(pos + 1) } });
				break;
			}
			continue;
		}

		if (arg.find("-") == 0)
		{
			arg = arg.substr(1);
			auto pos = arg.find("=");
			auto key = arg.substr(0, pos);
			auto it = shortNameToOptionInfo.find(key);
			if (it == shortNameToOptionInfo.end())
				THROW_QINP_ERROR("Unknown option: " + key);
			key = it->second.longName;
			switch(it->second.type)
			{
			case OptionInfo::Type::NoValue:
				if (pos != std::string::npos)
					THROW_QINP_ERROR("Option " + key + " does not take value!");
				args.options.insert({ key, {} });
				break;
			case OptionInfo::Type::Multi:
				if (pos == std::string::npos)
					THROW_QINP_ERROR("Option " + key + " requires value!");
				args.options[key].push_back(arg.substr(pos + 1));
				break;
			case OptionInfo::Type::Single:
				if (pos == std::string::npos)
					THROW_QINP_ERROR("Option " + key + " requires value!");
				if(args.hasOption(key))
					THROW_QINP_ERROR("Option " + key + " already specified!");
				args.options.insert({ key, { arg.substr(pos + 1) } });
				break;
			}
			continue;
		}

		args.values.push_back(arg);
	}

	return args;
}