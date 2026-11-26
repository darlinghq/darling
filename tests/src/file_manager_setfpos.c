// Classic SetFPos on CarbonCore fork references (32-bit processes only, where macOS has it).
// SetFPos moves the same fork mark as FSSetForkPosition, so it follows its positioning rules:
// only the low two bits of the mode select the base, a target before the start fails with
// posErr and moves the mark to 0 (fsFromMark: eofErr, mark at the end), and a target after the
// end fails with eofErr and moves the mark to the end. Inside Macintosh: Files, p. 2-95.
// Build for i386 and run with `darling shell`. Exits non-zero on failure.
#include <CoreServices/CoreServices.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static void mark(FSIORefNum fork, SInt64 expected)
{
	SInt64 observed = -1;
	CHECK_ERR(FSGetForkPosition(fork, &observed), noErr);
	CHECK(observed == expected);
}

// The fork holds 32 bytes.
static void position_cases(FSIORefNum fork)
{
	mark(fork, 0);
	CHECK_ERR(SetFPos(fork, fsFromStart, 10), noErr);
	mark(fork, 10);
	CHECK_ERR(SetFPos(fork, fsAtMark, INT32_MIN), noErr); // the offset is ignored
	mark(fork, 10);
	CHECK_ERR(SetFPos(fork, fsFromMark, -4), noErr);
	mark(fork, 6);
	CHECK_ERR(SetFPos(fork, fsFromLEOF, -3), noErr);
	mark(fork, 29);
	CHECK_ERR(SetFPos(fork, fsFromStart | 0x0020 /* noCacheMask */, 5), noErr);
	mark(fork, 5);
	CHECK_ERR(SetFPos(fork, fsFromStart, -1), posErr);
	mark(fork, 0);
	CHECK_ERR(SetFPos(fork, fsFromStart, 7), noErr);
	CHECK_ERR(SetFPos(fork, fsFromMark, -30), eofErr);
	mark(fork, 32);
	CHECK_ERR(SetFPos(fork, fsFromLEOF, 0), noErr);
	mark(fork, 32);
	CHECK_ERR(SetFPos(fork, fsFromStart, 1), noErr);
	CHECK_ERR(SetFPos(fork, fsFromLEOF, 1), eofErr);
	mark(fork, 32);
	CHECK_ERR(SetFPos(fork, fsFromStart, INT32_MAX), eofErr);
	mark(fork, 32);
}

int main(void)
{
	char path[] = "/tmp/darling-setfpos-XXXXXX";
	const char payload[32] = "abcdefghijklmnopqrstuvwxyz01234";
	int fd = mkstemp(path);
	if (fd < 0 || write(fd, payload, sizeof(payload)) != sizeof(payload))
		return 2;

	FSRef ref;
	FSIORefNum data = -1, second = -1, resource = -1;
	CHECK_ERR(FSPathMakeRef((const UInt8*) path, &ref, NULL), noErr);
	CHECK_ERR(FSOpenFork(&ref, 0, NULL, fsRdPerm, &data), noErr);
	CHECK_ERR(FSOpenFork(&ref, 0, NULL, fsRdPerm, &second), noErr);
	CHECK(second != data);

	position_cases(data);
	mark(second, 0); // each open reference has its own mark

	HFSUniStr255 resourceName;
	CHECK_ERR(FSGetResourceForkName(&resourceName), noErr);
	CHECK(setxattr(path, "com.apple.ResourceFork", payload, sizeof(payload), 0, 0) == 0);
	CHECK_ERR(FSOpenFork(&ref, resourceName.length, resourceName.unicode, fsRdPerm, &resource), noErr);
	position_cases(resource);
	CHECK_ERR(FSCloseFork(resource), noErr);

	// Beyond 2 GiB: the 32-bit offset is relative to a 64-bit mark.
	const SInt64 sparse = (SInt64) INT32_MAX + 33;
	CHECK(ftruncate(fd, sparse) == 0);
	CHECK_ERR(SetFPos(data, fsFromStart, INT32_MAX), noErr);
	CHECK_ERR(SetFPos(data, fsFromMark, 2), noErr);
	mark(data, (SInt64) INT32_MAX + 2);
	CHECK_ERR(SetFPos(data, fsFromLEOF, 1), eofErr);
	mark(data, sparse);

	CHECK_ERR(FSCloseFork(data), noErr);
	CHECK_ERR(FSCloseFork(second), noErr);
	CHECK_ERR(SetFPos(data, fsFromStart, 0), fnOpnErr);
	CHECK_ERR(SetFPos(-1, fsFromStart, 0), fnOpnErr);

	close(fd);
	unlink(path);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
