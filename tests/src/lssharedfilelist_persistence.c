// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// LSSharedFileList lists persist: a second list object of the same type, and another process,
// see the items (order, display names, URLs, identifiers), the item and list properties, and
// every change moves the seed value. Passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

static CFURLRef makeURL(const char *path)
{
	CFStringRef s = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, s, kCFURLPOSIXPathStyle, true);
	CFRelease(s);
	return url;
}

static CFStringRef makeType(const char *suffix)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("com.example.test.lssharedfilelist.%s"), suffix);
}

// macOS only inserts items that exist, so the test items are real files.
static void touch(const char *directory, const char *name, char *path, size_t size)
{
	snprintf(path, size, "%s/%s", directory, name);
	FILE *f = fopen(path, "w");
	if (f)
		fclose(f);
}

static int nameIs(LSSharedFileListItemRef item, const char *expected)
{
	CFStringRef name = LSSharedFileListItemCopyDisplayName(item);
	CFStringRef want = CFStringCreateWithCString(NULL, expected, kCFStringEncodingUTF8);
	int equal = name != NULL && CFEqual(name, want);
	if (name)
		CFRelease(name);
	CFRelease(want);
	return equal;
}

// The other process: reads what the parent stored, adds an item after the first one and
// changes a property of the list.
static int child(const char *suffix, const char *directory)
{
	CFStringRef type = makeType(suffix);
	LSSharedFileListRef list = LSSharedFileListCreate(NULL, type, NULL);
	UInt32 seed = 0;
	CFArrayRef snapshot = LSSharedFileListCopySnapshot(list, &seed);
	CHECK(snapshot != NULL && CFArrayGetCount(snapshot) == 2);
	if (snapshot && CFArrayGetCount(snapshot) == 2) {
		CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(snapshot, 0), "A"));
		CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(snapshot, 1), "B"));
		CFTypeRef value = LSSharedFileListCopyProperty(list, CFSTR("com.example.list"));
		CHECK(value != NULL && CFEqual(value, CFSTR("list value")));
		if (value)
			CFRelease(value);
		char path[1024];
		touch(directory, "c", path, sizeof(path));
		CFURLRef url = makeURL(path);
		LSSharedFileListItemRef c = LSSharedFileListInsertItemURL(list, (LSSharedFileListItemRef) CFArrayGetValueAtIndex(snapshot, 0), CFSTR("C"), NULL, url, NULL, NULL);
		CHECK(c != NULL);
		CFRelease(url);
		if (c)
			CFRelease(c);
		CHECK(LSSharedFileListSetProperty(list, CFSTR("com.example.list"), CFSTR("changed by child")) == noErr);
	}
	if (snapshot)
		CFRelease(snapshot);
	CFRelease(list);
	CFRelease(type);
	return failures != 0;
}

