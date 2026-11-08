// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of mldr's vchroot_unexpand_interpreter: an interpreter path
// under the prefix root loses the root, any other path gets SYSTEM_ROOT in
// front, and a result that does not fit the 4095-character buffer ends the
// process with exit status 1 instead of overflowing it.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <darling-config.h>
#include "interpreter_path.h"

#define ROOT "/home/user/.darling"
#define LIMIT 4095

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// Runs vchroot_unexpand_interpreter on `path` in a child, so that an exit can
// be observed, and passes the resulting argv[0] back through a pipe. Returns
// the child's exit status, or -1 if it did not exit normally.
static int unexpand(const char* root, const char* path, char* out, size_t out_size)
{
	int fds[2];
	if (pipe(fds) != 0)
		return -1;

	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		if (!freopen("/dev/null", "w", stderr))
			_exit(2);

		char* argv[] = { strdup(path), NULL };
		struct load_results lr = {
			.root_path = (char*) root,
			.root_path_length = root ? strlen(root) : 0,
			.argv = argv,
		};

		vchroot_unexpand_interpreter(&lr);

		size_t len = strlen(lr.argv[0]) + 1;
		if (write(fds[1], lr.argv[0], len) != (ssize_t) len)
			_exit(2);
		_exit(0);
	}
	close(fds[1]);

	size_t got = 0;
	ssize_t n;
	while (got < out_size - 1 && (n = read(fds[0], out + got, out_size - 1 - got)) > 0)
		got += n;
	out[got] = '\0';
	close(fds[0]);

	int status;
	if (pid == -1 || waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

// ROOT followed by `count` characters 'a'
static char* under_root(size_t count)
{
	size_t root_len = strlen(ROOT);
	char* path = malloc(root_len + count + 1);
	memcpy(path, ROOT, root_len);
	memset(path + root_len, 'a', count);
	path[root_len + count] = '\0';
	return path;
}

int main(void)
{
	static char out[1 << 18];
	char* path;

	check(unexpand(ROOT, ROOT "/usr/bin/perl", out, sizeof(out)) == 0
		&& strcmp(out, "/usr/bin/perl") == 0,
		"a path under the root loses the root");

	check(unexpand(ROOT, "/usr/bin/python3", out, sizeof(out)) == 0
		&& strcmp(out, SYSTEM_ROOT "/usr/bin/python3") == 0,
		"a host path gets SYSTEM_ROOT in front");

	check(unexpand(NULL, "/usr/bin/env", out, sizeof(out)) == 0
		&& strcmp(out, "/usr/bin/env") == 0,
		"without a root the path is left alone");

	path = under_root(LIMIT);
	check(unexpand(ROOT, path, out, sizeof(out)) == 0 && strlen(out) == LIMIT,
		"4095 characters after the root fit");
	free(path);

	path = under_root(LIMIT + 1);
	check(unexpand(ROOT, path, out, sizeof(out)) == 1,
		"4096 characters after the root are rejected");
	free(path);

	path = under_root(128 * 1024);
	check(unexpand(ROOT, path, out, sizeof(out)) == 1,
		"128 KiB after the root are rejected");
	free(path);

	path = malloc(LIMIT + 2);
	path[0] = '/';
	memset(path + 1, 'b', LIMIT);
	path[LIMIT + 1] = '\0';
	check(unexpand(ROOT, path, out, sizeof(out)) == 1,
		"a host path too long for SYSTEM_ROOT in front is rejected");
	free(path);

	if (failures)
	{
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
