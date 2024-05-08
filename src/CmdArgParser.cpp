#include "CmdArgParser.h"

#include "StringHelpers.h"

const char* CMD_ARGS_POS_ARGS = "pos-args";

void register_argument(int& argc, const char**& argv, CmdArgMap& args, const CmdArgDesc& desc, std::string param)
{
    std::string arg_name = std::string(&desc.short_name, 1);

    // add arg to map if not already existent
    args[arg_name];

    if (!desc.has_param)
    {
        if (!param.empty())
            throw 1; // TODO: throw proper exception when a parameter was provided despite the argument not taking any parameters
        return;
    }

    if (param.empty()) // no parameter attached to argument
    {
        if (argc-- == 0)
            throw 1; // TODO: throw proper exception on missing parameter
        param = argv++[0];
    }

    args[arg_name].push_back(param);
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
                args[CMD_ARGS_POS_ARGS].push_back(argv++[0]);

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
            for (auto& desc : argDescs)
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
            for (auto& desc : argDescs)
            {
                if (std::string(&desc.short_name, 1) != arg)
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
        args[CMD_ARGS_POS_ARGS].push_back(arg);
    }

    return args;
}