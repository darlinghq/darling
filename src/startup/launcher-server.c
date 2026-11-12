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

bool launcherServerStatusRealUid(const char* status, size_t length, uid_t* uid)
{
	const char* line = status;
	const char* limit = status + length;
	while (line < limit)
	{
		const char* end = memchr(line, '\n', (size_t)(limit - line));
		if (!end)
			end = limit;
		if ((size_t)(end - line) >= 4 && memcmp(line, "Uid:", 4) == 0)
		{
			const char* value = line + 4;
			while (value < end && (*value == ' ' || *value == '\t'))
				value++;
			if (value == end || *value < '0' || *value > '9')
				return false;
			errno = 0;
			char* parsedEnd;
			unsigned long parsed = strtoul(value, &parsedEnd, 10);
			if (errno != 0 || parsedEnd == value || parsedEnd > end || (uid_t)parsed != parsed)
				return false;
			*uid = (uid_t)parsed;
			return true;
		}
		line = end < limit ? end + 1 : limit;
	}
	return false;
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

// Scan only processes whose real uid is the invoking uid. A server can have
// effective uid 0 while it prepares its prefix, so proc-directory ownership
// is not a reliable identity check. Candidates are pinned through /proc/<pid>
// before status, executable and arguments are examined. The scan never signals
// or otherwise changes a process.
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
		struct stat exe;
		int processfd, cmdfd, commfd, nsfd = -1;
		char cmdline[1024];
		char comm[64];
		char status[4096];
		ssize_t length;
		uid_t realUid;
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
		cmdfd = openat(processfd, "status", O_RDONLY | O_CLOEXEC);
		if (cmdfd < 0)
		{
			int saved = errno;
			close(processfd);
			errno = saved;
			if (errno == ENOENT || errno == ESRCH)
				continue;
			goto error;
		}
		length = read(cmdfd, status, sizeof(status));
		int statusError = errno;
		close(cmdfd);
		if (length < 0)
		{
			close(processfd);
			errno = statusError;
			goto error;
		}
		if ((size_t)length == sizeof(status))
			length--;
		status[length] = '\0';
		if (!launcherServerStatusRealUid(status, (size_t)length, &realUid))
		{
			close(processfd);
			goto error;
		}
		if (realUid != uid)
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

bool launcherStatusIsNamespaceInit(const char* status, size_t length)
{
	const char* line = status;
	const char* limit = status + length;

	while (line < limit)
	{
		const char* end = memchr(line, '\n', (size_t)(limit - line));
		if (!end)
			end = limit;
		if ((size_t)(end - line) >= 6 && memcmp(line, "NSpid:", 6) == 0)
		{
			unsigned long last = 0;
			size_t entries = 0;
			const char* value = line + 6;

			while (value < end)
			{
				char* parsedEnd;

				while (value < end && (*value == ' ' || *value == '\t'))
					value++;
				if (value == end)
					break;
				if (*value < '0' || *value > '9')
					return false;
				errno = 0;
				last = strtoul(value, &parsedEnd, 10);
				if (errno != 0 || parsedEnd == value || parsedEnd > end)
					return false;
				entries++;
				value = parsedEnd;
			}
			return entries > 1 && last == 1;
		}
		line = end < limit ? end + 1 : limit;
	}
	return false;
}

bool launcherEnvironmentHas(const char* environ, size_t length, const char* name, const char* value)
{
	size_t nameLength = strlen(name), valueLength = strlen(value);
	const char* entry = environ;
	const char* limit = environ + length;

	while (entry < limit)
	{
		const char* end = memchr(entry, '\0', (size_t)(limit - entry));
		size_t entryLength = (size_t)((end ? end : limit) - entry);

		if (entryLength == nameLength + 1 + valueLength && memcmp(entry, name, nameLength) == 0
			&& entry[nameLength] == '=' && memcmp(entry + nameLength + 1, value, valueLength) == 0)
			return true;
		if (!end)
			break;
		entry = end + 1;
	}
	return false;
}

// Reads a whole small file of a pinned /proc/<pid> directory into a buffer
// that the caller frees. Returns false with errno set on failure.
static bool readProcFile(int processfd, const char* name, size_t maxLength, char** data, size_t* length)
{
	int fd = openat(processfd, name, O_RDONLY | O_CLOEXEC);
	char* buffer;
	size_t used = 0;

	if (fd < 0)
		return false;
	buffer = malloc(maxLength + 1);
	if (!buffer)
	{
		close(fd);
		errno = ENOMEM;
		return false;
	}
	for (;;)
	{
		ssize_t got = read(fd, buffer + used, maxLength - used);

		if (got < 0)
		{
			int saved = errno;
			free(buffer);
			close(fd);
			errno = saved;
			return false;
		}
		if (got == 0)
			break;
		used += (size_t)got;
		if (used == maxLength)
			break;
	}
	close(fd);
	buffer[used] = '\0';
	*data = buffer;
	*length = used;
	return true;
}

int launcherFindOrphanedInit(const char* procPath, const char* prefix, uid_t uid,
	const char* mldrPath, pid_t* pid)
{
	struct stat mldrExe;
	int procfd, scanfd, result = 0;
	DIR* procDir;
	struct dirent* entry;
	char socketPath[PATH_MAX + 32];

	*pid = 0;
	if (snprintf(socketPath, sizeof(socketPath), "%s/.darlingserver.sock", prefix) >= (int)sizeof(socketPath)
		|| stat(mldrPath, &mldrExe) != 0)
		return -1;

	procfd = open(procPath, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (procfd < 0)
		return -1;
	scanfd = dup(procfd);
	procDir = scanfd >= 0 ? fdopendir(scanfd) : NULL;
	if (!procDir)
	{
		if (scanfd >= 0)
			close(scanfd);
		close(procfd);
		return -1;
	}

	for (;;)
	{
		pid_t candidate;
		struct stat exe;
		int processfd;
		char* data;
		size_t length;
		uid_t realUid;
		bool matches;

		errno = 0;
		entry = readdir(procDir);
		if (!entry)
		{
			if (errno != 0)
				result = -1;
			break;
		}
		if (!parsePid(entry->d_name, &candidate))
			continue;
		processfd = openat(procfd, entry->d_name, O_PATH | O_DIRECTORY | O_CLOEXEC);
		if (processfd < 0)
		{
			if (errno == ENOENT || errno == ESRCH || errno == EACCES)
				continue;
			result = -1;
			break;
		}

		// A process that disappears or cannot be read is not our candidate;
		// the cheap, discriminating checks come first.
		if (fstatat(processfd, "exe", &exe, 0) != 0 || !sameFile(&exe, &mldrExe)
			|| !readProcFile(processfd, "status", 8192, &data, &length))
		{
			close(processfd);
			continue;
		}
		matches = launcherServerStatusRealUid(data, length, &realUid) && realUid == uid
			&& launcherStatusIsNamespaceInit(data, length);
		free(data);
		if (!matches || !readProcFile(processfd, "environ", 1024 * 1024, &data, &length))
		{
			close(processfd);
			continue;
		}
		matches = launcherEnvironmentHas(data, length, "__mldr_sockpath", socketPath);
		free(data);
		// The pinned directory still names the same live process.
		matches = matches && fstatat(processfd, "exe", &exe, 0) == 0 && sameFile(&exe, &mldrExe);
		close(processfd);
		if (matches)
		{
			*pid = candidate;
			result = 1;
			break;
		}
	}
	closedir(procDir);
	close(procfd);
	return result;
}
