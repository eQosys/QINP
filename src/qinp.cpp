#include <iostream>

#include <map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sys/wait.h>

#include "Errors/QinpError.h"
#include "Tokenizer.h"
#include "ArgsParser.h"
#include "ProgramGenerator.h"
#include "PlatformName.h"

#include "NasmGenerators/NasmGen_Linux_x86_64.h"

int execCmd(const std::string& command)
{
	pid_t p = fork();

	if (p == -1)
		THROW_QINP_ERROR("Fork failed!");
	
	if (p == 0)
	{
		execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
		THROW_QINP_ERROR("Exec failed!");
	}

	int status;
	if (waitpid(p, &status, 0) == -1)
		THROW_QINP_ERROR("Waitpid failed!");

	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;
}

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
};

#define HELP_TEXT \
	"Usage: qinp [options] [input file]\n" \
	"Options:\n" \
	"  -h, --help\n" \
	"    Print this help message.\n" \
	"  -v, --verbose\n" \
	"    Print verbose output.\n" \
	"  -i, --import=[path]\n" \
	"    Specify an import directory.\n" \
	"  -o, --output=[path]\n" \
	"    Specify the output path of the generated executable.\n" \
	"  -k, --keep\n" \
	"    Keep the generated assembly file.\n" \
	"  -r, --run\n" \
	"    Run the generated program.\n" \
	"  -p, --platform=[platform]\n" \
	"    Specify the target platform. (linux, windows, macos)\n" \
	"    Only linux is supported for now.\n" \
	"  -a, --runarg=[arg]\n" \
	"    Specify a single argument to pass to the generated program.\n" \
	"	  Only used when --run is specified.\n"

class Timer
{
public:
	Timer(bool printOnDestruction)
		: m_printOnDestruction(printOnDestruction)
	{
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		if (!m_printOnDestruction)
			return;

		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end - start;
		std::cout << diff.count() << "s" << std::endl;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> end;
	bool m_printOnDestruction;
};

int main(int argc, char** argv, char** environ)
{
	bool verbose = true;
	int runRet = 0;
	try
	{
		auto args = parseArgs(getArgs(argc, argv), argNames);
		auto env = getEnv(environ);

		if (args.hasOption("help"))
		{
			std::cout << HELP_TEXT;
			return 0;
		}

		std::string platform = QINP_PLATFORM;
		if (args.hasOption("platform"))
			platform = args.getOption("platform").front();

		if (platform != "linux")
		{
			std::cout << "Platform not supported!\n";
			return -1;
		}

		verbose = args.hasOption("verbose");

		auto inFilename = args.values[0];

		std::set<std::string> importDirs;
		if (args.hasOption("import"))
			for (auto& dir : args.getOption("import"))
				importDirs.insert(dir);

		ProgramRef program;
		{
			if (verbose) std::cout << "Code gen: ";
			Timer timer(verbose);
			auto code = readTextFile(inFilename);
			auto tokens = tokenize(code, inFilename);
			program = generateProgram(tokens, importDirs, platform, inFilename);
		}

		if (verbose)
			for (auto sym : *program->symbols)
				if (isFuncSpec(sym) && !isDefined(sym))
					std::cout << "WARN: Missing definition for function '" << getMangledName(sym) << "' declared at " << getPosStr(sym->pos) << "!" << std::endl;

		std::string output = generateNasm_Linux_x86_64(program);
	
		std::string asmFilename = std::filesystem::path(inFilename).replace_extension(".asm").string();
		writeTextFileOverwrite(asmFilename, output);

		auto objFilename = std::filesystem::path(inFilename).replace_extension(".o").string();
		auto outFilename = args.hasOption("output") ? args.getOption("output").front() : std::filesystem::path(inFilename).replace_extension(".out").string();
		auto nasmCmd = "nasm -f elf64 -o '" + objFilename + "' '" + asmFilename + "'";
		auto ldCmd = "ld -m elf_x86_64 -o '" + outFilename + "' '" + objFilename + "'";

		{
			if (verbose) std::cout << "Nasm: ";
			Timer timer(verbose);
			if (execCmd(nasmCmd))
				THROW_QINP_ERROR("Assembler Error!");
		}
		{
			if (verbose) std::cout << "Linker: ";
			Timer timer(verbose);
			if (execCmd(ldCmd))
				THROW_QINP_ERROR("Linker Error!");
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
			runRet = execCmd(runCmd);
			if (verbose) std::cout << std::endl << "Exit code: " << runRet << std::endl;
		}
	}
	catch (const QinpError& e)
	{
		std::cout << "QNP ERROR: " << e.what() << std::endl;
		if (verbose)
			std::cout << "WHERE: " << e.where() << std::endl;
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cout << "STD ERROR: " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cout << "ERROR: Unknown error!" << std::endl;
		return -1;
	}

    return runRet;
}
