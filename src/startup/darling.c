/*
This file is part of Darling.

Copyright (C) 2016-2023 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include <sched.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <linux/capability.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>
#include <termios.h>
#include <pty.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <sys/auxv.h>
#include "../shellspawn/shellspawn.h"
#include "darling.h"
#include "darling-config.h"
#include "launcher-env.h"
#include "launcher-ids.h"
#include "launcher-mode.h"
#include "launcher-prefix.h"
#include "launcher-server.h"
#include "prefix-state.h"
#include "profile-prefix.h"

// Between Linux 4.9 and 4.11, a strange bug has been introduced
// which prevents connecting to Unix sockets if the socket was
// created in a different mount namespace or under overlayfs
// (dunno which one is really responsible for this).
#define USE_LINUX_4_11_HACK 1

char *prefix;
uid_t g_originalUid, g_originalGid;
bool g_fixPermissions = false;
char g_workingDirectory[4096];
static struct launcher_ids g_invokingIds;
static enum launcher_mode g_mode = LAUNCHER_MODE_ROOT;

// Where the installation is: compiled in for the root mode, and for the unprivileged
// modes wherever this executable lives (see launcherInstallPrefix()).
static char g_serverPath[4096];
static char g_mldrPath[4096];
static char g_libexecPath[4096];

#if defined(__ANDROID__)
	// Android hosts have neither setuid installs nor user namespaces to count on.
	#define LAUNCHER_DEFAULT_NONROOT true
#else
	#define LAUNCHER_DEFAULT_NONROOT false
#endif

// Only the root mode has ids to switch. Without root, the launcher already has only the
// invoking user's ids, and the container's processes are the user's.
static bool privilegedMode(void)
{
	return g_mode == LAUNCHER_MODE_ROOT;
}

static void joinRootlessNamespaces(pid_t pid);

// Everything the launcher does inside the user's prefix (which the user names
// and controls) runs with the user's effective ids, so that it can only do what
// the user could do without Darling. See launcher-ids.h for the states.
static void useInvokingIds(void)
{
	if (!privilegedMode())
		return;
	if (!launcherUseInvokingIds(&g_invokingIds))
	{
		fprintf(stderr, "Cannot switch to your user and group ids: %s\n", strerror(errno));
		exit(1);
	}
}

// For the rest of its life the launcher only talks to processes of the user.
static void dropPrivileges(void)
{
	if (!privilegedMode())
		return;
	if (!launcherDropPrivileges(&g_invokingIds))
	{
		fprintf(stderr, "Cannot drop root privileges: %s\n", strerror(errno));
		exit(1);
	}
}

static void restoreRootIds(void)
{
	if (!privilegedMode())
		return;
	if (!launcherRestoreRootIds())
	{
		fprintf(stderr, "Cannot switch back to root ids: %s\n", strerror(errno));
		exit(1);
	}
}

// A handle that becomes readable when the process has exited, or -1 when the
// kernel has none (it then falls back to polling the pid).
static int openProcessHandle(pid_t pid)
{
#ifdef SYS_pidfd_open
	return (int) syscall(SYS_pidfd_open, pid, 0);
#else
	(void) pid;
	return -1;
#endif
}

// Waits up to ten seconds for the process to exit, then gives up quietly: it
// has been sent SIGKILL, and only a process stuck in the kernel can outlast that.
static void waitForProcessExit(int handle, pid_t pid)
{
	if (handle >= 0)
	{
		struct pollfd pfd = { .fd = handle, .events = POLLIN };

		while (poll(&pfd, 1, 10000) < 0 && errno == EINTR)
			;
		close(handle);
		return;
	}

	for (int i = 0; i < 1000 && (kill(pid, 0) == 0 || errno != ESRCH); i++)
		usleep(10000);
}

// Whether the process has any effective or permitted capability. A failure to tell counts as having one.
static bool hasCapabilities(void)
{
	struct __user_cap_header_struct header = { _LINUX_CAPABILITY_VERSION_3, 0 };
	struct __user_cap_data_struct data[2] = { 0 };

	if (syscall(SYS_capget, &header, data) != 0)
		return true;
	return (data[0].effective | data[1].effective | data[0].permitted | data[1].permitted) != 0;
}

int main(int argc, char ** argv)
{
	pid_t pidInit;

	if (argc <= 1)
	{
		showHelp(argv[0]);
		return 1;
	}

	// Whether the launcher has root's power decides the mode; the environment only chooses
	// between the unprivileged modes (see launcherSelectMode()).
	enum launcher_privilege privilege = launcherClassifyPrivilege(geteuid() == 0, getauxval(AT_SECURE) != 0, hasCapabilities());
	if (privilege == LAUNCHER_PRIVILEGED_NOT_ROOT)
	{
		fprintf(stderr, "darling was started with privileges, but not as root (installed set-group-ID or with file capabilities,\n"
			"or run with capabilities). Install it setuid root, or run it as an ordinary user without privileges.\n");
		return 1;
	}
	g_mode = launcherSelectMode(privilege == LAUNCHER_PRIVILEGED, getenv("DARLING_NONROOT"), getenv("DARLING_ROOTLESS"), LAUNCHER_DEFAULT_NONROOT);
	if (g_mode == LAUNCHER_MODE_NONE)
	{
		missingSetuidRoot();
		return 1;
	}
	unsetenv("DARLING_NONROOT");
	unsetenv("DARLING_ROOTLESS");

	{
		char exePath[4096], installPrefix[4096];
		ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);

		exePath[len > 0 ? len : 0] = '\0';
		if (!launcherInstallPrefix(g_mode, INSTALL_PREFIX, len > 0 ? exePath : NULL, installPrefix, sizeof(installPrefix))
			|| snprintf(g_serverPath, sizeof(g_serverPath), "%s/bin/darlingserver", installPrefix) >= (int) sizeof(g_serverPath)
			|| snprintf(g_libexecPath, sizeof(g_libexecPath), "%s/libexec/darling", installPrefix) >= (int) sizeof(g_libexecPath)
			|| snprintf(g_mldrPath, sizeof(g_mldrPath), "%s/usr/libexec/darling/mldr", g_libexecPath) >= (int) sizeof(g_mldrPath))
		{
			fprintf(stderr, "The installation path is too long.\n");
			return 1;
		}
	}

	if (!launcherSaveInvokingIds(&g_invokingIds))
	{
		fprintf(stderr, "Cannot read your user and group ids: %s\n", strerror(errno));
		return 1;
	}
	g_originalUid = g_invokingIds.uid;
	g_originalGid = g_invokingIds.gid;

	// darlingserver must start with real root ids, and creating namespaces needs
	// them as well. From here on the launcher is fully root, so every step that
	// acts on a path or file the user controls switches to the user's ids first.
	if (privilegedMode() && !launcherBecomeRoot())
	{
		fprintf(stderr, "Cannot switch to root ids: %s\n", strerror(errno));
		return 1;
	}

	enum darling_prefix_error prefixError;
	prefix = darlingSelectPrefix(getenv("DPREFIX"), getenv("DARLING_PROFILE"),
		getenv("HOME"), &prefixError);
	if (!prefix)
	{
		fprintf(stderr, "%s.\n", darlingPrefixError(prefixError));
		return 1;
	}
	enum launcher_prefix_check prefixCheck = launcherCheckPrefix(prefix);
	if (prefixCheck != LAUNCHER_PREFIX_OK)
	{
		fprintf(stderr, "%s: %s\n", launcherPrefixCheckMessage(prefixCheck), prefix);
		if (prefixCheck == LAUNCHER_PREFIX_TOO_LONG)
			fprintf(stderr, "Use a prefix path of at most %zu bytes.\n", launcherMaxPrefixLength());
		return 1;
	}
	unsetenv("DPREFIX");
	unsetenv("DARLING_PROFILE");
	getcwd(g_workingDirectory, sizeof(g_workingDirectory));

	if (!checkPrefixDir())
	{
		setupPrefix();
		g_fixPermissions = true;
	}
	checkPrefixOwner();
	ensureSystemAccounts();

	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"help", 	no_argument, 0, 0},
			{"version", no_argument, 0, 0},
			{0, 		0, 			 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "+", long_options, &option_index);

		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 0:
			if (strcmp(long_options[option_index].name, "help") == 0)
			{
				showHelp(argv[0]);
				exit(EXIT_SUCCESS);
			}
			else if (strcmp(long_options[option_index].name, "version") == 0)
			{
				showVersion(argv[0]);
				exit(EXIT_SUCCESS);
			}
			break;
			case '?':
			break;
			default:
			abort();
		}
	}

	pidInit = getInitProcess();

	if (strcmp(argv[1], "shutdown") == 0)
	{
		if (pidInit == 0)
		{
			// No darlingserver, but launchd may have outlived it (a server
			// that crashed, or one from before launchd was tied to it).
			// launchd is the first process of its pid namespace: ending it
			// ends every process of the container.
			pid_t orphan = 0;
			int scan;

			dropPrivileges();
			scan = launcherFindOrphanedInit("/proc", prefix, g_originalUid,
				g_mldrPath, &orphan);
			if (scan > 0)
			{
				int orphanFd = openProcessHandle(orphan);

				if (kill(orphan, SIGKILL) != 0 && errno != ESRCH)
				{
					fprintf(stderr, "Failed to shutdown Darling container: %s\n", strerror(errno));
					return 1;
				}
				waitForProcessExit(orphanFd, orphan);
				fprintf(stderr, "Stopped a Darling container whose darlingserver had exited\n");
				return 0;
			}
			if (scan < 0)
			{
				fprintf(stderr, "Cannot inspect running processes: %s\n", strerror(errno));
				return 1;
			}
			fprintf(stderr, "Darling container is not running\n");
			return 1;
		}

		// The server and launchd run as the user once they are up. Killing them
		// with the user's ids means that, should the pid be reused after the
		// check, the signal reaches nothing the user could not signal anyway.
		dropPrivileges();

		// TODO: when we have a working launchd,
		// this is where we ask it to shut down nicely

		char path_buf[128];
		FILE* file;
		pid_t launchd_pid;
		snprintf(path_buf, sizeof(path_buf), "/proc/%d/task/%d/children", pidInit, pidInit);
		file = fopen(path_buf, "r");
		if (!file || fscanf(file, "%d", &launchd_pid) != 1) {
			fprintf(stderr, "Failed to shutdown Darling container\n");
			if (file) {
				fclose(file);
			}
			return 1;
		}
		fclose(file);

		// Handles on both processes, taken before the signals so that a pid
		// reused in between cannot be mistaken for them.
		int launchdFd = openProcessHandle(launchd_pid);
		int serverFd = openProcessHandle(pidInit);

		// Without a PID namespace ending launchd does not end the container's other processes. They are
		// below launchd in the process tree (launchd adopts the orphans), so take them from there while
		// the tree is still whole.
		pid_t* descendants = NULL;
		int descendantCount = 0;
		if (g_mode == LAUNCHER_MODE_NONROOT)
		{
			const size_t maxDescendants = 4096;

			descendants = malloc(maxDescendants * sizeof(pid_t));
			if (descendants != NULL)
				descendantCount = launcherCollectDescendants("/proc", pidInit, descendants, maxDescendants);
			if (descendantCount < 0)
				descendantCount = 0;
		}

		kill(launchd_pid, SIGKILL);
		for (int i = 0; i < descendantCount; i++)
		{
			if (descendants[i] != launchd_pid)
				kill(descendants[i], SIGKILL);
		}
		free(descendants);
		if (kill(pidInit, SIGKILL) != 0 && errno != ESRCH)
		{
			fprintf(stderr, "Failed to shutdown Darling container: %s\n", strerror(errno));
			return 1;
		}

		// The kill is asynchronous. Returning before the server is gone lets a
		// container started right after this still find the dying server through
		// the prefix's pid file and fail to join it. launchd is the first
		// process of the container's pid namespace, so it is gone only once
		// every process of the container is.
		waitForProcessExit(launchdFd, launchd_pid);
		waitForProcessExit(serverFd, pidInit);
		return 0;
	}

	// If prefix's init is not running, start it up
	if (pidInit == 0)
	{
		pid_t liveServer = 0;
		int scan = launcherFindPrefixServer("/proc", prefix, g_originalUid,
			g_serverPath, &liveServer);
		if (scan < 0)
		{
			fprintf(stderr, "Cannot inspect existing Darling servers; refusing to reinitialize %s.\n", prefix);
			return 1;
		}
		if (scan > 0)
		{
			fprintf(stderr,
				"darlingserver %d is still running for %s but its .init.pid is missing or invalid; refusing to start a second server.\n"
				"If another Darling is starting this prefix, retry shortly; otherwise stop darlingserver %d before retrying.\n",
				(int)liveServer, prefix, (int)liveServer);
			return 1;
		}

		// A container left behind by a darlingserver that exited owns the
		// prefix's sockets; starting another one would take them over.
		pid_t orphanedInit = 0;
		useInvokingIds();
		scan = launcherFindOrphanedInit("/proc", prefix, g_originalUid,
			g_mldrPath, &orphanedInit);
		restoreRootIds();
		if (scan > 0)
		{
			fprintf(stderr,
				"launchd %d is still running for %s but its darlingserver has exited.\n"
				"Run 'darling shutdown' to stop it before starting the container again.\n",
				(int)orphanedInit, prefix);
			return 1;
		}

		char socketPath[4096];
		
		snprintf(socketPath, sizeof(socketPath), "%s"  SHELLSPAWN_SOCKPATH, prefix);
		
		// A stale socket in the user's prefix; remove it as the user, so that a
		// prefix path changed under us cannot make root remove anything else.
		useInvokingIds();
		unlink(socketPath);
		restoreRootIds();
		
		setupWorkdir();
		pidInit = spawnInitProcess();
		putInitPid(pidInit);
		
		// Wait until shellspawn starts. The socket is in the user's prefix, so
		// look for it with the user's ids: access() would check with the real
		// uid, which is 0.
		useInvokingIds();
		bool started = false;
		for (int i = 0; i < 60 && !started; i++)
		{
			started = faccessat(AT_FDCWD, socketPath, F_OK, AT_EACCESS) == 0;
			if (!started)
				sleep(1);
		}
		restoreRootIds();

		if (!started)
		{
			fprintf(stderr, "Timed out waiting for shellspawn in container\n");
			return 1;
		}
	}

#if USE_LINUX_4_11_HACK
	// The root mode joins the container's mount namespace as root. The rootless mode joins the user
	// namespace it made and the mount namespace in it, as the user who owns them: the overlay is mounted
	// in there, and a socket in the host's view of the prefix cannot be connected to. The non-root mode has
	// no namespaces; the shellspawn socket is reached through the prefix directory.
	if (privilegedMode())
		joinServerMountNamespace(pidInit);
	else if (g_mode == LAUNCHER_MODE_ROOTLESS)
		joinRootlessNamespaces(pidInit);
#endif

	// Joining the mount namespace was the last step that needs root. Drop it
	// for good: the rest only talks to shellspawn and resolves the program path
	// given on the command line, both as the user. Switching only the effective
	// ids, as before, left root in the saved uid, so any memory corruption in
	// the code below could regain it. Nothing may run as root inside the joined
	// namespace, whose files are not the host's: not even the C library loading
	// a name service module.
	dropPrivileges();

	if (strcmp(argv[1], "shell") == 0)
	{
		// Spawn the shell
		if (argc > 2)
			spawnShell((const char**) &argv[2]);
		else
			spawnShell(NULL);
	}
	else
	{
		bool doExec = strcmp(argv[1], "exec") == 0;
		int argvIndex = doExec ? 2 : 1;

		if (doExec && argc <= 2)
		{
			printf("'exec' subcommand requires a binary to execute.\n");
			return 1;
		}

		char *fullPath;
		char *path = realpath(argv[argvIndex], NULL);

		if (path == NULL)
		{
			printf("'%s' is not a supported command or a file.\n", argv[argvIndex]);
			return 1;
		}

		fullPath = malloc(strlen(SYSTEM_ROOT) + strlen(path) + 1);
		strcpy(fullPath, SYSTEM_ROOT);
		strcat(fullPath, path);
		free(path);

		argv[argvIndex] = fullPath;

		if (doExec)
			spawnBinary(argv[argvIndex], (const char**) &argv[argvIndex]);
		else
			spawnShell((const char**) &argv[argvIndex]);
	}

	return 0;
}

// Rootless mode: the container's mount namespace is owned by the user namespace the launcher's server
// child made, so it can only be joined from inside that user namespace. The user is that namespace's
// owner, which gives the launcher the right to join it, and nothing else: the checks below insist on a
// direct child of the launcher's own namespace that was made by this user, and the namespaces are
// opened through a pinned /proc/<pid> descriptor (see launcherOpenRootlessNamespaces()).
static void joinRootlessNamespaces(pid_t pid)
{
	struct stat exe, userNs, mntNs;
	char procPath[64];
	int procfd, userfd = -1, mntfd = -1;
	enum launcher_server_check check;

	if (stat(g_serverPath, &exe) != 0
		|| stat("/proc/self/ns/user", &userNs) != 0
		|| stat("/proc/self/ns/mnt", &mntNs) != 0)
	{
		fprintf(stderr, "Cannot examine darlingserver or this process's namespaces: %s\n", strerror(errno));
		exit(1);
	}

	snprintf(procPath, sizeof(procPath), "/proc/%d", (int) pid);
	procfd = open(procPath, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (procfd < 0)
		check = LAUNCHER_SERVER_GONE;
	else
	{
		check = launcherOpenRootlessNamespaces(procfd, &exe, &userNs, &mntNs, &userfd, &mntfd);
		int error = errno;
		close(procfd);
		errno = error;
	}

	switch (check)
	{
		case LAUNCHER_SERVER_OK:
			break;
		case LAUNCHER_SERVER_GONE:
			fprintf(stderr, "The Darling container (process %d) exited. Try again.\n", (int) pid);
			exit(1);
		case LAUNCHER_SERVER_WRONG_EXE:
			fprintf(stderr, "Process %d, named in %s/.init.pid, is not running %s.\n"
				"If Darling was updated while the container was running, run 'darling shutdown' and try again.\n",
				(int) pid, prefix, g_serverPath);
			exit(1);
		case LAUNCHER_SERVER_FOREIGN_USERNS:
		case LAUNCHER_SERVER_SAME_MNTNS:
			fprintf(stderr, "Process %d, named in %s/.init.pid, is not a rootless Darling container of this user; refusing to join it.\n"
				"Run 'darling shutdown' and try again.\n", (int) pid, prefix);
			exit(1);
		case LAUNCHER_SERVER_ERROR:
		default:
			fprintf(stderr, "Cannot examine the Darling container (process %d): %s\n", (int) pid, strerror(errno));
			exit(1);
	}

	if (setns(userfd, CLONE_NEWUSER) != 0 || setns(mntfd, CLONE_NEWNS) != 0)
	{
		fprintf(stderr, "Cannot join the container's namespaces: %s\n", strerror(errno));
		exit(1);
	}
	close(userfd);
	close(mntfd);

	// Joining a user namespace grants every capability in it. The rest of the launcher only talks to
	// shellspawn and resolves paths, none of which needs them: give them all up.
	{
		struct __user_cap_header_struct header = { _LINUX_CAPABILITY_VERSION_3, 0 };
		struct __user_cap_data_struct data[2];

		memset(data, 0, sizeof(data));
		if (syscall(SYS_capset, &header, data) != 0)
		{
			fprintf(stderr, "Cannot drop the capabilities of the container's user namespace: %s\n", strerror(errno));
			exit(1);
		}
	}
}

// Joins the mount namespace of the container's darlingserver. This runs as
// root and the pid may come from the pid file in the user's prefix, so the
// process is checked first, through a pinned /proc/<pid> descriptor: it must
// run the installed darlingserver binary, and its mount namespace must be a
// different one owned by our own user namespace. A user can create mount
// namespaces only inside user namespaces of their own, so a namespace owned
// by ours was created by root, as darlingserver does. setns() then uses the
// namespace descriptor that was checked, not a path looked up again.
void joinServerMountNamespace(pid_t pid)
{
	struct stat exe, userNs, mntNs;
	char procPath[64];
	int procfd, nsfd;
	enum launcher_server_check check;

	if (stat(g_serverPath, &exe) != 0
		|| stat("/proc/self/ns/user", &userNs) != 0
		|| stat("/proc/self/ns/mnt", &mntNs) != 0)
	{
		fprintf(stderr, "Cannot examine darlingserver or this process's namespaces: %s\n", strerror(errno));
		exit(1);
	}

	snprintf(procPath, sizeof(procPath), "/proc/%d", (int) pid);
	procfd = open(procPath, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (procfd < 0)
		check = LAUNCHER_SERVER_GONE;
	else
	{
		check = launcherOpenServerMountNs(procfd, &exe, &userNs, &mntNs, &nsfd);
		int error = errno;
		close(procfd);
		errno = error;
	}

	switch (check)
	{
		case LAUNCHER_SERVER_OK:
			break;
		case LAUNCHER_SERVER_GONE:
			fprintf(stderr, "The Darling container (process %d) exited. Try again.\n", (int) pid);
			exit(1);
		case LAUNCHER_SERVER_WRONG_EXE:
			fprintf(stderr, "Process %d, named in %s/.init.pid, is not running %s.\n"
				"If Darling was updated while the container was running, run 'darling shutdown' and try again.\n",
				(int) pid, prefix, g_serverPath);
			exit(1);
		case LAUNCHER_SERVER_FOREIGN_USERNS:
		case LAUNCHER_SERVER_SAME_MNTNS:
			fprintf(stderr, "Process %d, named in %s/.init.pid, does not have a Darling container's mount namespace; refusing to join it.\n"
				"Run 'darling shutdown' and try again.\n", (int) pid, prefix);
			exit(1);
		case LAUNCHER_SERVER_ERROR:
		default:
			fprintf(stderr, "Cannot examine the Darling container (process %d): %s\n", (int) pid, strerror(errno));
			exit(1);
	}

	if (setns(nsfd, CLONE_NEWNS) != 0)
	{
		fprintf(stderr, "Cannot join the container's mount namespace: %s\n", strerror(errno));
		exit(1);
	}
	close(nsfd);
}

static void pushShellspawnCommandData(int sockfd, shellspawn_cmd_type_t type, const void* data, size_t data_length)
{
	struct shellspawn_cmd* cmd;
	size_t length;

	length = sizeof(*cmd) + data_length;

	cmd = (struct shellspawn_cmd*) malloc(length);
	cmd->cmd = type;
	cmd->data_length = data_length;

	if (data != NULL)
		memcpy(cmd->data, data, data_length);

	if (write(sockfd, cmd, length) != length)
	{
		fprintf(stderr, "Error sending command to shellspawn: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	free(cmd);
}

static void pushShellspawnCommand(int sockfd, shellspawn_cmd_type_t type, const char* value)
{
	if (!value)
		pushShellspawnCommandData(sockfd, type, NULL, 0);
	else
		pushShellspawnCommandData(sockfd, type, value, strlen(value) + 1);
}

static void pushShellspawnCommandFDs(int sockfd, shellspawn_cmd_type_t type, const int fds[3])
{
	struct shellspawn_cmd cmd;
	char cmsgbuf[CMSG_SPACE(sizeof(int) * 3)];
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *cmptr;

	cmd.cmd = type;
	cmd.data_length = 0;

	iov.iov_base = &cmd;

	memset(&msg, 0, sizeof(msg));
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	iov.iov_base = &cmd;
	iov.iov_len = sizeof(cmd);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int) * 3);
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	memcpy(CMSG_DATA(cmptr), fds, sizeof(fds[0])*3);

	if (sendmsg(sockfd, &msg, 0) < 0)
	{
		fprintf(stderr, "Error sending command to shellspawn: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void restoreTermios(void);
static int _shSockfd = -1;
static struct termios orig_termios;
static int pty_master;
static volatile sig_atomic_t sigint_count = 0;
static volatile sig_atomic_t termios_saved = 0;

static void signalHandler(int signo)
{
	int forwarded = signo;
	bool terminate = false;

	// Forward window size changes
	if (signo == SIGWINCH && pty_master != -1)
	{
		struct winsize win;

		ioctl(0, TIOCGWINSZ, &win);
		ioctl(pty_master, TIOCSWINSZ, &win);
		return;
	}

	// These end the launcher. A second SIGINT does too, so that a guest
	// that ignores the first one cannot leave the host terminal stuck.
	if (signo == SIGTERM || signo == SIGHUP || signo == SIGQUIT)
		terminate = true;
	else if (signo == SIGINT && ++sigint_count > 1)
		terminate = true;

	// Foreground process loopkup in shellspawn doesn't work
	// if we're not running in TTY mode, so shellspawn falls back
	// to forwarding signals to the Bash subprocess.
	// 
	// Hence we translate SIGINT to SIGTERM for user convenience,
	// because Bash will not terminate on SIGINT.
	if (pty_master == -1 && signo == SIGINT)
		forwarded = SIGTERM;

	// The guest gets the signal even when the launcher goes away.
	pushShellspawnCommandData(_shSockfd, SHELLSPAWN_SIGNAL, &forwarded, sizeof(forwarded));

	if (terminate)
	{
		// Only async-signal-safe calls from here on.
		restoreTermios();
		_exit(128 + signo);
	}
}

static void shellLoop(int sockfd, int master)
{
	struct sigaction sa;
	struct pollfd pfds[3];
	const int fdcount = (master != -1) ? 3 : 1; // do we do pty proxying?

	// Vars for signal handler
	_shSockfd = sockfd;
	pty_master = master;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalHandler;
	sigfillset(&sa.sa_mask);

	for (int i = 1; i < 32; i++)
		sigaction(i, &sa, NULL);

	pfds[2].fd = master;
	pfds[2].events = POLLIN;
	pfds[2].revents = 0;
	pfds[1].fd = STDIN_FILENO;
	pfds[1].events = POLLIN;
	pfds[1].revents = 0;
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;
	pfds[0].revents = 0;

	if (master != -1)
		fcntl(master, F_SETFL, O_NONBLOCK);
	//fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	while (1)
	{
		char buf[4096];

		if (poll(pfds, fdcount, -1) < 0)
		{
			if (errno != EINTR)
			{
				perror("poll");
				break;
			}
		}

		if (pfds[2].revents & POLLIN)
		{
			int rd;
			do
			{
				rd = read(master, buf, sizeof(buf));
				if (rd > 0)
					write(STDOUT_FILENO, buf, rd);
			}
			while (rd == sizeof(buf));
		}

		if (pfds[1].revents & POLLIN)
		{
			int rd = 0;
			do
			{
				if (ioctl(STDIN_FILENO, FIONREAD, &rd) < 0) {
					perror("ioctl");
					exit(1);
				}
				if (rd > sizeof(buf)) {
					rd = sizeof(buf);
				}
				rd = read(STDIN_FILENO, buf, rd);
				if (rd > 0)
					write(master, buf, rd);
				else {
					perror("read");
					exit(1);
				}
			}
			while (rd == sizeof(buf));
		}

		if (pfds[0].revents & (POLLHUP | POLLIN))
		{
			int exitStatus;
			
			if (read(sockfd, &exitStatus, sizeof(int)) == sizeof(int))
				exit(exitStatus);
			else
				exit(1);
		}
	}
}

static void restoreTermios(void)
{
	// Never apply the zeroed orig_termios to a terminal we did not change.
	if (termios_saved)
		tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Glibc openpty() fails for me on Debian, because grantpty() fails to chown() the pty node with error code EPERM.
// This is a more lenient version of openpty() that just works.
static int openpty_darling(int* amaster, int* aslave, char* name_unused, const struct termios* tos, const struct winsize* wsz)
{
	const char* slave_name;

	*amaster = posix_openpt(O_RDWR);
	if (*amaster == -1)
		return -1;

	grantpt(*amaster);
	if (unlockpt(*amaster) < 0)
		return -1;

	slave_name = ptsname(*amaster);
	*aslave = open(slave_name, O_RDWR | O_NOCTTY);
	if (*aslave == -1)
		return -1;

	if (tos != NULL)
		tcsetattr(*amaster, TCSANOW, tos);
	if (wsz != NULL)
		ioctl(*amaster, TIOCSWINSZ, wsz);

	return 0;
}

static void setupPtys(int fds[3], int* master)
{
	struct winsize win;
	struct termios termios;
	bool tty = true;

	if (tcgetattr(STDIN_FILENO, &termios) < 0)
		tty = false;

	if (openpty_darling(master, &fds[0], NULL, &termios, NULL) < 0)
	{
		perror("openpty");
		exit(1);
	}
	fds[2] = fds[1] = fds[0];

	if (tty)
	{
		orig_termios = termios;
		termios_saved = 1;

		ioctl(0, TIOCGWINSZ, &win);

		termios.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
		termios.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
				INPCK | ISTRIP | IXON | PARMRK);
		termios.c_oflag &= ~OPOST;
		termios.c_cc[VMIN] = 1;
		termios.c_cc[VTIME] = 0;

		if (tcsetattr(STDIN_FILENO, TCSANOW, &termios) < 0)
		{
			perror("tcsetattr");
			exit(1);
		}
		ioctl(*master, TIOCSWINSZ, &win);

		atexit(restoreTermios);
	}
}

// Replace each quote character (') with the sequence '\''
// (copying the result to dest, if non-null)
// and return the length of the result.
static size_t escapeQuotes(char *dest, const char *src)
{
	size_t len = 0;

	for (; *src != 0; src++)
	{
		if (*src == '\'')
		{
			if (dest)
				memcpy(&dest[len], "'\\''", 4);
			len += 4;
		}
		else
		{
			if (dest)
				dest[len] = *src;
			len++;
		}
	}
	if (dest)
		dest[len] = 0;

	return len;
}

int connectToShellspawn(void)
{
	struct sockaddr_un addr;
	int sockfd, prefixfd;

	// Connect to the shellspawn daemon in the container. sun_path holds only
	// 108 bytes, so the socket is named through a descriptor of the prefix
	// directory, which keeps the path short for a prefix of any length. This
	// runs after the permanent drop, so the prefix is opened as the user.
	prefixfd = open(prefix, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (prefixfd == -1)
	{
		fprintf(stderr, "Cannot open the prefix %s: %s\n", prefix, strerror(errno));
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (!launcherDirSocketPath(addr.sun_path, sizeof(addr.sun_path), prefixfd, SHELLSPAWN_SOCKPATH))
	{
		fprintf(stderr, "Cannot build the shellspawn socket path\n");
		exit(1);
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		fprintf(stderr, "Error creating a unix domain socket: %s\n", strerror(errno));
		exit(1);
	}

	if (connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		fprintf(stderr, "Error connecting to shellspawn in the container (%s" SHELLSPAWN_SOCKPATH "): %s\n", prefix, strerror(errno));
		exit(1);
	}
	close(prefixfd);

	return sockfd;
}

void setupShellspawnEnv(int sockfd)
{
	static const char* skip_vars[] = {
		"PATH",
		"TMPDIR",
		"HOME",
		"DARLING_PREFIX",
	};

	char buffer2[4096];

	// Push environment variables
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, 
		"PATH=/usr/bin:"
		"/bin:"
		"/usr/sbin:"
		"/sbin:"
		"/usr/local/bin");
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, "TMPDIR=/private/tmp");

	// The invoking user's own account, whatever ids this process has now.
	struct passwd* pw = getpwuid(g_originalUid);
	char loginBuffer[256] = "";
	const char* userEnv = getenv("USER");
	const char* lognameEnv = getenv("LOGNAME");
	const char* login;

	if (pw != NULL)
	{
		strncpy(loginBuffer, pw->pw_name, sizeof(loginBuffer) - 1);
		pw = NULL;
	}
	{
		struct passwd* byUser = (userEnv && userEnv[0]) ? getpwnam(userEnv) : NULL;
		bool userIsUid = byUser != NULL && byUser->pw_uid == g_originalUid;
		struct passwd* byLogname = (lognameEnv && lognameEnv[0]) ? getpwnam(lognameEnv) : NULL;
		bool lognameIsUid = byLogname != NULL && byLogname->pw_uid == g_originalUid;

		login = launcherPickLoginName(loginBuffer, userEnv, userIsUid, lognameEnv, lognameIsUid, getlogin());
	}
	if (!login)
	{
		fprintf(stderr, "Cannot determine your user name\n");
		exit(1);
	}

	snprintf(buffer2, sizeof(buffer2), "HOME=/Users/%s", login);
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, buffer2);

	// the prefix's path on the host, for darpath
	snprintf(buffer2, sizeof(buffer2), "DARLING_PREFIX=%s", prefix);
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, buffer2);

	for (char** var_ptr = environ; *var_ptr != NULL; ++var_ptr) {
		const char* var = *var_ptr;
		const char* equal = strchr(var, '=');
		size_t name_len = (equal != NULL) ? (size_t)(equal - var) : strlen(var);
		bool skip_it = false;

		for (size_t i = 0; i < sizeof(skip_vars) / sizeof(*skip_vars); ++i) {
			if (strlen(skip_vars[i]) == name_len && strncmp(var, skip_vars[i], name_len) == 0) {
				skip_it = true;
				break;
			}
		}

		if (skip_it || !launcherGuestMayGet(var)) {
			continue;
		}

		pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, var);
	}
}

void setupWorkingDir(int sockfd)
{
	char buffer2[4096];
	snprintf(buffer2, sizeof(buffer2), SYSTEM_ROOT "%s", g_workingDirectory);
	pushShellspawnCommand(sockfd, SHELLSPAWN_CHDIR, buffer2);
}

void setupIDs(int sockfd)
{
	// In the rootless mode the user is the only id of the container's user namespace, and it is 0 there.
	int ids[2] = { g_originalUid, g_originalGid };

	if (g_mode == LAUNCHER_MODE_ROOTLESS)
		ids[0] = ids[1] = 0;
	pushShellspawnCommandData(sockfd, SHELLSPAWN_SETUIDGID, ids, sizeof(ids));
}

void setupFDs(int fds[3], int* master)
{
	*master = -1;

	if (isatty(STDIN_FILENO))
		setupPtys(fds, master);
	else
		fds[0] = dup(STDIN_FILENO); // dup() because we close() after spawning
	
	if (*master == -1 || !isatty(STDOUT_FILENO))
		fds[1] = STDOUT_FILENO;
	if (*master == -1 || !isatty(STDERR_FILENO))
		fds[2] = STDERR_FILENO;
}

void spawnGo(int sockfd, int fds[3], int master)
{
	pushShellspawnCommandFDs(sockfd, SHELLSPAWN_GO, fds);
	close(fds[0]);

	shellLoop(sockfd, master);
	
	if (master != -1)
		close(master);
	close(sockfd);
}

void spawnShell(const char** argv)
{
	size_t total_len = 0;
	int count;
	int sockfd;
	char* buffer;
	int fds[3], master;

	if (argv != NULL)
	{
		for (count = 0; argv[count] != NULL; count++)
			total_len += escapeQuotes(NULL, argv[count]);

		buffer = malloc(total_len + count*3);

		char *to = buffer;
		for (int i = 0; argv[i] != NULL; i++)
		{
			if (to != buffer)
				to = stpcpy(to, " ");
			to = stpcpy(to, "'");
			to += escapeQuotes(to, argv[i]);
			to = stpcpy(to, "'");
		}
	}
	else
		buffer = NULL;

	sockfd = connectToShellspawn();

	setupShellspawnEnv(sockfd);

	// Push shell arguments
	if (buffer != NULL)
	{
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, "-c");
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, buffer);

		free(buffer);
	}

	setupWorkingDir(sockfd);
	setupIDs(sockfd);
	setupFDs(fds, &master);
	spawnGo(sockfd, fds, master);
}

void spawnBinary(const char* binary, const char** argv)
{
	int fds[3], master;
	int sockfd;

	sockfd = connectToShellspawn();
	setupShellspawnEnv(sockfd);

	pushShellspawnCommand(sockfd, SHELLSPAWN_SETEXEC, binary);

	for (; *argv != NULL; ++argv)
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, *argv);

	setupWorkingDir(sockfd);
	setupIDs(sockfd);
	setupFDs(fds, &master);
	spawnGo(sockfd, fds, master);
}

void showHelp(const char* argv0)
{
	fprintf(stderr, "This is Darling, translation layer for macOS software.\n\n");
	fprintf(stderr, "Copyright (C) 2012-2023 Lubos Dolezel\n\n");

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s <program-path> [arguments...]\n", argv0);
	fprintf(stderr, "\t%s shell [arguments...]\n", argv0);
	fprintf(stderr, "\t%s exec <program-path> [arguments...]\n", argv0);
	fprintf(stderr, "\t%s shutdown\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "Environment variables:\n"
		"DPREFIX - specifies the location of Darling prefix as an absolute path, defaults to ~/.darling\n"
		"DARLING_PROFILE - selects an isolated named prefix at ~/.darling.<name>\n");
}

void showVersion(const char* argv0) {
	fprintf(stderr, "%s " GIT_BRANCH " @ " GIT_COMMIT_HASH "\n", argv0);
	fprintf(stderr, "Copyright (C) 2012-2023 Lubos Dolezel\n");
}

void missingSetuidRoot(void)
{
	char path[4096];
	int len;

	len = readlink("/proc/self/exe", path, sizeof(path)-1);
	if (len < 0)
		strcpy(path, "darling");
	else
		path[len] = '\0';

	fprintf(stderr, "Sorry, the `%s' binary is not setuid root, which is mandatory.\n", path);
	fprintf(stderr, "Darling needs this in order to create mount and PID namespaces and to perform mounts.\n");
	fprintf(stderr, "Without it, set DARLING_ROOTLESS=1 to run in a user namespace of your own, or DARLING_NONROOT=1 to run without namespaces.\n");
}

// Rootless mode, in the child that becomes darlingserver: a user namespace in which the user is the
// only id (0, so that the child has the capabilities for mounts and a PID namespace there), and mount
// and UTS namespaces owned by it. The SysV IPC namespace stays shared, as in the root mode.
static bool writeProcFile(const char* path, const char* text)
{
	int fd = open(path, O_WRONLY | O_CLOEXEC);
	size_t length = strlen(text);
	bool ok = fd >= 0 && write(fd, text, length) == (ssize_t) length;

	if (fd >= 0)
		close(fd);
	return ok;
}

static bool enterRootlessNamespaces(void)
{
	char map[64];

	if (unshare(CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWUTS) != 0)
	{
		fprintf(stderr, "Cannot create the user namespace for rootless mode: %s\n"
			"If this host does not allow user namespaces, set DARLING_NONROOT=1 to run without namespaces.\n", strerror(errno));
		return false;
	}

	// An unprivileged process may map only its own ids, and only after giving up setgroups().
	snprintf(map, sizeof(map), "0 %u 1\n", (unsigned) g_originalUid);
	if (!writeProcFile("/proc/self/setgroups", "deny\n")
		|| !(snprintf(map, sizeof(map), "0 %u 1\n", (unsigned) g_originalGid), writeProcFile("/proc/self/gid_map", map))
		|| !(snprintf(map, sizeof(map), "0 %u 1\n", (unsigned) g_originalUid), writeProcFile("/proc/self/uid_map", map)))
	{
		fprintf(stderr, "Cannot map your user into the rootless user namespace: %s\n", strerror(errno));
		return false;
	}
	return true;
}

pid_t spawnInitProcess(void)
{
	pid_t pid;
	int pipefd[2];
	char buffer[1];

	if (pipe(pipefd) == -1)
	{
		fprintf(stderr, "Cannot create a pipe for synchronization: %s\n", strerror(errno));
		exit(1);
	}

	// The SysV IPC namespace stays shared with the host: native X11 clients in
	// the container (Mesa among them) pass SysV shared memory segment IDs to the
	// X server through MIT-SHM, which the server can only attach in its own
	// namespace.
	// Only root can do this here; the unprivileged modes give the child its own namespaces below.
	if (privilegedMode() && unshare(CLONE_NEWUTS) != 0)
	{
		fprintf(stderr, "Cannot unshare the UTS namespace to create darling-init: %s\n", strerror(errno));
		exit(1);
	}

	pid = fork();

	if (pid < 0)
	{
		fprintf(stderr, "Cannot fork() to create darling-init: %s\n", strerror(errno));
		exit(1);
	}

	if (pid == 0)
	{
		// The child

		char uid_str[21];
		char gid_str[21];
		char pipefd_str[21];

		snprintf(uid_str, sizeof(uid_str), "%d", g_originalUid);
		snprintf(gid_str, sizeof(gid_str), "%d", g_originalGid);
		snprintf(pipefd_str, sizeof(pipefd_str), "%d", pipefd[1]);

		close(pipefd[0]);

		if (g_mode == LAUNCHER_MODE_ROOTLESS && !enterRootlessNamespaces())
			exit(1);

		// An unprivileged launcher may run from anywhere; darlingserver is told where the system image is.
		// (A privileged launcher never passes this: darlingserver ignores it for root.)
		if (!privilegedMode() && strcmp(g_libexecPath, INSTALL_PREFIX "/libexec/darling") != 0)
			setenv("DARLING_LIBEXEC_PATH", g_libexecPath, 1);

		// This exec runs darlingserver as full root. When another user started
		// us setuid, the kernel marked our own exec secure (AT_SECURE) and the C
		// library ignored the dangerous loader variables, but darlingserver's
		// exec is not marked secure. So it gets only the variables it needs, and
		// a fixed PATH, so that the helpers it runs as root, such as
		// fuse-overlayfs, never come from the user's directories.
		char* const serverArgv[] = { "darlingserver", prefix, uid_str, gid_str, pipefd_str, g_fixPermissions ? "1" : "0", NULL };
		char** serverEnv = launcherServerEnvironment(environ, getauxval(AT_SECURE) != 0);
		if (!serverEnv)
		{
			fprintf(stderr, "Cannot set up the environment for darlingserver\n");
			exit(1);
		}

		execve(g_serverPath, serverArgv, serverEnv);

		fprintf(stderr, "Failed to start darlingserver\n");
		exit(1);
	}

	// Wait for the child to drop UID/GIDs and unshare stuff
	close(pipefd[1]);
	ssize_t got = read(pipefd[0], buffer, 1);
	close(pipefd[0]);

	if (got != 1)
	{
		// The server exits with EX_UNAVAILABLE if it cannot assemble the prefix in the selected filesystem mode
		int status;
		if (waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == EX_UNAVAILABLE && chooseFsMode())
			return spawnInitProcess();
	}

	/*
	snprintf(idmap, sizeof(idmap), "/proc/%d/uid_map", pid);

	file = fopen(idmap, "w");
	if (file != NULL)
	{
		fprintf(file, "0 %d 1\n", g_originalUid); // all users map to our user on the outside
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Cannot set uid_map for the init process: %s\n", strerror(errno));
	}

	snprintf(idmap, sizeof(idmap), "/proc/%d/gid_map", pid);

	file = fopen(idmap, "w");
	if (file != NULL)
	{
		fprintf(file, "0 %d 1\n", g_originalGid); // all groups map to our group on the outside
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Cannot set gid_map for the init process: %s\n", strerror(errno));
	}
	*/

	// Here's where we resume the child
	// if we enable user namespaces

	return pid;
}

