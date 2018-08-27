/*
This file is part of Darling.

Copyright (C) 2016-2017 Lubos Dolezel

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
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>
#include <termios.h>
#include <pty.h>
#include "../shellspawn/shellspawn.h"
#include "darling.h"
#include "darling-config.h"

// Between Linux 4.9 and 4.11, a strange bug has been introduced
// which prevents connecting to Unix sockets if the socket was
// created in a different mount namespace or under overlayfs
// (dunno which one is really responsible for this).
#define USE_LINUX_4_11_HACK 1

const char* DARLING_INIT_COMM = "darling-init";
char *prefix;
uid_t g_originalUid, g_originalGid;
bool g_fixPermissions = false;
char **g_argv, **g_envp;
char g_workingDirectory[4096];

int main(int argc, char ** argv, char ** envp)
{
	pid_t pidInit;
	int wstatus;

	if (argc <= 1)
	{
		showHelp(argv[0]);
		return 1;
	}

	g_argv = argv;
	g_envp = envp;

	if (geteuid() != 0)
	{
		missingSetuidRoot();
		return 1;
	}

	g_originalUid = getuid();
	g_originalGid = getgid();

	setuid(0);
	setgid(0);

	if (!isModuleLoaded())
		loadKernelModule();

	prefix = getenv("DPREFIX");
	if (!prefix)
		prefix = defaultPrefixPath();
	if (!prefix)
		return 1;
	if (strlen(prefix) > 255)
	{
		fprintf(stderr, "Prefix path too long\n");
		return 1;
	}
	unsetenv("DPREFIX");
	getcwd(g_workingDirectory, sizeof(g_workingDirectory));

	if (!checkPrefixDir())
	{
		setupPrefix();
		g_fixPermissions = true;
	}
	checkPrefixOwner();

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
			fprintf(stderr, "Darling container is not running\n");
			return 1;
		}

		// TODO: when we have a working launchd,
		// this is where we ask it to shut down nicely

		kill(pidInit, SIGKILL);
		return 0;
	}

start_init:
	// If prefix's init is not running, start it up
	if (pidInit == 0)
	{
		char socketPath[4096];
		
		snprintf(socketPath, sizeof(socketPath), "%s"  SHELLSPAWN_SOCKPATH, prefix);
		
		unlink(socketPath);
		
		setupWorkdir();
		pidInit = spawnInitProcess();
		putInitPid(pidInit);
		
		// Wait until shellspawn starts
		for (int i = 0; i < 15; i++)
		{
			if (access(socketPath, F_OK) == 0)
				break;
			sleep(1);
		}
	}

#if USE_LINUX_4_11_HACK
	joinNamespace(pidInit, CLONE_NEWNS, "mnt");
#endif

	seteuid(g_originalUid);

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
		char *fullPath;
		char** child_argv;
		char *path = realpath(argv[1], NULL);

		if (path == NULL)
		{
			printf("'%s' is not a supported command or a file.\n", argv[1]);
			return 1;
		}

		fullPath = malloc(strlen(SYSTEM_ROOT) + strlen(path) + 1);
		strcpy(fullPath, SYSTEM_ROOT);
		strcat(fullPath, path);
		free(path);

		argv[1] = fullPath;
		spawnShell((const char**) &argv[1]);
	}

	return 0;
}

void joinNamespace(pid_t pid, int type, const char* typeName)
{
	int fdNS;
	char pathNS[4096];
	
	snprintf(pathNS, sizeof(pathNS), "/proc/%d/ns/%s", pid, typeName);

	fdNS = open(pathNS, O_RDONLY);

	if (fdNS < 0)
	{
		fprintf(stderr, "Cannot open %s namespace file: %s\n", typeName, strerror(errno));
		exit(1);
	}

	// Calling setns() with a PID namespace doesn't move our process into it,
	// but our child process will be spawned inside the namespace
	if (setns(fdNS, type) != 0)
	{
		fprintf(stderr, "Cannot join %s namespace: %s\n", typeName, strerror(errno));
		exit(1);
	}
	close(fdNS);
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

static int _shSockfd = -1;
static struct termios orig_termios;
static int pty_master;
static void signalHandler(int signo)
{
	// printf("Received signal %d\n", signo);

	// Forward window size changes
	if (signo == SIGWINCH && pty_master != -1)
	{
		struct winsize win;

		ioctl(0, TIOCGWINSZ, &win);
		ioctl(pty_master, TIOCSWINSZ, &win);
	}
	
	// Foreground process loopkup in shellspawn doesn't work
	// if we're not running in TTY mode, so shellspawn falls back
	// to forwarding signals to the Bash subprocess.
	// 
	// Hence we translate SIGINT to SIGTERM for user convenience,
	// because Bash will not terminate on SIGINT.
	if (pty_master == -1 && signo == SIGINT)
		signo = SIGTERM;

	pushShellspawnCommandData(_shSockfd, SHELLSPAWN_SIGNAL, &signo, sizeof(signo));
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
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
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
			int rd;
			do
			{
				rd = read(STDIN_FILENO, buf, sizeof(buf));
				if (rd > 0)
					write(master, buf, rd);
				else
					exit(1);
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

void spawnShell(const char** argv)
{
	size_t total_len = 0;
	int count;
	char buffer1[4096];
	char buffer2[4096];
	int sockfd;
	struct sockaddr_un addr;
	char* buffer;

	if (argv != NULL)
	{
		for (count = 0; argv[count] != NULL; count++)
			total_len += strlen(argv[count]);

		buffer = malloc(total_len + count*3);

		char *to = buffer;
		for (int i = 0; argv[i] != NULL; i++)
		{
			if (to != buffer)
				to = stpcpy(to, " ");
			to = stpcpy(to, "'");
			to = stpcpy(to, argv[i]);
			to = stpcpy(to, "'");
		}
	}
	else
		buffer = NULL;

	// Connect to the shellspawn daemon in the container
	addr.sun_family = AF_UNIX;
#if USE_LINUX_4_11_HACK
	strcpy(addr.sun_path, SHELLSPAWN_SOCKPATH);
#else
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s"  SHELLSPAWN_SOCKPATH, prefix);
#endif

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		fprintf(stderr, "Error creating a unix domain socket: %s\n", strerror(errno));
		exit(1);
	}

	if (connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		fprintf(stderr, "Error connecting to shellspawn in the container (%s): %s\n", addr.sun_path, strerror(errno));
		exit(1);
	}

	// Push environment variables
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, 
		"PATH=/usr/bin:"
		"/bin:"
		"/usr/sbin:"
		"/sbin:"
		"/usr/local/bin");

	const char *home = getenv("HOME");
	if (home)
	{
		if (sscanf(home, "/home/%4096s", buffer1) == 1)
			snprintf(buffer2, sizeof(buffer2), "HOME=/Users/%s", buffer1);
		else
			snprintf(buffer2, sizeof(buffer2), "HOME=" SYSTEM_ROOT "%s", home);
		pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, buffer2);
	}

	// Push shell arguments
	if (buffer != NULL)
	{
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, "-c");
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, buffer);

		free(buffer);
	}

	snprintf(buffer2, sizeof(buffer2), SYSTEM_ROOT "%s", g_workingDirectory);
	pushShellspawnCommand(sockfd, SHELLSPAWN_CHDIR, buffer2);

	int ids[2] = { g_originalUid, g_originalGid };
	pushShellspawnCommandData(sockfd, SHELLSPAWN_SETUIDGID, ids, sizeof(ids));

	int fds[3], master = -1;
	
	if (isatty(STDIN_FILENO))
		setupPtys(fds, &master);
	else
		fds[0] = dup(STDIN_FILENO); // dup() because we close() a few lines below
	
	if (master == -1 || !isatty(STDOUT_FILENO))
		fds[1] = STDOUT_FILENO;
	if (master == -1 || !isatty(STDERR_FILENO))
		fds[2] = STDERR_FILENO;
	
	pushShellspawnCommandFDs(sockfd, SHELLSPAWN_GO, fds);
	close(fds[0]);

	shellLoop(sockfd, master);
	
	if (master != -1)
		close(master);
	close(sockfd);
}

void showHelp(const char* argv0)
{
	fprintf(stderr, "This is Darling, translation layer for macOS software.\n\n");
	fprintf(stderr, "Copyright (C) 2012-2017 Lubos Dolezel\n\n");

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s program-path [arguments...]\n", argv0);
	fprintf(stderr, "\t%s shell [arguments...]\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "Environment variables:\n"
		"DPREFIX - specifies the location of Darling prefix, defaults to ~/.darling\n");
}

void showVersion(const char* argv0) {
	fprintf(stderr, "%s " GIT_BRANCH " @ " GIT_COMMIT_HASH "\n", argv0);
	fprintf(stderr, "Copyright (C) 2012-2017 Lubos Dolezel\n");
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
}

void fixDirectoryPermissions(const char* path)
{
	DIR* dir;
	struct dirent* ent;

	dir = opendir(path);
	if (!dir)
		return;

	while ((ent = readdir(dir)) != NULL)
	{
		if (ent->d_type == DT_DIR)
		{
			char* subdir;

			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
				continue;

			subdir = (char*) malloc(strlen(path) + 2 + strlen(ent->d_name));
			sprintf(subdir, "%s/%s", path, ent->d_name);

			fixDirectoryPermissions(subdir);

			if (chown(subdir, g_originalUid, g_originalGid) == -1)
				fprintf(stderr, "Cannot chown %s: %s\n", subdir, strerror(errno));

			free(subdir);
		}
	}

	closedir(dir);
}

pid_t spawnInitProcess(void)
{
	pid_t pid;
	int pipefd[2];
	char path[1024];
	char buffer[1];
	FILE *file;

	if (pipe(pipefd) == -1)
	{
		fprintf(stderr, "Cannot create a pipe for synchronization: %s\n", strerror(errno));
		exit(1);
	}

	if (unshare(CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWIPC) != 0)
	{
		fprintf(stderr, "Cannot unshare PID, UTS and IPC namespaces to create darling-init: %s\n", strerror(errno));
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

		char *opts;
		char putOld[4096];
		char *p;

		close(pipefd[0]);

		// Since overlay cannot be mounted inside user namespaces, we have to setup a new mount namespace
		// and do the mount while we can be root
		if (unshare(CLONE_NEWNS) != 0)
		{
			fprintf(stderr, "Cannot unshare mount namespace: %s\n", strerror(errno));
			exit(1);
		}

		// Because systemd marks / as MS_SHARED and we would inherit this into the overlay mount,
		// causing it not to be unmounted once the init process dies.
		if (mount(NULL, "/", NULL, MS_REC | MS_SLAVE, NULL) != 0)
		{
			fprintf(stderr, "Cannot remount / as slave: %s\n", strerror(errno));
			exit(1);
		}

		umount("/dev/shm");
		if (mount("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) != 0)
		{
			fprintf(stderr, "Cannot mount new /dev/shm: %s\n", strerror(errno));
			exit(1);
		}

		opts = (char*) malloc(strlen(prefix)*2 + sizeof(LIBEXEC_PATH) + 100);
		sprintf(opts, "lowerdir=%s,upperdir=%s,workdir=%s.workdir", LIBEXEC_PATH, prefix, prefix);

		// Mount overlay onto our prefix
		if (mount("overlay", prefix, "overlay", 0, opts) != 0)
		{
			fprintf(stderr, "Cannot mount overlay: %s\n", strerror(errno));
			exit(1);
		}

		free(opts);

		// This is executed once at prefix creation
		if (g_fixPermissions)
			fixDirectoryPermissions(prefix);

		snprintf(putOld, sizeof(putOld), "%s" SYSTEM_ROOT, prefix);

		if (syscall(SYS_pivot_root, prefix, putOld) != 0)
		{
			fprintf(stderr, "Cannot pivot_root: %s\n", strerror(errno));
			exit(1);
		}

		// mount procfs for our new PID namespace
		if (mount("proc", "/proc", "proc", 0, "") != 0)
		{
			fprintf(stderr, "Cannot mount procfs: %s\n", strerror(errno));
			exit(1);
		}

		// Drop the privileges. It's important to drop GID first, because
		// non-root users can't change their GID.
		setresgid(g_originalGid, g_originalGid, g_originalGid);
		setresuid(g_originalUid, g_originalUid, g_originalUid);
		prctl(PR_SET_DUMPABLE, 1, 0, 0, 0);

		setupCoredumpPattern();

		// Set name to darling-init
		prctl(PR_SET_NAME, DARLING_INIT_COMM, 0, 0);
		p = stpcpy(g_argv[0], DARLING_INIT_COMM);
		memset(p, 0, g_envp[0] - p);

		/*
		if (unshare(CLONE_NEWUSER) != 0)
		{
			fprintf(stderr, "Cannot unshare user namespace: %s\n", strerror(errno));
			exit(1);
		}
		*/

		// Tell the parent we're ready
		write(pipefd[1], buffer, 1);
		close(pipefd[1]);

		// Here's where we wait for the parent to set up UID/GID mapping
		// if we enable user namespaces

		darlingPreInit();
		spawnLaunchd();

		// Never returns
	}

	// Wait for the child to drop UID/GIDs and unshare stuff
	close(pipefd[1]);
	read(pipefd[0], buffer, 1);
	close(pipefd[0]);

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

