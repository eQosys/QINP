#include "Program.h"
#include "errors/ErrorPrinter.h"
#include "utility/HelpPrinter.h"
#include "utility/CmdArgParser.h"
#include "utility/AutoDefinitions.h"
#include "utility/EnvironmentParser.h"

const char* CMD_ARG__HELP               = "h";
const char* CMD_ARG__VERBOSE            = "v";
const char* CMD_ARG__IMPORT_DIR         = "i";
const char* CMD_ARG__OUTPUT_FILE        = "o";
const char* CMD_ARG__KEEP_INTERMEDIATE  = "k";
const char* CMD_ARG__RUN                = "r";
const char* CMD_ARG__ARCHITECTURE       = "A";
const char* CMD_ARG__PLATFORM           = "P";
const char* CMD_ARG__RUNARG             = "a";
const char* CMD_ARG__EXTERN_LIBRARY     = "x";
const char* CMD_ARG__EXPORT_SYMBOL_INFO = "e";
const char* CMD_ARG__EXPORT_COMMENTS    = "c";

const std::vector<CmdArgDesc> CMD_ARG_DESCRIPTORS = {
    { CMD_ARG__HELP,               "help",               CmdArgParam::Unused },
    { CMD_ARG__VERBOSE,            "verbose",            CmdArgParam::Unused },
    { CMD_ARG__IMPORT_DIR,         "import-dir",         CmdArgParam::Multi  },
    { CMD_ARG__OUTPUT_FILE,        "output-file",        CmdArgParam::Single },
    { CMD_ARG__KEEP_INTERMEDIATE,  "keep-intermediate",  CmdArgParam::Unused },
    { CMD_ARG__RUN,                "run",                CmdArgParam::Unused },
    { CMD_ARG__ARCHITECTURE,       "architecture",       CmdArgParam::Single },
    { CMD_ARG__PLATFORM,           "platform",           CmdArgParam::Single },
    { CMD_ARG__RUNARG,             "runarg",             CmdArgParam::Multi  },
    { CMD_ARG__EXTERN_LIBRARY,     "extern-library",     CmdArgParam::Multi  },
    { CMD_ARG__EXPORT_SYMBOL_INFO, "export-symbol-info", CmdArgParam::Single },
    { CMD_ARG__EXPORT_COMMENTS,    "export-comments",    CmdArgParam::Single }
};

int main(int argc, const char** argv, const char** env)
{
    (void)env; // currently unused

    try {
        // parse arguments and environment variables
        CmdArgMap args = parse_cmd_args(argc - 1, argv + 1, CMD_ARG_DESCRIPTORS);
        EnvironmentMap environ = parse_environment(env);

        // Print help if specified and exit
        if (args.find(CMD_ARG__HELP) != args.end())
        {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        // Retrieve (selected) architecture
        Architecture architecture = QINP_CURRENT_ARCHITECTURE;
        if (args.find(CMD_ARG__ARCHITECTURE) != args.end())
            architecture = architecture_from_string(args[CMD_ARG__ARCHITECTURE][0]);
        if (architecture == Architecture::Unknown)
            throw QinpError("Unknown architecture '" + args[CMD_ARG__ARCHITECTURE][0] + "'");

        // Retrieve (selected) platform
        Platform platform = QINP_CURRENT_PLATFORM;
        if (args.find(CMD_ARG__PLATFORM) != args.end())
            platform = platform_from_string(args[CMD_ARG__ARCHITECTURE][0]);
        if (platform == Platform::Unknown)
            throw QinpError("Unknown platform '" + args[CMD_ARG__PLATFORM][0] + "'");

        // Retrieve wether to show verbose info or not
        bool verbose = false;
        if (args.find(CMD_ARG__VERBOSE) != args.end())
            verbose = true;

        Program program(verbose);

        // add stdlib import directory if specified in the environment
        if (environ.find("QINP_STDLIB") != environ.end())
            program.add_import_directory(environ["QINP_STDLIB"]);
        else if (verbose)
            printf("[ WARN ]: Could not locate stdlib directory\n");

        // add specified import directories
        for (const auto& path_str : args[CMD_ARG__IMPORT_DIR])
            program.add_import_directory(path_str);

        // add source files
        if (args[CMD_ARG__POSITIONAL].empty())
            throw QinpError("No source files specified");
        for (const auto& path_str : args[CMD_ARG__POSITIONAL])
            program.import_source_file(path_str, "", true);

        // generate assembly

        // compile to binary
    }
    catch (const QinpError& err)
    {
        print_error(err);
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& err)
    {
        print_error(err);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        print_generic_error("An unknown error occured");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}