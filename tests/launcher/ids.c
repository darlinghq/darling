// Host-side test of the setuid launcher's id switching (src/startup/launcher-ids.c).
//
// The test cannot run setuid root, so it defines the id system calls itself.
// Being part of the executable, these definitions are the ones launcher-ids.c
// links against. They model the Linux rules for a process without file
// capabilities: with an effective uid of 0 any id may be set; otherwise a uid or
// gid may only be set to one of the current real, effective or saved values, and
// the supplementary groups cannot be changed.
#include <errno.h>
#include <grp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "launcher-ids.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

#define USER_UID 1000
#define USER_GID 1001
#define MAX_GROUPS 8
#define MAX_CALLS 32

static struct
{
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;
	int ngroups;
	gid_t groups[MAX_GROUPS];
} k;

static const char* g_calls[MAX_CALLS];
static int g_ncalls;
static const char* g_failCall;  // this call fails with EPERM
static const char* g_ignoreCall; // this call reports success and does nothing

static void resetCalls(void)
{
	g_ncalls = 0;
	g_failCall = NULL;
	g_ignoreCall = NULL;
}

// Records the call; returns -1 if it is to fail, 1 if it is to be ignored.
static int enter(const char* name)
{
	if (g_ncalls < MAX_CALLS)
		g_calls[g_ncalls++] = name;
	if (g_failCall && strcmp(g_failCall, name) == 0)
	{
		errno = EPERM;
		return -1;
	}
	if (g_ignoreCall && strcmp(g_ignoreCall, name) == 0)
		return 1;
	return 0;
}

static int calledAt(const char* name)
{
	for (int i = 0; i < g_ncalls; i++)
	{
		if (strcmp(g_calls[i], name) == 0)
			return i;
	}
	return -1;
}

static bool privileged(void)
{
	return k.euid == 0;
}

static bool uidAllowed(uid_t id)
{
	return id == (uid_t) -1 || privileged() || id == k.ruid || id == k.euid || id == k.suid;
}

static bool gidAllowed(gid_t id)
{
	return id == (gid_t) -1 || privileged() || id == k.rgid || id == k.egid || id == k.sgid;
}

static int doSetresuid(const char* name, uid_t r, uid_t e, uid_t s)
{
	int how = enter(name);
	if (how < 0)
		return -1;
	if (!uidAllowed(r) || !uidAllowed(e) || !uidAllowed(s))
	{
		errno = EPERM;
		return -1;
	}
	if (how > 0)
		return 0;
	if (r != (uid_t) -1) k.ruid = r;
	if (e != (uid_t) -1) k.euid = e;
	if (s != (uid_t) -1) k.suid = s;
	return 0;
}

static int doSetresgid(const char* name, gid_t r, gid_t e, gid_t s)
{
	int how = enter(name);
	if (how < 0)
		return -1;
	if (!gidAllowed(r) || !gidAllowed(e) || !gidAllowed(s))
	{
		errno = EPERM;
		return -1;
	}
	if (how > 0)
		return 0;
	if (r != (gid_t) -1) k.rgid = r;
	if (e != (gid_t) -1) k.egid = e;
	if (s != (gid_t) -1) k.sgid = s;
	return 0;
}

int setresuid(uid_t r, uid_t e, uid_t s) { return doSetresuid("setresuid", r, e, s); }
int seteuid(uid_t e) { return doSetresuid("seteuid", -1, e, -1); }
int setresgid(gid_t r, gid_t e, gid_t s) { return doSetresgid("setresgid", r, e, s); }
int setegid(gid_t e) { return doSetresgid("setegid", -1, e, -1); }

int setgroups(size_t size, const gid_t* list)
{
	int how = enter("setgroups");
	if (how < 0)
		return -1;
	if (!privileged())
	{
		errno = EPERM;
		return -1;
	}
	if (size > MAX_GROUPS)
	{
		errno = EINVAL;
		return -1;
	}
	if (how > 0)
		return 0;
	k.ngroups = (int) size;
	memcpy(k.groups, list, size * sizeof(gid_t));
	return 0;
}

