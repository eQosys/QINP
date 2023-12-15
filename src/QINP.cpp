#include <iostream>

#include <map>
#include <vector>
#include <filesystem>
#include <fstream>

#include "Errors/QinpError.h"
#include "Warning.h"
#include "Tokenizer.h"
#include "ArgsParser.h"
#include "ProgramGenerator.h"
#include "PlatformName.h"
#include "ExecCmd.h"
#include "ExportSymbolInfo.h"
#include "ExportComments.h"
#include "pathToExecutableDir.h"

#include "NasmGenerator.h"


void writeTextFileOverwrite(const std::string& filename, const std::string& text)
{
	std::ofstream file(filename, std::ios::trunc);
	if (!file.is_open())
		THROW_QINP_ERROR("Unable to open file!");
	file << text;
}

std::map<std::string, std::string> getEnv(char** env)
{
	std::map<std::string, std::string> envMap;
	while (*env)
	{
		std::string str = *env;
		std::string key = str.substr(0, str.find_first_of("="));
		std::string value = str.substr(key.size() + 1);
		envMap.insert({ key, value });
		++env;
	}

	return envMap;
}

std::map<std::string, OptionInfo> argNames = 
{
	{ "h", { "help", OptionInfo::Type::NoValue } },
	{ "v", { "verbose", OptionInfo::Type::NoValue } },
	{ "i", { "import", OptionInfo::Type::Multi } },
	{ "o", { "output", OptionInfo::Type::Single } },
	{ "k", { "keep", OptionInfo::Type::NoValue } },
	{ "r", { "run", OptionInfo::Type::NoValue } },
	{ "p", { "platform", OptionInfo::Type::Single } },
	{ "a", { "runarg", OptionInfo::Type::Multi } },
	{ "x", { "extern", OptionInfo::Type::Multi } },
	{ "e", { "export-symbol-info", OptionInfo::Type::Single } },
	{ "c", { "export-comments", OptionInfo::Type::Single } },
};

#define HELP_TEXT \
	"Usage: qinp [options] [input file]\n" \
	"Options:\n" \
	"  -h, --help\n" \
	"    Prints this help message.\n" \
	"  -v, --verbose\n" \
	"    Prints verbose output.\n" \
	"    Generates informational comments in the assembly code.\n" \
	"  -i, --import=[path]\n" \
	"    Specifies an import directory.\n" \
	"  -o, --output=[path]\n" \
	"    Specifies the output path of the generated executable.\n" \
	"  -k, --keep\n" \
	"    Keeps the generated assembly file.\n" \
	"  -r, --run\n" \
	"    Runs the generated program.\n" \
	"  -p, --platform=[platform]\n" \
	"    Specifies the target platform. (linux, windows, macos)\n" \
	"    Only linux and windows are supported for now.\n" \
	"  -a, --runarg=[arg]\n" \
	"    Specifies a single argument to pass to the generated program.\n" \
	"	  Only used when --run is specified.\n" \
	"  -x, --extern=[filepath]\n" \
	"    Specifies a library/object file to link against.\n" \
	"  -e, --export-symbol-info\n" \
	"    Writes the symbols (including unused ones) of the parsed program code\n" \
	"    and additional info to the specified file.\n" \
	"  -c, --export-comments\n" \
	"    Writes the comments of the parsed program code to the specified file.\n"

