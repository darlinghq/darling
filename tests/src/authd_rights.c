// Copyright (C) 2026 Lubos Dolezel
//
// Authorization Services against authd: AuthorizationCreate, AuthorizationCopyRights,
// AuthorizationCopyInfo, the external form, and the rights database (AuthorizationRightGet,
// AuthorizationRightSet, AuthorizationRightRemove).
//
// The expected results of the first part were observed with the same calls on macOS 26.7.3 and
// 10.14.4 (identical on both) without ever passing kAuthorizationFlagInteractionAllowed, and
// follow the shipped authorization.plist rules: "allow" rights are granted, "deny" rights are
// refused, "user" rights need a credential, which cannot be obtained without interaction.
//
// The second part passes kAuthorizationFlagInteractionAllowed. macOS would ask the user to
// authenticate; Darling has no authentication UI, so authd treats the user who created the
// authorization as authenticated if they satisfy the rule (see OSX/authd/DARLING.md in the
// Security sources). That part only runs for members of the group "admin" and for root. It would
// show an authentication dialog on macOS: build with -DCONTRACT_ONLY to run only the first part
// there.
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Authorization.h>
#include <Security/AuthorizationDB.h>
#include <Security/AuthorizationTags.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static void check_status(const char *name, OSStatus got, OSStatus want)
{
	char buf[200];

	snprintf(buf, sizeof(buf), "%s (status %d, expected %d)", name, (int)got, (int)want);
	check(buf, got == want);
}

static int single_right(const AuthorizationRights *set, const char *name)
{
	return set && set->count == 1 && strcmp(set->items[0].name, name) == 0;
}

static OSStatus copy_one(AuthorizationRef ref, const char *right, AuthorizationFlags flags, AuthorizationRights **out)
{
	AuthorizationItem item = { right, 0, NULL, 0 };
	AuthorizationRights rights = { 1, &item };

	*out = NULL;
	return AuthorizationCopyRights(ref, &rights, kAuthorizationEmptyEnvironment, flags, out);
}

static char *class_of(CFDictionaryRef d, char *buf, size_t size)
{
	CFTypeRef c = d ? CFDictionaryGetValue(d, CFSTR("class")) : NULL;

	buf[0] = 0;
	if (c && CFGetTypeID(c) == CFStringGetTypeID())
		CFStringGetCString(c, buf, size, kCFStringEncodingUTF8);
	return buf;
}

// Whether the user is listed as a member of the group (the account database, which is what
// authd checks, rather than the group list of this process).
static int in_group(const char *group)
{
	struct group *g = getgrnam(group);
	struct passwd *pw = getpwuid(geteuid());

	if (!g || !pw)
		return 0;
	for (char **m = g->gr_mem; m && *m; m++)
		if (strcmp(*m, pw->pw_name) == 0)
			return 1;
	return 0;
}