int getgroups(int size, gid_t list[])
{
	if (size == 0)
		return k.ngroups;
	if (size < k.ngroups)
	{
		errno = EINVAL;
		return -1;
	}
	memcpy(list, k.groups, k.ngroups * sizeof(gid_t));
	return k.ngroups;
}

uid_t getuid(void) { return k.ruid; }
gid_t getgid(void) { return k.rgid; }

int getresuid(uid_t* r, uid_t* e, uid_t* s)
{
	*r = k.ruid; *e = k.euid; *s = k.suid;
	return 0;
}

int getresgid(gid_t* r, gid_t* e, gid_t* s)
{
	*r = k.rgid; *e = k.egid; *s = k.sgid;
	return 0;
}

// The state right after a setuid-root (not setgid) exec by the user.
static void setuidExec(void)
{
	k.ruid = USER_UID; k.euid = 0; k.suid = 0;
	k.rgid = k.egid = k.sgid = USER_GID;
	k.ngroups = 2;
	k.groups[0] = USER_GID;
	k.groups[1] = 27;
	resetCalls();
}

// Fully root, with supplementary groups other than the user's.
static void fullyRoot(void)
{
	k.ruid = k.euid = k.suid = 0;
	k.rgid = k.egid = k.sgid = 0;
	k.ngroups = 1;
	k.groups[0] = 0;
	resetCalls();
}

static bool uidsAre(uid_t r, uid_t e, uid_t s) { return k.ruid == r && k.euid == e && k.suid == s; }
static bool gidsAre(gid_t r, gid_t e, gid_t s) { return k.rgid == r && k.egid == e && k.sgid == s; }

static struct launcher_ids savedIds(void)
{
	struct launcher_ids ids;

	setuidExec();
	CHECK(launcherSaveInvokingIds(&ids));
	return ids;
}

static void test_save(void)
{
	struct launcher_ids ids = savedIds();

	CHECK(ids.uid == USER_UID);
	CHECK(ids.gid == USER_GID);
	CHECK(ids.ngroups == 2);
	CHECK(ids.groups[0] == USER_GID && ids.groups[1] == 27);
	free(ids.groups);
}

static void test_become_root(void)
{
	setuidExec();
	CHECK(launcherBecomeRoot());
	CHECK(uidsAre(0, 0, 0));
	CHECK(gidsAre(0, 0, 0));
	// The supplementary groups are not touched.
	CHECK(k.ngroups == 2 && k.groups[1] == 27);

	const char* steps[] = { "setresuid", "setresgid" };
	for (size_t i = 0; i < sizeof(steps) / sizeof(steps[0]); i++)
	{
		setuidExec();
		g_failCall = steps[i];
		CHECK(!launcherBecomeRoot());

		setuidExec();
		g_ignoreCall = steps[i];
		CHECK(!launcherBecomeRoot());
	}
}

static void test_use_invoking_ids(void)
{
	struct launcher_ids ids = savedIds();

	fullyRoot();
	CHECK(launcherUseInvokingIds(&ids));
	CHECK(uidsAre(0, USER_UID, 0));
	// Switching the uid before the gid would leave the effective gid at 0.
	CHECK(gidsAre(0, USER_GID, 0));
	CHECK(calledAt("setegid") >= 0 && calledAt("setegid") < calledAt("seteuid"));

	// A failed step is reported and the uid is not switched after a failed gid
	// switch.
	fullyRoot();
	g_failCall = "setegid";
	CHECK(!launcherUseInvokingIds(&ids));
	CHECK(calledAt("seteuid") < 0);
	CHECK(uidsAre(0, 0, 0));

	fullyRoot();
	g_failCall = "seteuid";
	CHECK(!launcherUseInvokingIds(&ids));

	// A call that reports success without effect is caught by reading the ids back.
	fullyRoot();
	g_ignoreCall = "setegid";
	CHECK(!launcherUseInvokingIds(&ids));

	fullyRoot();
	g_ignoreCall = "seteuid";
	CHECK(!launcherUseInvokingIds(&ids));
	free(ids.groups);
}

