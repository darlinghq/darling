// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of the launcher's mode selection, install prefix and user name rules
// (src/startup/launcher-mode.c) and of the process tree walk (src/startup/launcher-server.c).
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "launcher-mode.h"
#include "launcher-server.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static void testSelect(void)
{
	// a privileged launcher is in the root mode whatever the environment says
	CHECK(launcherSelectMode(true, NULL, NULL, false) == LAUNCHER_MODE_ROOT);
	CHECK(launcherSelectMode(true, "1", NULL, false) == LAUNCHER_MODE_ROOT);
	CHECK(launcherSelectMode(true, "1", "1", true) == LAUNCHER_MODE_ROOT);
	CHECK(launcherSelectMode(true, NULL, "1", true) == LAUNCHER_MODE_ROOT);

	CHECK(launcherSelectMode(false, "1", NULL, false) == LAUNCHER_MODE_NONROOT);
	CHECK(launcherSelectMode(false, "true", NULL, false) == LAUNCHER_MODE_NONROOT);
	CHECK(launcherSelectMode(false, "T", NULL, false) == LAUNCHER_MODE_NONROOT);
	CHECK(launcherSelectMode(false, NULL, "1", false) == LAUNCHER_MODE_ROOTLESS);
	CHECK(launcherSelectMode(false, "1", "1", false) == LAUNCHER_MODE_NONROOT);
	CHECK(launcherSelectMode(false, "0", "1", false) == LAUNCHER_MODE_ROOTLESS);
	CHECK(launcherSelectMode(false, "", "", false) == LAUNCHER_MODE_NONE);
	CHECK(launcherSelectMode(false, "0", "no", false) == LAUNCHER_MODE_NONE);
	CHECK(launcherSelectMode(false, NULL, NULL, false) == LAUNCHER_MODE_NONE);
	CHECK(launcherSelectMode(false, NULL, NULL, true) == LAUNCHER_MODE_NONROOT);
	CHECK(launcherSelectMode(false, NULL, "1", true) == LAUNCHER_MODE_ROOTLESS);
}

static void testPrefix(void)
{
	char out[64];

	CHECK(launcherInstallPrefix(LAUNCHER_MODE_ROOT, "/usr/local", "/home/u/darling/bin/darling", out, sizeof(out)) && strcmp(out, "/usr/local") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", "/home/u/darling/bin/darling", out, sizeof(out)) && strcmp(out, "/home/u/darling") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_ROOTLESS, "/usr/local", "/opt/d/bin/darling", out, sizeof(out)) && strcmp(out, "/opt/d") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", "/home/u/darling/bin/other", out, sizeof(out)) && strcmp(out, "/usr/local") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", "relative/bin/darling", out, sizeof(out)) && strcmp(out, "/usr/local") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", NULL, out, sizeof(out)) && strcmp(out, "/usr/local") == 0);
	CHECK(launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", "/bin/darling", out, sizeof(out)) && strcmp(out, "/usr/local") == 0);
	CHECK(!launcherInstallPrefix(LAUNCHER_MODE_NONROOT, "/usr/local", "/some/long/path/bin/darling", out, 8));
	CHECK(!launcherInstallPrefix(LAUNCHER_MODE_ROOT, "/usr/local", NULL, out, 5));
}

static void testLoginName(void)
{
	CHECK(strcmp(launcherPickLoginName("alice", "bob", true, "carol", true, "dave"), "alice") == 0);
	CHECK(strcmp(launcherPickLoginName(NULL, "bob", true, "carol", true, "dave"), "bob") == 0);
	CHECK(strcmp(launcherPickLoginName(NULL, "bob", false, "carol", true, "dave"), "carol") == 0);
	CHECK(strcmp(launcherPickLoginName("", NULL, false, NULL, false, "dave"), "dave") == 0);
	// an environment name for another account is never taken
	CHECK(launcherPickLoginName(NULL, "root", false, "root", false, NULL) == NULL);
	CHECK(launcherPickLoginName(NULL, "", true, "", true, "") == NULL);
}