class Timer
{
public:
	Timer(const std::string& msg, bool doPrint)
		: m_doPrint(doPrint)
	{
		if (m_doPrint)
			std::cout << msg << "...\n";
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		if (!m_doPrint)
			return;

		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end - start;
		std::cout << " DONE: " << diff.count() << "s" << std::endl;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> end;
	bool m_doPrint;
};

int main(int argc, char** argv, char** _env)
{
	bool verbose = true;
	int runRet = 0;
	try
	{
		auto args = parseArgs(getArgs(argc, argv), argNames);
		auto env = getEnv(_env);

		if (args.hasOption("help"))
		{
			std::cout << HELP_TEXT;
			return 0;
		}

		std::string platform = QINP_PLATFORM;
		if (args.hasOption("platform"))
			platform = args.getOption("platform").front();

		if (platform != "linux" &&
			platform != "windows"
			)
		{
			std::cout << "Platform not supported!\n";
			return -1;
		}

		verbose = args.hasOption("verbose");

		if (args.values.empty())
		{
			std::cout << "Missing input files!\n";
			return -1;
		}

		const auto& inFilename = args.values[0];

		std::set<std::string> importDirs;
		if (args.hasOption("import"))
			for (auto& dir : args.getOption("import"))
				importDirs.insert(dir);
		importDirs.insert(pathToExecutableDir() + "stdlib/");

		std::string stdlibPath = pathToExecutableDir() + "stdlib";
		std::string stdlibOrigin = readTextFile(pathToExecutableDir() + "stdlib-origin.txt");
		stdlibOrigin.pop_back();

		ProgramRef program;
		auto comments = std::make_shared<CommentTokenMap>();
		{
			Timer timer("Parsing", verbose);
			auto code = readTextFile(inFilename);
			auto tokens = tokenize(code, std::filesystem::relative(inFilename, std::filesystem::current_path()).string(), comments);
			program = generateProgram(tokens, comments, importDirs, platform, inFilename, stdlibPath, stdlibOrigin);
		}

		if (args.hasOption("export-symbol-info"))
		{
			auto outFilename = args.getOption("export-symbol-info").front();
			std::ofstream outFile(outFilename);
			if (!outFile.is_open())
			{
				std::cout << "Failed to open file '" << outFilename << "' for writing!\n";
				return -1;
			}
			exportSymbolInfo(program->symbols, outFile);
			outFile.close();
		}

		if (args.hasOption("export-comments"))
		{
			auto outFilename = args.getOption("export-comments").front();
			std::ofstream outFile(outFilename);
			if (!outFile.is_open())
			{
				std::cout << "Failed to open file '" << outFilename << "' for writing!\n";
				return -1;
			}
			exportComments(comments, outFile);
			outFile.close();
		}

		if (verbose)
		{
			for (auto sym : *program->symbols)
			{
				if (isFuncSpec(sym))
				{
					if (!isDefined(sym))
					{
						PRINT_WARNING(MAKE_QINP_ERROR(getPosStr(getBestPos(sym)) + ": Function '" + getReadableName(sym) + "' declared but never defined!"));
					}
					if (!isReachable(sym))
					{
						PRINT_WARNING(MAKE_QINP_ERROR(getPosStr(getBestPos(sym)) + ": Stripping unused function '" + getReadableName(sym) + "' from binary!"));
					}
				}
			}
		}

		std::string output = genAsm(program, args.hasOption("verbose") && args.hasOption("keep"));
	
		std::string asmFilename = std::filesystem::path(inFilename).replace_extension(".asm").string();
		writeTextFileOverwrite(asmFilename, output);

		auto objFilename = std::filesystem::path(inFilename).replace_extension(".o").string();

		std::string outExt;
		if (platform == "linux")
			outExt = ".out";
		else if (platform == "windows")
			outExt = ".exe";

		auto outFilename = args.hasOption("output") ? args.getOption("output").front() : std::filesystem::path(inFilename).replace_extension(outExt).string();

		std::string nasmCmd;
		std::string linkCmd;

		if (platform == "linux")
		{
			nasmCmd = "nasm -f elf64 -o '" + objFilename + "' '" + asmFilename + "'";
			linkCmd = "ld -m elf_x86_64 -o '" + outFilename + "' '" + objFilename + "'";
		}
		else if (platform == "windows")
		{
			nasmCmd = "nasm -f win64 -o \"" + objFilename + "\" \"" + asmFilename + "\"";
			// TODO: Link without LARGEADDRESSAWARE:NO
			linkCmd = "vcvars64.bat >nul 2>nul && link /LARGEADDRESSAWARE:NO /MACHINE:X64 /SUBSYSTEM:CONSOLE /NODEFAULTLIB /ENTRY:_start /OUT:\"" + outFilename + "\" \"" + objFilename + "\" kernel32.lib";
		}

		if (args.hasOption("extern"))
		{
			auto& libs = args.getOption("extern");
			for (auto& lib : libs)
				linkCmd += " \"" + lib + "\"";
		}

		{
			Timer timer("Assembling", verbose);
			ExecCmdResult r;
			if ((r = execCmd(nasmCmd)).first)
				THROW_QINP_ERROR("Assembler Error:\n" + r.second);
		}
		{
			Timer timer("Linking", verbose);
			ExecCmdResult r;
			if ((r = execCmd(linkCmd)).first)
				THROW_QINP_ERROR("Linker Error:\n" + r.second);
		}

		std::filesystem::remove(objFilename);
		if (!args.hasOption("keep"))
			std::filesystem::remove(asmFilename);

		if (args.hasOption("run"))
		{
			auto runCmd = outFilename;
			if (args.hasOption("runarg"))
				for (auto& arg : args.getOption("runarg"))
					runCmd += " \"" + arg + "\""; // TODO: Proper quoting
			if (verbose) std::cout << "Running generated program..." << std::endl;
			runRet = execCmd(runCmd, false).first;
			if (verbose) std::cout << std::endl << "Exit code: " << runRet << std::endl;
		}
	}
	catch (const QinpError& e)
	{
		std::cout << "[ ERR ]: QNP: " << e.what() << std::endl;
		if (verbose)
			std::cout << "WHERE: " << e.where() << std::endl;
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cout << "[ ERR ]: STD: " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cout << "[ ERR ]: Unknown error!" << std::endl;
		return -1;
	}

    return runRet;
}
