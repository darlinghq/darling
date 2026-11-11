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

#ifndef _SHELLSPAWN_SHELL_H
#define _SHELLSPAWN_SHELL_H

#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Choice of the shell spawnShell() starts when the client did not name a
// program. Kept in a header so that it can be tested on the host.

#define SHELLSPAWN_DEFAULT_SHELL "/bin/zsh"

// The shell to start for an account whose shell field is pw_shell: that shell
// when it is an absolute path to an executable regular file, otherwise the
// default shell. pw_shell may be NULL.
static inline const char* shellspawn_choose_shell(const char* pw_shell)
{
	struct stat st;

	if (pw_shell == NULL || pw_shell[0] != '/')
		return SHELLSPAWN_DEFAULT_SHELL;
	if (stat(pw_shell, &st) != 0 || !S_ISREG(st.st_mode) || access(pw_shell, X_OK) != 0)
		return SHELLSPAWN_DEFAULT_SHELL;
	return pw_shell;
}

// Writes the argv[0] that makes the shell a login shell (a leading dash
// before the program's base name) into buffer. Returns 0, or -1 when the
// buffer is too small.
static inline int shellspawn_login_argv0(const char* shell, char* buffer, size_t size)
{
	const char* base = strrchr(shell, '/');
	base = base != NULL ? base + 1 : shell;

	if (size < strlen(base) + 2)
		return -1;
	buffer[0] = '-';
	strcpy(buffer + 1, base);
	return 0;
}

#endif
