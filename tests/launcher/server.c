// Host-side test of the checks the setuid launcher makes before it joins the
// mount namespace of the process named in the prefix's pid file
// (src/startup/launcher-server.c). The "server" is a child of this test.
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "launcher-server.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static struct stat g_exe, g_userNs, g_mntNs;

// A child that optionally creates its own user and mount namespaces, reports
// whether that worked, and waits to be killed.
static pid_t spawnChild(bool ownNamespaces, bool* namespacesCreated)
{
	int p[2];
	char ok = 0;
	pid_t pid;

	if (pipe(p) != 0)
		return -1;

	pid = fork();
	if (pid == 0)
	{
		close(p[0]);
		ok = 1;
		if (ownNamespaces && unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
			ok = 0;
		if (write(p[1], &ok, 1) != 1)
			_exit(1);
		for (;;)
			pause();
	}

	close(p[1]);
	if (pid < 0 || read(p[0], &ok, 1) != 1)
		ok = 0;
	close(p[0]);
	if (namespacesCreated)
		*namespacesCreated = ok;
	return pid;
}

static void reap(pid_t pid)
{
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
}

static int openProc(pid_t pid)
{
	char path[64];

	snprintf(path, sizeof(path), "/proc/%d", (int) pid);
	return open(path, O_PATH | O_DIRECTORY | O_CLOEXEC);
}

// The child shares our mount namespace.
static void test_same_namespace(void)
{
	pid_t pid = spawnChild(false, NULL);
	int procfd = openProc(pid), nsfd = 123;

	CHECK(procfd >= 0);
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &g_userNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_SAME_MNTNS);
	CHECK(nsfd == -1);

	// Not the expected executable.
	struct stat other;
	CHECK(stat("/bin/sh", &other) == 0);
	CHECK(launcherOpenServerMountNs(procfd, &other, &g_userNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_WRONG_EXE);
	CHECK(nsfd == -1);

	// The pinned process exits; a reused pid would not be looked at.
	reap(pid);
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &g_userNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_GONE);
	CHECK(nsfd == -1);
	close(procfd);
}

// The child has a mount namespace of its own, owned by a user namespace it
// created, which is what an unprivileged user can set up.
static void test_foreign_namespace(void)
{
	bool created;
	pid_t pid = spawnChild(true, &created);
	int procfd, nsfd = 123;
	char path[64];
	struct stat childUserNs, childMntNs, st;

	if (!created)
	{
		printf("SKIP: unprivileged user namespaces are not available\n");
		reap(pid);
		return;
	}

	procfd = openProc(pid);
	CHECK(procfd >= 0);
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &g_userNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_FOREIGN_USERNS);
	CHECK(nsfd == -1);

	// Taking the child's user namespace as "ours" stands in for a namespace
	// that root created: all checks pass and the namespace is handed out.
	snprintf(path, sizeof(path), "/proc/%d/ns/user", (int) pid);
	CHECK(stat(path, &childUserNs) == 0);
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", (int) pid);
	CHECK(stat(path, &childMntNs) == 0);
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &childUserNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_OK);
	CHECK(nsfd >= 0);
	if (nsfd >= 0)
	{
		CHECK(fstat(nsfd, &st) == 0 && st.st_dev == childMntNs.st_dev && st.st_ino == childMntNs.st_ino);
		close(nsfd);
	}

	// The same process as our own namespace (as if it were the launcher's).
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &childUserNs, &childMntNs, &nsfd) == LAUNCHER_SERVER_SAME_MNTNS);

	reap(pid);
	CHECK(launcherOpenServerMountNs(procfd, &g_exe, &childUserNs, &g_mntNs, &nsfd) == LAUNCHER_SERVER_GONE);
	close(procfd);
}

int main(void)
{
	if (stat("/proc/self/exe", &g_exe) != 0 || stat("/proc/self/ns/user", &g_userNs) != 0
		|| stat("/proc/self/ns/mnt", &g_mntNs) != 0)
	{
		printf("FAIL: cannot examine this process\n");
		return 1;
	}

	test_same_namespace();
	test_foreign_namespace();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher server: all checks passed\n");
	return 0;
}
