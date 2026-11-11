// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// LSSharedFileList and LSSharedFileListItem as Core Foundation types: create, insert, snapshot,
// resolve, remove and release. The lists are in-memory only.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

static CFURLRef makeURL(const char *path)
{
	CFStringRef s = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, s, kCFURLPOSIXPathStyle, true);
	CFRelease(s);
	return url;
}

int main(void)
{
	CHECK(LSSharedFileListGetTypeID() != 0);
	CHECK(LSSharedFileListItemGetTypeID() != 0);
	CHECK(LSSharedFileListGetTypeID() != LSSharedFileListItemGetTypeID());

	LSSharedFileListRef list = LSSharedFileListCreate(NULL, kLSSharedFileListFavoriteItems, NULL);
	CHECK(list != NULL);
	if (!list)
		return 1;
	CHECK(CFGetTypeID(list) == LSSharedFileListGetTypeID());

	UInt32 seed = 0;
	CFArrayRef snapshot = LSSharedFileListCopySnapshot(list, &seed);
	CHECK(snapshot != NULL && CFArrayGetCount(snapshot) == 0);
	if (snapshot)
		CFRelease(snapshot);
	UInt32 initialSeed = LSSharedFileListGetSeedValue(list);
	CHECK(initialSeed == seed);

	CFURLRef urlA = makeURL("/tmp/lssfl-a");
	CFURLRef urlB = makeURL("/tmp/lssfl-b");
	CFURLRef urlC = makeURL("/tmp/lssfl-c");
	LSSharedFileListItemRef a = LSSharedFileListInsertItemURL(list, kLSSharedFileListItemLast, CFSTR("A"), NULL, urlA, NULL, NULL);
	LSSharedFileListItemRef b = LSSharedFileListInsertItemURL(list, kLSSharedFileListItemLast, CFSTR("B"), NULL, urlB, NULL, NULL);
	LSSharedFileListItemRef c = LSSharedFileListInsertItemURL(list, kLSSharedFileListItemBeforeFirst, CFSTR("C"), NULL, urlC, NULL, NULL);
	CHECK(a && b && c);
	CHECK(CFGetTypeID(a) == LSSharedFileListItemGetTypeID());
	CHECK(LSSharedFileListItemGetID(a) != LSSharedFileListItemGetID(b));
	CHECK(LSSharedFileListGetSeedValue(list) != initialSeed);

	// Snapshot order: C (before first), A, B.
	snapshot = LSSharedFileListCopySnapshot(list, &seed);
	CHECK(snapshot != NULL && CFArrayGetCount(snapshot) == 3);
	if (snapshot && CFArrayGetCount(snapshot) == 3) {
		CHECK(CFArrayGetValueAtIndex(snapshot, 0) == c);
		CHECK(CFArrayGetValueAtIndex(snapshot, 1) == a);
		CHECK(CFArrayGetValueAtIndex(snapshot, 2) == b);
	}

	// Insert after an existing item.
	CFURLRef urlD = makeURL("/tmp/lssfl-d");
	LSSharedFileListItemRef d = LSSharedFileListInsertItemURL(list, a, CFSTR("D"), NULL, urlD, NULL, NULL);
	CFArrayRef afterInsert = LSSharedFileListCopySnapshot(list, NULL);
	CHECK(d && afterInsert && CFArrayGetCount(afterInsert) == 4 && CFArrayGetValueAtIndex(afterInsert, 2) == d);
	if (afterInsert)
		CFRelease(afterInsert);
	// The earlier snapshot is a copy and does not change.
	CHECK(snapshot && CFArrayGetCount(snapshot) == 3);
	if (snapshot)
		CFRelease(snapshot);

	// Resolve and display name.
	CFURLRef resolved = NULL;
	CHECK(LSSharedFileListItemResolve(a, 0, &resolved, NULL) == noErr);
	CHECK(resolved != NULL && CFEqual(resolved, urlA));
	if (resolved)
		CFRelease(resolved);
	CFStringRef name = LSSharedFileListItemCopyDisplayName(b);
	CHECK(name != NULL && CFEqual(name, CFSTR("B")));
	if (name)
		CFRelease(name);

	// Item and list properties.
	CHECK(LSSharedFileListItemSetProperty(a, kLSSharedFileListItemHidden, kCFBooleanTrue) == noErr);
	CFTypeRef property = LSSharedFileListItemCopyProperty(a, kLSSharedFileListItemHidden);
	CHECK(property == kCFBooleanTrue);
	if (property)
		CFRelease(property);
	CHECK(LSSharedFileListItemCopyProperty(b, kLSSharedFileListItemHidden) == NULL);
	CHECK(LSSharedFileListSetProperty(list, kLSSharedFileListRecentItemsMaxAmount, CFSTR("10")) == noErr);
	property = LSSharedFileListCopyProperty(list, kLSSharedFileListRecentItemsMaxAmount);
	CHECK(property != NULL && CFEqual(property, CFSTR("10")));
	if (property)
		CFRelease(property);

	// Remove one item, then all of them.
	CHECK(LSSharedFileListItemRemove(list, b) == noErr);
	snapshot = LSSharedFileListCopySnapshot(list, NULL);
	CHECK(snapshot != NULL && CFArrayGetCount(snapshot) == 3 && !CFArrayContainsValue(snapshot, CFRangeMake(0, 3), b));
	if (snapshot)
		CFRelease(snapshot);
	CHECK(LSSharedFileListRemoveAllItems(list) == noErr);
	snapshot = LSSharedFileListCopySnapshot(list, NULL);
	CHECK(snapshot != NULL && CFArrayGetCount(snapshot) == 0);
	if (snapshot)
		CFRelease(snapshot);

	// Invalid arguments.
	CHECK(LSSharedFileListCopySnapshot(NULL, NULL) == NULL);
	CHECK(LSSharedFileListInsertItemURL(list, kLSSharedFileListItemLast, CFSTR("X"), NULL, NULL, NULL, NULL) == NULL);
	CHECK(LSSharedFileListItemRemove(NULL, a) == paramErr);

	// Items stay valid after leaving the list; releasing everything must not crash.
	CFRelease(a);
	CFRelease(b);
	CFRelease(c);
	CFRelease(d);
	CFRelease(urlA);
	CFRelease(urlB);
	CFRelease(urlC);
	CFRelease(urlD);
	CFRelease(list);

	if (failures == 0)
		puts("OK");
	return failures != 0;
}