#define FSMODE_FILE "/.darling-fsmode"

static bool commandInPath(const char* name)
{
	const char* path = getenv("PATH");
	char* copy;
	char* dir;
	char* saveptr;
	bool found = false;

	if (!path)
		return false;

	copy = strdup(path);
	if (!copy)
		return false;

	// The directories come from the user's PATH; look into them as the user.
	useInvokingIds();
	for (dir = strtok_r(copy, ":", &saveptr); dir != NULL && !found; dir = strtok_r(NULL, ":", &saveptr))
	{
		char candidate[4096];
		snprintf(candidate, sizeof(candidate), "%s/%s", dir, name);
		found = faccessat(AT_FDCWD, candidate, X_OK, AT_EACCESS) == 0;
	}
	restoreRootIds();
	free(copy);

	return found;
}

// Records the filesystem mode in the prefix, where darlingserver looks for it
static bool recordFsMode(const char* mode)
{
	char* path = (char*) alloca(strlen(prefix) + sizeof(FSMODE_FILE));
	FILE* fp;

	strcpy(path, prefix);
	strcat(path, FSMODE_FILE);

	useInvokingIds();
	fp = fopen(path, "w");
	restoreRootIds();

	if (fp == NULL)
	{
		fprintf(stderr, "Cannot record the filesystem mode in %s: %s\n", path, strerror(errno));
		return false;
	}
	fprintf(fp, "%s\n", mode);
	fclose(fp);
	return true;
}

