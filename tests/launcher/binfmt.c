// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of how the launcher reads an invocation by binfmt_misc (src/startup/launcher-binfmt.c):
// the arguments the kernel builds for an entry with the O flag, and the path behind the descriptor
// it passes. The kernel's side of the contract is checked by tests/launcher/binfmt-kernel.sh.
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "launcher-binfmt.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static bool argvIs(char** got, const char* const* want)
{
	size_t i;

	for (i = 0; want[i] != NULL; i++)
		if (got[i] == NULL || strcmp(got[i], want[i]) != 0)
			return false;
	return got[i] == NULL;
}

static void testArgv(void)
{
	// "./prog a b" as the kernel passes it for an entry with the O flag and no P flag.
	char* kernel[] = { "/usr/bin/darling", "./prog", "a", "b c", NULL };
	const char* want[] = { "/usr/bin/darling", "exec", "/abs/prog", "a", "b c", NULL };
	char** got = launcherBinfmtExecArgv(4, kernel, "/abs/prog");

	CHECK(got != NULL && argvIs(got, want));
	free(got);

	// no arguments
	char* bare[] = { "/usr/bin/darling", "./prog", NULL };
	const char* wantBare[] = { "/usr/bin/darling", "exec", "/abs/prog", NULL };
	got = launcherBinfmtExecArgv(2, bare, "/abs/prog");
	CHECK(got != NULL && argvIs(got, wantBare));
	free(got);

	// A file named like a subcommand or an option stays the program, and so do arguments that look
	// like them: this is why the descriptor, not the argument, decides the form of the command.
	char* tricky[] = { "/usr/bin/darling", "shutdown", "shell", "-h", "exec", NULL };
	const char* wantTricky[] = { "/usr/bin/darling", "exec", "/abs/shutdown", "shell", "-h", "exec", NULL };
	got = launcherBinfmtExecArgv(5, tricky, "/abs/shutdown");
	CHECK(got != NULL && argvIs(got, wantTricky));
	free(got);

	// the program of the kernel's argv[1] is replaced, not kept
	char* typed[] = { "/usr/bin/darling", "/dev/fd/9", NULL };
	got = launcherBinfmtExecArgv(2, typed, "/abs/prog");
	CHECK(got != NULL && strcmp(got[2], "/abs/prog") == 0 && got[3] == NULL);
	free(got);

	// The program is copied: the launcher keeps the result after the buffer it read the path into
	// is out of scope, and an optimizing build reuses that stack space.
	char scratch[16] = "/abs/prog";
	got = launcherBinfmtExecArgv(2, typed, scratch);
	memset(scratch, 'X', sizeof(scratch));
	CHECK(got != NULL && strcmp(got[2], "/abs/prog") == 0);
	free(got);

	// there is always a path after the interpreter
	char* none[] = { "/usr/bin/darling", NULL };
	CHECK(launcherBinfmtExecArgv(1, none, "/abs/prog") == NULL);
	CHECK(launcherBinfmtExecArgv(0, none, "/abs/prog") == NULL);
}

static void testExecFd(void)
{
	// this process was not started by an entry with the O flag
	CHECK(launcherBinfmtExecFd() == -1);
}

static void testProgramPath(void)
{
	char dir[] = "/tmp/launcher-binfmt-XXXXXX";
	char file[256], link[256], linked[256], path[4096], resolved[4096], small[8];
	int fd, memfd;

	if (mkdtemp(dir) == NULL) { perror("mkdtemp"); exit(2); }
	snprintf(file, sizeof(file), "%s/prog", dir);
	snprintf(link, sizeof(link), "%s/alias", dir);

	fd = open(file, O_CREAT | O_RDWR, 0755);
	if (fd < 0) { perror(file); exit(2); }
	if (symlink("prog", link) != 0) { perror(link); exit(2); }
	CHECK(realpath(file, resolved) != NULL);

	// the path of the file, whatever it was opened as
	CHECK(launcherBinfmtProgramPath(fd, path, sizeof(path)) && strcmp(path, resolved) == 0);
	close(fd);
	fd = open(link, O_RDONLY);
	snprintf(linked, sizeof(linked), "%s", resolved);
	CHECK(fd >= 0 && launcherBinfmtProgramPath(fd, path, sizeof(path)) && strcmp(path, linked) == 0);

	// does not fit: a path of n bytes needs a buffer of n + 2, as one of n + 1 could be a cut short path
	size_t length = strlen(resolved);
	CHECK(!launcherBinfmtProgramPath(fd, path, length + 1));
	CHECK(launcherBinfmtProgramPath(fd, path, length + 2) && strcmp(path, resolved) == 0);
	CHECK(!launcherBinfmtProgramPath(fd, small, sizeof(small)));
	CHECK(!launcherBinfmtProgramPath(fd, path, 0));
	CHECK(!launcherBinfmtProgramPath(-1, path, sizeof(path)));

	// removed since the kernel opened it
	unlink(file);
	CHECK(!launcherBinfmtProgramPath(fd, path, sizeof(path)));
	close(fd);

	// not a file on a file system
	memfd = memfd_create("prog", 0);
	CHECK(memfd >= 0 && !launcherBinfmtProgramPath(memfd, path, sizeof(path)));
	close(memfd);

	// a descriptor with no path on a file system: a pipe reads "pipe:[..]"
	int pipefds[2];
	CHECK(pipe(pipefds) == 0 && !launcherBinfmtProgramPath(pipefds[0], path, sizeof(path)));
	close(pipefds[0]);
	close(pipefds[1]);

	unlink(link);
	rmdir(dir);
}

int main(void)
{
	testArgv();
	testExecFd();
	testProgramPath();

	if (g_failures == 0)
		printf("launcher binfmt: ok\n");
	return g_failures == 0 ? 0 : 1;
}
