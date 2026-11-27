// The classic File Manager calls for reading and writing a file by FSSpec (32-bit processes only, where macOS
// has them): FSpOpenDF, FSpOpenRF, FSRead, FSWrite, GetEOF, SetEOF, GetFPos, SetFPos and FSClose, on the fork
// references of the fork calls. Reads and writes happen at the mark, which they advance; a read that reaches the
// end first returns what there was with eofErr. Reference: File Manager Reference (Inside Macintosh: Files).
// Build for i386 and run with `darling shell`. Exits non-zero on failure.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static void make_spec(const char *path, FSSpec *spec)
{
	FSRef ref;
	FSCatalogInfo info;
	HFSUniStr255 name;
	FSRef parent;
	if (FSPathMakeRef((const UInt8 *) path, &ref, NULL) != noErr
	    || FSGetCatalogInfo(&ref, kFSCatInfoNone, &info, &name, NULL, &parent) != noErr) {
		printf("FAIL: no reference for %s\n", path);
		exit(2);
	}
	// the FSSpec of the file: the volume, the directory ID of the parent and the name
	char dir[256];
	strcpy(dir, path);
	*strrchr(dir, '/') = 0;
	FSRef parentRef;
	FSCatalogInfo parentInfo;
	if (FSPathMakeRef((const UInt8 *) dir, &parentRef, NULL) != noErr
	    || FSGetCatalogInfo(&parentRef, kFSCatInfoNodeID | kFSCatInfoVolume, &parentInfo, NULL, NULL, NULL) != noErr) {
		printf("FAIL: no parent for %s\n", path);
		exit(2);
	}
	const char *leaf = strrchr(path, '/') + 1;
	Str255 pname;
	pname[0] = (unsigned char) strlen(leaf);
	memcpy(pname + 1, leaf, pname[0]);
	if (FSMakeFSSpec(parentInfo.volume, (SInt32) parentInfo.nodeID, pname, spec) != noErr) {
		printf("FAIL: FSMakeFSSpec\n");
		exit(2);
	}
}

