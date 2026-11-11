// Copyright (C) 2026 Lubos Dolezel
//
// Plain arm64 runner for arm64e-dlsym-main. arm64e code needs a CPU with
// pointer authentication; without it the test is skipped (exit 77).

#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int cpu_has_pac(void)
{
	pid_t pid = fork();
	if (pid == 0) {
		// PACIA x0, x1: undefined (SIGILL) on a CPU without pointer authentication
		__asm__ volatile("mov x0, #0x1000\n\tmov x1, #0\n\t.inst 0xdac10020" ::: "x0", "x1");
		_exit(0);
	}
	int status = 0;
	if (pid < 0 || waitpid(pid, &status, 0) != pid)
		return 0;
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int main(int argc, char** argv)
{
	const char* prog = argc > 1 ? argv[1] : "./arm64e-dlsym-main";

	if (!cpu_has_pac()) {
		printf("SKIP: no pointer authentication on this CPU\n");
		return 77;
	}

	char* args[] = { (char*) prog, NULL };
	pid_t pid;
	if (posix_spawn(&pid, prog, NULL, NULL, args, environ) != 0) {
		printf("FAIL: cannot spawn %s\n", prog);
		return 1;
	}
	int status = 0;
	waitpid(pid, &status, 0);
	if (WIFSIGNALED(status)) {
		printf("FAIL: %s died with signal %d\n", prog, WTERMSIG(status));
		return 1;
	}
	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
