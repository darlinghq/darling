// Copyright (C) 2026 Lubos Dolezel
//
// FSCopyObjectSync copies a file or a whole directory tree, keeping symbolic
// links as links and file modes; it reports dupFNErr when the destination
// name is taken. FSMoveObjectSync renames and reports dupFNErr likewise.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void writeFile(const char* path, const char* content, mode_t mode)
{
	FILE* f = fopen(path, "w");
	CHECK(f != NULL);
	if (f)
	{
		fputs(content, f);
		fclose(f);
		chmod(path, mode);
	}
}

static int pathRef(const char* path, FSRef* ref)
{
	return FSPathMakeRef((const UInt8*) path, ref, NULL);
}

int main(void)
{
	char root[] = "/tmp/fsref_copy_object.XXXXXX";
	char path[PATH_MAX], path2[PATH_MAX], buf[PATH_MAX];
	FSRef srcRef, dstRef, newRef;
	struct stat st;

	if (!mkdtemp(root))
	{
		printf("FAIL mkdtemp\n");
		return 1;
	}

	snprintf(path, sizeof(path), "%s/src", root);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/src/sub", root);
	mkdir(path, 0500); // read-only directory, must still be filled in
	snprintf(path, sizeof(path), "%s/src/a.txt", root);
	writeFile(path, "alpha", 0640);
	snprintf(path, sizeof(path), "%s/src/link", root);
	CHECK(symlink("a.txt", path) == 0);
	snprintf(path, sizeof(path), "%s/dst", root);
	mkdir(path, 0755);

	// Files inside the read-only directory are created before it is locked
	snprintf(path, sizeof(path), "%s/src/sub", root);
	chmod(path, 0755);
	snprintf(path2, sizeof(path2), "%s/src/sub/b.txt", root);
	writeFile(path2, "bravo", 0600);
	chmod(path, 0500);

	snprintf(path, sizeof(path), "%s/src", root);
	CHECK(pathRef(path, &srcRef) == noErr);
	snprintf(path, sizeof(path), "%s/dst", root);
	CHECK(pathRef(path, &dstRef) == noErr);

	CHECK(FSCopyObjectSync(&srcRef, &dstRef, NULL, &newRef, 0) == noErr);

	snprintf(path, sizeof(path), "%s/dst/src/a.txt", root);
	CHECK(stat(path, &st) == 0 && (st.st_mode & 0777) == 0640 && st.st_size == 5);
	snprintf(path, sizeof(path), "%s/dst/src/sub/b.txt", root);
	CHECK(stat(path, &st) == 0 && (st.st_mode & 0777) == 0600 && st.st_size == 5);
	snprintf(path, sizeof(path), "%s/dst/src/sub", root);
	CHECK(stat(path, &st) == 0 && (st.st_mode & 0777) == 0500);
	snprintf(path, sizeof(path), "%s/dst/src/link", root);
	memset(buf, 0, sizeof(buf));
	CHECK(readlink(path, buf, sizeof(buf) - 1) == 5 && strcmp(buf, "a.txt") == 0);

	// The returned reference names the copy
	UInt8 resolved[PATH_MAX];
	CHECK(FSRefMakePath(&newRef, resolved, sizeof(resolved)) == noErr);
	CHECK(strstr((char*) resolved, "/dst/src") != NULL);

	// The destination name is taken now
	CHECK(FSCopyObjectSync(&srcRef, &dstRef, NULL, NULL, 0) == dupFNErr);

	// A different name for the copy
	CFStringRef name = CFSTR("renamed");
	CHECK(FSCopyObjectSync(&srcRef, &dstRef, name, NULL, 0) == noErr);
	snprintf(path, sizeof(path), "%s/dst/renamed/a.txt", root);
	CHECK(stat(path, &st) == 0);

	// A directory cannot be copied into itself
	snprintf(path, sizeof(path), "%s/src/a.txt", root);
	snprintf(path, sizeof(path), "%s/src/inner", root);
	mkdir(path, 0755);
	FSRef innerRef;
	CHECK(pathRef(path, &innerRef) == noErr);
	CHECK(FSCopyObjectSync(&srcRef, &innerRef, NULL, NULL, 0) == paramErr);

	// A missing source is an error, null arguments are rejected
	CHECK(FSCopyObjectSync(NULL, &dstRef, NULL, NULL, 0) == paramErr);
	CHECK(FSCopyObjectSync(&srcRef, NULL, NULL, NULL, 0) == paramErr);

	// Move: rename into the destination, then the name is taken
	snprintf(path, sizeof(path), "%s/moveme", root);
	writeFile(path, "m", 0644);
	FSRef moveRef;
	CHECK(pathRef(path, &moveRef) == noErr);
	CHECK(FSMoveObjectSync(&moveRef, &dstRef, NULL, NULL, 0) == noErr);
	snprintf(path, sizeof(path), "%s/dst/moveme", root);
	CHECK(stat(path, &st) == 0);
	snprintf(path, sizeof(path), "%s/moveme", root);
	CHECK(stat(path, &st) != 0);
	writeFile(path, "m", 0644);
	CHECK(pathRef(path, &moveRef) == noErr);
	CHECK(FSMoveObjectSync(&moveRef, &dstRef, NULL, NULL, 0) == dupFNErr);

	// Clean up (modes first, directories may be read-only)
	snprintf(path, sizeof(path), "chmod -R u+rwx '%s'; rm -rf '%s'", root, root);
	system(path);

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
