// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of how the rootless launcher examines the container's namespaces before joining them
// (launcherOpenRootlessNamespaces in src/startup/launcher-server.c), and that the descriptors it hands
// back can be joined with setns() by an unprivileged user. Needs unprivileged user namespaces;
// skips (and passes) where the host does not allow them.

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <linux/capability.h>
#include <unistd.h>
#include "launcher-server.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static int openProc(pid_t pid)
{
	char path[32];

	snprintf(path, sizeof(path), "/proc/%d", (int) pid);
	return open(path, O_PATH | O_DIRECTORY | O_CLOEXEC);
}

static bool writeFile(const char* path, const char* text)
{
	int fd = open(path, O_WRONLY);
	bool ok = fd >= 0 && write(fd, text, strlen(text)) == (ssize_t) strlen(text);

	if (fd >= 0)
		close(fd);
	return ok;
}

// Makes this process the user's only id in a new user and mount namespace, as the rootless launcher's
// server child does.
static bool enterRootlessNamespaces(void)
{
	char map[64];
	uid_t uid = getuid();
	gid_t gid = getgid();

	if (unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
		return false;
	snprintf(map, sizeof(map), "0 %u 1\n", (unsigned) gid);
	if (!writeFile("/proc/self/setgroups", "deny\n") || !writeFile("/proc/self/gid_map", map))
		return false;
	snprintf(map, sizeof(map), "0 %u 1\n", (unsigned) uid);
	return writeFile("/proc/self/uid_map", map);
}

// Runs as a child of the test: joins the namespaces named by the two descriptors the way the launcher
// does and reports the ids and capabilities it has afterwards through its exit status.
static void joinAndReport(int userfd, int mntfd)
{
	struct __user_cap_header_struct header = { _LINUX_CAPABILITY_VERSION_3, 0 };
	struct __user_cap_data_struct data[2];
	char buffer[64];
	int fd;

	if (setns(userfd, CLONE_NEWUSER) != 0 || setns(mntfd, CLONE_NEWNS) != 0)
		_exit(10);
	if (getuid() != 0)
		_exit(11);
	memset(data, 0, sizeof(data));
	if (syscall(SYS_capget, &header, data) != 0 || data[0].effective == 0)
		_exit(12);
	// the mount namespace is the container's: it has its own /proc/self/mountinfo, readable
	fd = open("/proc/self/mountinfo", O_RDONLY);
	if (fd < 0 || read(fd, buffer, sizeof(buffer)) <= 0)
		_exit(13);
	_exit(0);
}

int main(void)
{
	struct stat exe, userNs, mntNs;
	int ready[2], done[2], userfd, mntfd, selfFd;
	pid_t child;
	char c;

	CHECK(stat("/proc/self/exe", &exe) == 0);
	CHECK(stat("/proc/self/ns/user", &userNs) == 0 && stat("/proc/self/ns/mnt", &mntNs) == 0);

	// this process is in the launcher's own namespaces: not a rootless container
	selfFd = openProc(getpid());
	CHECK(selfFd >= 0);
	CHECK(launcherOpenRootlessNamespaces(selfFd, &exe, &userNs, &mntNs, &userfd, &mntfd) == LAUNCHER_SERVER_FOREIGN_USERNS);
	CHECK(userfd == -1 && mntfd == -1);
	close(selfFd);

	CHECK(pipe(ready) == 0 && pipe(done) == 0);
	child = fork();
	if (child == 0)
	{
		pid_t grandchild;

		c = enterRootlessNamespaces() ? 'y' : 'n';
		write(ready[1], &c, 1);
		if (c == 'y')
		{
			// a second level: a user namespace made inside the container's is not its direct child of the launcher's
			grandchild = fork();
			if (grandchild == 0)
			{
				c = unshare(CLONE_NEWUSER) == 0 ? 'y' : 'n';
				write(ready[1], &c, 1);
				read(done[0], &c, 1);
				_exit(0);
			}
			read(done[0], &c, 1);
			kill(grandchild, SIGKILL);
			waitpid(grandchild, NULL, 0);
		}
		_exit(0);
	}
	CHECK(read(ready[0], &c, 1) == 1);
	if (c == 'y')
	{
		int fd = openProc(child);
		pid_t joiner;
		int status;
		char nested;

		CHECK(fd >= 0);
		CHECK(launcherOpenRootlessNamespaces(fd, &exe, &userNs, &mntNs, &userfd, &mntfd) == LAUNCHER_SERVER_OK);
		CHECK(userfd >= 0 && mntfd >= 0);

		// another executable is not the server
		{
			struct stat bogus = exe;
			int u2, m2;

			bogus.st_ino++;
			CHECK(launcherOpenRootlessNamespaces(fd, &bogus, &userNs, &mntNs, &u2, &m2) == LAUNCHER_SERVER_WRONG_EXE);
			CHECK(u2 == -1 && m2 == -1);
		}
		// a launcher in some other user namespace than the container's parent is refused
		{
			struct stat otherUser = userNs;
			int u2, m2;

			otherUser.st_ino++;
			CHECK(launcherOpenRootlessNamespaces(fd, &exe, &otherUser, &mntNs, &u2, &m2) == LAUNCHER_SERVER_FOREIGN_USERNS);
		}

		if (userfd >= 0 && mntfd >= 0)
		{
			joiner = fork();
			if (joiner == 0)
				joinAndReport(userfd, mntfd);
			CHECK(waitpid(joiner, &status, 0) == joiner);
			CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
			if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
				printf("joiner exit status %d\n", WEXITSTATUS(status));
			close(userfd);
			close(mntfd);
		}

		// a user namespace below the container's is not a direct child of the launcher's: refused
		if (read(ready[0], &nested, 1) == 1 && nested == 'y')
		{
			/* the nested process is a grandchild; find it from the child's children */
			char path[64];
			FILE* f;
			long gc = 0;

			snprintf(path, sizeof(path), "/proc/%d/task/%d/children", (int) child, (int) child);
			f = fopen(path, "r");
			if (f != NULL)
			{
				if (fscanf(f, "%ld", &gc) == 1 && gc > 0)
				{
					int gfd = openProc((pid_t) gc);
					int u2, m2;

					CHECK(gfd >= 0);
					CHECK(launcherOpenRootlessNamespaces(gfd, &exe, &userNs, &mntNs, &u2, &m2) == LAUNCHER_SERVER_FOREIGN_USERNS);
					close(gfd);
				}
				fclose(f);
			}
		}
		close(fd);
	}
	else
		printf("skipped: unprivileged user namespaces are not allowed here\n");

	write(done[1], "x", 1);
	write(done[1], "x", 1);
	waitpid(child, NULL, 0);

	if (g_failures == 0)
		printf("all passed\n");
	return g_failures != 0;
}