int main(int argc, char **argv)
{
	char suffix[32];
	if (argc > 3 && strcmp(argv[1], "child") == 0)
		return child(argv[2], argv[3]);

	snprintf(suffix, sizeof(suffix), "%d", (int) getpid());
	CFStringRef type = makeType(suffix);
	LSSharedFileListRef list = LSSharedFileListCreate(NULL, type, NULL);
	CHECK(list != NULL);
	if (!list)
		return 1;
	LSSharedFileListRemoveAllItems(list);

	char directory[] = "/tmp/lssfl-persist-XXXXXX";
	CHECK(mkdtemp(directory) != NULL);
	char pathA[1024], pathB[1024];
	touch(directory, "a", pathA, sizeof(pathA));
	touch(directory, "b", pathB, sizeof(pathB));
	CFURLRef urlA = makeURL(pathA);
	CFURLRef urlB = makeURL(pathB);
	int sixty = 60;
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &sixty);
	const void *keys[] = { CFSTR("com.example.string"), CFSTR("com.example.number") };
	const void *values[] = { CFSTR("item value"), number };
	CFDictionaryRef properties = CFDictionaryCreate(NULL, keys, values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	LSSharedFileListItemRef a = LSSharedFileListInsertItemURL(list, kLSSharedFileListItemLast, CFSTR("A"), NULL, urlA, properties, NULL);
	LSSharedFileListItemRef b = LSSharedFileListInsertItemURL(list, kLSSharedFileListItemLast, CFSTR("B"), NULL, urlB, NULL, NULL);
	CHECK(a != NULL && b != NULL);
	CHECK(LSSharedFileListItemSetProperty(b, kLSSharedFileListItemHidden, kCFBooleanTrue) == noErr);
	CHECK(LSSharedFileListSetProperty(list, CFSTR("com.example.list"), CFSTR("list value")) == noErr);
	UInt32 idA = LSSharedFileListItemGetID(a), idB = LSSharedFileListItemGetID(b);
	CHECK(idA != idB);

	// A second list object of the same type sees the same state.
	LSSharedFileListRef second = LSSharedFileListCreate(NULL, type, NULL);
	UInt32 firstSeed = 0, secondSeed = 0;
	CFArrayRef snapshotFirst = LSSharedFileListCopySnapshot(list, &firstSeed);
	CFArrayRef snapshotSecond = LSSharedFileListCopySnapshot(second, &secondSeed);
	CHECK(snapshotSecond != NULL && CFArrayGetCount(snapshotSecond) == 2);
	CHECK(firstSeed == secondSeed);
	if (snapshotSecond && CFArrayGetCount(snapshotSecond) == 2) {
		LSSharedFileListItemRef a2 = (LSSharedFileListItemRef) CFArrayGetValueAtIndex(snapshotSecond, 0);
		LSSharedFileListItemRef b2 = (LSSharedFileListItemRef) CFArrayGetValueAtIndex(snapshotSecond, 1);
		CHECK(nameIs(a2, "A") && nameIs(b2, "B"));
		CHECK(LSSharedFileListItemGetID(a2) == idA && LSSharedFileListItemGetID(b2) == idB);
		CFURLRef resolved = NULL;
		CHECK(LSSharedFileListItemResolve(a2, 0, &resolved, NULL) == noErr);
		// The resolved URL may spell the same file differently (/tmp is a link on macOS).
		char resolvedPath[1024], resolvedReal[1024], wantReal[1024];
		CHECK(resolved != NULL && CFURLGetFileSystemRepresentation(resolved, true, (UInt8 *) resolvedPath, sizeof(resolvedPath)));
		CHECK(realpath(resolvedPath, resolvedReal) != NULL && realpath(pathA, wantReal) != NULL && strcmp(resolvedReal, wantReal) == 0);
		if (resolved)
			CFRelease(resolved);
		CFTypeRef value = LSSharedFileListItemCopyProperty(a2, CFSTR("com.example.string"));
		CHECK(value != NULL && CFEqual(value, CFSTR("item value")));
		if (value)
			CFRelease(value);
		value = LSSharedFileListItemCopyProperty(a2, CFSTR("com.example.number"));
		int read = 0;
		CHECK(value != NULL && CFNumberGetValue(value, kCFNumberIntType, &read) && read == 60);
		if (value)
			CFRelease(value);
		value = LSSharedFileListItemCopyProperty(b2, kLSSharedFileListItemHidden);
		CHECK(value == kCFBooleanTrue);
		if (value)
			CFRelease(value);
		value = LSSharedFileListCopyProperty(second, CFSTR("com.example.list"));
		CHECK(value != NULL && CFEqual(value, CFSTR("list value")));
		if (value)
			CFRelease(value);
	}

	// Another process changes the list; this one sees it, with a new seed.
	pid_t pid = fork();
	if (pid == 0) {
		execl(argv[0], argv[0], "child", suffix, directory, (char *) NULL);
		_exit(127);
	}
	int status = 0;
	CHECK(pid > 0 && waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	// macOS delivers a change made by another process to an existing list object through its run
	// loop, so give it some time; Darling sees it at once.
	UInt32 afterSeed = 0;
	CFArrayRef after = NULL;
	for (int attempt = 0; attempt < 50; attempt++) {
		if (after)
			CFRelease(after);
		after = LSSharedFileListCopySnapshot(list, &afterSeed);
		if (after && CFArrayGetCount(after) == 3)
			break;
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);
	}
	CHECK(afterSeed != firstSeed);
	CHECK(LSSharedFileListGetSeedValue(list) == afterSeed);
	CHECK(after != NULL && CFArrayGetCount(after) == 3);
	if (after && CFArrayGetCount(after) == 3) {
		CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 0), "A"));
		CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 1), "C"));
		CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 2), "B"));
		CHECK(LSSharedFileListItemGetID((LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 0)) == idA);
		CHECK(LSSharedFileListItemGetID((LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 2)) == idB);
	}
	CFTypeRef value = LSSharedFileListCopyProperty(list, CFSTR("com.example.list"));
	CHECK(value != NULL && CFEqual(value, CFSTR("changed by child")));
	if (value)
		CFRelease(value);

	// Removing an item through another object of the list removes it everywhere.
	if (after && CFArrayGetCount(after) == 3) {
		CHECK(LSSharedFileListItemRemove(second, (LSSharedFileListItemRef) CFArrayGetValueAtIndex(after, 1)) == noErr);
		CFArrayRef left = LSSharedFileListCopySnapshot(list, NULL);
		CHECK(left != NULL && CFArrayGetCount(left) == 2);
		if (left && CFArrayGetCount(left) == 2) {
			CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(left, 0), "A"));
			CHECK(nameIs((LSSharedFileListItemRef) CFArrayGetValueAtIndex(left, 1), "B"));
		}
		if (left)
			CFRelease(left);
	}

	// A new object created later (a later run) finds the list, and removing everything empties it.
	LSSharedFileListRef later = LSSharedFileListCreate(NULL, type, NULL);
	CFArrayRef laterSnapshot = LSSharedFileListCopySnapshot(later, NULL);
	CHECK(laterSnapshot != NULL && CFArrayGetCount(laterSnapshot) == 2);
	if (laterSnapshot)
		CFRelease(laterSnapshot);
	CHECK(LSSharedFileListRemoveAllItems(later) == noErr);
	CFArrayRef empty = LSSharedFileListCopySnapshot(list, NULL);
	CHECK(empty != NULL && CFArrayGetCount(empty) == 0);
	if (empty)
		CFRelease(empty);

	// Leave no list file behind (the path is Darling's; harmless elsewhere).
	const char *home = getenv("HOME");
	char cleanup[1024];
	char name[256];
	CFStringGetCString(type, name, sizeof(name), kCFStringEncodingUTF8);
	if (home) {
		snprintf(cleanup, sizeof(cleanup), "%s/Library/Application Support/com.apple.sharedfilelist/%s.plist", home, name);
		unlink(cleanup);
		strlcat(cleanup, ".lock", sizeof(cleanup));
		unlink(cleanup);
	}

	snprintf(cleanup, sizeof(cleanup), "rm -rf %s", directory);
	system(cleanup);

	if (after)
		CFRelease(after);
	if (snapshotFirst)
		CFRelease(snapshotFirst);
	if (snapshotSecond)
		CFRelease(snapshotSecond);
	CFRelease(later);
	CFRelease(second);
	CFRelease(list);
	CFRelease(a);
	CFRelease(b);
	CFRelease(properties);
	CFRelease(number);
	CFRelease(urlA);
	CFRelease(urlB);
	CFRelease(type);

	if (failures == 0)
		puts("OK");
	return failures != 0;
}
