// Copyright (C) 2026 Lubos Dolezel
//
// The membership API's name lookups: a group or user name to the UUID of the identity, and the
// membership check on those UUIDs. mbr_group_name_to_uuid and mbr_user_name_to_uuid are private
// (membershipPriv.h) but exported by libSystem; they are what authd uses to check the group of a
// rule.
//
// Expected results were observed with the same calls on macOS 26.7.3 and 10.14.4 (identical).
#include <errno.h>
#include <membership.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>

int mbr_group_name_to_uuid(const char *name, uuid_t uu);
int mbr_user_name_to_uuid(const char *name, uuid_t uu);

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

int main(void)
{
	uuid_t wheel, root, back;
	id_t id = -1;
	int type = -1, ismember = -1;

	check("group name wheel", mbr_group_name_to_uuid("wheel", wheel) == 0);
	check("the UUID of wheel maps back to gid 0 and a group", mbr_uuid_to_id(wheel, &id, &type) == 0 && id == 0 && type == ID_TYPE_GID);
	check("gid 0 gives the same UUID", mbr_gid_to_uuid(0, back) == 0 && uuid_compare(wheel, back) == 0);
	check("user name root", mbr_user_name_to_uuid("root", root) == 0);
	check("the UUID of root maps back to uid 0 and a user", mbr_uuid_to_id(root, &id, &type) == 0 && id == 0 && type == ID_TYPE_UID);
	check("root is a member of wheel", mbr_check_membership(root, wheel, &ismember) == 0 && ismember == 1);
	check("an unknown group name: ENOENT", mbr_group_name_to_uuid("membership-names-test-no-such-group", back) == ENOENT);
	check("an unknown user name: ENOENT", mbr_user_name_to_uuid("membership-names-test-no-such-user", back) == ENOENT);
	check("an empty group name: EINVAL", mbr_group_name_to_uuid("", back) == EINVAL);

	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures != 0;
}
