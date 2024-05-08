#include <iostream>

#include "CmdArgParser.h"

int main(int argc, const char** argv, const char** env)
{
    (void)env; // currently unused

    std::vector<CmdArgDesc> argDescs = {
        { 'h', "help",               CmdArgParam::Unused },
        { 'v', "verbose",            CmdArgParam::Unused },
        { 'i', "import",             CmdArgParam::Multi  },
        { 'o', "output",             CmdArgParam::Single },
        { 'k', "keep",               CmdArgParam::Unused },
        { 'r', "run",                CmdArgParam::Unused },
        { 'p', "platform",           CmdArgParam::Single },
        { 'a', "runarg",             CmdArgParam::Multi  },
        { 'x', "extern",             CmdArgParam::Multi  },
        { 'e', "export-symbol-info", CmdArgParam::Unused },
        { 'c', "export-comments",    CmdArgParam::Unused }
    };

    // read arguments
    auto args = parse_cmd_args(argc - 1, argv + 1, argDescs);

    for (auto& arg : args)
    {
        std::cout << arg.first << " -> ";
        for (auto& param : arg.second)
        {
            std::cout << "'" << param << "';";
        }
        std::cout << std::endl;
    }

    // parse code qrawlr

    // generate assembly

    // generate binary
    return 0;
}