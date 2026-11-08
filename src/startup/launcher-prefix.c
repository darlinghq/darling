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
#include "../shellspawn/shellspawn.h"
#include "launcher-prefix.h"

// darlingserver creates its socket here (src/external/darlingserver).
#define DARLINGSERVER_SOCKPATH "/.darlingserver.sock"

static const char* const socketSuffixes[] = {
	SHELLSPAWN_SOCKPATH,
	DARLINGSERVER_SOCKPATH,
};

size_t launcherMaxPrefixLength(void)
{
	const size_t pathSize = sizeof(((struct sockaddr_un*) NULL)->sun_path);
	size_t longest = 0;

	for (size_t i = 0; i < sizeof(socketSuffixes) / sizeof(socketSuffixes[0]); i++)
	{
		size_t len = strlen(socketSuffixes[i]);
		if (len > longest)
			longest = len;
	}

	// Room for the terminating NUL.
	return pathSize - 1 - longest;
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
			return "The prefix path is too long for the sockets in the prefix";
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