int chooseFsMode(void)
{
	static const char* const modes[] = { "overlayfs", "fuse-overlayfs", "copy" };
	char answer[16];
	int choice;

	fprintf(stderr, "\nDarling could not set up the prefix %s with the current filesystem mode.\n", prefix);
	fprintf(stderr, "The kernel's overlayfs cannot be used if the prefix is on a network or similar filesystem, such as NFS.\n");

	if (!isatty(STDIN_FILENO))
	{
		fprintf(stderr, "To select a different mode, write one of 'overlayfs', 'fuse-overlayfs' or 'copy' into %s%s\n", prefix, FSMODE_FILE);
		exit(1);
	}

	fprintf(stderr, "\nChoose how the prefix should be set up (the choice is recorded in %s%s):\n", prefix, FSMODE_FILE);
	fprintf(stderr, "  1) overlayfs       kernel overlay filesystem; the prefix has to be on a local filesystem\n");
	fprintf(stderr, "  2) fuse-overlayfs  userspace overlay filesystem; works on more filesystems, but needs the fuse-overlayfs tool%s\n",
		commandInPath("fuse-overlayfs") ? "" : " (NOT FOUND in PATH)");
	fprintf(stderr, "  3) copy            copy the system into the prefix; works anywhere, but uses more disk space\n");
	fprintf(stderr, "Choice [1-3]: ");

	if (fgets(answer, sizeof(answer), stdin) == NULL)
		exit(1);

	choice = atoi(answer);
	if (choice < 1 || choice > 3)
	{
		fprintf(stderr, "No valid choice made.\n");
		exit(1);
	}

	if (!recordFsMode(modes[choice - 1]))
		exit(1);
	return 1;
}

