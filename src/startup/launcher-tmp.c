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

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "launcher-tmp.h"

static bool plainAbsolutePath(const char* path)
{
	size_t length;

	if (path == NULL || path[0] != '/')
		return false;
	length = strlen(path);
	if (length > PATH_MAX / 2)
		return false;
	for (const char* c = path; *c; c++)
	{
		if ((unsigned char) *c < 0x20 || *c == 0x7f)
			return false;
	}
	for (const char* c = path; *c; )
	{
		const char* end = strchr(c + 1, '/');
		size_t part = end ? (size_t) (end - (c + 1)) : strlen(c + 1);

		if ((part == 1 && c[1] == '.') || (part == 2 && c[1] == '.' && c[2] == '.'))
			return false;
		if (!end)
			break;
		c = end;
	}
	return true;
}

bool launcherTmpDirAcceptable(const char* path, uid_t uid)
{
	struct stat st;
	int fd;
	bool ok;

	if (!plainAbsolutePath(path))
		return false;
	fd = open(path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return false;
	ok = fstat(fd, &st) == 0 && S_ISDIR(st.st_mode) && st.st_uid == uid
		&& ((st.st_mode & (S_IWGRP | S_IWOTH)) == 0);
	close(fd);
	return ok;
}

const char* launcherChooseTmpDir(const char* tmpdirEnv, const char* buildDefault, uid_t uid)
{
	if (tmpdirEnv != NULL && launcherTmpDirAcceptable(tmpdirEnv, uid))
		return tmpdirEnv;
	if (buildDefault != NULL && buildDefault[0] != '\0' && launcherTmpDirAcceptable(buildDefault, uid))
		return buildDefault;
	return NULL;
}

bool launcherLinkPrefixTmp(const char* prefix, const char* dir)
{
	char target[PATH_MAX], current[PATH_MAX];
	struct stat st;
	int prefixFd, privateFd;
	bool ok = false;

	if (!plainAbsolutePath(dir) || snprintf(target, sizeof(target), "/Volumes/SystemRoot%s", dir) >= (int) sizeof(target))
		return false;

	prefixFd = open(prefix, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (prefixFd < 0)
		return false;
	privateFd = openat(prefixFd, "private", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	close(prefixFd);
	if (privateFd < 0)
		return false;

	if (fstatat(privateFd, "tmp", &st, AT_SYMLINK_NOFOLLOW) == 0)
	{
		if (S_ISLNK(st.st_mode))
		{
			ssize_t got = readlinkat(privateFd, "tmp", current, sizeof(current) - 1);

			if (got > 0 && (size_t) got == strlen(target) && memcmp(current, target, got) == 0)
			{
				close(privateFd);
				return true;
			}
			if (unlinkat(privateFd, "tmp", 0) != 0)
				goto done;
		}
		else if (S_ISDIR(st.st_mode))
		{
			// only an empty directory, which is what an unused prefix has
			if (unlinkat(privateFd, "tmp", AT_REMOVEDIR) != 0)
				goto done;
		}
		else
			goto done;
	}
	ok = symlinkat(target, privateFd, "tmp") == 0;
done:
	close(privateFd);
	return ok;
}
