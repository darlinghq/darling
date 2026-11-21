// Copyright (C) 2026 Lubos Dolezel
//
// File Manager catalog iterators (FSOpenIterator, FSGetCatalogInfoBulk, FSCloseIterator), the
// fork mark (FSGetForkPosition, FSSetForkPosition, FSReadFork with fsAtMark, fsFromStart,
// fsFromLEOF, fsFromMark and the newline mode), FSEjectVolumeSync's error for an unknown volume,
// the node IDs and volume reference FSGetCatalogInfo reports, and, in 32-bit processes (where
// macOS has them), PBHGetVolParmsSync and PBMakeFSRefSync.
// The expected values were observed on macOS 10.14.4 (i386 and x86_64); the QUIRK comments in
// FileManager.cpp name this test. Build for x86_64 and i386.
#include <CoreServices/CoreServices.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static char g_root[256];

static void write_file(const char* name, const void* data, size_t size)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", g_root, name);
	int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd >= 0) {
		if (size)
			CHECK(write(fd, data, size) == (ssize_t) size);
		close(fd);
	} else {
		printf("FAIL: cannot create %s\n", path);
		g_failures++;
	}
}

static void make_ref(const char* name, FSRef* ref)
{
	char path[512];
	if (name)
		snprintf(path, sizeof(path), "%s/%s", g_root, name);
	else
		snprintf(path, sizeof(path), "%s", g_root);
	CHECK_ERR(FSPathMakeRef((const UInt8*) path, ref, NULL), noErr);
}

static void unicode_to_ascii(const HFSUniStr255* name, char* out)
{
	int i;
	for (i = 0; i < name->length && i < 250; i++)
		out[i] = (char) name->unicode[i];
	out[i] = 0;
}

static int seen(char names[][256], int count, const char* wanted)
{
	for (int i = 0; i < count; i++)
		if (strcmp(names[i], wanted) == 0)
			return 1;
	return 0;
}

// Reads from the iterator until it is exhausted; returns the number of items and records each
// item's name and whether it is a directory.
static int drain(FSIterator it, ItemCount per_call, char names[][256], int* is_dir, int* calls, OSErr* last_err)
{
	int total = 0;
	*calls = 0;
	for (;;) {
		FSCatalogInfo info[16];
		FSRef refs[16];
		HFSUniStr255 unames[16];
		ItemCount actual = 99;
		Boolean changed = 99;
		OSErr err = FSGetCatalogInfoBulk(it, per_call, &actual, &changed, kFSCatInfoNodeFlags, info, refs, NULL, unames);
		(*calls)++;
		CHECK(changed == false);
		for (ItemCount i = 0; i < actual; i++) {
			unicode_to_ascii(&unames[i], names[total]);
			is_dir[total] = (info[i].nodeFlags & kFSNodeIsDirectoryMask) != 0;
			char path[1024] = "";
			CHECK_ERR(FSRefMakePath(&refs[i], (UInt8*) path, sizeof(path)), noErr);
			CHECK(strstr(path, names[total]) != NULL);
			total++;
		}
		if (err != noErr) {
			*last_err = err;
			return total;
		}
		if (*calls > 20)
			return total;
	}
}