static void contract_checks(void)
{
	AuthorizationRef ref = NULL;
	AuthorizationRights *out;
	OSStatus s;

	s = AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &ref);
	check_status("Create with no rights", s, errAuthorizationSuccess);
	check("Create returns a reference", ref != NULL);
	check_status("Create with a NULL reference pointer", AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, NULL), errAuthorizationSuccess);
	{
		AuthorizationRef r2 = NULL;
		AuthorizationRights none = { 0, NULL };

		check_status("Create with zero rights and the empty environment", AuthorizationCreate(&none, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &r2), errAuthorizationSuccess);
		if (r2)
			AuthorizationFree(r2, kAuthorizationFlagDefaults);
	}
	if (!ref)
		return;

	// Rights whose rule is "allow" are granted, with or without ExtendRights.
	const char *allow[] = { "system.burn", "system.hdd.smart", "system.privilege.taskport.safe", "config.add.com.example.authd-test", NULL };
	for (int i = 0; allow[i]; i++) {
		char name[200];

		s = copy_one(ref, allow[i], kAuthorizationFlagDefaults, &out);
		snprintf(name, sizeof(name), "CopyRights %s", allow[i]);
		check_status(name, s, errAuthorizationSuccess);
		check("  the right is returned", single_right(out, allow[i]));
		if (out)
			AuthorizationFreeItemSet(out);
		s = copy_one(ref, allow[i], kAuthorizationFlagExtendRights | kAuthorizationFlagPartialRights, &out);
		check_status("  with ExtendRights|PartialRights", s, errAuthorizationSuccess);
		if (out)
			AuthorizationFreeItemSet(out);
	}

	// A right that does not exist and a "deny" right are refused; asking for more than the
	// defaults without interaction is not allowed for a right that is not in the database.
	s = copy_one(ref, "com.example.authd-test.nonexistent", kAuthorizationFlagDefaults, &out);
	check_status("CopyRights of a nonexistent right", s, errAuthorizationDenied);
	s = copy_one(ref, "com.example.authd-test.nonexistent", kAuthorizationFlagExtendRights, &out);
	check_status("CopyRights of a nonexistent right with ExtendRights", s, errAuthorizationInteractionNotAllowed);
	s = copy_one(ref, "config.config.com.example.authd-test", kAuthorizationFlagExtendRights, &out);
	check_status("CopyRights of a right with a deny rule", s, errAuthorizationDenied);

	// A right that needs authentication cannot be obtained without interaction.
	s = copy_one(ref, "system.preferences", kAuthorizationFlagDefaults, &out);
	check_status("CopyRights system.preferences without ExtendRights", s, errAuthorizationDenied);
	s = copy_one(ref, "system.preferences", kAuthorizationFlagExtendRights, &out);
	check_status("CopyRights system.preferences with ExtendRights", s, errAuthorizationInteractionNotAllowed);
	s = copy_one(ref, "system.privilege.admin", kAuthorizationFlagExtendRights, &out);
	check_status("CopyRights system.privilege.admin with ExtendRights", s, errAuthorizationInteractionNotAllowed);
	// A right that times out immediately can be pre-authorized.
	s = copy_one(ref, "config.modify.com.example.authd-test", kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize, &out);
	check_status("CopyRights config.modify.* with PreAuthorize", s, errAuthorizationSuccess);
	if (out)
		AuthorizationFreeItemSet(out);

	// Several rights; the ones that cannot be granted are left out with PartialRights.
	{
		AuthorizationItem items[2] = { { "system.burn", 0, NULL, 0 }, { "system.preferences", 0, NULL, 0 } };
		AuthorizationRights rights = { 2, items };

		out = NULL;
		s = AuthorizationCopyRights(ref, &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights | kAuthorizationFlagPartialRights, &out);
		check_status("CopyRights two rights, PartialRights", s, errAuthorizationSuccess);
		check("  only the granted right is returned", single_right(out, "system.burn"));
		if (out)
			AuthorizationFreeItemSet(out);
		s = AuthorizationCopyRights(ref, &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights, &out);
		check_status("CopyRights two rights, no PartialRights", s, errAuthorizationInteractionNotAllowed);
	}

	// Arguments.
	{
		AuthorizationItem item = { "system.burn", 0, NULL, 0 };
		AuthorizationRights rights = { 1, &item }, zero = { 0, NULL };
		AuthorizationItem empty = { "", 0, NULL, 0 };
		AuthorizationRights empties = { 1, &empty };

		out = NULL;
		s = AuthorizationCopyRights(NULL, &rights, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights, &out);
		check_status("CopyRights with a NULL reference", s, errAuthorizationInvalidRef);
		s = AuthorizationCopyRights(ref, NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &out);
		check_status("CopyRights with NULL rights", s, errAuthorizationSuccess);
		s = AuthorizationCopyRights(ref, &zero, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights, &out);
		check_status("CopyRights with zero rights", s, errAuthorizationSuccess);
		check("  no rights are returned", out && out->count == 0);
		if (out)
			AuthorizationFreeItemSet(out);
		s = AuthorizationCopyRights(ref, &rights, NULL, kAuthorizationFlagExtendRights, NULL);
		check_status("CopyRights with NULL environment and output", s, errAuthorizationSuccess);
		s = AuthorizationCopyRights(ref, &empties, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights, &out);
		check_status("CopyRights with an empty right name", s, errAuthorizationInteractionNotAllowed);
	}

	// AuthorizationCopyInfo
	{
		AuthorizationItemSet *info = NULL;

		s = AuthorizationCopyInfo(ref, NULL, &info);
		check_status("CopyInfo with no tag", s, errAuthorizationSuccess);
		check("  no items", info && info->count == 0);
		if (info)
			AuthorizationFreeItemSet(info);
		info = NULL;
		s = AuthorizationCopyInfo(ref, "com.example.authd-test.tag", &info);
		check_status("CopyInfo with an unknown tag", s, errAuthorizationSuccess);
		if (info)
			AuthorizationFreeItemSet(info);
		check_status("CopyInfo with a NULL reference", AuthorizationCopyInfo(NULL, NULL, &info), errAuthorizationInvalidRef);
		check_status("CopyInfo with a NULL output", AuthorizationCopyInfo(ref, NULL, NULL), errAuthorizationInvalidSet);
	}

	// External form
	{
		AuthorizationExternalForm ext, zeroed, junk;
		AuthorizationRef r2 = NULL;

		memset(&ext, 0, sizeof(ext));
		s = AuthorizationMakeExternalForm(ref, &ext);
		check_status("MakeExternalForm", s, errAuthorizationSuccess);
		check_status("MakeExternalForm with a NULL form", AuthorizationMakeExternalForm(ref, NULL), errAuthorizationInvalidPointer);
		check_status("MakeExternalForm with a NULL reference", AuthorizationMakeExternalForm(NULL, &ext), errAuthorizationInvalidRef);
		s = AuthorizationCreateFromExternalForm(&ext, &r2);
		check_status("CreateFromExternalForm", s, errAuthorizationSuccess);
		check("  a reference is returned", r2 != NULL);
		if (r2) {
			s = copy_one(r2, "system.burn", kAuthorizationFlagExtendRights, &out);
			check_status("  CopyRights with the new reference", s, errAuthorizationSuccess);
			if (out)
				AuthorizationFreeItemSet(out);
			check_status("  Free", AuthorizationFree(r2, kAuthorizationFlagDefaults), errAuthorizationSuccess);
		}
		r2 = NULL;
		s = AuthorizationCreateFromExternalForm(&ext, &r2);
		check_status("CreateFromExternalForm again with the same form", s, errAuthorizationSuccess);
		if (r2)
			AuthorizationFree(r2, kAuthorizationFlagDefaults);
		check_status("CreateFromExternalForm with a NULL form", AuthorizationCreateFromExternalForm(NULL, &r2), errAuthorizationInvalidPointer);
		check_status("CreateFromExternalForm with a NULL output", AuthorizationCreateFromExternalForm(&ext, NULL), errAuthorizationInvalidRef);
		memset(&zeroed, 0, sizeof(zeroed));
		r2 = NULL;
		s = AuthorizationCreateFromExternalForm(&zeroed, &r2);
		check_status("CreateFromExternalForm with a zeroed form", s, errAuthorizationDenied);
		check("  no reference is returned", r2 == NULL);
		memset(&junk, 0xA5, sizeof(junk));
		s = AuthorizationCreateFromExternalForm(&junk, &r2);
		check_status("CreateFromExternalForm with an invalid form", s, errAuthorizationDenied);
		check("  no reference is returned", r2 == NULL);
	}

	// Rights database, read only
	{
		CFDictionaryRef d = NULL;
		char cls[64];
		struct { const char *name; const char *cls; } known[] = {
			{ "system.burn", "allow" }, { "system.preferences", "user" }, { "system.privilege.admin", "user" },
			{ "config.config.", "deny" }, { "system.login.console", "evaluate-mechanisms" },
		};

		for (unsigned i = 0; i < sizeof(known) / sizeof(known[0]); i++) {
			char name[200];

			d = NULL;
			s = AuthorizationRightGet(known[i].name, &d);
			snprintf(name, sizeof(name), "RightGet %s (class %s)", known[i].name, known[i].cls);
			check_status(name, s, errAuthorizationSuccess);
			check("  the class matches", d && strcmp(class_of(d, cls, sizeof(cls)), known[i].cls) == 0);
			if (d)
				CFRelease(d);
		}
		d = NULL;
		s = AuthorizationRightGet("system.preferences", &d);
		{
			CFTypeRef g = d ? CFDictionaryGetValue(d, CFSTR("group")) : NULL;
			char grp[64] = "";

			if (g && CFGetTypeID(g) == CFStringGetTypeID())
				CFStringGetCString(g, grp, sizeof(grp), kCFStringEncodingUTF8);
			check("RightGet system.preferences has group admin", strcmp(grp, "admin") == 0);
		}
		if (d)
			CFRelease(d);
		check_status("RightGet of a nonexistent right", AuthorizationRightGet("com.example.authd-test.nonexistent", &d), errAuthorizationDenied);
		check_status("RightGet with a NULL name", AuthorizationRightGet(NULL, &d), errAuthorizationInvalidPointer);
		check_status("RightGet with a NULL output", AuthorizationRightGet("system.burn", NULL), errAuthorizationSuccess);
		check_status("RightSet with a NULL name", AuthorizationRightSet(ref, NULL, CFSTR("allow"), NULL, NULL, NULL), errAuthorizationInvalidPointer);
		check_status("RightSet with a NULL reference", AuthorizationRightSet(NULL, "com.example.authd-test.x", CFSTR("allow"), NULL, NULL, NULL), errAuthorizationInvalidRef);
		check_status("RightRemove with a NULL name", AuthorizationRightRemove(ref, NULL), errAuthorizationInvalidPointer);
		check_status("RightRemove with a NULL reference", AuthorizationRightRemove(NULL, "com.example.authd-test.x"), errAuthorizationInvalidRef);
		check_status("RightRemove of a nonexistent right", AuthorizationRightRemove(ref, "com.example.authd-test.nonexistent"), errAuthorizationSuccess);
		check_status("RightRemove of a right that needs authentication", AuthorizationRightRemove(ref, "system.burn"), errAuthorizationDenied);
	}

	check_status("Free", AuthorizationFree(ref, kAuthorizationFlagDefaults), errAuthorizationSuccess);
	check_status("Free with a NULL reference", AuthorizationFree(NULL, kAuthorizationFlagDefaults), errAuthorizationInvalidRef);
}

