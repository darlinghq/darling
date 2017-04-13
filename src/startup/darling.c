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
#include <getopt.h>
#include "darling.h"
#include "darling-config.h"

#define MLDR_PATH "/bin/mldr"

const char* DARLING_INIT_COMM = "darling-init";
char *prefix;
uid_t g_originalUid, g_originalGid;
bool g_fixPermissions = false;
char **g_argv, **g_envp;

int main(int argc, char ** argv, char ** envp)
{
	pid_t pidInit, pidChild;
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

	/* On Android we handle kernel modules manually */
#ifndef __ANDROID_API__
	if (!isModuleLoaded())
		loadKernelModule();
#endif

	prefix = getenv("DPREFIX");
	if (!prefix)
		prefix = defaultPrefixPath();
	if (!prefix)
		return 1;
	unsetenv("DPREFIX");

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

		c = getopt_long(argc, argv, "", long_options, &option_index);

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
		setupWorkdir();
		pidInit = spawnInitProcess();
		putInitPid(pidInit);
	}

	if (strcmp(argv[1], "shell") != 0)
	{
		char *path = realpath(argv[1], NULL);
		char *fullPath;

		if (path == NULL)
		{
			fprintf(stderr, "Cannot resolve path: %s\n", strerror(errno));
			exit(1);
		}

		const char *argv_child[argc + 1];

		argv_child[0] = MLDR_PATH;

		fullPath = malloc(strlen(SYSTEM_ROOT) + strlen(path) + 1);
		strcpy(fullPath, SYSTEM_ROOT);
		strcat(fullPath, path);
		argv_child[1] = fullPath;

		for (int i = 2; i < argc; i++)
			argv_child[i] = argv[i];
		argv_child[argc] = NULL;

		pidChild = spawnChild(pidInit, MLDR_PATH, argv_child);
		free(path);
		free(fullPath);
	}
	else
	{
		// Spawn the shell
		if (argc > 2)
		{
			size_t total_len = 0;
			for (int i = 2; i < argc; i++)
			total_len += strlen(argv[i]);

			char buffer[total_len + argc];

			char *to = buffer;
			for (int i = 2; i < argc; i++)
			to = stpcpy(stpcpy(to, argv[i]), " ");
			// Overwrite the last whitespace
			*(to - 1) = '\0';

			pidChild = spawnChild(pidInit, MLDR_PATH,
				(const char *[5]) {MLDR_PATH, "/bin/bash", "-c", buffer, NULL});
		}
		else
			pidChild = spawnChild(pidInit, MLDR_PATH,
				(const char *[3]) {MLDR_PATH, "/bin/bash", NULL});
	}
	if (pidChild == -ENOMEM)
	{
		pidInit = 0;
		goto start_init;
	}

	// Drop the privileges so that we can be killed, etc by the user
	seteuid(g_originalUid);

	waitpid(pidChild, &wstatus, 0);

	if (WIFEXITED(wstatus))
		return WEXITSTATUS(wstatus);
	if (WIFSIGNALED(wstatus))
		return WTERMSIG(wstatus);
	return 0;
}

pid_t spawnChild(int pidInit, const char *path, const char *const argv[])
{
	int fdNS;
	pid_t pidChild;
	char pathNS[4096], curPath[4096];

	if (getcwd(curPath, sizeof(curPath)) == NULL)
	{
		fprintf(stderr, "Cannot get current directory: %s\n", strerror(errno));
		exit(1);
	}

	snprintf(pathNS, sizeof(pathNS), "/proc/%d/ns/pid", pidInit);

	fdNS = open(pathNS, O_RDONLY);

	if (fdNS < 0)
	{
		fprintf(stderr, "Cannot open PID namespace file: %s\n", strerror(errno));
		exit(1);
	}

	// Calling setns() with a PID namespace doesn't move our process into it,
	// but our child process will be spawned inside the namespace
	if (setns(fdNS, CLONE_NEWPID) != 0)
	{
		fprintf(stderr, "Cannot join PID namespace: %s\n", strerror(errno));
		exit(1);
	}
	close(fdNS);

	pidChild = fork();
	if (pidChild < 0)
	{
		if (errno == ENOMEM)
		{
			// This condition happens specifically when the init process is a zombie.
			// We should simply start a new init process.
			return -ENOMEM;
		}
		fprintf(stderr, "Cannot spawn a child process: %s\n", strerror(errno));
		exit(1);
	}

	if (pidChild == 0)
	{
		// This is the child process

		// We still have the outside PIDs in /proc
		snprintf(pathNS, sizeof(pathNS), "/proc/%d/ns/mnt", pidInit);
		fdNS = open(pathNS, O_RDONLY);
		if (fdNS < 0)
		{
			fprintf(stderr, "Cannot open mount namespace file: %s\n", strerror(errno));
			exit(1);
		}

		if (setns(fdNS, CLONE_NEWNS) != 0)
		{
			fprintf(stderr, "Cannot join mount namespace: %s\n", strerror(errno));
			exit(1);
		}
		close(fdNS);

		/*
		snprintf(pathNS, sizeof(pathNS), SYSTEM_ROOT "/proc/%d/ns/user", pidInit);
		fdNS = open(pathNS, O_RDONLY);
		if (fdNS < 0)
		{
			fprintf(stderr, "Cannot open user namespace file: %s\n", strerror(errno));
			exit(1);
		}
		*/
		
		// Drop the privileges. It's important to drop GID first, because
		// non-root users can't change their GID.
		setresgid(g_originalGid, g_originalGid, g_originalGid);
		setresuid(g_originalUid, g_originalUid, g_originalUid);

		/*
		if (setns(fdNS, CLONE_NEWUSER) != 0)
		{
			fprintf(stderr, "Cannot join user namespace: %s\n", strerror(errno));
			exit(1);
		}
		close(fdNS);
		*/

		setupChild(curPath);

		execv(path, (char * const *) argv);

		fprintf(stderr, "Cannot exec the target program: %s\n", strerror(errno));
		exit(1);
	}

	return pidChild;
}

