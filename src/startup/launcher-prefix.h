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

#ifndef _LAUNCHER_PREFIX_H
#define _LAUNCHER_PREFIX_H

#include <stdbool.h>
#include <stddef.h>

enum launcher_prefix_check
{
	LAUNCHER_PREFIX_OK = 0,
	LAUNCHER_PREFIX_EMPTY,
	LAUNCHER_PREFIX_RELATIVE,
	LAUNCHER_PREFIX_TOO_LONG,
};

// The longest prefix path for which every Unix socket path in the prefix (the
// darlingserver and shellspawn sockets) fits into sockaddr_un.sun_path.
size_t launcherMaxPrefixLength(void);

// Checks a prefix path before anything is done with it. It must be absolute,
// so that it means the same to the launcher, darlingserver and the container,
// and short enough for the socket paths.
enum launcher_prefix_check launcherCheckPrefix(const char* prefix);

// A message for the user, or NULL for LAUNCHER_PREFIX_OK.
const char* launcherPrefixCheckMessage(enum launcher_prefix_check check);

// Writes prefix followed by suffix into out (size bytes, normally sun_path).
// Returns false, leaving an empty string, if it does not fit.
bool launcherSocketPath(char* out, size_t size, const char* prefix, const char* suffix);

#endif