int main(void)
{
	char path[] = "/tmp/darling-classic-io-XXXXXX";
	const char payload[32] = "abcdefghijklmnopqrstuvwxyz01234";
	int fd = mkstemp(path);
	if (fd < 0 || write(fd, payload, sizeof(payload)) != sizeof(payload))
		return 2;
	close(fd);
	FSSpec spec;
	make_spec(path, &spec);

	FSIORefNum ref = -1;
	long count, pos, eof;
	char buffer[64];

	// reading advances the mark
	CHECK_ERR(FSpOpenDF(&spec, fsRdPerm, &ref), noErr);
	CHECK(ref != -1);
	CHECK_ERR(GetEOF(ref, &eof), noErr);
	CHECK(eof == 32);
	CHECK_ERR(GetFPos(ref, &pos), noErr);
	CHECK(pos == 0);
	count = 10;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	CHECK(count == 10 && memcmp(buffer, payload, 10) == 0);
	CHECK_ERR(GetFPos(ref, &pos), noErr);
	CHECK(pos == 10);
	count = 4;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	CHECK(count == 4 && memcmp(buffer, payload + 10, 4) == 0);

	// SetFPos and a read that reaches the end first: what is there, with eofErr
	CHECK_ERR(SetFPos(ref, fsFromStart, 28), noErr);
	count = 10;
	CHECK_ERR(FSRead(ref, &count, buffer), eofErr);
	CHECK(count == 4 && memcmp(buffer, payload + 28, 4) == 0);
	CHECK_ERR(GetFPos(ref, &pos), noErr);
	CHECK(pos == 32);
	count = 1;
	CHECK_ERR(FSRead(ref, &count, buffer), eofErr);
	CHECK(count == 0);

	// a read that ends exactly at the end succeeds; a zero length read does too
	CHECK_ERR(SetFPos(ref, fsFromStart, 30), noErr);
	count = 2;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	CHECK(count == 2);
	CHECK_ERR(SetFPos(ref, fsFromStart, 0), noErr);
	count = 0;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	CHECK(count == 0);

	// a file opened for reading cannot be written
	count = 1;
	CHECK(FSWrite(ref, &count, "x") != noErr);
	CHECK(count == 0);
	CHECK_ERR(FSClose(ref), noErr);

	// a closed reference number is not accepted
	count = 1;
	CHECK(FSRead(ref, &count, buffer) != noErr);
	CHECK(GetEOF(ref, &eof) != noErr);
	CHECK(GetFPos(ref, &pos) != noErr);
	CHECK(FSClose(ref) != noErr);

	// writing at the mark, extending the file; SetEOF shortens it
	CHECK_ERR(FSpOpenDF(&spec, fsRdWrPerm, &ref), noErr);
	CHECK_ERR(SetFPos(ref, fsFromLEOF, 0), noErr);
	count = 6;
	CHECK_ERR(FSWrite(ref, &count, "012345"), noErr);
	CHECK(count == 6);
	CHECK_ERR(GetEOF(ref, &eof), noErr);
	CHECK(eof == 38);
	CHECK_ERR(GetFPos(ref, &pos), noErr);
	CHECK(pos == 38);
	CHECK_ERR(SetFPos(ref, fsFromStart, 30), noErr);
	count = 8;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	// the last byte of the first 32 (a zero) comes before what was written
	CHECK(count == 8 && memcmp(buffer, "4\0" "012345", 8) == 0);
	CHECK_ERR(SetEOF(ref, 5), noErr);
	CHECK_ERR(GetEOF(ref, &eof), noErr);
	CHECK(eof == 5);
	CHECK_ERR(GetFPos(ref, &pos), noErr);
	CHECK(pos == 5);
	CHECK_ERR(FSClose(ref), noErr);

	// fsCurPerm takes read and write where the file allows it
	CHECK_ERR(FSpOpenDF(&spec, fsCurPerm, &ref), noErr);
	count = 2;
	CHECK_ERR(FSWrite(ref, &count, "ZZ"), noErr);
	CHECK_ERR(FSClose(ref), noErr);
	CHECK_ERR(FSpOpenDF(&spec, fsRdWrShPerm, &ref), noErr);
	count = 5;
	CHECK_ERR(FSRead(ref, &count, buffer), noErr);
	CHECK(count == 5 && memcmp(buffer, "ZZcde", 5) == 0);
	CHECK_ERR(FSClose(ref), noErr);

	// the resource fork is opened by FSpOpenRF; a file without one has none
	FSIORefNum rsrc = -1;
	CHECK(FSpOpenRF(&spec, fsRdPerm, &rsrc) != noErr);
	CHECK(setxattr(path, "com.apple.ResourceFork", "resource!", 9, 0, 0) == 0);
	CHECK_ERR(FSpOpenRF(&spec, fsRdPerm, &rsrc), noErr);
	CHECK_ERR(GetEOF(rsrc, &eof), noErr);
	CHECK(eof == 9);
	count = 9;
	CHECK_ERR(FSRead(rsrc, &count, buffer), noErr);
	CHECK(count == 9 && memcmp(buffer, "resource!", 9) == 0);
	CHECK_ERR(FSClose(rsrc), noErr);

	// errors: no such file, bad parameters
	FSSpec missing = spec;
	missing.name[0] = 1;
	missing.name[1] = '~';
	CHECK(FSpOpenDF(&missing, fsRdPerm, &ref) != noErr);
	CHECK_ERR(FSpOpenDF(&spec, fsRdPerm, NULL), paramErr);
	CHECK_ERR(GetEOF(ref, NULL), paramErr);
	CHECK_ERR(GetFPos(ref, NULL), paramErr);
	CHECK_ERR(FSRead(ref, NULL, buffer), paramErr);
	CHECK_ERR(FSWrite(ref, NULL, buffer), paramErr);

	unlink(path);
	printf("%s\n", g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
