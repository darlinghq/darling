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

#include <stdlib.h>
#include <string.h>
#include "launcher-env.h"

// What darlingserver reads from its environment, and why each is safe to take
// from the invoking user:
static const char* const serverEnvAllowlist[] = {
	// Where /Users/<name>/LinuxHome points and where the user's
	// user-dirs.dirs is found; both are used with the user's ids.
	"HOME",
	"XDG_CONFIG_HOME",
	// Selects the copy mode, which the user can also select by writing the
	// prefix's .darling-fsmode file.
	"DARLING_NOOVERLAYFS",
	// The guest init program; it is started after the server has dropped to
	// the user's ids, inside the container.
	"DSERVER_INIT",
	// Logging switches; the log goes to the prefix or to standard error.
	"DSERVER_LOG_LEVEL",
	"DSERVER_LOG_STDERR",
	"DTAPE_LOG_SAFE_STUBS",
	// WSL 1 detection, which selects workarounds for its kernel.
	"WSLENV",
	"WSL_INTEROP",
};

bool launcherEnvEntryIs(const char* entry, const char* name)
{
	size_t len = strlen(name);

	return strncmp(entry, name, len) == 0 && entry[len] == '=';
}

static bool serverMayGet(const char* entry)
{
	for (size_t i = 0; i < sizeof(serverEnvAllowlist) / sizeof(serverEnvAllowlist[0]); i++)
	{
		if (launcherEnvEntryIs(entry, serverEnvAllowlist[i]))
			return true;
	}
	return false;
}

char** launcherServerEnvironment(char* const* envp, bool restricted)
{
	size_t count = 0, n = 0;
	char** result;

	while (envp[count] != NULL)
		count++;

	// One more for PATH and one for the terminating NULL.
	result = malloc(sizeof(char*) * (count + 2));
	if (!result)
		return NULL;

	if (restricted)
		result[n++] = (char*) LAUNCHER_SERVER_PATH;

	for (size_t i = 0; i < count; i++)
	{
		if (!restricted || serverMayGet(envp[i]))
			result[n++] = envp[i];
	}
	result[n] = NULL;

	return result;
}
