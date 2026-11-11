// Copyright (C) 2026 Lubos Dolezel
//
// File Manager forks: FSOpenFork, FSGetForkSize and FSCloseFork on the data fork
// and on the resource fork (kept in the com.apple.ResourceFork extended attribute),
// with the error codes for a closed reference number, an unknown fork name and a
// missing resource fork. Also checks the values of the kMDItemOrigin* and
// kMDItemDestinationRecipients metadata keys.
// The program passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <CoreServices/CoreServices.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

extern const CFStringRef kMDItemOriginApplicationIdentifier, kMDItemOriginMessageID,
	kMDItemOriginSenderDisplayName, kMDItemOriginSenderHandle, kMDItemOriginSubject,
	kMDItemDestinationRecipients;

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static void check_key(CFStringRef key, const char* expected)
{
	char buffer[128] = "";
	if (key == NULL || !CFStringGetCString(key, buffer, sizeof(buffer), kCFStringEncodingUTF8) || strcmp(buffer, expected) != 0) {
		printf("FAIL: metadata key %s is \"%s\"\n", expected, buffer);
		g_failures++;
	}
}

int main(void)
{
	check_key(kMDItemOriginApplicationIdentifier, "kMDItemOriginApplicationIdentifier");
	check_key(kMDItemOriginMessageID, "kMDItemOriginMessageID");
	check_key(kMDItemOriginSenderDisplayName, "kMDItemOriginSenderDisplayName");
	check_key(kMDItemOriginSenderHandle, "kMDItemOriginSenderHandle");
	check_key(kMDItemOriginSubject, "kMDItemOriginSubject");
	check_key(kMDItemDestinationRecipients, "kMDItemDestinationRecipients");

	char path[] = "/tmp/fork_test_XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0) {
		printf("FAIL: mkstemp\n");
		return 1;
	}
	CHECK(write(fd, "hello world", 11) == 11);
	close(fd);
	CHECK(setxattr(path, "com.apple.ResourceFork", "RSRC12345", 9, 0, 0) == 0);

	FSRef ref;
	CHECK_ERR(FSPathMakeRef((const UInt8*) path, &ref, NULL), noErr);

	HFSUniStr255 dataName, rsrcName;
	CHECK_ERR(FSGetDataForkName(&dataName), noErr);
	CHECK_ERR(FSGetResourceForkName(&rsrcName), noErr);

	FSIORefNum dataFork = 0, rsrcFork = 0;
	SInt64 size = -1;
	CHECK_ERR(FSOpenFork(&ref, dataName.length, dataName.unicode, fsRdPerm, &dataFork), noErr);
	CHECK_ERR(FSGetForkSize(dataFork, &size), noErr);
	CHECK(size == 11);
	CHECK_ERR(FSOpenFork(&ref, rsrcName.length, rsrcName.unicode, fsRdPerm, &rsrcFork), noErr);
	CHECK_ERR(FSGetForkSize(rsrcFork, &size), noErr);
	CHECK(size == 9);
	CHECK_ERR(FSCloseFork(dataFork), noErr);
	CHECK_ERR(FSCloseFork(rsrcFork), noErr);

	// A closed or never-opened reference number.
	CHECK_ERR(FSGetForkSize(dataFork, &size), rfNumErr);
	CHECK_ERR(FSCloseFork(dataFork), fnOpnErr);
	CHECK_ERR(FSCloseFork(12345), fnOpnErr);

	// An empty fork name opens the data fork.
	CHECK_ERR(FSOpenFork(&ref, 0, NULL, fsRdPerm, &dataFork), noErr);
	CHECK_ERR(FSCloseFork(dataFork), noErr);

	// Unknown fork name.
	UniChar bogus[3] = { 'x', 'y', 'z' };
	CHECK_ERR(FSOpenFork(&ref, 3, bogus, fsRdPerm, &dataFork), errFSBadForkName);

	// No resource fork.
	CHECK(removexattr(path, "com.apple.ResourceFork", 0) == 0);
	CHECK_ERR(FSOpenFork(&ref, rsrcName.length, rsrcName.unicode, fsRdPerm, &rsrcFork), eofErr);

	// Write access to the data fork.
	CHECK_ERR(FSOpenFork(&ref, dataName.length, dataName.unicode, fsWrPerm, &dataFork), noErr);
	CHECK_ERR(FSCloseFork(dataFork), noErr);

	unlink(path);
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