static void test_restore_root_ids(void)
{
	struct launcher_ids ids = savedIds();

	fullyRoot();
	CHECK(launcherUseInvokingIds(&ids));
	resetCalls();
	CHECK(launcherRestoreRootIds());
	CHECK(uidsAre(0, 0, 0));
	CHECK(gidsAre(0, 0, 0));
	CHECK(calledAt("seteuid") >= 0 && calledAt("seteuid") < calledAt("setegid"));

	const char* steps[] = { "seteuid", "setegid" };
	for (size_t i = 0; i < sizeof(steps) / sizeof(steps[0]); i++)
	{
		fullyRoot();
		CHECK(launcherUseInvokingIds(&ids));
		resetCalls();
		g_failCall = steps[i];
		CHECK(!launcherRestoreRootIds());

		fullyRoot();
		CHECK(launcherUseInvokingIds(&ids));
		resetCalls();
		g_ignoreCall = steps[i];
		CHECK(!launcherRestoreRootIds());
	}
	free(ids.groups);
}

static void test_drop_privileges(void)
{
	struct launcher_ids ids = savedIds();

	// Whatever the groups are now, the drop sets the user's own list.
	fullyRoot();
	CHECK(launcherDropPrivileges(&ids));
	CHECK(uidsAre(USER_UID, USER_UID, USER_UID));
	CHECK(gidsAre(USER_GID, USER_GID, USER_GID));
	CHECK(k.ngroups == 2 && k.groups[0] == USER_GID && k.groups[1] == 27);
	// Groups and gids can only be changed while the uid is 0.
	CHECK(calledAt("setgroups") >= 0 && calledAt("setgroups") < calledAt("setresgid"));
	CHECK(calledAt("setresgid") < calledAt("setresuid"));
	// Root cannot come back.
	CHECK(seteuid(0) != 0 && setegid(0) != 0);

	const char* steps[] = { "setgroups", "setresgid", "setresuid" };
	for (size_t i = 0; i < sizeof(steps) / sizeof(steps[0]); i++)
	{
		fullyRoot();
		g_failCall = steps[i];
		CHECK(!launcherDropPrivileges(&ids));

		fullyRoot();
		g_ignoreCall = steps[i];
		CHECK(!launcherDropPrivileges(&ids));
	}

	// Dropping only the effective uid (as seteuid() would) leaves the saved uid 0.
	fullyRoot();
	g_ignoreCall = "setresuid";
	k.euid = USER_UID;
	CHECK(!launcherDropPrivileges(&ids));

	free(ids.groups);
}

// Invoked by root itself: every switch is between root and root.
static void test_root_user(void)
{
	struct launcher_ids ids;

	fullyRoot();
	k.ngroups = 1;
	k.groups[0] = 0;
	CHECK(launcherSaveInvokingIds(&ids));
	CHECK(ids.uid == 0 && ids.gid == 0);
	CHECK(launcherBecomeRoot());
	CHECK(launcherUseInvokingIds(&ids));
	CHECK(launcherRestoreRootIds());
	CHECK(uidsAre(0, 0, 0) && gidsAre(0, 0, 0));
	CHECK(launcherDropPrivileges(&ids));
	CHECK(uidsAre(0, 0, 0) && gidsAre(0, 0, 0));
	free(ids.groups);
}

int main(void)
{
	test_save();
	test_become_root();
	test_use_invoking_ids();
	test_restore_root_ids();
	test_drop_privileges();
	test_root_user();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("launcher ids: all checks passed\n");
	return 0;
}
