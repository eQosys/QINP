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
const char* CMD_ARG__RENDER_QRAWLR_TREE = "";
const char* CMD_ARG__RENDER_SYMBOL_TREE = "";

const std::vector<CmdArgDesc<CmdFlag>> CMD_ARG_DESCRIPTORS = {
    { CMD_ARG__HELP,               "help",               CmdArgParam::Unused, CmdFlag::Help },
    { CMD_ARG__VERBOSE,            "verbose",            CmdArgParam::Unused, CmdFlag::Verbose },
    { CMD_ARG__IMPORT_DIR,         "import-dir",         CmdArgParam::Multi  },
    { CMD_ARG__OUTPUT_FILE,        "output-file",        CmdArgParam::Single },
    { CMD_ARG__KEEP_INTERMEDIATE,  "keep-intermediate",  CmdArgParam::Unused, CmdFlag::Keep_Intermediate_Files },
    { CMD_ARG__RUN,                "run",                CmdArgParam::Unused, CmdFlag::Run_Generated_Executable },
    { CMD_ARG__ARCHITECTURE,       "architecture",       CmdArgParam::Single },
    { CMD_ARG__PLATFORM,           "platform",           CmdArgParam::Single },
    { CMD_ARG__RUNARG,             "runarg",             CmdArgParam::Multi  },
    { CMD_ARG__EXTERN_LIBRARY,     "extern-library",     CmdArgParam::Multi  },
    { CMD_ARG__EXPORT_SYMBOL_INFO, "export-symbol-info", CmdArgParam::Single, CmdFlag::Export_Symbol_Info },
    { CMD_ARG__EXPORT_COMMENTS,    "export-comments",    CmdArgParam::Single, CmdFlag::Export_Comments },
    { CMD_ARG__RENDER_QRAWLR_TREE, "render-qrawlr-tree", CmdArgParam::Unused, CmdFlag::Render_Qrawlr_Tree },
    { CMD_ARG__RENDER_SYMBOL_TREE, "render-symbol-tree", CmdArgParam::Unused, CmdFlag::Render_Symbol_Tree },
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
        print_warning(QinpError("Missing STDLIB environment variable, compiling without standard library."));

    // add specified import directories
    if (args.find(CMD_ARG__IMPORT_DIR) != args.end())
        for (const auto& path_str : args.at(CMD_ARG__IMPORT_DIR))
            program->add_import_directory(path_str);
}

ProgramRef initialize_program(Architecture architecture, Platform platform, CmdFlags flags)
{
    Timer t("Program::init", flags.is_set(CmdFlag::Verbose));
    Program::init(architecture, platform, flags);
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
        CmdArgMap args;
        CmdFlags flags;
        parse_cmd_args(argc - 1, argv + 1, CMD_ARG_DESCRIPTORS, args, flags);
        EnvironmentMap environ = parse_environment(env);

        // Print help if specified and exit
        if (flags.is_set(CmdFlag::Help))
        {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        Architecture architecture = get_architecture_from_args(args);
        Platform platform = get_platform_from_args(args);
        bool verbose = flags.is_set(CmdFlag::Verbose);
        auto program = initialize_program(architecture, platform, flags);

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