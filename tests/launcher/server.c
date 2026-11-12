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
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
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

static pid_t spawnPrefixServer(const char* prefix)
{
	int ready[2];
	char fdText[24];
	pid_t pid;
	if (pipe(ready) != 0)
		return -1;
	pid = fork();
	if (pid == 0)
	{
		char* const argv[] = { "darlingserver", (char*)prefix, "--hold", fdText, NULL };
		close(ready[0]);
		snprintf(fdText, sizeof(fdText), "%d", ready[1]);
		execv("/proc/self/exe", argv);
		_exit(127);
	}
	close(ready[1]);
	if (pid < 0)
	{
		close(ready[0]);
		return -1;
	}
	char byte;
	if (read(ready[0], &byte, 1) != 1)
	{
		close(ready[0]);
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		return -1;
	}
	close(ready[0]);
	return pid;
}

static void writeFixture(const char* path, const void* contents, size_t length)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	CHECK(fd >= 0);
	if (fd < 0)
		return;
	CHECK(write(fd, contents, length) == (ssize_t)length);
	CHECK(close(fd) == 0);
}

static void test_root_effective_prefix_server(void)
{
	const char* prefix = "/synthetic-prefix";
	char procRoot[] = "/tmp/darling-proc-status-XXXXXX";
	char processPath[256], path[512], target[128], status[128];
	char cmdline[] = "darlingserver\0/synthetic-prefix\0--hold\0";
	uid_t invokingUid = geteuid() == 0 ? 1000 : getuid();
	pid_t child = spawnPrefixServer(prefix), found = 0;
	CHECK(child > 0);
	if (child <= 0)
		return;
	CHECK(mkdtemp(procRoot) != NULL);
	snprintf(processPath, sizeof(processPath), "%s/4242", procRoot);
	CHECK(mkdir(processPath, 0755) == 0);
	snprintf(path, sizeof(path), "%s/comm", processPath);
	writeFixture(path, "darlingserver\n", sizeof("darlingserver\n") - 1);
	snprintf(path, sizeof(path), "%s/status", processPath);
	snprintf(status, sizeof(status), "Name:\tdarlingserver\nUid:\t%u\t0\t0\t0\n", (unsigned)invokingUid);
	writeFixture(path, status, strlen(status));
	snprintf(path, sizeof(path), "%s/cmdline", processPath);
	writeFixture(path, cmdline, sizeof(cmdline) - 1);
	snprintf(path, sizeof(path), "%s/exe", processPath);
	snprintf(target, sizeof(target), "/proc/%d/exe", (int)child);
	CHECK(symlink(target, path) == 0);
	snprintf(path, sizeof(path), "%s/ns", processPath);
	CHECK(mkdir(path, 0755) == 0);
	snprintf(path, sizeof(path), "%s/ns/mnt", processPath);
	snprintf(target, sizeof(target), "/proc/%d/ns/mnt", (int)child);
	CHECK(symlink(target, path) == 0);
	if (geteuid() == 0)
	{
		// Linux exposes a root-owned proc directory while the status real uid
		// remains the invoking user and the effective uid is root.
		CHECK(chown(processPath, 0, 0) == 0);
	}

	CHECK(launcherFindPrefixServer(procRoot, prefix, invokingUid, "/proc/self/exe", &found) == 1);
	CHECK(found == 4242);

	kill(child, SIGKILL);
	waitpid(child, NULL, 0);
	snprintf(path, sizeof(path), "%s/ns/mnt", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/ns", processPath); rmdir(path);
	snprintf(path, sizeof(path), "%s/exe", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/cmdline", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/status", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/comm", processPath); unlink(path);
	rmdir(processPath);
	rmdir(procRoot);
}

static void test_prefix_server_scan(void)
{
	const char* prefix = "/synthetic-prefix";
	pid_t pid = spawnPrefixServer(prefix), found = 0;
	char procPath[64], cmdPath[80], command[128];
	struct stat process, server, candidate;
	CHECK(pid > 0);
	if (pid <= 0)
		return;
	snprintf(procPath, sizeof(procPath), "/proc/%d", (int)pid);
	snprintf(cmdPath, sizeof(cmdPath), "%s/cmdline", procPath);
	CHECK(stat(procPath, &process) == 0 && process.st_uid == getuid());
	CHECK(stat("/proc/self/exe", &server) == 0 && stat("/proc/self/exe", &candidate) == 0
		&& server.st_ino == candidate.st_ino && server.st_dev == candidate.st_dev);
	int cmdfd = open(cmdPath, O_RDONLY);
	ssize_t commandLength = cmdfd >= 0 ? read(cmdfd, command, sizeof(command)) : -1;
	if (cmdfd >= 0)
		close(cmdfd);
	CHECK(commandLength > 0 && launcherServerCommandLineMatchesPrefix(command,
		(size_t)commandLength, prefix));
	errno = 0;
	int scanResult = launcherFindPrefixServer("/proc", prefix, getuid(), "/proc/self/exe", &found);
	CHECK(scanResult == 1);
	CHECK(found == pid);
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
	scanResult = launcherFindPrefixServer("/proc", prefix, getuid(), "/proc/self/exe", &found);
	CHECK(scanResult == 0);
}

static void test_orphaned_init_parsers(void)
{
	static const char init[] = "Name:\tmldr\nNSpid:\t4321\t1\nUid:\t1000\t1000\t1000\t1000\n";
	static const char notInit[] = "NSpid:\t4321\t7\n";
	static const char hostInit[] = "NSpid:\t1\n";
	static const char nested[] = "NSpid:\t4321\t55\t1\n";
	static const char env[] = "A=1\0__mldr_sockpath=/home/u/.darling/.darlingserver.sock\0B=2\0";

	CHECK(launcherStatusIsNamespaceInit(init, sizeof(init) - 1));
	CHECK(launcherStatusIsNamespaceInit(nested, sizeof(nested) - 1));
	CHECK(!launcherStatusIsNamespaceInit(notInit, sizeof(notInit) - 1));
	// pid 1 of the host's own namespace is not a container's init
	CHECK(!launcherStatusIsNamespaceInit(hostInit, sizeof(hostInit) - 1));
	CHECK(!launcherStatusIsNamespaceInit("Name:\tmldr\n", 11));
	CHECK(!launcherStatusIsNamespaceInit("NSpid:\tx\t1\n", 12));

	CHECK(launcherEnvironmentHas(env, sizeof(env) - 1, "__mldr_sockpath", "/home/u/.darling/.darlingserver.sock"));
	CHECK(!launcherEnvironmentHas(env, sizeof(env) - 1, "__mldr_sockpath", "/home/u/.darling"));
	CHECK(!launcherEnvironmentHas(env, sizeof(env) - 1, "__mldr_sockpath", "/home/u/.darling/.darlingserver.sock2"));
	CHECK(!launcherEnvironmentHas(env, sizeof(env) - 1, "mldr_sockpath", "/home/u/.darling/.darlingserver.sock"));
	CHECK(!launcherEnvironmentHas(env, sizeof(env) - 1, "A", ""));
	CHECK(launcherEnvironmentHas(env, sizeof(env) - 1, "B", "2"));
	CHECK(!launcherEnvironmentHas("", 0, "A", "1"));
}

static void makeFixtureFile(const char* dir, const char* name, const char* contents)
{
	char path[512];

	snprintf(path, sizeof(path), "%s/%s", dir, name);
	writeFixture(path, contents, strlen(contents));
}

// A synthetic /proc with one process shaped like a container's init.
static void test_orphaned_init_scan(void)
{
	const char* prefix = "/synthetic-prefix";
	char procRoot[] = "/tmp/darling-proc-orphan-XXXXXX";
	char processPath[256], path[512], status[160], environ[160];
	uid_t uid = geteuid() == 0 ? 1000 : getuid();
	pid_t found = 0;

	CHECK(mkdtemp(procRoot) != NULL);
	snprintf(processPath, sizeof(processPath), "%s/4242", procRoot);
	CHECK(mkdir(processPath, 0755) == 0);
	snprintf(path, sizeof(path), "%s/exe", processPath);
	CHECK(symlink("/proc/self/exe", path) == 0);
	snprintf(status, sizeof(status), "Name:\tmldr\nNSpid:\t4242\t1\nUid:\t%u\t%u\t%u\t%u\n", (unsigned)uid, (unsigned)uid, (unsigned)uid, (unsigned)uid);
	makeFixtureFile(processPath, "status", status);
	// strlen() stops at the first NUL, so write the block with explicit lengths
	snprintf(path, sizeof(path), "%s/environ", processPath);
	snprintf(environ, sizeof(environ), "X=1%c__mldr_sockpath=%s/.darlingserver.sock%c", 0, prefix, 0);
	writeFixture(path, environ, 3 + 1 + strlen("__mldr_sockpath=") + strlen(prefix) + strlen("/.darlingserver.sock") + 1);

	CHECK(launcherFindOrphanedInit(procRoot, prefix, uid, "/proc/self/exe", &found) == 1);
	CHECK(found == 4242);
	// another prefix, another user, another executable
	CHECK(launcherFindOrphanedInit(procRoot, "/other-prefix", uid, "/proc/self/exe", &found) == 0);
	CHECK(launcherFindOrphanedInit(procRoot, prefix, uid + 1, "/proc/self/exe", &found) == 0);
	CHECK(launcherFindOrphanedInit(procRoot, prefix, uid, "/bin/sh", &found) == 0);
	CHECK(launcherFindOrphanedInit(procRoot, prefix, uid, "/nonexistent/mldr", &found) == -1);
	CHECK(launcherFindOrphanedInit("/nonexistent/proc", prefix, uid, "/proc/self/exe", &found) == -1);

	// not the first process of its namespace
	snprintf(status, sizeof(status), "Name:\tmldr\nNSpid:\t4242\t9\nUid:\t%u\t%u\t%u\t%u\n", (unsigned)uid, (unsigned)uid, (unsigned)uid, (unsigned)uid);
	snprintf(path, sizeof(path), "%s/status", processPath);
	unlink(path);
	makeFixtureFile(processPath, "status", status);
	CHECK(launcherFindOrphanedInit(procRoot, prefix, uid, "/proc/self/exe", &found) == 0);

	snprintf(path, sizeof(path), "%s/environ", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/status", processPath); unlink(path);
	snprintf(path, sizeof(path), "%s/exe", processPath); unlink(path);
	rmdir(processPath);
	rmdir(procRoot);
}

int main(int argc, char** argv)
{
	if (argc == 4 && !strcmp(argv[2], "--hold"))
	{
		int fd = atoi(argv[3]);
		char byte = 'x';
		prctl(PR_SET_NAME, "darlingserver", 0, 0, 0);
		write(fd, &byte, 1);
		close(fd);
		for (;;)
			pause();
	}

	static const char matching[] = "darlingserver\0/home/tester/.darling\0uid\0";
	static const char differentPrefix[] = "darlingserver\0/home/tester/.other\0uid\0";
	static const char differentExecutable[] = "mldr\0/home/tester/.darling\0uid\0";
	static const char rootEffectiveStatus[] = "Name:\tdarlingserver\nState:\tR\nUid:\t1000\t0\t0\t0\n";
	uid_t parsedUid = (uid_t)-1;
	CHECK(launcherServerStatusRealUid(rootEffectiveStatus, sizeof(rootEffectiveStatus) - 1, &parsedUid));
	CHECK(parsedUid == 1000); // real uid remains the invoking user while effective uid is root
	CHECK(!launcherServerStatusRealUid("Name:\tdarlingserver\n", 20, &parsedUid));
	CHECK(launcherServerCommandLineMatchesPrefix(matching, sizeof(matching) - 1,
		"/home/tester/.darling"));
	CHECK(!launcherServerCommandLineMatchesPrefix(differentPrefix, sizeof(differentPrefix) - 1,
		"/home/tester/.darling"));
	CHECK(!launcherServerCommandLineMatchesPrefix(differentExecutable, sizeof(differentExecutable) - 1,
		"/home/tester/.darling"));
	CHECK(!launcherServerCommandLineMatchesPrefix(matching, 12, "/home/tester/.darling"));

	if (stat("/proc/self/exe", &g_exe) != 0 || stat("/proc/self/ns/user", &g_userNs) != 0
		|| stat("/proc/self/ns/mnt", &g_mntNs) != 0)
	{
		printf("FAIL: cannot examine this process\n");
		return 1;
	}

	test_same_namespace();
	test_foreign_namespace();
	test_prefix_server_scan();
	test_root_effective_prefix_server();
	test_orphaned_init_parsers();
	test_orphaned_init_scan();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher server: all checks passed\n");
	return 0;
}
