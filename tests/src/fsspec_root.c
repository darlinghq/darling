// Copyright (C) 2026 Lubos Dolezel
//
// Directory IDs of the volume root in the 32-bit File Manager: fsRtParID with the volume's name names the root
// directory, fsRtDirID names the root directory itself. Round trip through FSGetCatalogInfo, FSMakeFSSpec and
// FSpMakeFSRef. Built with -fpascal-strings.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
#ifdef __LP64__
	printf("SKIP fsspec_root (32-bit only)\n");
	return 0;
#else
	FSRef rootRef, back, item, itemBack;
	FSSpec catalogSpec, spec, other;
	Str255 volumeName;
	Boolean isDirectory;

	CHECK(FSPathMakeRef((const UInt8*) "/", &rootRef, &isDirectory) == noErr && isDirectory);
	CHECK(FSGetCatalogInfo(&rootRef, kFSCatInfoNone, NULL, NULL, &catalogSpec, NULL) == noErr);
	CHECK(catalogSpec.parID == fsRtParID);
	memcpy(volumeName, catalogSpec.name, catalogSpec.name[0] + 1);

	// The spec FSGetCatalogInfo gave resolves through FSMakeFSSpec, to the same spec and the same FSRef
	memset(&spec, 0x55, sizeof(spec));
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtParID, volumeName, &spec) == noErr);
	CHECK(spec.vRefNum == catalogSpec.vRefNum && spec.parID == fsRtParID);
	CHECK(spec.name[0] == volumeName[0] && memcmp(spec.name, volumeName, volumeName[0] + 1) == 0);
	CHECK(FSpMakeFSRef(&catalogSpec, &back) == noErr && FSCompareFSRefs(&rootRef, &back) == noErr);
	CHECK(FSpMakeFSRef(&spec, &back) == noErr && FSCompareFSRefs(&rootRef, &back) == noErr);

	// fsRtDirID with an empty name is the root directory as well
	memset(&other, 0x55, sizeof(other));
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtDirID, "\p", &other) == noErr);
	CHECK(other.parID == fsRtParID && other.name[0] == volumeName[0] && memcmp(other.name, volumeName, volumeName[0] + 1) == 0);
	CHECK(FSpMakeFSRef(&other, &back) == noErr && FSCompareFSRefs(&rootRef, &back) == noErr);

	// Items under fsRtDirID
	CHECK(FSPathMakeRef((const UInt8*) "/tmp", &item, &isDirectory) == noErr);
	memset(&other, 0x55, sizeof(other));
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtDirID, "\ptmp", &other) == noErr);
	CHECK(other.parID == fsRtDirID && other.name[0] == 3 && memcmp(other.name + 1, "tmp", 3) == 0);
	CHECK(FSpMakeFSRef(&other, &itemBack) == noErr && FSCompareFSRefs(&item, &itemBack) == noErr);
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtDirID, "\pnonexistent_fsspec_root_zz", &other) == fnfErr);
	CHECK(other.parID == fsRtDirID && other.name[0] == 26);

	// An item named under fsRtParID, as the item's own parent, goes through the root spec of its FSRef
	CHECK(FSGetCatalogInfo(&item, kFSCatInfoParentDirID, NULL, NULL, &other, NULL) == noErr && other.parID == fsRtDirID);

	// Bad names under fsRtParID name no volume; the spec is not filled in
	memset(&other, 0x55, sizeof(other));
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtParID, "\pno such volume", &other) == nsvErr);
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtParID, "\p", &other) == nsvErr);
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtParID, "\ptmp", &other) == nsvErr);

	// A spec under fsRtParID that does not carry the volume's name has no item
	memcpy(&other, &catalogSpec, sizeof(other));
	other.name[0] = 3;
	memcpy(other.name + 1, "tmp", 3);
	CHECK(FSpMakeFSRef(&other, &back) != noErr);

	// A name that is too long and an unknown directory ID still fail as before
	{
		Str255 longName;
		longName[0] = 255;
		memset(longName + 1, 'a', 255);
		CHECK(FSMakeFSSpec(catalogSpec.vRefNum, fsRtDirID, longName, &other) == bdNamErr);
	}
	CHECK(FSMakeFSSpec(catalogSpec.vRefNum, 0x7fffff00, "\ptmp", &other) == fnfErr);

	printf(g_failures ? "FAIL fsspec_root\n" : "PASS fsspec_root\n");
	return g_failures != 0;
#endif
}
