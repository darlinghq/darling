/*
This file is part of Darling.

Copyright (C) 2026 Cristian Magherusan-Stanciu

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

#include "exec_target.h"
#include <stddef.h>
#include <string.h>

const char* mldr_exec_target(int argc, char* const* argv, const char* env_path, enum mldr_exec_target_source* source)
{
	const char* bang = argc > 0 ? strchr(argv[0], '!') : NULL;

	if (bang != NULL)
	{
		*source = MLDR_TARGET_ARGV0;
		return bang + 1;
	}
	if (argc > 0 && env_path != NULL && env_path[0] != '\0')
	{
		*source = MLDR_TARGET_ENV;
		return env_path;
	}
	if (argc > 1)
	{
		*source = MLDR_TARGET_ARGV1;
		return argv[1];
	}
	*source = MLDR_TARGET_NONE;
	return NULL;
}

int mldr_emulator_argv(const char* emulator, const char* mldr32, int argc, char* const* argv, char** out)
{
	int n = 0;

	out[n++] = (char*) emulator;
	out[n++] = (char*) mldr32;
	for (int i = 1; i < argc; ++i)
		out[n++] = argv[i];
	out[n] = NULL;
	return n;
}
