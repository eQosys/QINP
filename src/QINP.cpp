#include "Program.h"
#include "errors/ErrorPrinter.h"
#include "utility/Timer.h"
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

Architecture get_architecture_from_args(const CmdArgMap& args)
{
    Architecture architecture = QINP_CURRENT_ARCHITECTURE;
    if (args.find(CMD_ARG__ARCHITECTURE) != args.end())
        if ((architecture = architecture_from_string(args.at(CMD_ARG__ARCHITECTURE)[0])) == Architecture::Unknown)
            throw QinpError("Unknown architecture '" + args.at(CMD_ARG__ARCHITECTURE)[0] + "'");
    return architecture;
}

Platform get_platform_from_args(const CmdArgMap& args)
{
    Platform platform = QINP_CURRENT_PLATFORM;
    if (args.find(CMD_ARG__PLATFORM) != args.end())
        if ((platform = platform_from_string(args.at(CMD_ARG__PLATFORM)[0])) == Platform::Unknown)
            throw QinpError("Unknown platform '" + args.at(CMD_ARG__PLATFORM)[0] + "'");
    return platform;
}

void add_import_directories_to_program(ProgramRef program, const CmdArgMap& args, const EnvironmentMap& environ, bool verbose)
{
    // add stdlib import directory if specified in the environment
    if (environ.find("QINP_STDLIB") != environ.end())
        program->add_import_directory(environ.at("QINP_STDLIB"));
    else if (verbose)
        print_warning(QinpError("Missing STDLIB environment variable, compiling without standard library"));

    // add specified import directories
    if (args.find(CMD_ARG__IMPORT_DIR) != args.end())
        for (const auto& path_str : args.at(CMD_ARG__IMPORT_DIR))
            program->add_import_directory(path_str);
}

bool get_verbose_enabled(const CmdArgMap& args)
{
    return args.find(CMD_ARG__VERBOSE) != args.end();
}

ProgramRef initialize_program(Architecture architecture, Platform platform, bool verbose)
{
    Timer t("Program::init", verbose);
    Program::init(architecture, platform, verbose);
    return Program::get();
}

void import_source_files(ProgramRef program, const CmdArgMap& args, bool verbose)
{
    if (args.find(CMD_ARG__POSITIONAL) == args.end())
        throw QinpError("No source files specified");
        
    Timer t("Source file import", verbose);
    for (const auto& path_str : args.at(CMD_ARG__POSITIONAL))
        program->import_source_file(path_str, "", true);
}

int main(int argc, const char** argv, const char** env)
{
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

        Architecture architecture = get_architecture_from_args(args);
        Platform platform = get_platform_from_args(args);
        bool verbose = get_verbose_enabled(args);
        auto program = initialize_program(architecture, platform, verbose);

        add_import_directories_to_program(program, args, environ, verbose);

        import_source_files(program, args, verbose);

        {
            Timer t("Assembly generation", verbose);
            // TODO: generate assembly
        }

        {
            Timer t("Assembler invocation", verbose);
            // TODO: compile to binary
        }
    }
    catch (const qrawlr::GrammarException& err)
    {
        print_generic_error(std::string("Qrawlr: ") + err.what());
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& err)
    {
        print_error(err);
        return EXIT_FAILURE;
    }
    catch (std::exception& err)
    {
        print_generic_error(std::string("STD: ") + err.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        print_generic_error("An unknown error occured");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}