void setupChild(const char *curPath)
{
	char buffer1[4096];
	char buffer2[4096];


	unsetenv("LESSOPEN");
	unsetenv("LESSCLOSE");
	unsetenv("LESSECHO");

	setenv("PATH",
		"/usr/bin:"
		"/bin:"
		"/usr/sbin:"
		"/sbin:"
		"/usr/local/bin",
		1);

	sscanf(getenv("HOME"), "/home/%4096s", buffer1);
	snprintf(buffer2, sizeof(buffer2), "/Users/%s", buffer1);
	setenv("HOME", buffer2, 1);

	if (sscanf(curPath, "/home/%4096s", buffer1) == 1)
	{
		// We're currently inside our home directory
		snprintf(buffer2, sizeof(buffer2), "/Users/%s", buffer1);
		setenv("PWD", buffer2, 1);
		chdir(buffer2);
	}
	else
	{
		snprintf(buffer2, sizeof(buffer2), SYSTEM_ROOT "%s", curPath);
		setenv("PWD", buffer2, 1);
		chdir(buffer2);
	}
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
	// char idmap[100];
	char buffer[1];
	FILE *file;

	if (pipe(pipefd) == -1)
	{
		fprintf(stderr, "Cannot create a pipe for synchronization: %s\n", strerror(errno));
		exit(1);
	}

	if (unshare(CLONE_NEWPID) != 0)
	{
		fprintf(stderr, "Cannot unshare pid namespace to create darling-init: %s\n", strerror(errno));
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

void darlingPreInit(void)
{
	// TODO: Run /usr/libexec/makewhatis

	// TODO: this is where we will exec() launchd in future.
	// Instead, we just reap zombies.
	while (1)
	{
		int status, sig;
		sigset_t chld;

		sigemptyset(&chld);
		sigaddset(&chld, SIGCHLD);
		sigwait(&chld, &sig);

		while (waitpid(-1, &status, 0) != -1);
	}
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

	fprintf(stderr, "Setting up a new Darling prefix at %s\n", prefix);

	seteuid(g_originalUid);
	setegid(g_originalGid);

	createDir(prefix);

	// The user needs to be able to create mountpoints,
	snprintf(path, sizeof(path), "%s/Volumes", prefix);
	createDir(path);
	// ... to install applications,
	snprintf(path, sizeof(path), "%s/Applications", prefix);
	createDir(path);

	// ... to put stuff in /usr/local,
	snprintf(path, sizeof(path), "%s/usr", prefix);
	createDir(path);
	snprintf(path, sizeof(path), "%s/usr/local", prefix);
	createDir(path);
	snprintf(path, sizeof(path), "%s/usr/local/share", prefix);
	createDir(path);

	// ... and to install plists to /var/db
	snprintf(path, sizeof(path), "%s/private", prefix);
	createDir(path);
	snprintf(path, sizeof(path), "%s/private/var", prefix);
	createDir(path);
	snprintf(path, sizeof(path), "%s/private/var/db", prefix);
	createDir(path);

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

	if (strcmp(exeBuf, DARLING_INIT_COMM) != 0)
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