void putInitPid(pid_t pidInit)
{
	const char pidFile[] = "/.init.pid";
	char* pidPath;
	FILE *fp;

	pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
	strcpy(pidPath, prefix);
	strcat(pidPath, pidFile);

	useInvokingIds();
	fp = fopen(pidPath, "w");
	restoreRootIds();

	if (fp == NULL)
	{
		fprintf(stderr, "Cannot write out PID of the init process: %s\n", strerror(errno));
		return;
	}
	fprintf(fp, "%d", (int) pidInit);
	fclose(fp);
}

char* defaultPrefixPath(void)
{
	const char defaultPath[] = "/.darling";
	const char* home = getenv("HOME");
	char* buf;

	if (!home)
	{
		fprintf(stderr, "Cannot detect your home directory!\n");
		return NULL;
	}

	buf = (char*) malloc(strlen(home) + sizeof(defaultPath));
	strcpy(buf, home);
	strcat(buf, defaultPath);

	return buf;
}

void createDir(const char* path)
{
	struct stat st;

	if (stat(path, &st) == 0)
	{
		if (!S_ISDIR(st.st_mode))
		{
			fprintf(stderr, "%s already exists and is a file. Remove the file.\n", path);
			exit(1);
		}
	}
	else
	{
		if (errno == ENOENT)
		{
			if (mkdir(path, 0755) != 0)
			{
				fprintf(stderr, "Cannot create %s: %s\n", path, strerror(errno));
				exit(1);
			}
		}
		else
		{
			fprintf(stderr, "Cannot access %s: %s\n", path, strerror(errno));
			exit(1);
		}
	}
}

