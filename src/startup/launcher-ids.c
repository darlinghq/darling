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

#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "launcher-ids.h"

static bool uidsAre(uid_t r, uid_t e, uid_t s)
{
	uid_t ruid, euid, suid;

	if (getresuid(&ruid, &euid, &suid) != 0)
		return false;
	return ruid == r && euid == e && suid == s;
}

static bool gidsAre(gid_t r, gid_t e, gid_t s)
{
	gid_t rgid, egid, sgid;

	if (getresgid(&rgid, &egid, &sgid) != 0)
		return false;
	return rgid == r && egid == e && sgid == s;
}

static bool groupsAre(const struct launcher_ids* ids)
{
	gid_t* current;
	int count;
	bool same;

	count = getgroups(0, NULL);
	if (count != ids->ngroups)
		return false;
	if (count == 0)
		return true;

	current = malloc(sizeof(gid_t) * count);
	if (!current)
		return false;

	same = getgroups(count, current) == count
		&& memcmp(current, ids->groups, sizeof(gid_t) * count) == 0;
	free(current);

	return same;
}

bool launcherSaveInvokingIds(struct launcher_ids* ids)
{
	int count;

	ids->uid = getuid();
	ids->gid = getgid();
	ids->ngroups = 0;
	ids->groups = NULL;

	count = getgroups(0, NULL);
	if (count < 0)
		return false;

	ids->groups = malloc(sizeof(gid_t) * (count > 0 ? count : 1));
	if (!ids->groups)
		return false;

	if (count > 0 && getgroups(count, ids->groups) != count)
	{
		free(ids->groups);
		ids->groups = NULL;
		return false;
	}

	ids->ngroups = count;
	return true;
}

bool launcherBecomeRoot(void)
{
	// The setuid-root exec gave us an effective and saved uid of 0. With the
	// effective uid 0 any uid may be set, so the real uid becomes 0 as well;
	// darlingserver requires real root. Being fully root, any gid may then be set.
	if (setresuid(0, 0, 0) != 0)
		return false;
	if (setresgid(0, 0, 0) != 0)
		return false;

	return uidsAre(0, 0, 0) && gidsAre(0, 0, 0);
}

bool launcherUseInvokingIds(const struct launcher_ids* ids)
{
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;

	// Changing the effective gid to an arbitrary value needs an effective uid of
	// 0, so the gid goes first. Switching the uid the other way round would leave
	// the effective gid at 0 while acting for the user.
	if (setegid(ids->gid) != 0)
		return false;
	// The real and saved uids stay 0, which is what lets launcherRestoreRootIds()
	// come back. While they are 0 the invoking user cannot signal or ptrace this
	// process, so nothing the user runs can act with these saved ids.
	if (seteuid(ids->uid) != 0)
		return false;

	if (getresuid(&ruid, &euid, &suid) != 0 || getresgid(&rgid, &egid, &sgid) != 0)
		return false;
	return euid == ids->uid && egid == ids->gid;
}

bool launcherRestoreRootIds(void)
{
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;

	// The real and saved uids are 0, so an unprivileged effective uid may switch
	// back to 0. Only then may the effective gid change back to 0.
	if (seteuid(0) != 0)
		return false;
	if (setegid(0) != 0)
		return false;

	if (getresuid(&ruid, &euid, &suid) != 0 || getresgid(&rgid, &egid, &sgid) != 0)
		return false;
	return euid == 0 && egid == 0;
}

bool launcherDropPrivileges(const struct launcher_ids* ids)
{
	const uid_t uid = ids->uid;
	const gid_t gid = ids->gid;

	// Runs as root. The supplementary groups and the gids can only be changed
	// while the uid is still 0, so they go first and the uids last. Setting the
	// supplementary groups to the user's own list keeps exactly the access the
	// user had when starting the launcher.
	if (setgroups(ids->ngroups, ids->groups) != 0)
		return false;
	if (setresgid(gid, gid, gid) != 0)
		return false;
	if (setresuid(uid, uid, uid) != 0)
		return false;

	if (!uidsAre(uid, uid, uid) || !gidsAre(gid, gid, gid) || !groupsAre(ids))
		return false;

	// With every uid set to the user's, the kernel must refuse to give root
	// back. If it does not, the drop did not happen.
	if (uid != 0 && setresuid((uid_t) -1, 0, (uid_t) -1) == 0)
		return false;
	if (uid != 0 && gid != 0 && setresgid((gid_t) -1, 0, (gid_t) -1) == 0)
		return false;

	return true;
}
