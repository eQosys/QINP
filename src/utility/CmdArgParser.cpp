#include "CmdArgParser.h"

#include "StringHelpers.h"
#include "errors/QinpError.h"

const char* CMD_ARG__POSITIONAL = "pos-args";

void register_argument(int& argc, const char**& argv, CmdArgMap& args, CmdFlags& flags, const CmdArgDesc<CmdFlag>& desc, std::string param)
{
    // add arg to map if not already existent
    args[desc.short_name];

    if (desc.modify_flags) // modify corresponding flag
        flags.set(desc.flag_to_set);

    switch (desc.param)
    {
    case CmdArgParam::Unused:
        if (!param.empty())
            throw QinpError("Argument -" + desc.short_name + "/--" + desc.long_name + " does not take any parameters");
        return;
    case CmdArgParam::Single:
        if (!args[desc.short_name].empty())
            throw QinpError("Argument -" + desc.short_name + "/--" + desc.long_name + " has already been specified");
        break;
    case CmdArgParam::Multi:
        break;
    }

    if (param.empty()) // no parameter attached to argument
    {
        if (argc-- == 0)
            throw QinpError("Argument -" + desc.short_name + "/--" + desc.long_name + " requires a parameter");
        param = argv++[0];
    }

    // add parameter to argument map
    args[desc.short_name].push_back(param);
}

void parse_cmd_args(int argc, const char** argv, const std::vector<CmdArgDesc<CmdFlag>>& argDescs, CmdArgMap& args_out, CmdFlags& flags_out)
{
    while (argc > 0)
    {
        --argc;
        std::string arg = argv++[0];

        if (arg == "--") // remaining args are positional
        {
            while (argc-- > 0)
                args_out[CMD_ARG__POSITIONAL].push_back(argv++[0]);

            break;
        }

        if (startswith(arg, "--")) // long argument
        {
            size_t eq_pos = arg.find("=");
            bool has_param_attached = eq_pos != std::string::npos;
            std::string param = has_param_attached ? arg.substr(eq_pos + 1) : "";
            arg = arg.substr(2, eq_pos - 2);

            if (has_param_attached && param.empty())
                throw QinpError("Argument --" + arg + " requires a parameter");

            bool found_arg = false;
            for (const auto& desc : argDescs)
            {
                if (desc.long_name.empty() || desc.long_name != arg)
                    continue;

                found_arg = true;
                register_argument(argc, argv, args_out, flags_out, desc, param);
                break;
            }

            if (!found_arg)
                throw QinpError("Unknown argument --" + arg);

            continue;
        }

        if (startswith(arg, "-")) // short argument
        {
            bool has_param_attached = arg.size() > 2;
            std::string param = has_param_attached ? arg.substr(2) : "";
            arg = arg.substr(1, 1);

            bool found_arg = false;
            for (const auto& desc : argDescs)
            {
                if (desc.short_name.empty() || desc.short_name != arg)
                    continue;

                found_arg = true;
                register_argument(argc, argv, args_out, flags_out, desc, param);
                break;
            }

            if (!found_arg)
                throw QinpError("Unknown argument -" + arg);

            continue;
        }

        // positional argument
        args_out[CMD_ARG__POSITIONAL].push_back(arg);
    }
}