/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

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

#ifndef VIRTUALPREFIX_H
#define VIRTUALPREFIX_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	
#define __SYSTEM_ROOT "/system-root"

// Set prefix path. Should be run only once at startup.
// If this is never called, then __prefix_translate_path()
// is a no-op.
void __prefix_set(const char* path);

// Returns the current prefix or NULL;
const char* __prefix_get(void);

// Translate from path in prefix to physical path.
const char* __prefix_translate_path(const char* path);

// Translate from physical path to path in prefix.
// The path is expected to be canonical.
const char* __prefix_untranslate_path(const char* path, unsigned long count);

// Called whenever current working directory changes.
// This is used to resolve relative paths passed to
// __prefix_translate_path().
void __prefix_cwd(const char* path);

void __prefix_cwd_fd(int fd);

// Is the given path equivalent to __SYSTEM_ROOT?
bool __prefix_is_system_root(const char* path);

int __prefix_get_dyld_path(char* buf, unsigned long size);

#ifdef __cplusplus
}
#endif

#endif /* VIRTUALPREFIX_H */

