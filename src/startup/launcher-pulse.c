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

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "launcher-pulse.h"

static bool isSocket(const char* path)
{
	struct stat st;

	return stat(path, &st) == 0 && S_ISSOCK(st.st_mode);
}

static bool nonEmpty(const char* value)
{
	return value != NULL && value[0] != '\0';
}

// The server keeps the runtime directory, named <machine id>-runtime, as a link in ~/.config/pulse to a
// private directory it made (under the temporary directory when there is no XDG_RUNTIME_DIR). The
// first link whose target holds a socket named native is the running server's.
static bool findRuntimeLink(const char* configDir, char* out, size_t size)
{
	DIR* dir = opendir(configDir);
	struct dirent* ent;
	bool found = false;

	if (dir == NULL)
		return false;
	while (!found && (ent = readdir(dir)) != NULL)
	{
		static const char suffix[] = "-runtime";
		size_t length = strlen(ent->d_name);
		char link[PATH_MAX], target[PATH_MAX], socketPath[PATH_MAX + 8];
		ssize_t got;

		if (length <= sizeof(suffix) - 1 || strcmp(ent->d_name + length - (sizeof(suffix) - 1), suffix) != 0)
			continue;
		if (snprintf(link, sizeof(link), "%s/%s", configDir, ent->d_name) >= (int) sizeof(link))
			continue;
		got = readlink(link, target, sizeof(target) - 1);
		if (got <= 0 || target[0] != '/')
			continue;
		target[got] = '\0';
		if (snprintf(socketPath, sizeof(socketPath), "%s/native", target) >= (int) sizeof(socketPath) || !isSocket(socketPath))
			continue;
		if ((size_t) got >= size)
			continue;
		memcpy(out, target, got + 1);
		found = true;
	}
	closedir(dir);
	return found;
}

bool launcherFindPulse(const char* home, const char* xdgRuntimeDir, const char* server,
	const char* runtimePath, const char* cookie, struct launcher_pulse_env* result)
{
	char configDir[PATH_MAX], file[PATH_MAX], target[PATH_MAX];
	bool any = false;

	result->runtimePath[0] = '\0';
	result->cookie[0] = '\0';
	if (!nonEmpty(home) || home[0] != '/')
		return false;
	if (snprintf(configDir, sizeof(configDir), "%s/.config/pulse", home) >= (int) sizeof(configDir))
		return false;

	if (!nonEmpty(server) && !nonEmpty(runtimePath))
	{
		bool xdgWorks = false;

		if (nonEmpty(xdgRuntimeDir) && xdgRuntimeDir[0] == '/'
			&& snprintf(file, sizeof(file), "%s/pulse/native", xdgRuntimeDir) < (int) sizeof(file))
			xdgWorks = isSocket(file);
		if (!xdgWorks && findRuntimeLink(configDir, target, sizeof(target)))
		{
			snprintf(result->runtimePath, sizeof(result->runtimePath), "PULSE_RUNTIME_PATH=%s", target);
			any = true;
		}
	}

	if (!nonEmpty(cookie))
	{
		struct stat st;

		if (snprintf(file, sizeof(file), "%s/cookie", configDir) < (int) sizeof(file)
			&& stat(file, &st) == 0 && S_ISREG(st.st_mode))
		{
			snprintf(result->cookie, sizeof(result->cookie), "PULSE_COOKIE=%s", file);
			any = true;
		}
	}
	return any;
}
