#include "ExecCmd.h"

#include "Errors/QinpError.h"

#if defined QINP_PLATFORM_UNIX

#include <sys/wait.h>
#include <unistd.h>

// Returns the exit code and the output of the command
ExecCmdResult execCmd(const std::string& command, bool grabOutput)
{
	int link[2];

	if (grabOutput && pipe(link) == -1)
		THROW_QINP_ERROR("Pipe failed! ( CMD: '" + command + "' )");

	pid_t p = fork();

	if (p == -1)
		THROW_QINP_ERROR("Fork failed! ( CMD: '" + command + "' )");
	
	if (p == 0)
	{
		if (grabOutput)
		{
			dup2(link[1], STDOUT_FILENO);
			close(link[0]);
			close(link[1]);
		}
		execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
		THROW_QINP_ERROR("Exec failed! ( CMD: '" + command + "' )");
	}

	int status;
	if (waitpid(p, &status, 0) == -1)
		THROW_QINP_ERROR("Waitpid failed! ( CMD: '" + command + "' )");

	std::string output;

	if (grabOutput)
	{
		char buffer[16384];

		close(link[1]);
		int nBytes = read(link[0], buffer, sizeof(buffer));
		output = std::string(buffer, nBytes);
	}

	if (WIFEXITED(status))
		return { WEXITSTATUS(status), output };
	else
		return { -1, "" };
}

#elif defined QINP_PLATFORM_WINDOWS

#include <windows.h>

ExecCmdResult execCmd(const std::string& command, bool grabOutput)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessA(
		NULL, (LPSTR)command.c_str(),
		NULL, NULL,
		FALSE, NULL,
		NULL, NULL,
		&si, &pi)
	)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD dwExitCode = 0;
		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		std::string output;

		return { dwExitCode, output };
	}
	THROW_QINP_ERROR("CreateProcess failed! ( CMD: '" + command + "' )");
}

#endif
