// Copyright (C) 2026 Lubos Dolezel
//
// As root, setgid(-2) and setuid(-2) switch the process to nobody (uid and gid
// 4294967294 on macOS): getuid(), geteuid(), getgid() and getegid() report -2,
// and so does a program executed afterwards (this program again, and
// /usr/bin/id). setuid(-1) and setgid(-1) as root leave the ids unchanged.
// Run as root. Build for x86_64 and i386; prints PASS or FAIL.
#include <errno.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// nobody on macOS: `id -u nobody` and `id -g nobody` print 4294967294
static const uid_t nobody_uid = (uid_t) -2;
static const gid_t nobody_gid = (gid_t) -2;

static int failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "ok" : "FAIL", what);
	if (!ok)
		failures++;
}

static void finish(int code)
{
	fflush(stdout);
	_exit(code);
}

// Runs argv with stdout captured into buf. Returns the exit status, or -1.
static int capture(char* const argv[], char* buf, size_t size)
{
	int fds[2];
	if (pipe(fds) != 0)
		return -1;

	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0)
		return -1;
	if (pid == 0)
	{
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
		execv(argv[0], argv);
		_exit(127);
	}

	close(fds[1]);
	size_t len = 0;
	ssize_t n;
	while (len < size - 1 && (n = read(fds[0], buf + len, size - 1 - len)) > 0)
		len += n;
	buf[len] = '\0';
	close(fds[0]);

	int status;
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

static void check_id_tool(const char* flag, unsigned int expected, const char* what)
{
	char* argv[] = { "/usr/bin/id", (char*) flag, NULL };
	char buf[64];
	char want[32];

	if (access(argv[0], X_OK) != 0)
	{
		printf("skip: %s (no /usr/bin/id)\n", what);
		return;
	}

	snprintf(want, sizeof(want), "%u\n", expected);
	int status = capture(argv, buf, sizeof(buf));
	printf("  id %s printed: %s", flag, buf[0] ? buf : "(nothing)\n");
	check(status == 0 && strcmp(buf, want) == 0, what);
}

static void as_nobody(void)
{
	uid_t uid = getuid();
	gid_t gid = getgid();

	// -1 is not an id but "no change" for the kernel, so as root these succeed without changing anything
	errno = 0;
	int ret = setgid((gid_t) -1);
	printf("  setgid(-1) = %d, errno %d\n", ret, ret ? errno : 0);
	check(getgid() == gid && getegid() == gid, "setgid(-1) leaves the gid unchanged");
	errno = 0;
	ret = setuid((uid_t) -1);
	printf("  setuid(-1) = %d, errno %d\n", ret, ret ? errno : 0);
	check(getuid() == uid && geteuid() == uid, "setuid(-1) leaves the uid unchanged");

	check(setgid(nobody_gid) == 0, "setgid(-2) succeeds");
	check(setuid(nobody_uid) == 0, "setuid(-2) succeeds");
	printf("  uid %u euid %u gid %u egid %u\n", getuid(), geteuid(), getgid(), getegid());
	check(getuid() == nobody_uid && geteuid() == nobody_uid, "getuid() and geteuid() return -2");
	check(getgid() == nobody_gid && getegid() == nobody_gid, "getgid() and getegid() return -2");

	char self[MAXPATHLEN];
	uint32_t self_size = sizeof(self);
	char want[128];
	char buf[128];

	if (_NSGetExecutablePath(self, &self_size) != 0)
	{
		check(0, "_NSGetExecutablePath");
		return;
	}
	char* argv[] = { self, "--report", NULL };
	snprintf(want, sizeof(want), "%u %u %u %u\n", nobody_uid, nobody_uid, nobody_gid, nobody_gid);
	int status = capture(argv, buf, sizeof(buf));
	printf("  executed copy reports: %s", buf[0] ? buf : "(nothing)\n");
	check(status == 0 && strcmp(buf, want) == 0, "a program executed afterwards sees uid and gid -2");

	check_id_tool("-u", nobody_uid, "/usr/bin/id -u prints 4294967294");
	check_id_tool("-g", nobody_gid, "/usr/bin/id -g prints 4294967294");
}

int main(int argc, char** argv)
{
	alarm(30);

	if (argc > 1 && strcmp(argv[1], "--report") == 0)
	{
		printf("%u %u %u %u\n", getuid(), geteuid(), getgid(), getegid());
		finish(0);
	}

	if (getuid() != 0)
	{
		printf("must run as root (uid %u)\nFAIL\n", getuid());
		finish(1);
	}

	// switch identity in a child, so the caller keeps running as root
	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		printf("FAIL\n");
		finish(1);
	}
	if (pid == 0)
	{
		as_nobody();
		finish(failures ? 1 : 0);
	}

	int status;
	int ok = waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0;
	check(getuid() == 0, "the parent is still root");
	ok = ok && failures == 0;
	printf("%s\n", ok ? "PASS" : "FAIL");
	finish(ok ? 0 : 1);
}