void putInitPid(pid_t pidInit)
{
	const char pidFile[] = "/.init.pid";
	char* pidPath;
	FILE *fp;

	pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
	strcpy(pidPath, prefix);
	strcat(pidPath, pidFile);

	seteuid(g_originalUid);
	setegid(g_originalGid);

	fp = fopen(pidPath, "w");

	seteuid(0);
	setegid(0);

	if (fp == NULL)
	{
		fprintf(stderr, "Cannot write out PID of the init process: %s\n", strerror(errno));
		return;
	}
	fprintf(fp, "%d", (int) pidInit);
	fclose(fp);
}

void spawnLaunchd(void)
{
	puts("Bootstrapping the container with launchd...");
	
	// putenv("KQUEUE_DEBUG=1");
	execl("/sbin/launchd", "launchd", NULL);

	fprintf(stderr, "Failed to exec launchd: %s\n", strerror(errno));
	abort();
}

static void wipeDir(const char* dirpath)
{
	char path[4096];
	struct dirent* ent;
	DIR* dir = opendir(dirpath);

	if (!dir)
		return;

	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		snprintf(path, sizeof(path), "%s/%s", dirpath, ent->d_name);

		if (ent->d_type == DT_DIR)
		{
			wipeDir(path);
			rmdir(path);
		}
		else
			unlink(path);
	}

	closedir(dir);
}

