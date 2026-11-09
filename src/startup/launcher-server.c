/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/nsfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "launcher-server.h"

static bool sameFile(const struct stat* a, const struct stat* b)
{
	return a->st_dev == b->st_dev && a->st_ino == b->st_ino;
}

static enum launcher_server_check lookupFailure(void)
{
	// A process that has exited (or a zombie) loses its exe link and its
	// namespace files.
	return (errno == ENOENT || errno == ESRCH) ? LAUNCHER_SERVER_GONE : LAUNCHER_SERVER_ERROR;
}

enum launcher_server_check launcherOpenServerMountNs(int procfd, const struct stat* exe,
	const struct stat* userNs, const struct stat* mntNs, int* nsfd)
{
	struct stat st;
	int fd, ownerfd;

	*nsfd = -1;

	if (fstatat(procfd, "exe", &st, 0) != 0)
		return lookupFailure();
	if (!sameFile(&st, exe))
		return LAUNCHER_SERVER_WRONG_EXE;

	fd = openat(procfd, "ns/mnt", O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return lookupFailure();

	ownerfd = ioctl(fd, NS_GET_USERNS);
	if (ownerfd < 0)
	{
		// EPERM: the owner is outside the caller's user namespace.
		enum launcher_server_check result = (errno == EPERM) ? LAUNCHER_SERVER_FOREIGN_USERNS : LAUNCHER_SERVER_ERROR;
		close(fd);
		return result;
	}
	if (fstat(ownerfd, &st) != 0)
	{
		close(ownerfd);
		close(fd);
		return LAUNCHER_SERVER_ERROR;
	}
	close(ownerfd);
	if (!sameFile(&st, userNs))
	{
		close(fd);
		return LAUNCHER_SERVER_FOREIGN_USERNS;
	}

	if (fstat(fd, &st) != 0)
	{
		close(fd);
		return LAUNCHER_SERVER_ERROR;
	}
	if (sameFile(&st, mntNs))
	{
		close(fd);
		return LAUNCHER_SERVER_SAME_MNTNS;
	}

	// The executable and the namespace were looked up through the same pinned
	// directory, so they belong to the same process. Make sure it was still
	// alive after the namespace was opened.
	if (fstatat(procfd, "exe", &st, 0) != 0)
	{
		enum launcher_server_check result = lookupFailure();
		close(fd);
		return result;
	}
	if (!sameFile(&st, exe))
	{
		close(fd);
		return LAUNCHER_SERVER_WRONG_EXE;
	}

	*nsfd = fd;
	return LAUNCHER_SERVER_OK;
}

bool launcherServerCommandLineMatchesPrefix(const char* cmdline, size_t length,
	const char* prefix)
{
	const char* firstEnd = memchr(cmdline, '\0', length);
	if (!firstEnd || firstEnd + 1 >= cmdline + length)
		return false;
	if ((size_t)(firstEnd - cmdline) != sizeof("darlingserver") - 1
		|| memcmp(cmdline, "darlingserver", sizeof("darlingserver") - 1) != 0)
		return false;
	const char* second = firstEnd + 1;
	const char* secondEnd = memchr(second, '\0', (size_t)(cmdline + length - second));
	if (!secondEnd)
		return false;
	size_t prefixLength = strlen(prefix);
	return (size_t)(secondEnd - second) == prefixLength && memcmp(second, prefix, prefixLength) == 0;
}

static bool parsePid(const char* name, pid_t* pid)
{
	char* end;
	long value;
	errno = 0;
	value = strtol(name, &end, 10);
	if (errno != 0 || end == name || *end != '\0' || value <= 1 || value > INT_MAX)
		return false;
	*pid = (pid_t)value;
	return true;
}

// Scan only processes owned by the invoking uid. Each candidate is pinned
// through /proc/<pid> before its executable and arguments are examined. The
// scan refuses to initialize a prefix if a matching server is still present;
// it never signals or otherwise changes a process.
int launcherFindPrefixServer(const char* procPath, const char* prefix, uid_t uid,
	const char* serverPath, pid_t* serverPid)
{
	struct stat serverExe, userNs, mntNs;
	int procfd = -1, scanfd = -1;
	DIR* procDir = NULL;
	struct dirent* entry;
	int result = 0;

	*serverPid = 0;
	if (stat(serverPath, &serverExe) != 0 || stat("/proc/self/ns/user", &userNs) != 0
		|| stat("/proc/self/ns/mnt", &mntNs) != 0)
		return -1;

	procfd = open(procPath, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (procfd < 0)
		return -1;
	scanfd = dup(procfd);
	if (scanfd < 0)
		goto error;
	procDir = fdopendir(scanfd);
	if (!procDir)
		goto error;
	scanfd = -1;

	for (;;)
	{
		errno = 0;
		entry = readdir(procDir);
		if (!entry)
		{
			if (errno != 0)
				goto error;
			break;
		}
		pid_t candidate;
		struct stat process, exe;
		int processfd, cmdfd, commfd, nsfd = -1;
		char cmdline[1024];
		char comm[64];
		ssize_t length;
		enum launcher_server_check check;

		if (!parsePid(entry->d_name, &candidate))
			continue;
		processfd = openat(procfd, entry->d_name, O_PATH | O_DIRECTORY | O_CLOEXEC);
		if (processfd < 0)
		{
			if (errno == ENOENT || errno == ESRCH || errno == EACCES)
				continue;
			goto error;
		}
		if (fstat(processfd, &process) != 0)
		{
			int saved = errno;
			close(processfd);
			errno = saved;
			if (errno == ENOENT || errno == ESRCH)
				continue;
			goto error;
		}
		if (process.st_uid != uid)
		{
			close(processfd);
			continue;
		}
		commfd = openat(processfd, "comm", O_RDONLY | O_CLOEXEC);
		if (commfd < 0)
		{
			int saved = errno;
			close(processfd);
			errno = saved;
			if (errno == ENOENT || errno == ESRCH || errno == EACCES)
				continue;
			goto error;
		}
		length = read(commfd, comm, sizeof(comm) - 1);
		int commError = errno;
		close(commfd);
		if (length < 0)
		{
			close(processfd);
			errno = commError;
			if (errno == ENOENT || errno == ESRCH || errno == EACCES)
				continue;
			goto error;
		}
		comm[length] = '\0';
		if (length > 0 && comm[length - 1] == '\n')
			comm[length - 1] = '\0';
		if (strcmp(comm, "darlingserver") != 0)
		{
			close(processfd);
			continue;
		}
		if (fstatat(processfd, "exe", &exe, 0) != 0)
		{
			int saved = errno;
			close(processfd);
			errno = saved;
			if (errno == ENOENT || errno == ESRCH)
				continue;
			goto error;
		}
		if (!sameFile(&exe, &serverExe))
		{
			close(processfd);
			continue;
		}
		cmdfd = openat(processfd, "cmdline", O_RDONLY | O_CLOEXEC);
		if (cmdfd < 0)
		{
			int saved = errno;
			close(processfd);
			errno = saved;
			if (errno == ENOENT || errno == ESRCH)
				continue;
			goto error;
		}
		length = read(cmdfd, cmdline, sizeof(cmdline));
		int readError = errno;
		close(cmdfd);
		if (length < 0)
		{
			close(processfd);
			errno = readError;
			goto error;
		}
		if (!launcherServerCommandLineMatchesPrefix(cmdline, (size_t)length, prefix))
		{
			close(processfd);
			continue;
		}

		// Recheck the pinned process after reading cmdline. A vanished process
		// is not a live server; a changed executable cannot be trusted.
		check = launcherOpenServerMountNs(processfd, &serverExe, &userNs, &mntNs, &nsfd);
		if (nsfd >= 0)
			close(nsfd);
		close(processfd);
		if (check == LAUNCHER_SERVER_GONE || check == LAUNCHER_SERVER_WRONG_EXE)
			continue;
		if (check == LAUNCHER_SERVER_ERROR)
			goto error;
		*serverPid = candidate;
		result = 1;
		break;
	}
	closedir(procDir);
	close(procfd);
	return result;

error:
	if (procDir)
		closedir(procDir);
	if (scanfd >= 0)
		close(scanfd);
	if (procfd >= 0)
		close(procfd);
	return -1;
}
