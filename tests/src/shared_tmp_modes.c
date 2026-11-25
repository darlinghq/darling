/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// The shared temporary directories /private/tmp, /private/var/tmp and /var/tmp are
// world-writable with the sticky bit (mode 1777), as on macOS, so that any process can
// create files in them.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures = 0;

static void check(const char* what, int ok)
{
	if (!ok)
	{
		printf("FAIL: %s\n", what);
		failures++;
	}
}

static void checkDir(const char* path)
{
	struct stat st;
	char what[256], file[300];
	int fd;

	snprintf(what, sizeof(what), "%s exists", path);
	if (stat(path, &st) != 0)
	{
		check(what, 0);
		return;
	}
	snprintf(what, sizeof(what), "%s is a directory", path);
	check(what, S_ISDIR(st.st_mode));
	snprintf(what, sizeof(what), "%s is mode 1777 (got %o)", path, st.st_mode & 07777);
	check(what, (st.st_mode & 07777) == 01777);

	snprintf(file, sizeof(file), "%s/shared-tmp-modes-%d", path, (int) getpid());
	fd = open(file, O_CREAT | O_EXCL | O_WRONLY, 0600);
	snprintf(what, sizeof(what), "a file can be created in %s (errno %d)", path, errno);
	check(what, fd >= 0);
	if (fd >= 0)
	{
		close(fd);
		unlink(file);
	}
}

int main(void)
{
	checkDir("/private/tmp");
	checkDir("/tmp");
	checkDir("/private/var/tmp");
	checkDir("/var/tmp");

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
