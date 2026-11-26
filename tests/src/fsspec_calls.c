// Copyright (C) 2026 Lubos Dolezel
//
// The FSSpec calls of the 32-bit File Manager: FSMakeFSSpec, FSpMakeFSRef, FSpCreate, FSpDelete and the
// Finder information calls, and the FSSpec FSCreateFileUnicode returns. Built with -fpascal-strings.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
#ifdef __LP64__
	printf("SKIP fsspec_calls (32-bit only)\n");
	return 0;
#else
	char dir[] = "/tmp/fsspec_calls.XXXXXX";
	FSRef dirRef, fileRef, same;
	FSSpec spec, dirSpec, other;
	FInfo info;
	Boolean isDirectory;
	Str255 name = "\pdoc.txt";
	char path[300];

	CHECK(mkdtemp(dir) != NULL);
	CHECK(FSPathMakeRef((const UInt8*) dir, &dirRef, &isDirectory) == noErr && isDirectory);

	// The spec of an item that is not there is filled in, with fnfErr
	HFSUniStr255 leaf;
	FSSpec parentSpec;
	CHECK(FSGetCatalogInfo(&dirRef, kFSCatInfoNodeID, NULL, &leaf, &parentSpec, NULL) == noErr);
	FSCatalogInfo catalog;
	CHECK(FSGetCatalogInfo(&dirRef, kFSCatInfoNodeID, &catalog, NULL, NULL, NULL) == noErr);
	CHECK(FSMakeFSSpec(-100, catalog.nodeID, name, &spec) == fnfErr);
	CHECK(spec.parID == (SInt32) catalog.nodeID && spec.name[0] == 7 && memcmp(spec.name + 1, "doc.txt", 7) == 0);
	CHECK(FSpMakeFSRef(&spec, &fileRef) == fnfErr);

	// QUIRK: a directory ID that names no directory gives fnfErr (observed on macOS 10.14.4 i386, tier 2)
	CHECK(FSMakeFSSpec(-100, 0x7fffff00, name, &other) == fnfErr);

	// An empty name is the directory itself
	CHECK(FSMakeFSSpec(-100, catalog.nodeID, "\p", &other) == noErr && other.name[0] == strlen(strrchr(dir, 0x2f) + 1));

	CHECK(FSpCreate(&spec, 'ttxt', 'TEXT', 0) == noErr);
	CHECK(FSpCreate(&spec, 'ttxt', 'TEXT', 0) == dupFNErr);
	CHECK(FSMakeFSSpec(-100, catalog.nodeID, name, &other) == noErr);
	CHECK(FSpMakeFSRef(&other, &fileRef) == noErr);
	CHECK(FSRefMakePath(&fileRef, (UInt8*) path, sizeof(path)) == noErr);
	CHECK(strcmp(strrchr(path, 0x2f), "/doc.txt") == 0);

	CHECK(FSpGetFInfo(&spec, &info) == noErr && info.fdType == 'TEXT' && info.fdCreator == 'ttxt');
	info.fdType = 'PDF ';
	info.fdCreator = 'CSKt';
	info.fdFlags = 0x0100;
	CHECK(FSpSetFInfo(&spec, &info) == noErr);
	memset(&info, 0, sizeof(info));
	CHECK(FSpGetFInfo(&spec, &info) == noErr && info.fdType == 'PDF ' && info.fdCreator == 'CSKt' && info.fdFlags == 0x0100);

	// The spec FSCreateFileUnicode gives names the new file
	UniChar created[] = { 'n', 'e', 'w' };
	CHECK(FSCreateFileUnicode(&dirRef, 3, created, kFSCatInfoNone, NULL, &same, &other) == noErr);
	CHECK(other.parID == (SInt32) catalog.nodeID && other.name[0] == 3 && memcmp(other.name + 1, "new", 3) == 0);
	CHECK(FSpDelete(&other) == noErr);
	CHECK(FSpDelete(&other) == fnfErr);
	CHECK(FSpDelete(&spec) == noErr);
	CHECK(FSpGetFInfo(&spec, &info) == fnfErr);

	// A directory that is not empty is busy
	CHECK(FSMakeFSSpec(-100, catalog.nodeID, "\psub", &dirSpec) == fnfErr);
	snprintf(path, sizeof(path), "%s/sub", dir);
	mkdir(path, 0777);
	CHECK(FSMakeFSSpec(-100, catalog.nodeID, "\psub", &dirSpec) == noErr);
	snprintf(path, sizeof(path), "%s/sub/x", dir);
	close(open(path, O_CREAT | O_WRONLY, 0666));
	CHECK(FSpDelete(&dirSpec) == fBsyErr);
	unlink(path);
	CHECK(FSpDelete(&dirSpec) == noErr);
	rmdir(dir);

	printf(g_failures ? "FAIL fsspec_calls\n" : "PASS fsspec_calls\n");
	return g_failures != 0;
#endif
}
