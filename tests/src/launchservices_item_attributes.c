// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// LSCopyItemAttribute and LSCopyItemAttributes: the item attribute keys and the results for hidden
// and ordinary files. The expected values are what macOS reports.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

static int equalsString(CFStringRef value, const char *expected)
{
	CFStringRef e = CFStringCreateWithCString(NULL, expected, kCFStringEncodingUTF8);
	int r = value && CFEqual(value, e);
	CFRelease(e);
	return r;
}

int main(void)
{
	char dir[] = "/tmp/lsattrXXXXXX";
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	char plain[1024], hidden[1024];
	snprintf(plain, sizeof(plain), "%s/plain.txt", dir);
	snprintf(hidden, sizeof(hidden), "%s/.hidden", dir);
	FILE *f = fopen(plain, "w");
	fclose(f);
	f = fopen(hidden, "w");
	fclose(f);

	CHECK(equalsString(kLSItemIsInvisible, "LSItemIsInvisible"));
	CHECK(equalsString(kLSItemExtensionIsHidden, "LSItemExtensionIsHidden"));
	CHECK(equalsString(kLSItemQuarantineProperties, "LSItemQuarantineProperties"));

	FSRef plainRef, hiddenRef;
	CHECK(FSPathMakeRef((const UInt8 *) plain, &plainRef, NULL) == noErr);
	CHECK(FSPathMakeRef((const UInt8 *) hidden, &hiddenRef, NULL) == noErr);

	CFTypeRef value = NULL;
	CHECK(LSCopyItemAttribute(&plainRef, kLSRolesAll, kLSItemIsInvisible, &value) == noErr);
	CHECK(value == kCFBooleanFalse);
	if (value) CFRelease(value);

	value = NULL;
	CHECK(LSCopyItemAttribute(&hiddenRef, kLSRolesAll, kLSItemIsInvisible, &value) == noErr);
	CHECK(value == kCFBooleanTrue);
	if (value) CFRelease(value);

	value = NULL;
	CHECK(LSCopyItemAttribute(&plainRef, kLSRolesAll, kLSItemExtensionIsHidden, &value) == noErr);
	CHECK(value == kCFBooleanFalse);
	if (value) CFRelease(value);

	value = NULL;
	CHECK(LSCopyItemAttribute(&plainRef, kLSRolesAll, kLSItemQuarantineProperties, &value) == kLSDataUnavailableErr);
	CHECK(value == NULL);

	CHECK(LSCopyItemAttribute(NULL, kLSRolesAll, kLSItemIsInvisible, &value) == paramErr);

	CHECK(LSCopyItemAttribute(&plainRef, kLSRolesAll, CFSTR("NoSuchAttribute"), &value) == paramErr);

	CFStringRef available[] = { kLSItemIsInvisible, kLSItemExtensionIsHidden };
	CFArrayRef availableNames = CFArrayCreate(NULL, (const void **) available, 2, &kCFTypeArrayCallBacks);
	CFDictionaryRef values = NULL;
	CHECK(LSCopyItemAttributes(&hiddenRef, kLSRolesAll, availableNames, &values) == noErr);
	CHECK(values != NULL && CFDictionaryGetCount(values) == 2);
	if (values) {
		CHECK(CFDictionaryGetValue(values, kLSItemIsInvisible) == kCFBooleanTrue);
		CHECK(CFDictionaryGetValue(values, kLSItemExtensionIsHidden) == kCFBooleanFalse);
		CFRelease(values);
	}
	CFRelease(availableNames);

	// One unavailable attribute fails the whole call, without a result.
	CFStringRef withQuarantine[] = { kLSItemIsInvisible, kLSItemQuarantineProperties };
	CFArrayRef quarantineNames = CFArrayCreate(NULL, (const void **) withQuarantine, 2, &kCFTypeArrayCallBacks);
	values = NULL;
	CHECK(LSCopyItemAttributes(&hiddenRef, kLSRolesAll, quarantineNames, &values) == kLSDataUnavailableErr);
	CHECK(values == NULL);
	CFRelease(quarantineNames);
	CHECK(LSCopyItemAttributes(&hiddenRef, kLSRolesAll, NULL, &values) == paramErr);

	unlink(plain);
	unlink(hidden);
	rmdir(dir);
	if (failures == 0)
		puts("OK");
	return failures != 0;
}
