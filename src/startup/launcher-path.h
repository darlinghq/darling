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

#ifndef _LAUNCHER_PATH_H
#define _LAUNCHER_PATH_H

// The path by which the guest reaches a file of the host: its real path (absolute, every symbolic link followed)
// under the host root, which every prefix has as /Volumes/SystemRoot. The host root itself is /Volumes/SystemRoot.
// Returns a string to free, or NULL with errno set as realpath() sets it (ENOENT for a missing file or a dangling
// link). A relative hostPath is taken from workingDirectory (an absolute path), not from the process's working
// directory: the root mode's launcher resolves paths after joining the container's mount namespace, which moves it
// to the namespace's root.
char* launcherGuestPath(const char* hostPath, const char* workingDirectory);

#endif
