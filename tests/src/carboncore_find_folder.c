// Copyright (C) 2026 Lubos Dolezel
//
// Darling takes the temporary folder from TMPDIR and the desktop from the home directory, so that these
// tests can place them. FSFindFolder reports why a folder cannot be had instead of returning noErr with an unusable FSRef: a missing
// folder is fnfErr unless it is to be created, a path that is not a folder is dirNFErr, a missing home directory
// is fnfErr and a NULL result is paramErr. On 32-bit the FindFolder adapter returns the volume and directory ID
// of the same folder, with the same errors. Built with -fpascal-strings.
#include <CoreServices/CoreServices.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// kOnSystemDisk of Folders.h
#define volumeOnSystemDisk (-32768)

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// FSRefMakePath gives the resolved path, which differs from a path through a symbolic link such as /tmp
static int samePath(const char* a, const char* b)
{
	char ra[PATH_MAX], rb[PATH_MAX];
	return realpath(a, ra) && realpath(b, rb) && strcmp(ra, rb) == 0;
}

static int isDirectory(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int main(void)
{
	char root[] = "/tmp/carboncore_find_folder.XXXXXX";
	char path[600], missing[600], file[600], got[1024];
	FSRef ref;

	CHECK(mkdtemp(root) != NULL);
	snprintf(missing, sizeof(missing), "%s/missing", root);
	snprintf(file, sizeof(file), "%s/file", root);
	close(open(file, O_CREAT | O_WRONLY, 0644));

	// An existing temporary folder
	setenv("TMPDIR", root, 1);
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &ref) == noErr);
	CHECK(FSRefMakePath(&ref, (UInt8*) got, sizeof(got)) == noErr && samePath(got, root));

	// A missing one is an error unless it is to be created
	setenv("TMPDIR", missing, 1);
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &ref) == fnfErr);
	CHECK(!isDirectory(missing));
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, true, &ref) == noErr);
	CHECK(isDirectory(missing));
	CHECK(FSRefMakePath(&ref, (UInt8*) got, sizeof(got)) == noErr && samePath(got, missing));

	// A path that is not a folder
	setenv("TMPDIR", file, 1);
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &ref) == dirNFErr);
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, true, &ref) == dirNFErr);

	// Nothing to write the result to
	setenv("TMPDIR", root, 1);
	CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, NULL) == paramErr);

	// The desktop is under the home directory; a missing home directory gives fnfErr
	setenv("HOME", root, 1);
	snprintf(path, sizeof(path), "%s/Desktop", root);
	CHECK(FSFindFolder(volumeOnSystemDisk, kDesktopFolderType, false, &ref) == fnfErr);
	CHECK(FSFindFolder(volumeOnSystemDisk, kDesktopFolderType, true, &ref) == noErr);
	CHECK(isDirectory(path));
	CHECK(FSRefMakePath(&ref, (UInt8*) got, sizeof(got)) == noErr && samePath(got, path));
	setenv("HOME", "", 1);
	CHECK(FSFindFolder(volumeOnSystemDisk, kDesktopFolderType, false, &ref) == fnfErr);
	setenv("HOME", root, 1);

	// A folder type that is not supported
	CHECK(FSFindFolder(volumeOnSystemDisk, 'zzzz', false, &ref) == unimpErr);

#ifndef __LP64__
	{
		SInt16 volume = -1;
		SInt32 directory = -1;
		FSCatalogInfo info;

		setenv("TMPDIR", root, 1);
		CHECK(FindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &volume, &directory) == noErr);
		CHECK(FSFindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &ref) == noErr);
		CHECK(FSGetCatalogInfo(&ref, kFSCatInfoNodeID | kFSCatInfoVolume, &info, NULL, NULL, NULL) == noErr);
		CHECK(directory == (SInt32) info.nodeID && volume == info.volume);

		setenv("TMPDIR", file, 1);
		volume = directory = 77;
		CHECK(FindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &volume, &directory) == dirNFErr);
		setenv("TMPDIR", missing, 1);
		CHECK(FindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &volume, &directory) == noErr);
		CHECK(FindFolder(volumeOnSystemDisk, 'zzzz', false, &volume, &directory) == unimpErr);
		CHECK(FindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, NULL, &directory) == paramErr);
		CHECK(FindFolder(volumeOnSystemDisk, kTemporaryFolderType, false, &volume, NULL) == paramErr);
	}
#endif

	rmdir(missing);
	snprintf(path, sizeof(path), "%s/Desktop", root);
	rmdir(path);
	unlink(file);
	rmdir(root);
	if (g_failures) {
		printf("%d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS carboncore_find_folder\n");
	return 0;
}
