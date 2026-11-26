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

#ifndef _LAUNCHER_HOSTLINK_H
#define _LAUNCHER_HOSTLINK_H

#include <stdbool.h>

// Link an existing host Unix socket into the guest. `hostPath` is an absolute
// host path and `guestPath` an absolute path inside the prefix. The link is
// made only when the host path is a socket; a link this helper made earlier is
// removed when the socket disappears. Other files at the guest path are left
// alone. All paths are inspected with the invoking user's ids.
bool launcherLinkHostSocket(const char* prefix, const char* hostPath, const char* guestPath);

// The same for a regular host file.
bool launcherLinkHostFile(const char* prefix, const char* hostPath, const char* guestPath);

// Prefer $KRB5_CONFIG when it names an existing host file, otherwise use the
// host's /etc/krb5.conf. A successful link is always made at the guest's
// /private/etc/krb5.conf (/etc is a link to it), so the caller can name that
// stable path in $KRB5_CONFIG. Returns whether a config was linked.
bool launcherLinkHostKrb5Config(const char* prefix, const char* configuredPath);

#endif
