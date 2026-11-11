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

#ifndef _OD_PASSWD_FILE_H
#define _OD_PASSWD_FILE_H

#include <stdbool.h>

// Rewriting of one user's entry in passwd(5) and master.passwd(5) style files,
// used by the local directory node to store account changes. This is plain C
// without CoreFoundation so that it can be tested on the host.
//
// Formats (fields separated by ':'):
//   ODPW_PASSWD  name:passwd:uid:gid:gecos:dir:shell
//   ODPW_MASTER  name:passwd:uid:gid:class:change:expire:gecos:dir:shell

enum odpw_format
{
	ODPW_PASSWD,
	ODPW_MASTER,
};

// Fields to change; NULL keeps the field as it is, "" empties it.
struct odpw_changes
{
	const char* gecos;
	const char* shell;
};

// Replaces the given fields of the entry for `name` in the file at `path`.
// Every other line and every other field of the entry is kept as it was
// (including comments, blank lines and a missing final newline). The new file
// is written next to the old one with the old one's permissions and
// ownership (the ownership as far as the caller is allowed to set it),
// flushed to disk and renamed over it, so readers see either the old or
// the new file.
//
// *found tells whether the file has a well-formed entry for `name`; when it
// has not, the file is left untouched. Returns 0 on success (also when there
// is no such entry) or an errno value: EINVAL when a new value contains ':'
// or a line break, ENOENT when the file does not exist, otherwise what the
// failing system call reported.
int odpw_update_user(const char* path, enum odpw_format format, const char* name,
		const struct odpw_changes* changes, bool* found);

#endif