void setupWorkdir()
{
	char* workdir;
	const char suffix[] = ".workdir";
	size_t len;

	len = strlen(prefix);
	workdir = (char*) alloca(len + sizeof(suffix));
	strcpy(workdir, prefix);

	// Remove trailing /
	while (workdir[len-1] == '/')
		len--;
	workdir[len] = '\0';

	strcat(workdir, suffix);

	// The work directory goes next to the prefix, into a directory the user
	// names. Created as root, a prefix path changed under us could make root
	// create a directory anywhere. darlingserver uses it as root, which works
	// whoever owns it.
	useInvokingIds();
	createDir(workdir);
	restoreRootIds();
}

int checkPrefixDir()
{
	struct stat st;
	int result, error;

	// As the user: as root this would also tell the user about paths they
	// cannot see.
	useInvokingIds();
	result = stat(prefix, &st);
	error = errno;
	restoreRootIds();

	if (result == 0)
	{
		if (!S_ISDIR(st.st_mode))
		{
			fprintf(stderr, "%s is a file. Remove the file.\n", prefix);
			exit(1);
		}
		// Inspect the user-controlled tree with the invoking user's credentials.
		useInvokingIds();
		enum darling_prefix_state state = darlingClassifyPrefix(prefix);
		int classifyError = errno;
		restoreRootIds();
		if (state == DARLING_PREFIX_STATE_INSPECTION_ERROR)
		{
			fprintf(stderr, "Cannot inspect prefix %s: %s.\n", prefix, strerror(classifyError));
			exit(1);
		}
		if (state == DARLING_PREFIX_STATE_UNINITIALIZED)
		{
			fprintf(stderr,
				"Prefix %s is non-empty but is not an initialized Darling prefix; refusing to overwrite it.\n"
				"Use an absent/empty directory for a new prefix, or point DPREFIX at an initialized prefix.\n",
				prefix);
			exit(1);
		}
		return state == DARLING_PREFIX_STATE_INITIALIZED;
	}
	if (error == ENOENT)
		return 0; // not found
	fprintf(stderr, "Cannot access %s: %s\n", prefix, strerror(error));
	exit(1);
}

