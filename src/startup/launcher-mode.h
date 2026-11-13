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

#ifndef _LAUNCHER_MODE_H
#define _LAUNCHER_MODE_H

#include <stdbool.h>
#include <stddef.h>

// How the launcher runs the container.
enum launcher_mode
{
	// Started with root's power (installed setuid root, or run by root): namespaces,
	// mounts and the PID namespace are made as root.
	LAUNCHER_MODE_ROOT,
	// No setuid: the launcher's child makes a user namespace (with the user mapped
	// to a single id), a mount namespace and a UTS namespace of its own.
	LAUNCHER_MODE_ROOTLESS,
	// No setuid and no namespaces at all; the prefix is a plain copy. For hosts that
	// do not allow user namespaces.
	LAUNCHER_MODE_NONROOT,
	// Not setuid, and no unprivileged mode was asked for.
	LAUNCHER_MODE_NONE,
};

// Chooses the mode from whether the process has root's power and from the
// DARLING_NONROOT and DARLING_ROOTLESS settings (a value starting with '1', 't' or
// 'T' enables one; DARLING_NONROOT wins if both are set).
//
// The settings are looked at only when the launcher is not privileged. A process
// with root's power (effective uid 0) is always in the root mode, whatever the
// environment says: the environment is the user's, and must not be able to turn
// a privileged launcher into a different one. An unprivileged process can do
// whatever either unprivileged mode does by itself, so letting its environment
// choose between them gives it nothing.
//
// With no setting, an unprivileged launcher built for Android (defaultNonRoot)
// runs in the non-root mode, since such hosts do not allow the setuid install or
// user namespaces; any other host gets LAUNCHER_MODE_NONE.
enum launcher_mode launcherSelectMode(bool privileged, const char* nonRoot, const char* rootless, bool defaultNonRoot);

// The installation prefix to run from. The root mode always uses compiledPrefix.
// An unprivileged launcher may be installed anywhere (an Android app's private
// directory, say), so it takes the prefix from the path of its own executable
// (exePath, which must end in "/bin/darling" and be absolute), and falls back to
// compiledPrefix. Writes the result into out; returns false if it does not fit.
bool launcherInstallPrefix(enum launcher_mode mode, const char* compiledPrefix, const char* exePath, char* out, size_t size);

// The user name to give the container for uid. login is what getpwuid(uid) found
// (NULL if nothing), userEnv and lognameEnv are $USER and $LOGNAME, userEnvIsUid and
// lognameEnvIsUid say whether the passwd entry of that name has this uid, and fallback
// is the controlling terminal's login name. The first usable name wins in that order;
// an environment name is taken only when its entry has the uid, so it cannot make the
// container name another account. Returns NULL when none is usable.
const char* launcherPickLoginName(const char* login, const char* userEnv, bool userEnvIsUid,
	const char* lognameEnv, bool lognameEnvIsUid, const char* fallback);

#endif
