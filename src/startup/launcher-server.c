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
#include <fcntl.h>
#include <linux/nsfs.h>
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