static void test_iterator(void)
{
	FSRef root, file, empty;
	make_ref(NULL, &root);
	make_ref("a.txt", &file);
	make_ref("empty", &empty);

	FSIterator it = NULL;
	CHECK_ERR(FSOpenIterator(&root, kFSIterateFlat, &it), noErr);
	CHECK(it != NULL);

	// Two at a time: three calls with two items each succeed, the fourth reports the end.
	char names[16][256];
	int is_dir[16], calls;
	OSErr last = noErr;
	int total = drain(it, 2, names, is_dir, &calls, &last);
	CHECK(total == 6);
	CHECK(calls == 4);
	CHECK(last == errFSNoMoreItems);
	CHECK(seen(names, total, "a.txt") && seen(names, total, "b.bin") && seen(names, total, "lines.txt"));
	CHECK(seen(names, total, ".hidden") && seen(names, total, "sub") && seen(names, total, "empty"));
	for (int i = 0; i < total; i++) {
		int should_be_dir = strcmp(names[i], "sub") == 0 || strcmp(names[i], "empty") == 0;
		if (is_dir[i] != should_be_dir) {
			printf("FAIL: %s directory flag is %d\n", names[i], is_dir[i]);
			g_failures++;
		}
	}
	CHECK_ERR(FSCloseIterator(it), noErr);

	// All at once: the items come back together with errFSNoMoreItems.
	CHECK_ERR(FSOpenIterator(&root, kFSIterateFlat, &it), noErr);
	total = drain(it, 16, names, is_dir, &calls, &last);
	CHECK(total == 6 && calls == 1 && last == errFSNoMoreItems);
	ItemCount actual = 99;
	CHECK_ERR(FSGetCatalogInfoBulk(it, 16, &actual, NULL, kFSCatInfoNone, NULL, NULL, NULL, NULL), errFSNoMoreItems);
	CHECK(actual == 0);
	CHECK_ERR(FSCloseIterator(it), noErr);

	// No outputs wanted; zero items requested.
	CHECK_ERR(FSOpenIterator(&root, kFSIterateFlat, &it), noErr);
	actual = 99;
	CHECK_ERR(FSGetCatalogInfoBulk(it, 3, &actual, NULL, kFSCatInfoNone, NULL, NULL, NULL, NULL), noErr);
	CHECK(actual == 3);
	actual = 99;
	CHECK_ERR(FSGetCatalogInfoBulk(it, 0, &actual, NULL, kFSCatInfoNone, NULL, NULL, NULL, NULL), noErr);
	CHECK(actual == 0);
	CHECK_ERR(FSCloseIterator(it), noErr);

	// The list is fixed when the iterator is opened, and no change is reported.
	CHECK_ERR(FSOpenIterator(&root, kFSIterateFlat, &it), noErr);
	Boolean changed = 99;
	FSRef ref1;
	actual = 99;
	CHECK_ERR(FSGetCatalogInfoBulk(it, 1, &actual, &changed, kFSCatInfoNone, NULL, &ref1, NULL, NULL), noErr);
	write_file("added-later", "n", 1);
	total = drain(it, 16, names, is_dir, &calls, &last);
	CHECK(total == 5);
	CHECK(!seen(names, total, "added-later"));
	CHECK_ERR(FSCloseIterator(it), noErr);
	char added[512];
	snprintf(added, sizeof(added), "%s/added-later", g_root);
	unlink(added);

	// Empty directory.
	CHECK_ERR(FSOpenIterator(&empty, kFSIterateFlat, &it), noErr);
	actual = 99;
	CHECK_ERR(FSGetCatalogInfoBulk(it, 4, &actual, NULL, kFSCatInfoNone, NULL, NULL, NULL, NULL), errFSNoMoreItems);
	CHECK(actual == 0);
	CHECK_ERR(FSCloseIterator(it), noErr);

	// Flags and containers: kFSIterateDelete is accepted, subtree and unknown bits are not.
	CHECK_ERR(FSOpenIterator(&root, kFSIterateDelete, &it), noErr);
	CHECK_ERR(FSCloseIterator(it), noErr);
	const OptionBits bad_flags[] = { kFSIterateSubtree, 3, 4, 0x80000000u, kFSIterateReserved };
	for (unsigned i = 0; i < sizeof(bad_flags) / sizeof(bad_flags[0]); i++) {
		it = NULL;
		CHECK_ERR(FSOpenIterator(&root, bad_flags[i], &it), errFSBadIteratorFlags);
		CHECK(it == NULL);
	}
	it = (FSIterator) 1;
	CHECK_ERR(FSOpenIterator(&file, kFSIterateFlat, &it), errFSNotAFolder);
	CHECK(it == (FSIterator) 1);
	CHECK_ERR(FSCloseIterator(NULL), paramErr);
}

