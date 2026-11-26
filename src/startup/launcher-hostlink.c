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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "launcher-hostlink.h"

// The guest's view of the host filesystem (darling-config.h is not available
// here, so that this file can also be built for the host-side tests).
#define HOST_ROOT "/Volumes/SystemRoot"

static bool buildTarget(char* target, size_t size, const char* hostPath)
{
	if (hostPath[0] != '/')
		return false;
	return snprintf(target, size, HOST_ROOT "%s", hostPath) < (int) size;
}

static bool buildGuestPath(char* path, size_t size, const char* prefix, const char* guestPath)
{
	if (guestPath[0] != '/')
		return false;
	return snprintf(path, size, "%s%s", prefix, guestPath) < (int) size;
}

static bool isExpectedLink(const char* path, const char* target)
{
	char current[PATH_MAX];
	ssize_t length = readlink(path, current, sizeof(current) - 1);

	if (length < 0 || (size_t) length >= sizeof(current))
		return false;
	current[length] = '\0';
	return strcmp(current, target) == 0;
}

static bool linkHostPath(const char* prefix, const char* hostPath,
	const char* guestPath, mode_t type)
{
	char target[PATH_MAX], path[PATH_MAX];
	struct stat hostStat, guestStat;
	bool hostExists;

	if (!buildTarget(target, sizeof(target), hostPath) ||
		!buildGuestPath(path, sizeof(path), prefix, guestPath))
		return false;

	hostExists = stat(hostPath, &hostStat) == 0 &&
		(type == S_IFSOCK ? S_ISSOCK(hostStat.st_mode) : S_ISREG(hostStat.st_mode));

	if (hostExists)
	{
		if (lstat(path, &guestStat) == 0)
		{
			// Keep any configuration or other file the user put here. Only a
			// link to this host path is ours to own.
			return S_ISLNK(guestStat.st_mode) && isExpectedLink(path, target);
		}
		if (errno != ENOENT)
			return false;
		return symlink(target, path) == 0;
	}

	// The host file is missing or is not the expected kind. Remove only a link
	// we made, so guests never start with a dangling passthrough path.
	if (lstat(path, &guestStat) == 0 && S_ISLNK(guestStat.st_mode) &&
		isExpectedLink(path, target))
		unlink(path);
	return false;
}

bool launcherLinkHostSocket(const char* prefix, const char* hostPath, const char* guestPath)
{
	return linkHostPath(prefix, hostPath, guestPath, S_IFSOCK);
}

bool launcherLinkHostFile(const char* prefix, const char* hostPath, const char* guestPath)
{
	return linkHostPath(prefix, hostPath, guestPath, S_IFREG);
}

bool launcherLinkHostKrb5Config(const char* prefix, const char* configuredPath)
{
	const char* guestPath = "/private/etc/krb5.conf";

	// A relative $KRB5_CONFIG is interpreted by the process that uses it and
	// cannot be mapped into the guest, so fall back to the usual host file.
	if (configuredPath != NULL && configuredPath[0] == '/' &&
		launcherLinkHostFile(prefix, configuredPath, guestPath))
		return true;

	return launcherLinkHostFile(prefix, "/etc/krb5.conf", guestPath);
}
