/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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
#ifndef _LIBXCSELECT_H_
#define _LIBXCSELECT_H_
#include <stdbool.h>
#include <stddef.h>

#define XCSELECT_FLAG_REQUIRE_XCODE	1

int xcselect_invoke_xcrun(const char* tool, int argc, char* argv[], int flags);

bool xcselect_get_developer_dir_path(char* path, size_t len, bool* is_cmd_line);
bool xcselect_find_developer_contents_from_path(const char* p, char* dst, bool* is_cmd_line, size_t dst_size);

typedef struct __xcselect_manpaths xcselect_manpaths;
xcselect_manpaths* xcselect_get_manpaths(const char* sdkname);
unsigned int xcselect_manpaths_get_num_paths(xcselect_manpaths* p);
const char* xcselect_manpaths_get_path(xcselect_manpaths* p, unsigned int idx);
void xcselect_manpaths_free(xcselect_manpaths* p);

#endif