void darlingPreInit(void)
{
	// TODO: Run /usr/libexec/makewhatis
	const char* dirs[] = {
		"/var/tmp",
		"/var/run"
	};

	for (size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++)
		wipeDir(dirs[i]);
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

	createDir(workdir);
}

int checkPrefixDir()
{
	struct stat st;

	if (stat(prefix, &st) == 0)
	{
		if (!S_ISDIR(st.st_mode))
		{
			fprintf(stderr, "%s is a file. Remove the file.\n", prefix);
			exit(1);
		}
		return 1; // OK
	}
	if (errno == ENOENT)
		return 0; // not found
	fprintf(stderr, "Cannot access %s: %s\n", prefix, strerror(errno));
	exit(1);
}

void setupPrefix()
{
	char path[4096];
	size_t plen;
	
	const char* dirs[] = {
		"/Volumes",
		"/Applications",
		"/usr",
		"/usr/local",
		"/usr/local/share",
		"/private",
		"/private/var",
		"/private/var/db",
		"/var",
		"/var/run",
		"/var/tmp",
		"/var/log"
	};

	fprintf(stderr, "Setting up a new Darling prefix at %s\n", prefix);

	seteuid(g_originalUid);
	setegid(g_originalGid);

	createDir(prefix);
	strcpy(path, prefix);
	strcat(path, "/");
	plen = strlen(path);

	for (size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++)
	{
		path[plen] = '\0';
		strcat(path, dirs[i]);
		createDir(path);
	}
	
	seteuid(0);
	setegid(0);
}

