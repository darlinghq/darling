// Copyright (C) 2026 Lubos Dolezel
//
// The CFOpenDirectory local node serves the accounts the user database
// knows about, with CF objects and error codes as on macOS:
// - nodes "/Local/Default" (kODNodeTypeLocalNodes) and "/Search"
//   (kODNodeTypeAuthentication); other names and types fail with
//   kODErrorNodeUnknownName / kODErrorNodeUnknownType;
// - user and group records carry the getpwnam()/getgrnam() fields and the
//   membership UUID; a missing record is NULL without an error, a missing
//   attribute is NULL without an error;
// - queries match record names and other attributes;
// - a wrong password fails with kODErrorCredentialsInvalid.
// The program passes unchanged on macOS 27.0.1.
#include <CoreFoundation/CoreFoundation.h>
#include <OpenDirectory/OpenDirectory.h>
#include <grp.h>
#include <membership.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

// Checks and releases the error: NULL when code is 0, else that OD code
static int error_is(CFErrorRef error, CFIndex code)
{
	int ok;
	if (code == 0)
		ok = (error == NULL);
	else
		ok = error != NULL && CFErrorGetCode(error) == code
			&& CFEqual(CFErrorGetDomain(error), CFSTR("com.apple.OpenDirectory"));
	if (!ok && error != NULL)
		printf("  (got error %ld)\n", (long) CFErrorGetCode(error));
	if (error != NULL)
		CFRelease(error);
	return ok;
}

static CFStringRef str(const char* s)
{
	return CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
}

// The record has the attribute, and its values include the string
static int has_value(ODRecordRef record, CFStringRef attribute, const char* value)
{
	CFErrorRef error = NULL;
	CFArrayRef values = ODRecordCopyValues(record, attribute, &error);
	int found = 0;

	if (error != NULL)
		CFRelease(error);
	if (values == NULL)
		return 0;

	CFStringRef wanted = str(value);
	found = CFArrayContainsValue(values, CFRangeMake(0, CFArrayGetCount(values)), wanted);
	CFRelease(wanted);
	CFRelease(values);
	return found;
}

