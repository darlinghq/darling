// Copyright (C) 2026 Lubos Dolezel
//
// pbcopy and pbpaste work without a display: text written by pbcopy is returned by pbpaste, with no DISPLAY or
// WAYLAND_DISPLAY in the environment. Needs NSPasteboard over the CoreFoundation pasteboard (the pbcopy and pbpaste
// programs use NSPasteboard) and the pboard service; it does not pass before both exist.
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;
static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static int run(const char *program, int in, int out)
{
	posix_spawn_file_actions_t fa;
	posix_spawn_file_actions_init(&fa);
	if (in >= 0) posix_spawn_file_actions_adddup2(&fa, in, 0);
	if (out >= 0) posix_spawn_file_actions_adddup2(&fa, out, 1);
	char *argv[] = { (char *)program, NULL };
	char *envp[] = { "HOME=/tmp", "PATH=/usr/bin:/bin", NULL };   // no display variables
	pid_t pid;
	if (posix_spawn(&pid, program, &fa, NULL, argv, envp) != 0) return -1;
	int status;
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int main(void)
{
	char in[] = "/tmp/pboard-nodisplay-in.XXXXXX", out[] = "/tmp/pboard-nodisplay-out.XXXXXX";
	char text[64];
	snprintf(text, sizeof(text), "hello-%d", (int)getpid());
	int fin = mkstemp(in), fout = mkstemp(out);
	write(fin, text, strlen(text));
	lseek(fin, 0, SEEK_SET);
	check("pbcopy succeeds", run("/usr/bin/pbcopy", fin, -1) == 0);
	check("pbpaste succeeds", run("/usr/bin/pbpaste", -1, fout) == 0);
	char got[64] = {0};
	lseek(fout, 0, SEEK_SET);
	ssize_t n = read(fout, got, sizeof(got) - 1);
	check("pbpaste prints what pbcopy stored", n == (ssize_t)strlen(text) && strcmp(got, text) == 0);
	unlink(in);
	unlink(out);
	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures ? 1 : 0;
}
