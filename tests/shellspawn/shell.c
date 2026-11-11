// Copyright (C) 2026 Lubos Dolezel
//
// Host test of how spawnShell() picks the shell for the user (shellspawn-shell.h):
// the account's shell when it is an absolute path to an executable regular
// file, /bin/zsh otherwise (no entry, empty or relative field, missing file,
// directory, file without the execute permission); and the login argv[0].
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shellspawn-shell.h"

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

int main(void)
{
	CHECK(strcmp(shellspawn_choose_shell("/bin/sh"), "/bin/sh") == 0);
	CHECK(strcmp(shellspawn_choose_shell(NULL), SHELLSPAWN_DEFAULT_SHELL) == 0);
	CHECK(strcmp(shellspawn_choose_shell(""), SHELLSPAWN_DEFAULT_SHELL) == 0);
	CHECK(strcmp(shellspawn_choose_shell("sh"), SHELLSPAWN_DEFAULT_SHELL) == 0);
	CHECK(strcmp(shellspawn_choose_shell("/nonexistent/shell"), SHELLSPAWN_DEFAULT_SHELL) == 0);
	CHECK(strcmp(shellspawn_choose_shell("/tmp"), SHELLSPAWN_DEFAULT_SHELL) == 0);

	char file[] = "/tmp/shellspawn-shell-test.XXXXXX";
	int fd = mkstemp(file);
	CHECK(fd != -1);
	if (fd != -1)
	{
		close(fd);
		chmod(file, 0644);
		CHECK(strcmp(shellspawn_choose_shell(file), SHELLSPAWN_DEFAULT_SHELL) == 0 || geteuid() == 0);
		chmod(file, 0755);
		CHECK(strcmp(shellspawn_choose_shell(file), file) == 0);
		unlink(file);
	}

	char argv0[16];
	CHECK(shellspawn_login_argv0("/bin/bash", argv0, sizeof(argv0)) == 0 && strcmp(argv0, "-bash") == 0);
	CHECK(shellspawn_login_argv0("zsh", argv0, sizeof(argv0)) == 0 && strcmp(argv0, "-zsh") == 0);
	CHECK(shellspawn_login_argv0("/usr/local/bin/a-very-long-shell-name", argv0, sizeof(argv0)) == -1);
	CHECK(shellspawn_login_argv0("/bin/12345678901234", argv0, sizeof(argv0)) == 0);
	CHECK(shellspawn_login_argv0("/bin/123456789012345", argv0, sizeof(argv0)) == -1);

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
