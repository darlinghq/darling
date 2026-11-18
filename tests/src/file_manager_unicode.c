// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// File Manager Unicode-name calls (FSCreateFileUnicode, FSMakeFSRefUnicode, the HFSUniStr255
// string conversions, FSMoveObjectToTrashSync) and the LSSharedFileList calls that take an FSRef
// or resolve to a URL with a CFError. test_quirk_* functions pin observed, undocumented behavior.
// The program passes unchanged on macOS 26.7.3 (arm64).
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wnonnull"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERR(expr, expected) do { OSStatus e_ = (expr); if (e_ != (expected)) { printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #expr, (int) e_, (int) (expected)); g_failures++; } } while (0)

static char g_dir[1024];
static FSRef g_dirRef;

static void path_of(const FSRef* ref, char* out)
{
	out[0] = 0;
	FSRefMakePath(ref, (UInt8*) out, 1024);
}

static const char* leaf_of(const char* path)
{
	const char* slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

static void test_hfsunistr(void)
{
	HFSUniStr255 u;
	UniChar chars[] = { 'a', 'b', 0x00e9 };
	CFStringRef s;

	memset(&u, 0, sizeof(u));
	u.length = 3;
	memcpy(u.unicode, chars, sizeof(chars));
	s = FSCreateStringFromHFSUniStr(NULL, &u);
	CHECK(s != NULL);
	if (s) {
		CHECK(CFStringGetLength(s) == 3);
		CHECK(CFStringGetCharacterAtIndex(s, 2) == 0x00e9);
		CFRelease(s);
	}

	u.length = 0;
	s = FSCreateStringFromHFSUniStr(NULL, &u);
	CHECK(s != NULL && CFStringGetLength(s) == 0);
	if (s)
		CFRelease(s);

	CHECK_ERR(FSGetHFSUniStrFromString(CFSTR("ab\xc3\xa9"), &u) , noErr);
	CHECK(u.length == 3 && u.unicode[0] == 'a' && u.unicode[2] == 0x00e9);

	CFMutableStringRef longStr = CFStringCreateMutable(NULL, 0);
	for (int i = 0; i < 300; i++)
		CFStringAppend(longStr, CFSTR("a"));
	CHECK_ERR(FSGetHFSUniStrFromString(longStr, &u), errFSNameTooLong);
	CFRelease(longStr);
}

static void test_create_and_make_ref(void)
{
	UniChar name[] = { 'f', '.', 't', 'x', 't' };
	FSRef ref, ref2;
	char path[1024];
	struct stat st;

	CHECK_ERR(FSCreateFileUnicode(&g_dirRef, 5, name, kFSCatInfoNone, NULL, &ref, NULL), noErr);
	path_of(&ref, path);
	CHECK(strcmp(leaf_of(path), "f.txt") == 0);
	CHECK(stat(path, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 0);

	CHECK_ERR(FSCreateFileUnicode(&g_dirRef, 5, name, kFSCatInfoNone, NULL, &ref2, NULL), dupFNErr);
	CHECK_ERR(FSCreateFileUnicode(&g_dirRef, 0, name, kFSCatInfoNone, NULL, &ref2, NULL), errFSMissingName);

	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 5, name, kTextEncodingUnknown, &ref2), noErr);
	CHECK_ERR(FSCompareFSRefs(&ref, &ref2), noErr);

	UniChar missing[] = { 'n', 'o' };
	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 2, missing, kTextEncodingUnknown, &ref2), fnfErr);
	CHECK_ERR(FSMakeFSRefUnicode(&ref, 2, missing, kTextEncodingUnknown, &ref2), errFSNotAFolder);
	CHECK_ERR(FSMakeFSRefUnicode(NULL, 5, name, kTextEncodingUnknown, &ref2), errFSBadFSRef);
	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 5, name, kTextEncodingUnknown, NULL), errFSBadFSRef);

	// A non-ASCII name round-trips through the UTF-8 directory entry.
	UniChar uni[] = { 'u', 0x00e9, '.', 't' };
	CHECK_ERR(FSCreateFileUnicode(&g_dirRef, 4, uni, kFSCatInfoNone, NULL, &ref, NULL), noErr);
	path_of(&ref, path);
	// (macOS stores the name decomposed; compare the canonical composed form.)
	CFMutableStringRef leaf = CFStringCreateMutable(NULL, 0);
	CFStringAppendCString(leaf, leaf_of(path), kCFStringEncodingUTF8);
	CFStringNormalize(leaf, kCFStringNormalizationFormC);
	CHECK(CFStringGetLength(leaf) == 4 && CFStringGetCharacterAtIndex(leaf, 1) == 0x00e9);
	CFRelease(leaf);
	unlink(path);

	snprintf(path, sizeof(path), "%s/f.txt", g_dir);
	unlink(path);
}

static void test_quirk_create_file_slash_becomes_colon(void)
{
	UniChar slash[] = { 'a', '/', 'b' };
	FSRef ref;
	char path[1024];

	CHECK_ERR(FSCreateFileUnicode(&g_dirRef, 3, slash, kFSCatInfoNone, NULL, &ref, NULL), noErr);
	path_of(&ref, path);
	CHECK(strcmp(leaf_of(path), "a:b") == 0);
	unlink(path);
}

