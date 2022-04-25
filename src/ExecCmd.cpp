#include "ExecCmd.h"

#include "Errors/QinpError.h"

#if defined QINP_PLATFORM_UNIX

#include <sys/wait.h>
#include <unistd.h>

int execCmd(const std::string& command)
{
	pid_t p = fork();

	if (p == -1)
		THROW_QINP_ERROR("Fork failed! ( CMD: '" + command + "' )");
	
	if (p == 0)
	{
		execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
		THROW_QINP_ERROR("Exec failed! ( CMD: '" + command + "' )");
	}

	int status;
	if (waitpid(p, &status, 0) == -1)
		THROW_QINP_ERROR("Waitpid failed! ( CMD: '" + command + "' )");

	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;
}

#elif defined QINP_PLATFORM_WINDOWS

#include <windows.h>

int execCmd(const std::string& command)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD dwExitCode = 0;
		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		return dwExitCode;
	}
	THROW_QINP_ERROR("CreateProcess failed! ( CMD: '" + command + "' )");
}

#endif
