// FSWriteFork and FSSetForkSize on data forks: writing at the mark, from the start, past the end
// (the gap reads as zeros), truncation moving the mark back, a target before the start, a
// read-only fork, a resource fork (kept in memory, not writable) and a closed reference.
// Build for i386 and x86_64 and run with `darling shell`. Exits non-zero on failure.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

int main(void)
{
	char path[] = "/tmp/darling-forkwrite-XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0)
		return 2;
	close(fd);
	FSRef ref;
	FSIORefNum fork = 0, ro = 0, rsrc = 0;
	CHECK_ERR(FSPathMakeRef((const UInt8*) path, &ref, NULL), noErr);
	CHECK_ERR(FSOpenFork(&ref, 0, NULL, fsRdWrPerm, &fork), noErr);

	ByteCount done = 0;
	SInt64 size = -1, mark = -1;
	CHECK_ERR(FSWriteFork(fork, fsFromStart, 0, 10, "0123456789", &done), noErr);
	CHECK(done == 10);
	CHECK_ERR(FSWriteFork(fork, fsAtMark, 0, 5, "abcde", &done), noErr);
	CHECK_ERR(FSGetForkSize(fork, &size), noErr);
	CHECK(size == 15);
	CHECK_ERR(FSGetForkPosition(fork, &mark), noErr);
	CHECK(mark == 15);

	// Past the end: the gap is zero-filled.
	CHECK_ERR(FSWriteFork(fork, fsFromLEOF, 2, 1, "Z", &done), noErr);
	CHECK_ERR(FSGetForkSize(fork, &size), noErr);
	CHECK(size == 18);
	char buf[32] = {0};
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 18, buf, &done), noErr);
	CHECK(done == 18 && memcmp(buf, "0123456789abcde", 15) == 0 && buf[15] == 0 && buf[16] == 0 && buf[17] == 'Z');

	CHECK_ERR(FSWriteFork(fork, fsFromStart, -1, 1, "x", &done), posErr);
	CHECK(done == 0);

	// Truncation; the mark past the new end moves to it.
	CHECK_ERR(FSSetForkSize(fork, fsFromStart, 4), noErr);
	CHECK_ERR(FSGetForkSize(fork, &size), noErr);
	CHECK(size == 4);
	CHECK_ERR(FSGetForkPosition(fork, &mark), noErr);
	CHECK(mark == 4);
	CHECK_ERR(FSSetForkSize(fork, fsFromStart, -1), posErr);

	CHECK_ERR(FSOpenFork(&ref, 0, NULL, fsRdPerm, &ro), noErr);
	CHECK_ERR(FSWriteFork(ro, fsFromStart, 0, 1, "x", &done), wrPermErr);
	CHECK_ERR(FSCloseFork(ro), noErr);

	HFSUniStr255 rsrcName;
	CHECK_ERR(FSGetResourceForkName(&rsrcName), noErr);
	CHECK(setxattr(path, "com.apple.ResourceFork", "abc", 3, 0, 0) == 0);
	CHECK_ERR(FSOpenFork(&ref, rsrcName.length, rsrcName.unicode, fsRdWrPerm, &rsrc), noErr);
	CHECK_ERR(FSWriteFork(rsrc, fsFromStart, 0, 1, "x", &done), wrPermErr);
	CHECK_ERR(FSCloseFork(rsrc), noErr);

	CHECK_ERR(FSCloseFork(fork), noErr);
	CHECK_ERR(FSWriteFork(fork, fsFromStart, 0, 1, "x", &done), fnOpnErr);
	CHECK_ERR(FSSetForkSize(fork, fsFromStart, 0), fnOpnErr);
	unlink(path);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