pid_t getInitProcess()
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

	if (strcmp(exeBuf, "launchd") != 0)
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
				uidMatch = 1;
				uidMatch &= rid == g_originalUid;
				uidMatch &= eid == g_originalUid;
				uidMatch &= sid == g_originalUid;
				uidMatch &= fid == g_originalUid;
			}
			if (sscanf(statusBuf, "Gid: %d %d %d %d", &rid, &eid, &sid, &fid) == 4)
			{
				gidMatch = 1;
				gidMatch &= rid == g_originalGid;
				gidMatch &= eid == g_originalGid;
				gidMatch &= sid == g_originalGid;
				gidMatch &= fid == g_originalGid;
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

void checkPrefixOwner()
{
	struct stat st;

	if (stat(prefix, &st) == 0)
	{
		if (g_originalUid != 0 && st.st_uid != g_originalUid)
		{
			fprintf(stderr, "You do not own the prefix directory.\n");
			exit(1);
		}
	}
	else if (errno == EACCES)
	{
		fprintf(stderr, "You do not own the prefix directory.\n");
		exit(1);
	}
}

int isModuleLoaded()
{
	size_t len = 0;
	ssize_t read = 0;
	char * line = NULL;
	FILE *fp = NULL;

	if ((fp = fopen("/proc/modules", "r")) == NULL)
	{
		fprintf(stderr, "Failure opening /proc/modules: %s\n", strerror(errno));
		fclose(fp);
		return 0;
	}

	while (!feof(fp))
	{
		read = getline(&line, &len, fp);
		if (read > 0 && strstr(line, "darling_mach") != NULL)
		{
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}

void loadKernelModule()
{
	int status;
	FILE *fp = popen("/sbin/modprobe darling-mach", "w");

	if (fp == NULL)
	{
		fprintf(stderr, "Failed to run modprobe: %s\n", strerror(errno));
		exit(1);
	}

	status = pclose(fp);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
	{
		fprintf(stderr, "Loaded the kernel module\n");
		return;
	}
	else
	{
		fprintf(stderr, "Failed to load the kernel module\n");
		exit(1);
	}
}

void setupCoredumpPattern(void)
{
	FILE* f = fopen("/proc/sys/kernel/core_pattern", "w");
	if (f != NULL)
	{
		// This is how macOS saves core dumps
		fputs("/cores/core.%p\n", f);
		fclose(f);
	}
}