void setupPrefix()
{
	char path[4096];
	size_t plen;
	FILE* file;
	struct passwd* passwd_entry;
	
	const char* dirs[] = {
		"/Volumes",
		"/Applications",
		"/usr",
		"/usr/local",
		"/usr/local/share",
		"/private",
		"/private/var",
		"/private/var/log",
		"/private/var/db",
		"/private/etc",
		"/var",
		"/var/run",
		"/var/tmp",
		"/var/log"
	};

	fprintf(stderr, "Setting up a new Darling prefix at %s\n", prefix);

	// The prefix and everything in it is created by and for the user.
	useInvokingIds();

	// The new prefix is private to the user whatever the umask: it holds the
	// darlingserver socket and the container's files.
	createDir(prefix);
	if (chmod(prefix, 0700) != 0)
	{
		fprintf(stderr, "Cannot set the permissions of %s: %s\n", prefix, strerror(errno));
		exit(1);
	}
	strcpy(path, prefix);
	strcat(path, "/");
	plen = strlen(path);

	for (size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++)
	{
		path[plen] = '\0';
		strcat(path, dirs[i]);
		createDir(path);
	}

	// create passwd, master.passwd, and group

	passwd_entry = getpwuid(g_originalUid);
	if (!passwd_entry) {
		fprintf(stderr, "Failed to find Linux /etc/passwd entry for current user\n");
		exit(1);
	}

	path[plen] = '\0';
	strcat(path, "/private/etc/passwd");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/passwd within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"root:*:0:0:System Administrator:/var/root:/bin/sh\n"
		"%s:*:%d:%d:Darling User:/Users/%s:/bin/zsh\n",
		passwd_entry->pw_name,
		passwd_entry->pw_uid,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);

	path[plen] = '\0';
	strcat(path, "/private/etc/master.passwd");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/master.passwd within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"root:*:0:0::0:0:System Administrator:/var/root:/bin/sh\n"
		"%s:*:%d:%d::0:0:Darling User:/Users/%s:/bin/zsh\n",
		passwd_entry->pw_name,
		passwd_entry->pw_uid,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);

	path[plen] = '\0';
	strcat(path, "/private/etc/group");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/group within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"wheel:*:0:root,%s\n"
		"%s:*:%d:%s\n",
		passwd_entry->pw_name,
		passwd_entry->pw_name,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);

	restoreRootIds();
}

