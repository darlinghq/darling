/*
This file is part of Darling.

Copyright (C) 2016 Lubos Dolezel

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
#include <sched.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include "darling.h"
#include "darling-config.h"

const char* DARLING_INIT_COMM = "darling-init";
uid_t g_originalUid;

int main(int argc, const char** argv)
{
	const char* dprefix;
	int pidInit;
	
	if (argc <= 1)
	{
		showHelp(argv[0]);
		return 1;
	}
	
	/*if (geteuid() != 0)
	{
		missingSetuidRoot();
		return 1;
	}*/
	
	// Temporarily drop root privileges
	g_originalUid = getuid();
	seteuid(getuid());
	setegid(getgid());
	
	dprefix = getenv("DPREFIX");
	if (!dprefix)
		dprefix = defaultPrefixPath();
	if (!dprefix)
		return 1;
	checkPrefixOwner(dprefix);
	
	pidInit = getInitProcess(dprefix);
	
	// If prefix's init is not running, start it up
	if (pidInit == 0)
	{
		char* opts;
		
		createDir(dprefix);
		setupWorkdir(dprefix);
		
		// Since overlay cannot be mounted inside user namespaces, we have to setup a new mount namespace
		// and do the mount while we can be root
		//seteuid(0);
		if (unshare(CLONE_NEWNS) != 0)
		{
			fprintf(stderr, "Cannot unshare(CLONE_NEWNS): %s\n", strerror(errno));
			return 1;
		}
		
		// Because IDIOTIC systemd marks / as MS_SHARED and we would inherit this into the overlay mount,
		// causing it not to be unmounted once the init process dies.
		if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0)
		{
			fprintf(stderr, "Cannot remount / as private: %s\n", strerror(errno));
			return 1;
		}
	
		opts = (char*) malloc(strlen(dprefix)*2 + sizeof(LIBEXEC_PATH) + 50);
		sprintf(opts, "lowerdir=%s,upperdir=%s,workdir=%s.workdir", LIBEXEC_PATH, dprefix, dprefix);
		
		// Mount overlay onto our prefix
		if (mount("overlay", dprefix, "overlay", 0, opts) != 0)
		{
			fprintf(stderr, "Cannot mount overlay: %s\n", strerror(errno));
			return 1;
		}
		
		free(opts);
		//seteuid(getuid());
		
		pidInit = spawnInitProcess(dprefix);
	}
	
	// TODO: Spawn the executable in the namespace, wait for it and return its exit code
	return 0;
}

void showHelp(const char* argv0)
{
	fprintf(stderr, "This is Darling, translation layer for macOS software.\n\n");
	fprintf(stderr, "Copyright (C) 2012-2016 Lubos Dolezel\n\n");

	fprintf(stderr, "Usage: %s program-path [arguments...]\n\n", argv0);
	fprintf(stderr, "Environment variables:\n"
		"DPREFIX - specifies the location of Darling prefix, defaults to ~/.darling\n");
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

int spawnInitProcess(const char* prefix)
{
	char* childStack;
	int pid;
	char uidmap[100];
	
	childStack = (char*) malloc(16*1024);
	childStack += 16*1024;
	
	//setuid(0);
	pid = clone(darlingPreInit, childStack, /*CLONE_NEWPID |*/ CLONE_NEWUSER, (void*) prefix);
	
	if (pid <= 0)
	{
		fprintf(stderr, "Cannot clone() to create darling-init: %s\n", strerror(errno));
		exit(1);
	}
	
	sprintf(uidmap, "/proc/%d/uid_map", pid);
	
	FILE* file = fopen(uidmap, "w");
	if (file != NULL)
	{
		fprintf(file, "0 %d 1\n", getuid()); // all users map to our user on the outside
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Failure setting uid_map in init process\n");
	}
	
	seteuid(g_originalUid);
	setuid(g_originalUid);
	
	return pid;
}

int darlingPreInit(void* arg)
{
	const char* prefix = (const char*) arg;
	
	prctl(PR_SET_NAME, DARLING_INIT_COMM, 0, 0);
	
	// Wait until the parent sets our uid_map
	while (getuid() == 65534);
	
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
	
	return 0;
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
			fprintf(stderr, "Problem accessing %s: %s\n", path, strerror(errno));
			exit(1);
		}
	}
}

void setupWorkdir(const char* prefix)
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

int getInitProcess(const char* prefix)
{
	const char pidFile[] = "/.init.pid";
	char* pidPath;
	int fd, rd, pid;
	char pidBuf[10];
	char exeBuf[17], procBuf[100];
	
	pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
	strcpy(pidPath, prefix);
	strcat(pidPath, pidFile);
	
	fd = open(pidPath, O_RDONLY);
	if (fd == -1)
		return 0;
	
	rd = read(fd, pidBuf, sizeof(pidBuf)-1);
	close(fd);
	
	if (rd <= 0)
		return 0;
	
	pidBuf[rd-1] = '\0';
	pid = atoi(pidBuf);

	// Does the process exist?
	if (kill(pid, 0) == -1)
	{
		unlink(pidPath);
		return 0;
	}
	
	// Is it actually an init process?
	sprintf(procBuf, "/proc/%d/comm", pid);
	fd = open(procBuf, O_RDONLY);
	if (fd == -1)
	{
		unlink(pidPath);
		return 0;
	}
	
	rd = read(fd, exeBuf, sizeof(exeBuf)-1);
	close(fd);
	
	if (rd <= 0)
	{
		unlink(pidPath);
		return 0;
	}
	
	exeBuf[rd] = '\0';
	if (strcmp(exeBuf, DARLING_INIT_COMM) != 0)
	{
		unlink(pidPath);
		return 0;
	}
	
	return pid;
}

void checkPrefixOwner(const char* prefix)
{
	struct stat st;
	
	if (stat(prefix, &st) == 0)
	{
		if (st.st_uid != getuid())
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