#ifndef CONTRACT_ONLY
// Darling policy: no authentication UI, the creator is treated as authenticated.
static void headless_checks(void)
{
	AuthorizationRef ref = NULL;
	AuthorizationRights *out;
	OSStatus s;
	const AuthorizationFlags interactive = kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed;

	if (geteuid() != 0 && !in_group("admin")) {
		printf("SKIP: the user is not a member of the group admin\n");
		return;
	}
	check_status("Create", AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &ref), errAuthorizationSuccess);
	if (!ref)
		return;

	s = copy_one(ref, "system.preferences", interactive, &out);
	check_status("CopyRights system.preferences with interaction allowed", s, errAuthorizationSuccess);
	check("  the right is returned", single_right(out, "system.preferences"));
	if (out)
		AuthorizationFreeItemSet(out);
	s = copy_one(ref, "system.privilege.admin", interactive, &out);
	check_status("CopyRights system.privilege.admin with interaction allowed", s, errAuthorizationSuccess);
	if (out)
		AuthorizationFreeItemSet(out);
	// Rules that deny stay denied.
	s = copy_one(ref, "config.config.com.example.authd-test", interactive, &out);
	check_status("CopyRights of a deny right with interaction allowed", s, errAuthorizationDenied);
	// A right that is not in the database is decided by the "default" rule, which needs an admin.
	s = copy_one(ref, "com.example.authd-test.nonexistent", interactive, &out);
	check_status("CopyRights of a nonexistent right with interaction allowed", s, errAuthorizationSuccess);
	if (out)
		AuthorizationFreeItemSet(out);

	// Rights database round trip.
	{
		char name[100];
		CFDictionaryRef d = NULL;
		char cls[64];

		snprintf(name, sizeof(name), "com.example.authd-test.right%d", (int)getpid());
		CFStringRef keys[1] = { CFSTR("class") };
		CFStringRef values[1] = { CFSTR("allow") };
		CFDictionaryRef def = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		s = AuthorizationRightSet(ref, name, def, CFSTR("test right"), NULL, NULL);
		check_status("RightSet adds a right", s, errAuthorizationSuccess);
		s = AuthorizationRightGet(name, &d);
		check_status("RightGet reads it back", s, errAuthorizationSuccess);
		check("  its class is allow", d && strcmp(class_of(d, cls, sizeof(cls)), "allow") == 0);
		if (d)
			CFRelease(d);
		s = copy_one(ref, name, kAuthorizationFlagDefaults, &out);
		check_status("CopyRights of the new right", s, errAuthorizationSuccess);
		if (out)
			AuthorizationFreeItemSet(out);
		s = AuthorizationRightRemove(ref, name);
		check_status("RightRemove removes it", s, errAuthorizationSuccess);
		check_status("RightGet no longer finds it", AuthorizationRightGet(name, &d), errAuthorizationDenied);
		CFRelease(def);
	}
	AuthorizationFree(ref, kAuthorizationFlagDefaults);
}
#endif

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	contract_checks();
#ifndef CONTRACT_ONLY
	headless_checks();
#endif
	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures != 0;
}