// macOS ships these service accounts in its local user database. Launchd jobs
// we install name them in their UserName key, and launchd refuses to start a
// job whose user it cannot look up.
static const struct
{
	const char* name;
	unsigned int id;
	const char* gecos;
} systemAccounts[] = {
	{ "_trustd", 282, "trustd" },
};

enum accountFile
{
	ACCOUNT_FILE_PASSWD,
	ACCOUNT_FILE_MASTER_PASSWD,
	ACCOUNT_FILE_GROUP,
};

static void addMissingAccounts(enum accountFile type, const char* name)
{
	char path[4096];
	struct stat st;
	FILE* file;
	char* line = NULL;
	size_t lineCapacity = 0;
	bool haveName[sizeof(systemAccounts) / sizeof(systemAccounts[0])] = { false };
	bool haveId[sizeof(systemAccounts) / sizeof(systemAccounts[0])] = { false };
	bool endsWithNewline = true;

	snprintf(path, sizeof(path), "%s/private/etc/%s", prefix, name);

	// Older prefixes may link these files to the host's own database, which
	// must not be modified.
	if (lstat(path, &st) != 0 || !S_ISREG(st.st_mode))
		return;

	file = fopen(path, "r+");
	if (!file)
		return;

	while (getline(&line, &lineCapacity, file) != -1)
	{
		size_t len = strlen(line);
		char* fields[3] = { line, NULL, NULL };
		char* p = line;

		endsWithNewline = len > 0 && line[len - 1] == '\n';

		if (line[0] == '#')
			continue;

		for (int i = 1; i < 3 && p; i++)
		{
			p = strchr(p, ':');
			if (p)
			{
				*p++ = '\0';
				fields[i] = p;
			}
		}

		for (size_t i = 0; i < sizeof(systemAccounts) / sizeof(systemAccounts[0]); i++)
		{
			if (strcmp(fields[0], systemAccounts[i].name) == 0)
				haveName[i] = true;
			else if (fields[2] && strtoul(fields[2], NULL, 10) == systemAccounts[i].id)
				haveId[i] = true;
		}
	}
	free(line);

	fseek(file, 0, SEEK_END);
	if (!endsWithNewline)
		fputc('\n', file);

	for (size_t i = 0; i < sizeof(systemAccounts) / sizeof(systemAccounts[0]); i++)
	{
		if (haveName[i])
			continue;
		if (haveId[i])
		{
			fprintf(stderr, "Cannot add account %s to the prefix's /etc/%s: ID %u is already in use\n",
				systemAccounts[i].name, name, systemAccounts[i].id);
			continue;
		}

		switch (type)
		{
			case ACCOUNT_FILE_PASSWD:
				fprintf(file, "%s:*:%u:%u:%s:/var/empty:/usr/bin/false\n",
					systemAccounts[i].name, systemAccounts[i].id, systemAccounts[i].id, systemAccounts[i].gecos);
				break;
			case ACCOUNT_FILE_MASTER_PASSWD:
				fprintf(file, "%s:*:%u:%u::0:0:%s:/var/empty:/usr/bin/false\n",
					systemAccounts[i].name, systemAccounts[i].id, systemAccounts[i].id, systemAccounts[i].gecos);
				break;
			case ACCOUNT_FILE_GROUP:
				fprintf(file, "%s:*:%u:\n", systemAccounts[i].name, systemAccounts[i].id);
				break;
		}
	}

	fclose(file);
}

