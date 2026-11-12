// Copyright (C) 2026 Lubos Dolezel
//
// FSCreateResFile reports its result through ResError(): creation, existing
// names, names that are not a single path component, and permissions given in
// the catalog information. FSSetCatalogInfo applies the permissions. EventRecord
// is packed to two bytes (what 0, message 2, when 10, where 14, modifiers 18).
// The program passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static OSErr create(const FSRef* dir, const char* name, const FSCatalogInfo* info, FSCatalogInfoBitmap which)
{
	UniChar chars[64];
	size_t length = strlen(name);
	for (size_t i = 0; i < length; i++)
		chars[i] = (UniChar) name[i];
	FSRef created;
	FSSpec spec;
	memset(&created, 0, sizeof(created));
	FSCreateResFile(dir, (UniCharCount) length, chars, which, info, &created, &spec);
	return ResError();
}

static int exists(const char* dir, const char* name)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	return access(path, F_OK) == 0;
}

static void test_create_res_file(void)
{
	char dir[] = "/tmp/create_res_file_XXXXXX";
	CHECK(mkdtemp(dir) != NULL);
	FSRef dirRef;
	CHECK(FSPathMakeRef((const UInt8*) dir, &dirRef, NULL) == noErr);

	CHECK(create(&dirRef, "a.rsrc", NULL, 0) == noErr);
	CHECK(exists(dir, "a.rsrc"));
	CHECK(create(&dirRef, "a.rsrc", NULL, 0) == dupFNErr);
	CHECK(create(&dirRef, "..", NULL, 0) == bdNamErr);
	CHECK(create(&dirRef, ".", NULL, 0) == errFSBadForkName);

	// A slash in a File Manager name is stored as a colon.
	CHECK(create(&dirRef, "x/y", NULL, 0) == noErr);
	CHECK(exists(dir, "x:y"));

	FSCatalogInfo info;
	memset(&info, 0, sizeof(info));
	info.permissions.mode = 0600;
	CHECK(create(&dirRef, "perm.rsrc", &info, kFSCatInfoPermissions) == noErr);
	char path[512];
	snprintf(path, sizeof(path), "%s/perm.rsrc", dir);
	struct stat st;
	CHECK(stat(path, &st) == 0 && (st.st_mode & 07777) == 0600);

	FSRef fileRef;
	CHECK(FSPathMakeRef((const UInt8*) path, &fileRef, NULL) == noErr);
	info.permissions.mode = 0640;
	CHECK(FSSetCatalogInfo(&fileRef, kFSCatInfoPermissions, &info) == noErr);
	CHECK(stat(path, &st) == 0 && (st.st_mode & 07777) == 0640);
	CHECK(FSSetCatalogInfo(&fileRef, kFSCatInfoPermissions, NULL) == errFSMissingCatInfo);

	char command[600];
	snprintf(command, sizeof(command), "rm -rf %s", dir);
	CHECK(system(command) == 0);
}

static void test_event_record_layout(void)
{
	CHECK(sizeof(EventRecord) == 20);
	CHECK(offsetof(EventRecord, what) == 0);
	CHECK(offsetof(EventRecord, message) == 2);
	CHECK(offsetof(EventRecord, when) == 10);
	CHECK(offsetof(EventRecord, where) == 14);
	CHECK(offsetof(EventRecord, modifiers) == 18);

	// IsCmdChar compares key codes: the period key is 0x2F, so a message holding only the
	// character '.' (key code 0, the A key) does not match, while key code 0x2F does.
	EventRecord key = { keyDown, '.', 0, { 0, 0 }, cmdKey };
	CHECK(!IsCmdChar(&key, '.'));
	key.message = 0x2F00 | '.';
	CHECK(IsCmdChar(&key, '.'));
}

int main(void)
{
	test_create_res_file();
	test_event_record_layout();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
