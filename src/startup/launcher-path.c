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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "darling-config.h"
#include "launcher-path.h"

char* launcherGuestPath(const char* hostPath, const char* workingDirectory)
{
	char* absolute = NULL;
	char* real;
	char* guest;

	if (hostPath[0] != '/')
	{
		if (asprintf(&absolute, "%s/%s", workingDirectory, hostPath) < 0)
			return NULL;
		hostPath = absolute;
	}
	real = realpath(hostPath, NULL);
	free(absolute);
	if (real == NULL)
		return NULL;
	if (asprintf(&guest, "%s%s", SYSTEM_ROOT, strcmp(real, "/") == 0 ? "" : real) < 0)
		guest = NULL;
	free(real);
	return guest;
}