static int count_results(ODNodeRef node, CFTypeRef types, CFStringRef attribute, ODMatchType match,
	CFTypeRef value, CFIndex max)
{
	CFErrorRef error = NULL;
	ODQueryRef query = ODQueryCreateWithNode(NULL, node, types, attribute, match, value, NULL, max, &error);
	CHECK(query != NULL && error_is(error, 0));
	if (query == NULL)
		return -1;

	CFArrayRef results = ODQueryCopyResults(query, false, &error);
	CHECK(results != NULL && error_is(error, 0));
	int count = results ? (int) CFArrayGetCount(results) : -1;
	if (results != NULL)
	{
		for (CFIndex i = 0; i < CFArrayGetCount(results); i++)
			CHECK(CFGetTypeID(CFArrayGetValueAtIndex(results, i)) == ODRecordGetTypeID());
		CFRelease(results);
	}
	CFRelease(query);
	return count;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	CFErrorRef error = NULL;

	// Session and node names
	ODSessionRef session = ODSessionCreate(NULL, NULL, &error);
	CHECK(session != NULL && error_is(error, 0));
	CHECK(session && CFGetTypeID(session) == ODSessionGetTypeID());

	CFArrayRef names = ODSessionCopyNodeNames(NULL, session, &error);
	CHECK(names != NULL && error_is(error, 0));
	if (names)
	{
		CFRange all = CFRangeMake(0, CFArrayGetCount(names));
		CHECK(CFArrayContainsValue(names, all, CFSTR("/Local/Default")));
		CHECK(CFArrayContainsValue(names, all, CFSTR("/Search")));
		CFRelease(names);
	}

	// Nodes
	ODNodeRef local = ODNodeCreateWithName(NULL, session, CFSTR("/Local/Default"), &error);
	CHECK(local != NULL && error_is(error, 0));
	if (local == NULL)
		finish(1);
	CHECK(CFGetTypeID(local) == ODNodeGetTypeID());
	CHECK(CFEqual(ODNodeGetName(local), CFSTR("/Local/Default")));

	ODNodeRef typed = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeLocalNodes, &error);
	CHECK(typed != NULL && error_is(error, 0));
	CHECK(typed && CFEqual(ODNodeGetName(typed), CFSTR("/Local/Default")));

	ODNodeRef search = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeAuthentication, &error);
	CHECK(search != NULL && error_is(error, 0));
	CHECK(search && CFEqual(ODNodeGetName(search), CFSTR("/Search")));

	CHECK(ODNodeCreateWithName(NULL, session, CFSTR("/No/Such/Node"), &error) == NULL);
	CHECK(error_is(error, kODErrorNodeUnknownName));
	CHECK(ODNodeCreateWithNodeType(NULL, NULL, 0x1234, &error) == NULL);
	CHECK(error_is(error, kODErrorNodeUnknownType));

	CFArrayRef types = ODNodeCopySupportedRecordTypes(local, &error);
	CHECK(types != NULL && error_is(error, 0));
	if (types)
	{
		CFRange all = CFRangeMake(0, CFArrayGetCount(types));
		CHECK(CFArrayContainsValue(types, all, kODRecordTypeUsers));
		CHECK(CFArrayContainsValue(types, all, kODRecordTypeGroups));
		CFRelease(types);
	}

	if (search)
	{
		// Every subnode of the search node is reachable.
		CHECK(ODNodeCopyUnreachableSubnodeNames(search, &error) == NULL);
		CHECK(error_is(error, 0));
	}

	// The root user record
	struct passwd* pw = getpwnam("root");
	CHECK(pw != NULL);
	if (pw == NULL)
		finish(1);
	char home[1024], shell[1024];
	snprintf(home, sizeof(home), "%s", pw->pw_dir);
	snprintf(shell, sizeof(shell), "%s", pw->pw_shell);

	ODRecordRef root = ODNodeCopyRecord(local, kODRecordTypeUsers, CFSTR("root"), NULL, &error);
	CHECK(root != NULL && error_is(error, 0));
	if (root)
	{
		CHECK(CFGetTypeID(root) == ODRecordGetTypeID());
		CHECK(CFEqual(ODRecordGetRecordType(root), kODRecordTypeUsers));
		CHECK(CFEqual(ODRecordGetRecordName(root), CFSTR("root")));
		CHECK(has_value(root, kODAttributeTypeRecordName, "root"));
		CHECK(has_value(root, kODAttributeTypeRecordType, "dsRecTypeStandard:Users"));
		CHECK(has_value(root, kODAttributeTypeUniqueID, "0"));
		CHECK(has_value(root, kODAttributeTypePrimaryGroupID, "0"));
		CHECK(has_value(root, kODAttributeTypeNFSHomeDirectory, home));
		CHECK(has_value(root, kODAttributeTypeUserShell, shell));
		CHECK(has_value(root, kODAttributeTypeMetaNodeLocation, "/Local/Default"));

		uuid_t uuid;
		if (mbr_uid_to_uuid(0, uuid) == 0)
		{
			uuid_string_t text;
			uuid_unparse_upper(uuid, text);
			CHECK(has_value(root, kODAttributeTypeGUID, text));
		}

		CHECK(ODRecordCopyValues(root, CFSTR("dsAttrTypeStandard:NoSuchAttribute"), &error) == NULL);
		CHECK(error_is(error, 0));

		CFDictionaryRef details = ODRecordCopyDetails(root, NULL, &error);
		CHECK(details != NULL && error_is(error, 0));
		CHECK(details && CFDictionaryContainsKey(details, kODAttributeTypeUniqueID));
		if (details)
			CFRelease(details);

		const void* one[] = { kODAttributeTypeUniqueID };
		CFArrayRef wanted = CFArrayCreate(NULL, one, 1, &kCFTypeArrayCallBacks);
		details = ODRecordCopyDetails(root, wanted, &error);
		CHECK(details != NULL && error_is(error, 0));
		CHECK(details && CFDictionaryGetCount(details) == 1);
		if (details)
			CFRelease(details);
		CFRelease(wanted);

		CFRelease(root);
	}

	// A wrong password for the calling user (root may be disabled)
	struct passwd* me = getpwuid(getuid());
	CHECK(me != NULL);
	if (me)
	{
		CFStringRef myName = str(me->pw_name);
		ODRecordRef mine = ODNodeCopyRecord(local, kODRecordTypeUsers, myName, NULL, &error);
		CHECK(mine != NULL && error_is(error, 0));
		if (mine)
		{
			CHECK(!ODRecordVerifyPassword(mine, CFSTR("not the right password, surely"), &error));
			CHECK(error_is(error, kODErrorCredentialsInvalid));
			CFRelease(mine);
		}
		CFRelease(myName);
	}

	// Missing records and unsupported types
	CHECK(ODNodeCopyRecord(local, kODRecordTypeUsers, CFSTR("no_such_user_od_test"), NULL, &error) == NULL);
	CHECK(error_is(error, 0));
	CHECK(ODNodeCopyRecord(local, CFSTR("dsRecTypeStandard:NoSuchType"), CFSTR("root"), NULL, &error) == NULL);
	CHECK(error_is(error, kODErrorRecordAttributeUnknownType));

	// The wheel group
	struct group* gr = getgrgid(0);
	CHECK(gr != NULL);
	if (gr)
	{
		CFStringRef groupName = str(gr->gr_name);
		ODRecordRef group = ODNodeCopyRecord(local, kODRecordTypeGroups, groupName, NULL, &error);
		CHECK(group != NULL && error_is(error, 0));
		if (group)
		{
			CHECK(CFEqual(ODRecordGetRecordType(group), kODRecordTypeGroups));
			CHECK(has_value(group, kODAttributeTypePrimaryGroupID, "0"));
			CFRelease(group);
		}
		CFRelease(groupName);
	}

	// Records found through the search node live in the local node.
	if (search)
	{
		ODRecordRef viaSearch = ODNodeCopyRecord(search, kODRecordTypeUsers, CFSTR("root"), NULL, &error);
		CHECK(viaSearch != NULL && error_is(error, 0));
		if (viaSearch)
		{
			CHECK(has_value(viaSearch, kODAttributeTypeMetaNodeLocation, "/Local/Default"));
			CFRelease(viaSearch);
		}
	}

	// Queries
	CHECK(count_results(local, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchEqualTo, CFSTR("root"), 0) == 1);
	CHECK(count_results(local, kODRecordTypeUsers, kODAttributeTypeUniqueID, kODMatchEqualTo, CFSTR("0"), 0) >= 1);
	CHECK(count_results(local, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchBeginsWith, CFSTR("roo"), 0) >= 1);
	CHECK(count_results(local, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchContains,
		CFSTR("no_such_user_od_test"), 0) == 0);
	CHECK(count_results(local, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchAny, NULL, 1) == 1);
	CHECK(count_results(local, kODRecordTypeGroups, kODAttributeTypePrimaryGroupID, kODMatchEqualTo, CFSTR("0"), 0) >= 1);

	ODQueryRef bad = ODQueryCreateWithNode(NULL, local, CFSTR("dsRecTypeStandard:NoSuchType"),
		kODAttributeTypeRecordName, kODMatchAny, NULL, NULL, 0, &error);
	CHECK(bad != NULL && error_is(error, 0));
	if (bad)
	{
		CHECK(ODQueryCopyResults(bad, false, &error) == NULL);
		CHECK(error_is(error, kODErrorRecordAttributeUnknownType));
		CFRelease(bad);
	}

	CHECK(ODQueryCreateWithNode(NULL, NULL, kODRecordTypeUsers, kODAttributeTypeRecordName,
		kODMatchAny, NULL, NULL, 0, &error) == NULL);
	CHECK(error_is(error, kODErrorRecordParameterError));

	if (search)
		CFRelease(search);
	if (typed)
		CFRelease(typed);
	CFRelease(local);
	if (session)
		CFRelease(session);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}
