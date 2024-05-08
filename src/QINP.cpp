#include <iostream>

#include "CmdArgParser.h"

int main(int argc, const char** argv, const char** env)
{
    (void)env; // currently unused

    std::vector<CmdArgDesc> argDescs = {
        { 'h', "help",               false },
        { 'v', "verbose",            false },
        { 'i', "import",             true },
        { 'o', "output",             true },
        { 'k', "keep",               false },
        { 'r', "run",                false },
        { 'p', "platform",           true },
        { 'a', "runarg",             true },
        { 'x', "extern",             true },
        { 'e', "export-symbol-info", false },
        { 'c', "export-comments",    false }
    };

    // read arguments
    auto args = parse_cmd_args(argc, argv, argDescs);

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