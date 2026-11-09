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

#ifndef _LAUNCHER_SERVER_H
#define _LAUNCHER_SERVER_H

#include <sys/stat.h>

// Whether the process named in the prefix's pid file is a darlingserver whose
// mount namespace the launcher may join as root.
enum launcher_server_check
{
	LAUNCHER_SERVER_OK = 0,
	// The process is gone.
	LAUNCHER_SERVER_GONE,
	// Its executable is not the installed darlingserver.
	LAUNCHER_SERVER_WRONG_EXE,
	// Its mount namespace is owned by another user namespace. darlingserver
	// creates its namespace as real root, so it is owned by the launcher's.
	LAUNCHER_SERVER_FOREIGN_USERNS,
	// It is in the launcher's own mount namespace.
	LAUNCHER_SERVER_SAME_MNTNS,
	// Something could not be examined.
	LAUNCHER_SERVER_ERROR,
};

// Examines the process through procfd, a descriptor of its /proc/<pid>
// directory. Every lookup goes through that descriptor, so a pid reused after it
// was opened cannot substitute another process. exe, userNs and mntNs are the
// stat() results of the installed darlingserver binary and of the launcher's
// own user and mount namespaces.
//
// On LAUNCHER_SERVER_OK, *nsfd is an open descriptor of the checked mount
// namespace, to be passed to setns(). Otherwise it is -1.
enum launcher_server_check launcherOpenServerMountNs(int procfd, const struct stat* exe,
	const struct stat* userNs, const struct stat* mntNs, int* nsfd);

#endif