static void test_forks(void)
{
	FSRef bin, lines, dir;
	make_ref("b.bin", &bin);
	make_ref("lines.txt", &lines);
	make_ref(NULL, &dir);

	HFSUniStr255 data_name;
	CHECK_ERR(FSGetDataForkName(&data_name), noErr);
	FSIORefNum fork = -1;
	CHECK_ERR(FSOpenFork(&bin, data_name.length, data_name.unicode, fsRdPerm, &fork), noErr);

	unsigned char buf[300];
	ByteCount actual = 77;
	SInt64 pos = -5;
	CHECK_ERR(FSGetForkPosition(fork, &pos), noErr);
	CHECK(pos == 0);

	CHECK_ERR(FSReadFork(fork, fsAtMark, 0, 10, buf, &actual), noErr);
	CHECK(actual == 10 && buf[0] == 0 && buf[9] == 9);
	CHECK_ERR(FSGetForkPosition(fork, &pos), noErr);
	CHECK(pos == 10);

	CHECK_ERR(FSReadFork(fork, fsFromStart, 100, 5, buf, &actual), noErr);
	CHECK(actual == 5 && buf[0] == 100);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 105);

	CHECK_ERR(FSReadFork(fork, fsFromMark, -5, 5, buf, &actual), noErr);
	CHECK(actual == 5 && buf[0] == 100);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 105);

	CHECK_ERR(FSReadFork(fork, fsFromMark, 10, 5, buf, &actual), noErr);
	CHECK(actual == 5 && buf[0] == 115);

	// Reading up to the last byte succeeds; reading more than is there returns what is there.
	CHECK_ERR(FSReadFork(fork, fsFromLEOF, -10, 10, buf, &actual), noErr);
	CHECK(actual == 10 && buf[0] == 190 && buf[9] == 199);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	CHECK_ERR(FSReadFork(fork, fsAtMark, 0, 10, buf, &actual), eofErr);
	CHECK(actual == 0);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 190, 50, buf, &actual), eofErr);
	CHECK(actual == 10 && buf[0] == 190);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 300, 10, buf, &actual), eofErr);
	CHECK(actual == 0);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	CHECK_ERR(FSReadFork(fork, fsFromLEOF, 5, 10, buf, &actual), eofErr);

	// A read that begins before the start of the fork fails with posErr and rewinds.
	CHECK_ERR(FSReadFork(fork, fsFromStart, -1, 10, buf, &actual), posErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 0);
	FSSetForkPosition(fork, fsFromStart, 20);
	CHECK_ERR(FSReadFork(fork, fsFromMark, -30, 4, buf, &actual), posErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 0);
	CHECK_ERR(FSReadFork(fork, fsFromLEOF, -300, 10, buf, &actual), posErr);

	// Only the low two bits select the mode; caching flags are ignored; the count may be zero.
	FSSetForkPosition(fork, fsFromStart, 0);
	CHECK_ERR(FSReadFork(fork, 7, 0, 10, buf, &actual), noErr);
	CHECK(actual == 10 && buf[0] == 0);
	CHECK_ERR(FSReadFork(fork, fsFromStart | kFSPleaseCacheMask, 20, 4, buf, &actual), noErr);
	CHECK(actual == 4 && buf[0] == 20);
	CHECK_ERR(FSReadFork(fork, fsFromStart | kFSNoCacheMask, 30, 4, buf, &actual), noErr);
	CHECK(actual == 4 && buf[0] == 30);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 0, buf, &actual), noErr);
	CHECK(actual == 0);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 4, buf, NULL), noErr);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 4, NULL, &actual), paramErr);

	// FSSetForkPosition.
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, 50), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 50);
	CHECK_ERR(FSSetForkPosition(fork, fsFromMark, 25), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 75);
	CHECK_ERR(FSSetForkPosition(fork, fsFromLEOF, 0), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	CHECK_ERR(FSSetForkPosition(fork, fsFromLEOF, -20), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 180);
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, 20), noErr);
	CHECK_ERR(FSSetForkPosition(fork, fsFromMark, -20), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 0);
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, 20), noErr);
	CHECK_ERR(FSSetForkPosition(fork, fsFromMark, 180), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	// Past the end: eofErr, mark at the end.
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, 20), noErr);
	CHECK_ERR(FSSetForkPosition(fork, fsFromMark, 181), eofErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	CHECK_ERR(FSSetForkPosition(fork, fsFromLEOF, 20), eofErr);
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, 500), eofErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	// Before the start: posErr and mark 0, but eofErr and mark at the end for fsFromMark.
	FSSetForkPosition(fork, fsFromStart, 150);
	CHECK_ERR(FSSetForkPosition(fork, fsFromStart, -3), posErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 0);
	FSSetForkPosition(fork, fsFromStart, 150);
	CHECK_ERR(FSSetForkPosition(fork, fsFromLEOF, -300), posErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 0);
	FSSetForkPosition(fork, fsFromStart, 75);
	CHECK_ERR(FSSetForkPosition(fork, fsFromMark, -100), eofErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 200);
	// fsAtMark leaves the mark; extra mode bits do not change the base.
	FSSetForkPosition(fork, fsFromStart, 150);
	CHECK_ERR(FSSetForkPosition(fork, fsAtMark, 77), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 150);
	CHECK_ERR(FSSetForkPosition(fork, 9, 5), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 5);
	CHECK_ERR(FSSetForkPosition(fork, 0x0100 | fsFromStart, 7), noErr);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 7);

	// Unknown and closed reference numbers.
	CHECK_ERR(FSGetForkPosition(1234, &pos), fnOpnErr);
	CHECK_ERR(FSSetForkPosition(1234, fsFromStart, 0), fnOpnErr);
	CHECK_ERR(FSReadFork(1234, fsFromStart, 0, 1, buf, &actual), fnOpnErr);
	CHECK_ERR(FSCloseFork(fork), noErr);
	CHECK_ERR(FSGetForkPosition(fork, &pos), fnOpnErr);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 1, buf, &actual), fnOpnErr);
	CHECK_ERR(FSCloseFork(fork), fnOpnErr);

	// A fork opened for writing only can still be read.
	CHECK_ERR(FSOpenFork(&bin, data_name.length, data_name.unicode, fsWrPerm, &fork), noErr);
	CHECK_ERR(FSReadFork(fork, fsFromStart, 0, 4, buf, &actual), noErr);
	CHECK(actual == 4 && buf[3] == 3);
	CHECK_ERR(FSCloseFork(fork), noErr);

	// Newline mode: the newline character is returned, then the read stops.
	CHECK_ERR(FSOpenFork(&lines, data_name.length, data_name.unicode, fsRdPerm, &fork), noErr);
	CHECK_ERR(FSReadFork(fork, fsFromStart | kFSNewLineMask | ('\n' << 8), 0, 100, buf, &actual), noErr);
	CHECK(actual == 3 && memcmp(buf, "ab\n", 3) == 0);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 3);
	CHECK_ERR(FSReadFork(fork, fsAtMark | kFSNewLineMask | ('\n' << 8), 0, 100, buf, &actual), noErr);
	CHECK(actual == 3 && memcmp(buf, "cd\n", 3) == 0);
	CHECK_ERR(FSReadFork(fork, fsAtMark | kFSNewLineMask | ('\n' << 8), 0, 100, buf, &actual), eofErr);
	CHECK(actual == 2 && memcmp(buf, "ef", 2) == 0);
	FSGetForkPosition(fork, &pos);
	CHECK(pos == 8);
	CHECK_ERR(FSReadFork(fork, fsFromStart | kFSNewLineMask | ('c' << 8), 0, 100, buf, &actual), noErr);
	CHECK(actual == 4);
	CHECK_ERR(FSReadFork(fork, fsFromStart | kFSNewLineMask | ('\n' << 8), 0, 2, buf, &actual), noErr);
	CHECK(actual == 2);
	CHECK_ERR(FSCloseFork(fork), noErr);

	// There is no data fork on a directory, and no resource fork on a plain file.
	CHECK_ERR(FSOpenFork(&dir, data_name.length, data_name.unicode, fsRdPerm, &fork), errFSForkNotFound);
	HFSUniStr255 rsrc_name;
	CHECK_ERR(FSGetResourceForkName(&rsrc_name), noErr);
	CHECK_ERR(FSOpenFork(&bin, rsrc_name.length, rsrc_name.unicode, fsRdPerm, &fork), eofErr);
}

