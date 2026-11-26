// Copyright (C) 2026 Lubos Dolezel
//
// Alias Manager: FSNewAlias and FSNewAliasMinimal make an alias record of an
// item, FSCopyAliasInfo reads its target name, volume name, resolved path,
// and the dates and flags that are valid, from the record alone. The size
// of the record is in its header and GetAliasSize reports it.
// The program passes unchanged on macOS 26.7.3.
#include <CoreServices/CoreServices.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void makeRef(const char* path, FSRef* ref)
{
	OSStatus err = FSPathMakeRef((const UInt8*) path, ref, NULL);
	if (err != noErr) {
		printf("FAIL: FSPathMakeRef(%s) = %d\n", path, (int) err);
		g_failures++;
	}
}

static void utf8(const HFSUniStr255* name, char* out, size_t size)
{
	CFStringRef string = CFStringCreateWithCharacters(NULL, name->unicode, name->length);
	out[0] = 0;
	CFStringGetCString(string, out, (CFIndex) size, kCFStringEncodingUTF8);
	CFRelease(string);
}

static void pathOf(CFStringRef string, char* out, size_t size)
{
	out[0] = 0;
	if (string)
		CFStringGetCString(string, out, (CFIndex) size, kCFStringEncodingUTF8);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	char dir[] = "/tmp/vdaliasXXXXXX";
	if (mkdtemp(dir) == NULL) {
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	char file[PATH_MAX], sub[PATH_MAX], link[PATH_MAX], other[PATH_MAX], unicodeFile[PATH_MAX];
	snprintf(file, sizeof file, "%s/file.txt", dir);
	snprintf(sub, sizeof sub, "%s/sub", dir);
	snprintf(link, sizeof link, "%s/link", dir);
	snprintf(other, sizeof other, "%s/other.txt", dir);
	snprintf(unicodeFile, sizeof unicodeFile, "%s/\xC3\x9Cn\xC3\xAF file.txt", dir);
	FILE* f = fopen(file, "w");
	fputs("x", f);
	fclose(f);
	f = fopen(other, "w");
	fputs("x", f);
	fclose(f);
	f = fopen(unicodeFile, "w");
	fputs("x", f);
	fclose(f);
	mkdir(sub, 0755);
	symlink(file, link);

	char expected[PATH_MAX];
	FSRef target, from;

	// A file.
	makeRef(file, &target);
	AliasHandle alias = NULL;
	CHECK(FSNewAlias(NULL, &target, &alias) == noErr);
	CHECK(alias != NULL);
	if (alias == NULL)
		return 1;
	HFSUniStr255 targetName, volumeName;
	CFStringRef path = NULL;
	FSAliasInfoBitmap which = 0xFFFFFFFF;
	FSAliasInfo info;
	memset(&info, 0x5A, sizeof info);
	CHECK(FSCopyAliasInfo(alias, &targetName, &volumeName, &path, &which, &info) == noErr);
	char text[1024];
	utf8(&targetName, text, sizeof text);
	CHECK(strcmp(text, "file.txt") == 0);
	utf8(&volumeName, text, sizeof text);
	CHECK(strlen(text) > 0);
	pathOf(path, text, sizeof text);
	CHECK(realpath(file, expected) != NULL && strcmp(text, expected) == 0);
	if (path)
		CFRelease(path);
	const FSAliasInfoBitmap wanted = kFSAliasInfoVolumeCreateDate | kFSAliasInfoTargetCreateDate | kFSAliasInfoIsDirectory | kFSAliasInfoVolumeFlags;
	CHECK((which & wanted) == wanted);
	CHECK(info.isDirectory == false);
	CHECK(info.volumeIsBootVolume == true);
	FSCatalogInfo catalog;
	CHECK(FSGetCatalogInfo(&target, kFSCatInfoCreateDate, &catalog, NULL, NULL, NULL) == noErr);
	CHECK(info.targetCreateDate.highSeconds == catalog.createDate.highSeconds && info.targetCreateDate.lowSeconds == catalog.createDate.lowSeconds);
	CHECK(info.volumeCreateDate.lowSeconds != 0 || info.volumeCreateDate.highSeconds != 0);

	// The record: size in the header (big-endian) after a zero user type, and the same as the handle's.
	Size size = GetAliasSize(alias);
	// (macOS gives a handle of exactly that size; here it may be rounded up.)
	CHECK(size > 6 && size <= GetHandleSize((Handle) alias));
	const unsigned char* bytes = (const unsigned char*) *alias;
	CHECK(bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 0 && bytes[3] == 0);
	CHECK((((unsigned) bytes[4] << 8) | bytes[5]) == (unsigned) size);

	// Output parameters are all optional.
	CHECK(FSCopyAliasInfo(alias, NULL, NULL, NULL, NULL, NULL) == noErr);
	DisposeHandle((Handle) alias);

	// The file the alias is made from does not matter.
	makeRef(other, &from);
	alias = NULL;
	CHECK(FSNewAlias(&from, &target, &alias) == noErr && alias != NULL);
	path = NULL;
	CHECK(FSCopyAliasInfo(alias, NULL, NULL, &path, NULL, NULL) == noErr);
	pathOf(path, text, sizeof text);
	CHECK(strcmp(text, expected) == 0);
	if (path)
		CFRelease(path);
	DisposeHandle((Handle) alias);

	// A directory.
	makeRef(sub, &target);
	alias = NULL;
	CHECK(FSNewAlias(NULL, &target, &alias) == noErr && alias != NULL);
	memset(&info, 0, sizeof info);
	which = 0;
	CHECK(FSCopyAliasInfo(alias, &targetName, NULL, NULL, &which, &info) == noErr);
	utf8(&targetName, text, sizeof text);
	CHECK(strcmp(text, "sub") == 0);
	CHECK((which & kFSAliasInfoIsDirectory) && info.isDirectory == true);
	DisposeHandle((Handle) alias);

	// A symbolic link: the alias holds the path of what it points to.
	makeRef(link, &target);
	alias = NULL;
	CHECK(FSNewAlias(NULL, &target, &alias) == noErr && alias != NULL);
	path = NULL;
	CHECK(FSCopyAliasInfo(alias, NULL, NULL, &path, NULL, NULL) == noErr);
	pathOf(path, text, sizeof text);
	CHECK(strcmp(text, expected) == 0);
	if (path)
		CFRelease(path);
	DisposeHandle((Handle) alias);

	// A minimal alias has the names and the path but no further information.
	makeRef(file, &target);
	alias = NULL;
	CHECK(FSNewAliasMinimal(&target, &alias) == noErr && alias != NULL);
	which = 0xFFFFFFFF;
	path = NULL;
	CHECK(FSCopyAliasInfo(alias, &targetName, NULL, &path, &which, NULL) == noErr);
	utf8(&targetName, text, sizeof text);
	CHECK(strcmp(text, "file.txt") == 0);
	pathOf(path, text, sizeof text);
	CHECK(strcmp(text, expected) == 0);
	if (path)
		CFRelease(path);
	CHECK(which == kFSAliasInfoNone);
	CHECK(GetAliasSize(alias) > 6 && GetAliasSize(alias) <= GetHandleSize((Handle) alias));
	DisposeHandle((Handle) alias);

	// A name outside ASCII; its length is counted in UTF-16 units.
	makeRef(unicodeFile, &target);
	alias = NULL;
	CHECK(FSNewAlias(NULL, &target, &alias) == noErr && alias != NULL);
	path = NULL;
	CHECK(FSCopyAliasInfo(alias, &targetName, NULL, &path, NULL, NULL) == noErr);
	CHECK(targetName.length == 12);
	utf8(&targetName, text, sizeof text);
	CHECK(strcmp(text, "\xC3\x9Cn\xC3\xAF file.txt") == 0);
	pathOf(path, text, sizeof text);
	CHECK(strstr(text, "\xC3\x9Cn\xC3\xAF file.txt") != NULL);
	if (path)
		CFRelease(path);
	DisposeHandle((Handle) alias);

	// Errors.
	CHECK(FSCopyAliasInfo(NULL, NULL, NULL, NULL, NULL, NULL) == paramErr);
	FSRef bad;
	memset(&bad, 0, sizeof bad);
	alias = NULL;
	CHECK(FSNewAlias(NULL, &bad, &alias) == errFSBadFSRef);
	CHECK(alias == NULL);

	unlink(link);
	unlink(file);
	unlink(other);
	unlink(unicodeFile);
	rmdir(sub);
	rmdir(dir);
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
