#include <iostream>

#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sys/wait.h>

#include "Errors/QinpError.h"
#include "Tokenizer.h"
#include "ArgsParser.h"
#include "ProgramGenerator.h"

#include "NasmGenerators/NasmGen_Linux_x86_64.h"

std::string readTextFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw QinpError("Unable to open file!");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int execCmd(const std::string& command)
{
	pid_t p = fork();

	if (p == -1)
		throw QinpError("Fork failed!");
	
	if (p == 0)
	{
		execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
		throw QinpError("Exec failed!");
	}

	int status;
	if (waitpid(p, &status, 0) == -1)
		throw QinpError("Waitpid failed!");

	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;

	//auto pipe = popen(command.c_str(), "r");
	//if (!pipe) throw QinpError("popen() failed!");
	//return pclose(pipe);
}

void writeTextFileOverwrite(const std::string& filename, const std::string& text)
{
	std::ofstream file(filename, std::ios::trunc);
	if (!file.is_open())
		throw QinpError("Unable to open file!");
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

std::map<std::string, std::string> argNames = 
{
	{ "h", "help" },
	{ "v", "verbose" },
};

int main(int argc, char** argv, char** environ)
{
	try
	{
		auto args = parseArgs(getArgs(argc, argv), argNames);
		auto env = getEnv(environ);

		auto inFilename = args.values[0];
	
		auto code = readTextFile(inFilename);

		std::cout << "------ CODE ------\n" << code << "\n------------------\n" << std::endl;

		auto tokens = tokenize(code, inFilename);

		std::cout << "------ TOKENS ------\n";
		for (auto& token : tokens)
			std::cout << token << std::endl;
		std::cout << "------------------\n" << std::endl;

		auto program = generateProgram(tokens);

		std::string output = generateNasm_Linux_x86_64(program);

		std::cout << "------ OUTPUT ------\n" << output << "------------------\n" << std::endl;
	
		std::string asmFilename = std::filesystem::path(inFilename).replace_extension(".asm").string();
		writeTextFileOverwrite(asmFilename, output);

		auto objFilename = std::filesystem::path(inFilename).replace_extension(".o").string();
		auto nasmCmd = "nasm -f elf64 -o '" + objFilename + "' '" + asmFilename + "'";
		std::cout << "Executing: '" << nasmCmd << "'..." << std::endl;
		if (execCmd(nasmCmd))
			throw QinpError("Assembler Error!");

		auto outFilename = std::filesystem::path(inFilename).replace_extension(".out").string();
		auto ldCmd = "ld -m elf_x86_64 -o '" + outFilename + "' '" + objFilename + "'";
		std::cout << "Executing: '" << ldCmd << "'..." << std::endl;
		if (execCmd(ldCmd))
			throw QinpError("Linker Error!");

		auto runCmd = "./" + outFilename;
		std::cout << "Executing: '" << runCmd << "'..." << std::endl;
		int runRet = execCmd(runCmd);
		std::cout << "Exit code: " << runRet << std::endl;
	}
	catch (const QinpError& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
		return 1;
	}

    return 0;
}
