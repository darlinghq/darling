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

#ifndef _LAUNCHER_IDS_H
#define _LAUNCHER_IDS_H

#include <stdbool.h>
#include <sys/types.h>

// Identity switching for the setuid-root launcher.
//
// The launcher runs in three states:
//   root     real, effective and saved ids are 0
//   user     effective ids are the invoking user's, real and saved ids stay 0,
//            so that root can be restored; used for everything done in the
//            user's prefix on the user's behalf
//   dropped  all ids are the invoking user's and root cannot be regained
//
// The supplementary groups are never changed from the invoking user's, so in
// the user state the process has exactly the user's access rights.
//
// Each function checks every call it makes and reads the resulting ids back.
// On failure it returns false and the caller must not continue.

struct launcher_ids
{
	uid_t uid;
	gid_t gid;
	int ngroups;
	gid_t* groups;
};

// Records the invoking user's real uid, real gid and supplementary groups.
// Must be called before any id is changed.
bool launcherSaveInvokingIds(struct launcher_ids* ids);

// From the state the setuid exec left (effective uid 0), sets the real, effective
// and saved uids and gids to 0.
bool launcherBecomeRoot(void);

// root -> user.
bool launcherUseInvokingIds(const struct launcher_ids* ids);

// user -> root.
bool launcherRestoreRootIds(void);

// root -> dropped.
bool launcherDropPrivileges(const struct launcher_ids* ids);

#endif
