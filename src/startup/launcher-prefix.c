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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "launcher-prefix.h"

// The launcher's limit since before the socket paths were bounded.
#define LAUNCHER_PREFIX_MAX_LENGTH 255

size_t launcherMaxPrefixLength(void)
{
	return LAUNCHER_PREFIX_MAX_LENGTH;
}

enum launcher_prefix_check launcherCheckPrefix(const char* prefix)
{
	if (prefix[0] == '\0')
		return LAUNCHER_PREFIX_EMPTY;
	if (prefix[0] != '/')
		return LAUNCHER_PREFIX_RELATIVE;
	if (strlen(prefix) > launcherMaxPrefixLength())
		return LAUNCHER_PREFIX_TOO_LONG;
	return LAUNCHER_PREFIX_OK;
}

const char* launcherPrefixCheckMessage(enum launcher_prefix_check check)
{
	switch (check)
	{
		case LAUNCHER_PREFIX_OK:
			return NULL;
		case LAUNCHER_PREFIX_EMPTY:
			return "The prefix path is empty";
		case LAUNCHER_PREFIX_RELATIVE:
			return "The prefix path must be absolute";
		case LAUNCHER_PREFIX_TOO_LONG:
			return "The prefix path is too long";
	}
	return "The prefix path is not valid";
}

bool launcherPrefixModeAcceptable(mode_t mode)
{
	return (mode & S_IWOTH) == 0;
}

bool launcherSocketPath(char* out, size_t size, const char* prefix, const char* suffix)
{
	int len;

	if (size == 0)
		return false;

	len = snprintf(out, size, "%s%s", prefix, suffix);
	if (len < 0 || (size_t) len >= size)
	{
		out[0] = '\0';
		return false;
	}
	return true;
}

bool launcherDirSocketPath(char* out, size_t size, int dirfd, const char* suffix)
{
	char dir[32];

	if (dirfd < 0)
	{
		if (size > 0)
			out[0] = '\0';
		return false;
	}

	snprintf(dir, sizeof(dir), "/proc/self/fd/%d", dirfd);
	return launcherSocketPath(out, size, dir, suffix);
}