void ensureSystemAccounts()
{
	// The account files belong to the user who owns the prefix.
	useInvokingIds();

	addMissingAccounts(ACCOUNT_FILE_PASSWD, "passwd");
	addMissingAccounts(ACCOUNT_FILE_MASTER_PASSWD, "master.passwd");
	addMissingAccounts(ACCOUNT_FILE_GROUP, "group");

	restoreRootIds();
}

static pid_t findInitProcess(void)
{
	const char pidFile[] = "/.init.pid";
	char* pidPath;
	pid_t pid;
	int pid_i;
	FILE *fp;
	char procBuf[100];
	char *exeBuf, *statusBuf;
	int uidMatch = 0, gidMatch = 0;

	pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
	strcpy(pidPath, prefix);
	strcat(pidPath, pidFile);

	fp = fopen(pidPath, "r");
	if (fp == NULL)
		return 0;

	if (fscanf(fp, "%d", &pid_i) != 1)
	{
		fclose(fp);
		unlink(pidPath);
		return 0;
	}
	fclose(fp);
	pid = (pid_t) pid_i;

	// Does the process exist?
	if (kill(pid, 0) == -1)
	{
		unlink(pidPath);
		return 0;
	}

	// Is it actually an init process?
	snprintf(procBuf, sizeof(procBuf), "/proc/%d/comm", pid);
	fp = fopen(procBuf, "r");
	if (fp == NULL)
	{
		unlink(pidPath);
		return 0;
	}

	if (fscanf(fp, "%ms", &exeBuf) != 1)
	{
		fclose(fp);
		unlink(pidPath);
		return 0;
	}
	fclose(fp);

	if (strcmp(exeBuf, "darlingserver") != 0)
	{
		unlink(pidPath);
		return 0;
	}
	free(exeBuf);

	// Is it owned by the current user?
	if (g_originalUid != 0)
	{
		snprintf(procBuf, sizeof(procBuf), "/proc/%d/status", pid);
		fp = fopen(procBuf, "r");
		if (fp == NULL)
		{
			unlink(pidPath);
			return 0;
		}

		while (1)
		{
			statusBuf = NULL;
			size_t len;
			if (getline(&statusBuf, &len, fp) == -1)
				break;
			int rid, eid, sid, fid;
			if (sscanf(statusBuf, "Uid: %d %d %d %d", &rid, &eid, &sid, &fid) == 4)
			{
				uidMatch = (rid == g_originalUid && eid == g_originalUid && (sid == g_originalUid || sid == 0));
			}
			if (sscanf(statusBuf, "Gid: %d %d %d %d", &rid, &eid, &sid, &fid) == 4)
			{
				gidMatch = (rid == g_originalGid && eid == g_originalGid && (sid == g_originalGid || sid == 0));
			}
			free(statusBuf);
		}
		fclose(fp);

		if (!uidMatch || !gidMatch)
		{
			unlink(pidPath);
			return 0;
		}
	}

	return pid;
}

pid_t getInitProcess()
{
	pid_t pid;

	// The pid file is in the user's prefix, and a stale one is removed; both
	// as the user. The /proc files read to confirm the pid are readable by the
	// user as well.
	useInvokingIds();
	pid = findInitProcess();
	restoreRootIds();

	return pid;
}

// Whether the prefix's group holds nobody but the user (see launcher-prefix.h).
static bool prefixGroupIsUserPrivate(gid_t gid)
{
	struct passwd* pw;
	struct group* gr;
	bool result = false;

	// The account databases are the host's; look them up as the user.
	useInvokingIds();
	pw = getpwuid(g_originalUid);
	if (pw != NULL)
	{
		char* name = strdup(pw->pw_name);
		gr = getgrgid(gid);
		if (name != NULL && gr != NULL)
			result = launcherGroupIsUserPrivate(gr->gr_gid, gr->gr_name, gr->gr_mem, g_originalGid, name);
		free(name);
	}
	restoreRootIds();

	return result;
}

void checkPrefixOwner()
{
	struct stat st;
	int result, error;

	useInvokingIds();
	result = stat(prefix, &st);
	error = errno;
	restoreRootIds();

	if (result == 0)
	{
		if (g_originalUid != 0 && st.st_uid != g_originalUid)
		{
			fprintf(stderr, "You do not own the prefix directory.\n");
			exit(1);
		}
		if (!launcherPrefixModeAcceptable(st.st_mode, (st.st_mode & S_IWGRP) && prefixGroupIsUserPrivate(st.st_gid)))
		{
			fprintf(stderr, "The prefix directory %s is writable by other users. To use it, run: chmod go-w '%s'\n", prefix, prefix);
			exit(1);
		}
	}
	else if (error == EACCES)
	{
		fprintf(stderr, "You do not own the prefix directory.\n");
		exit(1);
	}
}
