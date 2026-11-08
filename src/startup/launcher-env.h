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

#ifndef _LAUNCHER_ENV_H
#define _LAUNCHER_ENV_H

#include <stdbool.h>

// The PATH darlingserver gets when the environment is restricted. It names only
// directories that belong to root on a normal system.
#define LAUNCHER_SERVER_PATH "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

// Whether the environment entry `entry` ("NAME=value") sets the variable `name`.
bool launcherEnvEntryIs(const char* entry, const char* name);

// Builds the environment darlingserver is started with from `envp`.
//
// darlingserver starts with real root ids, so the kernel does not mark its exec
// as secure: its dynamic loader, its C library and its own getenv() calls trust
// the environment. When `restricted` is set (the launcher was started setuid by
// another user), only the variables darlingserver reads for its own purposes and
// that are safe to take from that user are kept, and PATH is replaced by
// LAUNCHER_SERVER_PATH. Otherwise the caller is root and `envp` is kept as is.
//
// Returns a NULL-terminated array that the caller frees; its strings are those
// of `envp` (or static). Returns NULL if out of memory.
char** launcherServerEnvironment(char* const* envp, bool restricted);

#endif
