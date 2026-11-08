// Copyright (C) 2026 Lubos Dolezel
//
// A child made by fork(), vfork() or posix_spawn() is its own process:
// getpid() in it returns the pid its parent was given, not the parent's
// pid, and getppid() returns the parent's pid (fork(2), vfork(2),
// posix_spawn(2), getpid(2)). The parent calls getpid() first, so a pid
// the C library caches must be replaced in the child.
//
// A failing fork() or vfork() returns -1 and sets errno. The failure is
// provoked with RLIMIT_NPROC = 1 (EAGAIN); if the limit is not enforced,
// that check is skipped.
//
// Build for x86_64 or i386 and run without arguments. Prints PASS and
// exits 0 on success.
#include <errno.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

struct report {
	pid_t pid;
	pid_t ppid;
};

static void timeout(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static void writeReport(int fd)
{
	struct report r = { getpid(), getppid() };
	if (write(fd, &r, sizeof(r)) != sizeof(r))
		_exit(3);
}

static int readReport(int fd, struct report *r)
{
	ssize_t n = read(fd, r, sizeof(*r));
	return n == (ssize_t) sizeof(*r);
}

static void checkChild(const char *what, pid_t child, int status, const struct report *r, pid_t parent)
{
	if (child <= 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		printf("FAIL: %s child did not exit cleanly\n", what);
		g_failures++;
		return;
	}
	if (r->pid != child || r->pid == parent || r->ppid != parent) {
		printf("FAIL: %s: child getpid()=%d getppid()=%d, parent %d, fork returned %d\n",
			what, (int) r->pid, (int) r->ppid, (int) parent, (int) child);
		g_failures++;
	}
}

static void checkFork(pid_t parent)
{
	int fds[2];
	CHECK(pipe(fds) == 0);

	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		close(fds[0]);
		writeReport(fds[1]);

		// and in a grandchild
		pid_t self = getpid();
		pid_t grandchild = fork();
		if (grandchild == 0) {
			struct report r = { getpid(), getppid() };
			_exit(r.pid == self || r.ppid != self);
		}
		int status = 0;
		if (grandchild < 0 || waitpid(grandchild, &status, 0) != grandchild || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
			_exit(4);
		_exit(0);
	}

	close(fds[1]);
	struct report r = { 0, 0 };
	CHECK(readReport(fds[0], &r));
	close(fds[0]);

	int status = -1;
	CHECK(child > 0 && waitpid(child, &status, 0) == child);
	checkChild("fork", child, status, &r, parent);
	CHECK(getpid() == parent);
}

static void checkVfork(pid_t parent)
{
	int fds[2];
	CHECK(pipe(fds) == 0);

	fflush(stdout);
	pid_t child = vfork();
	if (child == 0) {
		// only async-signal-safe calls before _exit
		struct report r = { getpid(), getppid() };
		_exit(write(fds[1], &r, sizeof(r)) == sizeof(r) ? 0 : 3);
	}

	close(fds[1]);
	struct report r = { 0, 0 };
	CHECK(readReport(fds[0], &r));
	close(fds[0]);

	int status = -1;
	CHECK(child > 0 && waitpid(child, &status, 0) == child);
	checkChild("vfork", child, status, &r, parent);
	CHECK(getpid() == parent);
}

static void checkSpawn(const char *self, pid_t parent)
{
	int fds[2];
	CHECK(pipe(fds) == 0);

	char fdArg[16];
	snprintf(fdArg, sizeof(fdArg), "%d", fds[1]);
	char *argv[] = { (char *) self, "--report", fdArg, NULL };

	pid_t child = -1;
	CHECK(posix_spawn(&child, self, NULL, NULL, argv, environ) == 0);
	close(fds[1]);

	struct report r = { 0, 0 };
	CHECK(readReport(fds[0], &r));
	close(fds[0]);

	int status = -1;
	CHECK(child > 0 && waitpid(child, &status, 0) == child);
	checkChild("posix_spawn", child, status, &r, parent);
}

// Runs in a child of its own, as it lowers the process limit
static int failingForkChild(int useVfork)
{
	struct rlimit limit;
	if (getrlimit(RLIMIT_NPROC, &limit) != 0)
		return 10;
	limit.rlim_cur = 1;
	if (setrlimit(RLIMIT_NPROC, &limit) != 0)
		return 11;

	errno = 0;
	pid_t pid = useVfork ? vfork() : fork();
	if (pid == 0)
		_exit(0);
	if (pid > 0) {
		waitpid(pid, NULL, 0);
		return 12; // not enforced
	}
	if (pid != -1)
		return 13;
	return errno == EAGAIN ? 0 : 14;
}

static void checkFailure(int useVfork)
{
	const char *what = useVfork ? "vfork" : "fork";

	fflush(stdout);
	pid_t child = fork();
	if (child == 0)
		_exit(failingForkChild(useVfork));

	int status = -1;
	CHECK(child > 0 && waitpid(child, &status, 0) == child);
	if (!WIFEXITED(status)) {
		printf("FAIL: failing %s: helper died\n", what);
		g_failures++;
		return;
	}

	switch (WEXITSTATUS(status)) {
		case 0:
			break;
		case 10: case 11: case 12:
			printf("SKIP: failing %s: RLIMIT_NPROC cannot be used to make it fail (%d)\n", what, WEXITSTATUS(status));
			break;
		case 13:
			printf("FAIL: failing %s did not return -1\n", what);
			g_failures++;
			break;
		default:
			printf("FAIL: failing %s: errno is not EAGAIN\n", what);
			g_failures++;
			break;
	}
}

int main(int argc, char **argv)
{
	signal(SIGALRM, timeout);
	alarm(30);

	if (argc == 3 && strcmp(argv[1], "--report") == 0) {
		writeReport(atoi(argv[2]));
		return 0;
	}

	pid_t parent = getpid();
	CHECK(parent > 0 && getpid() == parent);

	checkFork(parent);
	checkVfork(parent);
	checkSpawn(argv[0], parent);
	checkFailure(0);
	checkFailure(1);

	if (g_failures == 0)
		puts("PASS");
	fflush(stdout);
	_exit(g_failures != 0);
}
