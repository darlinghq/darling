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

#ifndef _MLDR_EXEC_TARGET_H_
#define _MLDR_EXEC_TARGET_H_

enum mldr_exec_target_source
{
	MLDR_TARGET_NONE = 0, // no program path was given
	MLDR_TARGET_ARGV0,    // after the '!' in argv[0] (what sys_execve() passes)
	MLDR_TARGET_ENV,      // the __mldr_exec_path environment variable (mldr32 started by an emulator)
	MLDR_TARGET_ARGV1,    // argv[1]
};

/**
 * Picks the path of the program mldr has to start: after the '!' in argv[0]
 * if there is one, else `env_path` if it is neither NULL nor empty (and there
 * is an argv[0]), else argv[1]. The result points into argv or at env_path, or
 * is NULL (with *source MLDR_TARGET_NONE) when no path is available.
 */
const char* mldr_exec_target(int argc, char* const* argv, const char* env_path, enum mldr_exec_target_source* source);

/**
 * Builds the arguments for running `mldr32` under `emulator`: emulator, mldr32,
 * then argv[1..argc-1], then NULL. The program path does not travel in them
 * (see MLDR_TARGET_ENV). `out` must have room for argc + 2 pointers, argc >= 1.
 * Returns the number of pointers written before the NULL.
 */
int mldr_emulator_argv(const char* emulator, const char* mldr32, int argc, char* const* argv, char** out);

#endif