static void test_catalog_ids(void)
{
	FSRef root, file, sub, top;
	make_ref(NULL, &root);
	make_ref("a.txt", &file);
	make_ref("sub", &sub);
	CHECK_ERR(FSPathMakeRef((const UInt8*) "/", &top, NULL), noErr);

	FSCatalogInfo root_info, file_info, sub_info, top_info;
	const FSCatalogInfoBitmap which = kFSCatInfoVolume | kFSCatInfoNodeID | kFSCatInfoParentDirID;
	CHECK_ERR(FSGetCatalogInfo(&root, which, &root_info, NULL, NULL, NULL), noErr);
	CHECK_ERR(FSGetCatalogInfo(&file, which, &file_info, NULL, NULL, NULL), noErr);
	CHECK_ERR(FSGetCatalogInfo(&sub, which, &sub_info, NULL, NULL, NULL), noErr);
	CHECK_ERR(FSGetCatalogInfo(&top, which, &top_info, NULL, NULL, NULL), noErr);

	CHECK(root_info.volume == file_info.volume && file_info.volume == sub_info.volume);
	CHECK(root_info.volume == -100);
	CHECK(file_info.nodeID != 0 && file_info.nodeID != root_info.nodeID);
	CHECK(file_info.parentDirID == root_info.nodeID);
	CHECK(sub_info.parentDirID == root_info.nodeID);
	CHECK(top_info.nodeID == fsRtDirID && top_info.parentDirID == fsRtParID);
}

