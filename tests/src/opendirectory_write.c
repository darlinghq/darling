// Copyright (C) 2026 Lubos Dolezel
//
// Changing the calling user's own account through the CFOpenDirectory local
// node, as chsh(1) and chfn(1) do:
// - ODRecordSetValue() changes the record at hand (ODRecordCopyValues()
//   shows the new value) and ODRecordSynchronize() stores it, so that
//   getpwnam() returns the new shell and gecos;
// - the gecos parts are the full name, building, phone and home phone
//   attributes; an empty value list removes an attribute;
// - a user may not change another user's record unless root
//   (kODErrorRecordPermissionError), group records are read-only
//   (kODErrorRecordReadOnlyNode), values with ':' or a line break and more
//   than one value are kODErrorRecordAttributeValueSchemaError;
// - ODRecordSetNodeCredentials() fails with kODErrorCredentialsInvalid for an
//   unknown account.
// The values the test changes are put back at the end. It modifies the user
// database of the system it runs on, so it is meant for a Darling prefix.
#include <CoreFoundation/CoreFoundation.h>
#include <OpenDirectory/OpenDirectory.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

// Checks and releases the error: NULL when code is 0, else that OD code
static int error_is(CFErrorRef error, CFIndex code)
{
	int ok = code == 0 ? error == NULL
		: error != NULL && CFErrorGetCode(error) == code
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

static int first_value_is(ODRecordRef record, CFStringRef attribute, const char* expected)
{
	CFErrorRef error = NULL;
	CFArrayRef values = ODRecordCopyValues(record, attribute, &error);
	int ok;

	if (error != NULL)
		CFRelease(error);
	if (expected == NULL)
		ok = values == NULL || CFArrayGetCount(values) == 0;
	else
	{
		CFStringRef wanted = str(expected);
		ok = values != NULL && CFArrayGetCount(values) == 1 && CFEqual(CFArrayGetValueAtIndex(values, 0), wanted);
		CFRelease(wanted);
	}
	if (values != NULL)
		CFRelease(values);
	return ok;
}

static int set_and_sync(ODRecordRef record, CFStringRef attribute, const char* value)
{
	CFErrorRef error = NULL;
	CFStringRef s = value ? str(value) : NULL;
	CFArrayRef values = CFArrayCreate(NULL, (const void**) &s, value ? 1 : 0, &kCFTypeArrayCallBacks);
	int ok = ODRecordSetValue(record, attribute, values, &error) && error == NULL
		&& ODRecordSynchronize(record, &error) && error == NULL;
	if (error != NULL)
		CFRelease(error);
	CFRelease(values);
	if (s != NULL)
		CFRelease(s);
	return ok;
}

static ODRecordRef copy_user(ODNodeRef node, const char* name)
{
	CFErrorRef error = NULL;
	CFStringRef n = str(name);
	ODRecordRef record = ODNodeCopyRecord(node, kODRecordTypeUsers, n, NULL, &error);
	CFRelease(n);
	if (error != NULL)
		CFRelease(error);
	return record;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	CFErrorRef error = NULL;
	ODNodeRef node = ODNodeCreateWithName(NULL, kODSessionDefault, CFSTR("/Local/Default"), &error);
	CHECK(node != NULL && error_is(error, 0));
	if (node == NULL)
		return 1;

	struct passwd* pw = getpwuid(getuid());
	CHECK(pw != NULL);
	if (pw == NULL)
		return 1;
	char name[256];
	strlcpy(name, pw->pw_name, sizeof(name));

	ODRecordRef record = copy_user(node, name);
	CHECK(record != NULL);
	if (record == NULL)
		return 1;

	// Originals, to put back
	CFStringRef gecosAttributes[4] = { kODAttributeTypeFullName, kODAttributeTypeBuilding,
		kODAttributeTypePhoneNumber, kODAttributeTypeHomePhoneNumber };
	CFArrayRef original[5];
	for (int i = 0; i < 4; i++)
		original[i] = ODRecordCopyValues(record, gecosAttributes[i], NULL);
	original[4] = ODRecordCopyValues(record, kODAttributeTypeUserShell, NULL);

	// Not changed until written: another record of the user still has the old shell
	CHECK(ODRecordSetValue(record, kODAttributeTypeUserShell, CFSTR("/bin/bash"), &error) && error_is(error, 0));
	CHECK(first_value_is(record, kODAttributeTypeUserShell, "/bin/bash"));
	pw = getpwnam(name);
	CHECK(pw != NULL && strcmp(pw->pw_shell, "/bin/bash") != 0);

	CHECK(ODRecordSynchronize(record, &error) && error_is(error, 0));
	pw = getpwnam(name);
	CHECK(pw != NULL && strcmp(pw->pw_shell, "/bin/bash") == 0);
	ODRecordRef again = copy_user(node, name);
	CHECK(again != NULL && first_value_is(again, kODAttributeTypeUserShell, "/bin/bash"));
	if (again != NULL)
		CFRelease(again);

	// Synchronizing without changes is a success
	CHECK(ODRecordSynchronize(record, &error) && error_is(error, 0));

	// The gecos parts
	CFStringRef parts[4] = { CFSTR("Test User"), CFSTR("Room 1"), CFSTR("555-0100"), CFSTR("555-0101") };
	for (int i = 0; i < 4; i++)
		CHECK(ODRecordSetValue(record, gecosAttributes[i], parts[i], &error) && error_is(error, 0));
	CHECK(ODRecordSynchronize(record, &error) && error_is(error, 0));
	pw = getpwnam(name);
	CHECK(pw != NULL && strcmp(pw->pw_gecos, "Test User,Room 1,555-0100,555-0101") == 0);
	again = copy_user(node, name);
	CHECK(again != NULL && first_value_is(again, kODAttributeTypeBuilding, "Room 1")
		&& first_value_is(again, kODAttributeTypeHomePhoneNumber, "555-0101"));
	if (again != NULL)
		CFRelease(again);

	// Removing a part keeps the others
	CFArrayRef none = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	CHECK(ODRecordSetValue(record, kODAttributeTypeBuilding, none, &error) && error_is(error, 0));
	CHECK(first_value_is(record, kODAttributeTypeBuilding, NULL));
	CHECK(ODRecordSynchronize(record, &error) && error_is(error, 0));
	pw = getpwnam(name);
	CHECK(pw != NULL && strcmp(pw->pw_gecos, "Test User,,555-0100,555-0101") == 0);
	CFRelease(none);

	// Values that cannot be stored, and unsupported changes
	CHECK(!ODRecordSetValue(record, kODAttributeTypeUserShell, CFSTR("/bin/sh:x"), &error));
	CHECK(error_is(error, kODErrorRecordAttributeValueSchemaError));
	CHECK(!ODRecordSetValue(record, kODAttributeTypeFullName, CFSTR("a\nb"), &error));
	CHECK(error_is(error, kODErrorRecordAttributeValueSchemaError));
	CHECK(!ODRecordSetValue(record, kODAttributeTypeFullName, CFSTR("a,b"), &error));
	CHECK(error_is(error, kODErrorRecordAttributeValueSchemaError));
	CFStringRef two[2] = { CFSTR("/bin/sh"), CFSTR("/bin/zsh") };
	CFArrayRef twoValues = CFArrayCreate(NULL, (const void**) two, 2, &kCFTypeArrayCallBacks);
	CHECK(!ODRecordSetValue(record, kODAttributeTypeUserShell, twoValues, &error));
	CHECK(error_is(error, kODErrorRecordAttributeValueSchemaError));
	CFRelease(twoValues);
	CHECK(!ODRecordSetValue(record, kODAttributeTypeUniqueID, CFSTR("0"), &error));
	CHECK(error_is(error, kODErrorRecordPermissionError));
	CHECK(!ODRecordSetValue(record, NULL, CFSTR("x"), &error));
	CHECK(error_is(error, kODErrorRecordParameterError));
	pw = getpwnam(name);
	CHECK(pw != NULL && strcmp(pw->pw_shell, "/bin/bash") == 0 && pw->pw_uid == getuid());

	// Group records are read-only
	CFStringRef groupName = str("wheel");
	ODRecordRef group = ODNodeCopyRecord(node, kODRecordTypeGroups, groupName, NULL, &error);
	CHECK(error_is(error, 0));
	if (group != NULL)
	{
		CHECK(!ODRecordSetValue(group, kODAttributeTypeFullName, CFSTR("x"), &error));
		CHECK(error_is(error, kODErrorRecordReadOnlyNode));
		CFRelease(group);
	}
	CFRelease(groupName);

	// Another user's record is only for root
	ODRecordRef root = copy_user(node, "root");
	CHECK(root != NULL);
	if (root != NULL && geteuid() != 0)
	{
		CHECK(!ODRecordSetValue(root, kODAttributeTypeUserShell, CFSTR("/bin/bash"), &error));
		CHECK(error_is(error, kODErrorRecordPermissionError));
		pw = getpwnam("root");
		CHECK(pw != NULL && strcmp(pw->pw_shell, "/bin/bash") != 0);
	}
	if (root != NULL)
		CFRelease(root);

	// Credentials of accounts that do not exist
	CHECK(!ODRecordSetNodeCredentials(record, CFSTR("no-such-user-for-od-test"), CFSTR("x"), &error));
	CHECK(error_is(error, kODErrorCredentialsInvalid));

	// Put everything back
	for (int i = 0; i < 4; i++)
	{
		CFStringRef value = original[i] && CFArrayGetCount(original[i]) ? CFArrayGetValueAtIndex(original[i], 0) : NULL;
		char buffer[256] = "";
		if (value)
			CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8);
		CHECK(set_and_sync(record, gecosAttributes[i], value ? buffer : NULL));
	}
	{
		CFStringRef value = original[4] && CFArrayGetCount(original[4]) ? CFArrayGetValueAtIndex(original[4], 0) : NULL;
		char buffer[256] = "";
		if (value)
			CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8);
		CHECK(set_and_sync(record, kODAttributeTypeUserShell, value ? buffer : NULL));
	}
	for (int i = 0; i < 5; i++)
		if (original[i] != NULL)
			CFRelease(original[i]);
	CFRelease(record);
	CFRelease(node);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	fflush(stdout);
	return g_failures ? 1 : 0;
}
