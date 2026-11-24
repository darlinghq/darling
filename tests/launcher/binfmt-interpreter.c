// Copyright (C) 2026 Lubos Dolezel
//
// A stand-in for the launcher as a binfmt_misc interpreter, for tests/launcher/binfmt-kernel.sh: it
// reads the invocation with the launcher's own code and prints the arguments the launcher would go on
// with, one per line, preceded by "execfd <n>".
#include <stdio.h>
#include <unistd.h>
#include "launcher-binfmt.h"

int main(int argc, char** argv)
{
	char program[4096];
	char** execArgv;
	int execFd = launcherBinfmtExecFd();

	printf("execfd %s\n", execFd >= 0 ? "present" : "absent");
	if (execFd < 0)
	{
		for (int i = 0; i < argc; i++)
			printf("arg <%s>\n", argv[i]);
		return 0;
	}

	if (!launcherBinfmtProgramPath(execFd, program, sizeof(program)))
	{
		printf("no path\n");
		return 1;
	}
	execArgv = launcherBinfmtExecArgv(argc, argv, program);
	if (execArgv == NULL)
		return 1;
	for (int i = 0; execArgv[i] != NULL; i++)
		printf("arg <%s>\n", execArgv[i]);
	return 0;
}
