// Copyright (C) 2026 Darling Developers
//
// The LSCopy* lookups take a nullable outError: with NULL they must not
// crash, and with the address of a NULL CFErrorRef a failed lookup must
// store an error there.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define NO_SUCH_ID CFSTR("org.darlinghq.tests.no-such-application")
#define NO_SUCH_TYPE CFSTR("org.darlinghq.tests.no-such-type")

static void checkError(CFErrorRef err)
{
	CHECK(err != NULL);
	if (err)
	{
		CHECK(CFEqual(CFErrorGetDomain(err), kCFErrorDomainOSStatus));
		CHECK(CFErrorGetCode(err) != noErr);
		CFRelease(err);
	}
}

int main(void)
{
	CFErrorRef err;

	// LSCopyApplicationURLsForBundleIdentifier
	CHECK(LSCopyApplicationURLsForBundleIdentifier(NO_SUCH_ID, NULL) == NULL);
	err = NULL;
	CHECK(LSCopyApplicationURLsForBundleIdentifier(NO_SUCH_ID, &err) == NULL);
	checkError(err);

	// LSCopyDefaultApplicationURLForContentType
	CHECK(LSCopyDefaultApplicationURLForContentType(NO_SUCH_TYPE, kLSRolesAll, NULL) == NULL);
	err = NULL;
	CHECK(LSCopyDefaultApplicationURLForContentType(NO_SUCH_TYPE, kLSRolesAll, &err) == NULL);
	checkError(err);

	// LSCopyDefaultApplicationURLForURL, for a file type nothing handles
	CFURLRef url = CFURLCreateWithString(NULL, CFSTR("file:///tmp/test.org-darlinghq-no-such-extension"), NULL);
	CHECK(url != NULL);
	if (url)
	{
		CHECK(LSCopyDefaultApplicationURLForURL(url, kLSRolesAll, NULL) == NULL);
		err = NULL;
		CHECK(LSCopyDefaultApplicationURLForURL(url, kLSRolesAll, &err) == NULL);
		checkError(err);
		CFRelease(url);
	}

	if (g_failures == 0)
		puts("PASS");
	return g_failures != 0;
}
