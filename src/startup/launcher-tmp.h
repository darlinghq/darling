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

#ifndef _LAUNCHER_TMP_H
#define _LAUNCHER_TMP_H

#include <stdbool.h>
#include <sys/types.h>

// The guest's /private/tmp on Android hosts.
//
// An Android application's prefix is in its files directory (/data/data/<package>/files/...), and the
// directory meant for temporary files is its cache directory next to it, which differs per application;
// under Termux it is $PREFIX/tmp. So nothing fixed can name it. The launcher takes the directory it
// was started with in TMPDIR, or else the one named at build time, and links the prefix's /private/tmp
// to it (through the host root every prefix links in as /Volumes/SystemRoot).
//
// All of this runs with the invoking user's ids (also when the launcher is setuid root) on paths the
// user controls: a directory is accepted only if, opened without following a final link, it is a
// directory owned by the user that nobody else can write to, and the prefix's entries are changed
// relative to descriptors without following links.

// Whether path names a directory the guest may use as its temporary directory: absolute, without
// control characters, "." or ".." components; a directory owned by uid, not writable by group or
// others, found without following a link as the last component.
bool launcherTmpDirAcceptable(const char* path, uid_t uid);

// The directory to use: tmpdirEnv (the launcher's TMPDIR, may be NULL) if acceptable, else
// buildDefault (may be NULL or empty) if acceptable. Returns NULL if neither is.
const char* launcherChooseTmpDir(const char* tmpdirEnv, const char* buildDefault, uid_t uid);

// Makes <prefix>/private/tmp a link to /Volumes/SystemRoot<dir>. An existing link is replaced and an
// empty directory is replaced by the link; anything else (a directory with contents, a file) is left
// alone and false is returned. Nothing outside the prefix's own private directory is touched.
bool launcherLinkPrefixTmp(const char* prefix, const char* dir);

#endif