#if !__LP64__
static void ref_path(const FSRef* ref, char* out, size_t size)
{
	out[0] = 0;
	FSRefMakePath(ref, (UInt8*) out, (UInt32) size);
}

static OSErr make_ref_sync(const char* name, short vref, SInt32 dir_id, FSRef* out)
{
	struct FSRefParam pb;
	unsigned char pname[300];
	memset(&pb, 0, sizeof(pb));
	if (name) {
		pname[0] = (unsigned char) strlen(name);
		memcpy(pname + 1, name, strlen(name));
		pb.ioNamePtr = (ConstStringPtr) pname;
	}
	pb.ioVRefNum = vref;
	pb.ioDirID = dir_id;
	pb.newRef = out;
	OSErr err = PBMakeFSRefSync(&pb);
	CHECK(pb.ioResult == err);
	return err;
}

static void test_legacy_32bit(void)
{
	FSRef root;
	make_ref(NULL, &root);
	FSCatalogInfo ci;
	CHECK_ERR(FSGetCatalogInfo(&root, kFSCatInfoVolume | kFSCatInfoNodeID, &ci, NULL, NULL, NULL), noErr);
	const short vol = ci.volume;

	// PBHGetVolParmsSync.
	HParamBlockRec pb;
	GetVolParmsInfoBuffer parms;
	const short vols_ok[] = { vol, 0, 1, 12345 };
	for (unsigned i = 0; i < sizeof(vols_ok) / sizeof(vols_ok[0]); i++) {
		memset(&pb, 0, sizeof(pb));
		memset(&parms, 0, sizeof(parms));
		pb.ioParam.ioVRefNum = vols_ok[i];
		pb.ioParam.ioBuffer = (Ptr) &parms;
		pb.ioParam.ioReqCount = sizeof(parms);
		CHECK_ERR(PBHGetVolParmsSync(&pb), noErr);
		CHECK(pb.ioParam.ioResult == noErr);
		CHECK(pb.ioParam.ioActCount == (SInt32) sizeof(parms));
		CHECK(parms.vMVersion == 5);
		CHECK(parms.vMAttrib == 0x28084408);
		CHECK(parms.vMExtendedAttributes == 0x10ac2432);
		CHECK(parms.vMVolumeGrade == 0 && parms.vMForeignPrivID == 0 && parms.vMServerAdr == 0);
		CHECK(parms.vMDeviceID != NULL);
		CHECK(parms.vMMaxNameLength == 85);
	}
	const short vols_bad[] = { -1, -32768 };
	for (unsigned i = 0; i < 2; i++) {
		memset(&pb, 0, sizeof(pb));
		pb.ioParam.ioVRefNum = vols_bad[i];
		pb.ioParam.ioBuffer = (Ptr) &parms;
		pb.ioParam.ioReqCount = sizeof(parms);
		CHECK_ERR(PBHGetVolParmsSync(&pb), nsvErr);
		CHECK(pb.ioParam.ioResult == nsvErr && pb.ioParam.ioActCount == 0);
	}
	const SInt32 requests[] = { 0, 2, 10, 14, (SInt32) sizeof(parms) };
	for (unsigned i = 0; i < 5; i++) {
		memset(&pb, 0, sizeof(pb));
		pb.ioParam.ioVRefNum = vol;
		pb.ioParam.ioBuffer = (Ptr) &parms;
		pb.ioParam.ioReqCount = requests[i];
		OSErr err = PBHGetVolParmsSync(&pb);
		if (requests[i] == 0) {
			CHECK(err == paramErr);
		} else {
			CHECK(err == noErr);
			CHECK(pb.ioParam.ioActCount == requests[i]);
		}
	}

	// PBMakeFSRefSync.
	FSRef out;
	char path[1024], expected[1024];
	CHECK_ERR(make_ref_sync("a.txt", vol, ci.nodeID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	snprintf(expected, sizeof(expected), "%s/a.txt", g_root);
	CHECK(strcmp(path, expected) == 0);

	CHECK_ERR(make_ref_sync(NULL, vol, ci.nodeID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	CHECK(strcmp(path, g_root) == 0);

	CHECK_ERR(make_ref_sync(NULL, vol, fsRtDirID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	CHECK(strcmp(path, "/") == 0);
	CHECK_ERR(make_ref_sync("usr", vol, fsRtDirID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	CHECK(strcmp(path, "/usr") == 0);
	CHECK_ERR(make_ref_sync(NULL, vol, fsRtParID, &out), nsvErr);

	CHECK_ERR(make_ref_sync(":sub:c.txt", vol, ci.nodeID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	snprintf(expected, sizeof(expected), "%s/sub/c.txt", g_root);
	CHECK(strcmp(path, expected) == 0);
	CHECK_ERR(make_ref_sync("sub:c.txt", vol, ci.nodeID, &out), nsvErr);
	CHECK_ERR(make_ref_sync("sub/c.txt", vol, ci.nodeID, &out), fnfErr);
	CHECK_ERR(make_ref_sync("::", vol, ci.nodeID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	snprintf(expected, sizeof(expected), "%s", g_root);
	*strrchr(expected, '/') = 0;
	CHECK(strcmp(path, expected) == 0);

	// A directory ID found through a nested item, the volume number is not looked at, and the
	// error codes of missing items.
	FSRef sub;
	make_ref("sub", &sub);
	FSCatalogInfo sub_info;
	CHECK_ERR(FSGetCatalogInfo(&sub, kFSCatInfoNodeID, &sub_info, NULL, NULL, NULL), noErr);
	CHECK_ERR(make_ref_sync("c.txt", vol, sub_info.nodeID, &out), noErr);
	ref_path(&out, path, sizeof(path));
	snprintf(expected, sizeof(expected), "%s/sub/c.txt", g_root);
	CHECK(strcmp(path, expected) == 0);
	CHECK_ERR(make_ref_sync("a.txt", 12345, ci.nodeID, &out), noErr);
	CHECK_ERR(make_ref_sync("a.txt", 0, ci.nodeID, &out), noErr);
	CHECK_ERR(make_ref_sync("missing", vol, ci.nodeID, &out), fnfErr);
	CHECK_ERR(make_ref_sync("a.txt", vol, 999999999, &out), fnfErr);
	CHECK_ERR(make_ref_sync("a.txt", vol, ci.nodeID, NULL), errFSBadFSRef);

	// Full path with a volume name; the name of the volume as FSGetVolumeInfo reports it when that
	// works, else the one Darling gives its volume.
	HFSUniStr255 vname;
	FSVolumeRefNum actual_vol;
	char volume[256] = "Macintosh HD";
	if (FSGetVolumeInfo(vol, 0, &actual_vol, 0, NULL, &vname, NULL) == noErr)
		unicode_to_ascii(&vname, volume);
	char full[300];
	snprintf(full, sizeof(full), "%s:", volume);
	CHECK_ERR(make_ref_sync(full, 0, 0, &out), noErr);
	ref_path(&out, path, sizeof(path));
	CHECK(strcmp(path, "/") == 0);
	snprintf(full, sizeof(full), "%s:usr", volume);
	CHECK_ERR(make_ref_sync(full, 0, 0, &out), noErr);
	ref_path(&out, path, sizeof(path));
	CHECK(strcmp(path, "/usr") == 0);
	CHECK_ERR(make_ref_sync("NoSuchVolume:usr", 0, 0, &out), nsvErr);
	CHECK_ERR(make_ref_sync("usr:bin", vol, 0, &out), nsvErr);
}
#endif

static void test_volume_info(void)
{
	FSVolumeInfo info;
	FSVolumeRefNum actual = 77;
	HFSUniStr255 name;
	FSRef root;
	char path[64] = "";

	memset(&info, 0, sizeof(info));
	CHECK_ERR(FSGetVolumeInfo(-100, 0, &actual, 0xFFFF, &info, &name, &root), noErr);
	CHECK(actual == -100);
	CHECK(name.length > 0);
	FSRefMakePath(&root, (UInt8*) path, sizeof(path));
	CHECK(strcmp(path, "/") == 0);
	CHECK(info.blockSize > 0 && info.totalBlocks > 0);
	CHECK(info.totalBytes == (UInt64) info.blockSize * info.totalBlocks);
	CHECK(info.freeBytes == (UInt64) info.blockSize * info.freeBlocks);
	CHECK(info.freeBytes <= info.totalBytes);
	CHECK(info.createDate.lowSeconds != 0);
	CHECK(info.flags & 0x0020); // the default volume
	CHECK(info.rsrcClumpSize == 4096 && info.dataClumpSize == 4096);

	// Only the fields named in the bitmap are written.
	memset(&info, 0x5a, sizeof(info));
	CHECK_ERR(FSGetVolumeInfo(-100, 0, NULL, 0x0080, &info, NULL, NULL), noErr);
	CHECK(info.blockSize > 0 && info.blockSize != 0x5a5a5a5a);
	CHECK(info.fileCount == 0x5a5a5a5a && info.flags == 0x5a5a && info.totalBytes == 0x5a5a5a5a5a5a5a5aULL);

	// The volume can be found by index 1 or by 0 with no volume; unknown ones fail.
	CHECK_ERR(FSGetVolumeInfo(0, 1, &actual, 0, NULL, NULL, NULL), noErr);
	CHECK(actual == -100);
	CHECK_ERR(FSGetVolumeInfo(0, 0, &actual, 0, NULL, NULL, NULL), nsvErr);
	CHECK_ERR(FSGetVolumeInfo(12345, 0, &actual, 0, NULL, NULL, NULL), nsvErr);
}

int main(void)
{
	strcpy(g_root, "/tmp/fm_iterators_XXXXXX");
	if (!mkdtemp(g_root)) {
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	// FSRefMakePath returns resolved paths (/tmp is a link on macOS).
	char resolved[PATH_MAX];
	if (realpath(g_root, resolved) && strlen(resolved) < sizeof(g_root))
		strcpy(g_root, resolved);
	unsigned char bin[200];
	for (int i = 0; i < 200; i++)
		bin[i] = (unsigned char) i;
	write_file("a.txt", "hello", 5);
	write_file("b.bin", bin, sizeof(bin));
	write_file("lines.txt", "ab\ncd\nef", 8);
	write_file(".hidden", "x", 1);
	char path[512];
	snprintf(path, sizeof(path), "%s/sub", g_root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/empty", g_root);
	mkdir(path, 0755);
	write_file("sub/c.txt", "c", 1);

	test_iterator();
	test_forks();
	test_catalog_ids();
	test_volume_info();

	// FSEjectVolumeSync with a volume reference that does not exist.
	pid_t dissenter = 4242;
	CHECK_ERR(FSEjectVolumeSync(12345, 0, &dissenter), nsvErr);
	CHECK(dissenter == 4242);
	CHECK_ERR(FSEjectVolumeSync(0, 0, &dissenter), nsvErr);
	CHECK_ERR(FSEjectVolumeSync(-32768, 0, NULL), nsvErr);

#if !__LP64__
	test_legacy_32bit();
#endif

	char cmd[600];
	snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_root);
	if (system(cmd) != 0)
		printf("note: could not remove %s\n", g_root);

	if (g_failures == 0)
		printf("PASS\n");
	else
		printf("%d failure(s)\n", g_failures);
	return g_failures != 0;
}
