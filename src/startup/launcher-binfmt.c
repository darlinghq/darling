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

#include "launcher-binfmt.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/auxv.h>
#include <sys/stat.h>
#include <unistd.h>

int launcherBinfmtExecFd(void)
{
	unsigned long fd;

	// A descriptor of 0 and a missing entry both read as 0; only the second sets errno.
	errno = 0;
	fd = getauxval(AT_EXECFD);
	if (fd == 0 && errno == ENOENT)
		return -1;
	return fd > INT_MAX ? -1 : (int) fd;
}

bool launcherBinfmtProgramPath(int execfd, char* path, size_t size)
{
	char link[64];
	struct stat st;
	ssize_t length;

	if (size == 0 || execfd < 0)
		return false;

	// The kernel names a removed file as "<path> (deleted)", a path that leads nowhere.
	if (fstat(execfd, &st) != 0 || st.st_nlink == 0)
		return false;

	snprintf(link, sizeof(link), "/proc/self/fd/%d", execfd);
	length = readlink(link, path, size - 1);
	if (length <= 0 || (size_t) length >= size - 1 || path[0] != '/')
		return false;
	path[length] = '\0';
	return true;
}

char** launcherBinfmtExecArgv(int argc, char* const argv[], const char* program)
{
	char** execArgv;

	if (argc < 2)
		return NULL;

	execArgv = calloc((size_t) argc + 2, sizeof(*execArgv));
	if (execArgv == NULL)
		return NULL;

	execArgv[0] = argv[0];
	execArgv[1] = (char*) "exec";
	execArgv[2] = strdup(program);
	if (execArgv[2] == NULL)
	{
		free(execArgv);
		return NULL;
	}
	for (int i = 2; i < argc; i++)
		execArgv[i + 1] = argv[i];
	return execArgv;
}