static void test_quirk_make_fsref_special_names(void)
{
	FSRef ref;
	UniChar colon[] = { 'a', ':', 'b' }, slash[] = { 'a', '/', 'b' }, dots[] = { '.', '.' }, dummy[] = { 'x' };

	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 3, colon, kTextEncodingUnknown, &ref), bdNamErr);
	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 3, slash, kTextEncodingUnknown, &ref), fnfErr);
	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 2, dots, kTextEncodingUnknown, &ref), bdNamErr);
	CHECK_ERR(FSMakeFSRefUnicode(&g_dirRef, 0, dummy, kTextEncodingUnknown, &ref), noErr);
	CHECK_ERR(FSCompareFSRefs(&ref, &g_dirRef), noErr);
}

static void make_file(const char* name, FSRef* ref)
{
	char path[1024];
	Boolean isDir;

	snprintf(path, sizeof(path), "%s/%s", g_dir, name);
	close(open(path, O_CREAT | O_WRONLY, 0644));
	CHECK_ERR(FSPathMakeRef((const UInt8*) path, ref, &isDir), noErr);
}

static void test_trash(void)
{
	FSRef ref, trashed;
	char path[1024], path2[1024];

	make_file("trashme.txt", &ref);
	CHECK_ERR(FSMoveObjectToTrashSync(&ref, &trashed, kFSFileOperationDefaultOptions), noErr);
	path_of(&trashed, path);
	CHECK(strstr(path, "/.Trash/") != NULL);
	CHECK(access(path, F_OK) == 0);
	snprintf(path2, sizeof(path2), "%s/trashme.txt", g_dir);
	CHECK(access(path2, F_OK) != 0);

	unlink(path);

	// The FSRef of a file that is gone is reported as not found; a NULL target is allowed.
	make_file("trashme2.txt", &ref);
	snprintf(path2, sizeof(path2), "%s/trashme2.txt", g_dir);
	unlink(path2);
	CHECK_ERR(FSMoveObjectToTrashSync(&ref, NULL, 0), fnfErr);
	make_file("trashme3.txt", &ref);
	CHECK_ERR(FSMoveObjectToTrashSync(&ref, NULL, 0), noErr);
	snprintf(path2, sizeof(path2), "%s/trashme3.txt", g_dir);
	CHECK(access(path2, F_OK) != 0);
	snprintf(path2, sizeof(path2), "%s/.Trash/trashme3.txt", getenv("HOME"));
	unlink(path2);
}

static void test_quirk_trash_name_collision(void)
{
	FSRef ref, first, second;
	char p1[1024], p2[1024];
	size_t len;

	make_file("gapcoll.txt", &ref);
	CHECK_ERR(FSMoveObjectToTrashSync(&ref, &first, 0), noErr);
	make_file("gapcoll.txt", &ref);
	CHECK_ERR(FSMoveObjectToTrashSync(&ref, &second, 0), noErr);
	path_of(&first, p1);
	path_of(&second, p2);
	CHECK(strcmp(leaf_of(p1), "gapcoll.txt") == 0);
	// "gapcoll.txt HH-MM-SS-mmm.txt"
	const char* l2 = leaf_of(p2);
	len = strlen(l2);
	CHECK(strncmp(l2, "gapcoll.txt ", 12) == 0);
	CHECK(len == strlen("gapcoll.txt 00-00-00-000.txt"));
	CHECK(len > 4 && strcmp(l2 + len - 4, ".txt") == 0);
	CHECK(len > 8 && l2[14] == '-' && l2[17] == '-' && l2[20] == '-');
	unlink(p1);
	unlink(p2);
}

static void test_shared_file_list(void)
{
	LSSharedFileListRef list = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
	FSRef bin;
	Boolean isDir;

	CHECK(list != NULL);
	if (!list)
		return;
	CHECK_ERR(FSPathMakeRef((const UInt8*) "/usr/bin/true", &bin, &isDir), noErr);

	LSSharedFileListItemRef item = LSSharedFileListInsertItemFSRef(list, kLSSharedFileListItemLast,
		CFSTR("gap-probe-item"), NULL, &bin, NULL, NULL);
	CHECK(item != NULL);
	if (item) {
		CFErrorRef error = NULL;
		CFURLRef url = LSSharedFileListItemCopyResolvedURL(item, 0, &error);
		CHECK(url != NULL && error == NULL);
		if (url) {
			char path[1024] = "";
			CFURLGetFileSystemRepresentation(url, true, (UInt8*) path, sizeof(path));
			CHECK(strcmp(path, "/usr/bin/true") == 0);
			CFRelease(url);
		}
		LSSharedFileListItemRemove(list, item);

		url = LSSharedFileListItemCopyResolvedURL(NULL, 0, &error);
		CHECK(url == NULL);
		if (error)
			CFRelease(error);
		// The item is not released: macOS crashes on releasing an item that was removed from its list.
	}

	// The next call passes NULL for a parameter marked nonnull on macOS; it answers NULL.
	CHECK(LSSharedFileListInsertItemFSRef(list, kLSSharedFileListItemLast, CFSTR("x"), NULL, NULL, NULL, NULL) == NULL);
	CFRelease(list);
}

int main(void)
{
	const char* tmp = getenv("TMPDIR");
	Boolean isDir;

	setvbuf(stdout, NULL, _IOLBF, 0);

	snprintf(g_dir, sizeof(g_dir), "%s/fmuni.XXXXXX", tmp && *tmp ? tmp : "/tmp");
	if (!mkdtemp(g_dir)) {
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	CHECK_ERR(FSPathMakeRef((const UInt8*) g_dir, &g_dirRef, &isDir), noErr);

	test_hfsunistr();
	test_create_and_make_ref();
	test_quirk_create_file_slash_becomes_colon();
	test_quirk_make_fsref_special_names();
	test_trash();
	test_quirk_trash_name_collision();
	test_shared_file_list();

	rmdir(g_dir);
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