static void testDescendants(void)
{
	// parent -> child -> grandchild, and an unrelated process (this one's other child)
	int toParent[2];
	pid_t child, other, pids[16], grandchild = 0;
	int count;

	CHECK(pipe(toParent) == 0);
	child = fork();
	if (child == 0)
	{
		pid_t g = fork();

		if (g == 0)
		{
			pause();
			_exit(0);
		}
		write(toParent[1], &g, sizeof(g));
		pause();
		_exit(0);
	}
	other = fork();
	if (other == 0)
	{
		pause();
		_exit(0);
	}
	CHECK(read(toParent[0], &grandchild, sizeof(grandchild)) == sizeof(grandchild));

	count = launcherCollectDescendants("/proc", child, pids, 16);
	CHECK(count == 1 && pids[0] == grandchild);
	count = launcherCollectDescendants("/proc", getpid(), pids, 16);
	CHECK(count >= 3);
	{
		bool hasChild = false, hasGrandchild = false, hasOther = false;

		for (int i = 0; i < count; i++)
		{
			hasChild |= pids[i] == child;
			hasGrandchild |= pids[i] == grandchild;
			hasOther |= pids[i] == other;
		}
		CHECK(hasChild && hasGrandchild && hasOther);
	}
	CHECK(launcherCollectDescendants("/proc", getpid(), pids, 1) == 1);
	CHECK(launcherCollectDescendants("/proc", other, pids, 16) == 0);
	CHECK(launcherCollectDescendants("/nonexistent-proc", 1, pids, 16) == -1);

	kill(grandchild, SIGKILL);
	kill(child, SIGKILL);
	kill(other, SIGKILL);
	waitpid(child, NULL, 0);
	waitpid(other, NULL, 0);
}

static int openProc(pid_t pid)
{
	char path[32];

	snprintf(path, sizeof(path), "/proc/%d", (int) pid);
	return open(path, O_PATH | O_DIRECTORY | O_CLOEXEC);
}

// The mode of a running container is judged from its namespaces: here the "server" is this process (same
// namespaces as the launcher: the non-root layout) and, where the host allows it, a child in a user
// namespace of its own (the rootless layout).
static void testServerMode(void)
{
	struct stat userNs, mntNs;
	int self = openProc(getpid());
	int ready[2], done[2];
	pid_t child;
	char c;

	CHECK(self >= 0);
	CHECK(stat("/proc/self/ns/user", &userNs) == 0 && stat("/proc/self/ns/mnt", &mntNs) == 0);

	CHECK(launcherServerRunsInMode(self, LAUNCHER_MODE_NONROOT, &userNs, &mntNs) == 1);
	CHECK(launcherServerRunsInMode(self, LAUNCHER_MODE_ROOTLESS, &userNs, &mntNs) == 0);
	// the root mode is not examined
	CHECK(launcherServerRunsInMode(self, LAUNCHER_MODE_ROOT, &userNs, &mntNs) == 1);
	// a process that is gone cannot be examined
	{
		pid_t gone = fork();

		if (gone == 0)
			_exit(0);
		int fd = openProc(gone);
		waitpid(gone, NULL, 0);
		if (fd >= 0)
		{
			CHECK(launcherServerRunsInMode(fd, LAUNCHER_MODE_NONROOT, &userNs, &mntNs) == -1);
			close(fd);
		}
	}
	close(self);

	CHECK(pipe(ready) == 0 && pipe(done) == 0);
	child = fork();
	if (child == 0)
	{
		c = unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0 ? 'y' : 'n';
		write(ready[1], &c, 1);
		read(done[0], &c, 1);
		_exit(0);
	}
	CHECK(read(ready[0], &c, 1) == 1);
	if (c == 'y')
	{
		int fd = openProc(child);

		CHECK(fd >= 0);
		CHECK(launcherServerRunsInMode(fd, LAUNCHER_MODE_ROOTLESS, &userNs, &mntNs) == 1);
		CHECK(launcherServerRunsInMode(fd, LAUNCHER_MODE_NONROOT, &userNs, &mntNs) == 0);
		close(fd);
	}
	else
		printf("skipped the user namespace checks: unshare(CLONE_NEWUSER) is not allowed here\n");
	write(done[1], "x", 1);
	waitpid(child, NULL, 0);
}

int main(void)
{
	testSelect();
	testPrefix();
	testLoginName();
	testDescendants();
	testServerMode();
	if (g_failures == 0)
		printf("all passed\n");
	return g_failures != 0;
}
