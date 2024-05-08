#include "CmdArgParser.h"

#include "StringHelpers.h"

const char* CMD_ARG__POSITIONAL = "pos-args";

void register_argument(int& argc, const char**& argv, CmdArgMap& args, const CmdArgDesc& desc, std::string param)
{
    // add arg to map if not already existent
    args[desc.short_name];

    if (desc.param == CmdArgParam::Unused)
    {
        if (!param.empty())
            throw 1; // TODO: throw proper exception when a parameter was provided despite the argument not taking any parameters
        return;
    }

    switch (desc.param)
    {
    case CmdArgParam::Unused:
        if (!param.empty())
            throw 1; // TODO: throw proper exception when a parameter was provided despite the argument not taking any parameters
        return;
    case CmdArgParam::Single:
        if (!args[desc.short_name].empty())
            throw 1; // TODO: throw proper exception when 'arg_name' was already specified with a parameter
        break;
    case CmdArgParam::Multi:
        break;
    }

    if (param.empty()) // no parameter attached to argument
    {
        if (argc-- == 0)
            throw 1; // TODO: throw proper exception on missing parameter
        param = argv++[0];
    }

    args[desc.short_name].push_back(param);
}

CmdArgMap parse_cmd_args(int argc, const char** argv, const std::vector<CmdArgDesc>& argDescs)
{
    CmdArgMap args;

    while (argc > 0)
    {
        --argc;
        std::string arg = argv++[0];

        if (arg == "--") // remaining args are positional
        {
            while (argc-- > 0)
                args[CMD_ARG__POSITIONAL].push_back(argv++[0]);

            break;
        }

        if (startswith(arg, "--")) // long argument
        {
            size_t eq_pos = arg.find("=");
            bool has_param_attached = eq_pos != std::string::npos;
            std::string param = has_param_attached ? arg.substr(eq_pos + 1) : "";
            arg = arg.substr(2, eq_pos - 2);

            if (has_param_attached && param.empty())
                throw 1; // TODO: throw proper exception if parameter is missing after '='

            bool found_arg = false;
            for (const auto& desc : argDescs)
            {
                if (desc.long_name != arg)
                    continue;

                found_arg = true;
                register_argument(argc, argv, args, desc, param);
                break;
            }

            if (!found_arg)
                throw 1; // TODO: throw proper exception when arg does not match any of the descriptors

            continue;
        }

        if (startswith(arg, "-")) // short argument
        {
            bool has_param_attached = arg.size() > 1;
            std::string param = has_param_attached ? arg.substr(2) : "";
            arg = arg.substr(1, 1);

            bool found_arg = false;
            for (const auto& desc : argDescs)
            {
                if (desc.short_name != arg)
                    continue;

                found_arg = true;
                register_argument(argc, argv, args, desc, param);
                break;
            }

            if (!found_arg)
                throw 1; // TODO: throw proper exception when arg does not match any of the descriptors

            continue;
        }

        // positional argument
        args[CMD_ARG__POSITIONAL].push_back(arg);
    }

    return args;
}