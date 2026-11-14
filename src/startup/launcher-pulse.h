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

#ifndef _LAUNCHER_PULSE_H
#define _LAUNCHER_PULSE_H

#include <stdbool.h>
#include <stddef.h>

// Finding the host's PulseAudio server for the guest.
//
// The container's programs run with the guest's HOME, and the PulseAudio client library looks for the
// server's socket and for the authentication cookie relative to the home directory (and to
// XDG_RUNTIME_DIR). On a host without XDG_RUNTIME_DIR, such as Termux, the host's server keeps its
// runtime directory behind a link in the host user's ~/.config/pulse, which the guest cannot find from
// its own HOME. The launcher knows the host's HOME, so it names the host's locations in the variables
// the client library reads: PULSE_RUNTIME_PATH and PULSE_COOKIE. It never overrides a server the user
// chose (PULSE_SERVER, PULSE_RUNTIME_PATH, PULSE_COOKIE) and does nothing when XDG_RUNTIME_DIR already
// leads to a server.

struct launcher_pulse_env
{
	// "PULSE_RUNTIME_PATH=<dir>" and "PULSE_COOKIE=<file>"; empty strings when not to be set.
	char runtimePath[4200];
	char cookie[4200];
};

// Looks in the host user's files, so the caller does so with the user's ids. home and
// xdgRuntimeDir are the launcher's own HOME and XDG_RUNTIME_DIR; server, runtimePath and cookie are
// its PULSE_SERVER, PULSE_RUNTIME_PATH and PULSE_COOKIE (NULL when unset). Returns whether anything
// is to be set.
bool launcherFindPulse(const char* home, const char* xdgRuntimeDir, const char* server,
	const char* runtimePath, const char* cookie, struct launcher_pulse_env* result);

#endif
