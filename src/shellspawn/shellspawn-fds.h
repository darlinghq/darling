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

#ifndef _SHELLSPAWN_FDS_H
#define _SHELLSPAWN_FDS_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

// Descriptor helpers for spawnShell(). They are kept in a header so that they
// can be tested on the host. Descriptors received from a client can be 0, 1 or
// 2 when shellspawn itself runs with closed standard I/O.

// Moves fd above the standard I/O range (keeping it close-on-exec) and returns
// the new descriptor, or returns fd unchanged if it is already above it.
// Returns -1 on failure, leaving fd open.
static inline int shellspawn_move_above_stdio(int fd)
{
	if (fd > STDERR_FILENO)
		return fd;

	int moved = fcntl(fd, F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
	if (moved == -1)
		return -1;

	close(fd);
	return moved;
}

// Makes streams[0], streams[1] and streams[2] the standard input, output and
// error. Any of the streams may itself be 0, 1 or 2, or alias another one, so
// all of them are duplicated out of the way before any standard descriptor
// is replaced. The descriptors in streams are left open.
static inline int shellspawn_remap_stdio(const int streams[3])
{
	int saved[3] = { -1, -1, -1 };
	int i, err;

	for (i = 0; i < 3; i++)
	{
		saved[i] = fcntl(streams[i], F_DUPFD_CLOEXEC, STDERR_FILENO + 1);
		if (saved[i] == -1)
			goto fail;
	}

	for (i = 0; i < 3; i++)
	{
		if (dup2(saved[i], i) == -1)
			goto fail;
	}

	for (i = 0; i < 3; i++)
		close(saved[i]);
	return 0;

fail:
	err = errno;
	for (i = 0; i < 3; i++)
	{
		if (saved[i] != -1)
			close(saved[i]);
	}
	errno = err;
	return -1;
}

// Closes every distinct descriptor in streams that is not below min_fd and
// marks all entries as closed (-1), so that calling it again is harmless.
static inline void shellspawn_close_streams(int streams[3], int min_fd)
{
	for (int i = 0; i < 3; i++)
	{
		int fd = streams[i];

		if (fd == -1)
			continue;

		for (int j = i; j < 3; j++)
		{
			if (streams[j] == fd)
				streams[j] = -1;
		}

		if (fd >= min_fd)
			close(fd);
	}
}

#endif
