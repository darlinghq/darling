// Copyright (C) 2026 Lubos Dolezel
//
// Apple Event descriptors of file specifications (32-bit only: the FSSpec is not in the 64-bit API). A descriptor of
// the type typeFSS ('fss ') coerces to typeFSRef and back, the same file results, and a list whose one item is a
// file specification coerces to either of them (AEGetNthPtr with the desired type). A specification of a file that
// is not there does not coerce to a file reference; descriptors of other types do not coerce to either.
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__

int main(void)
{
	printf("PASS ae_fsspec_coercion (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// The file specification of a file: the volume and directory ID of its folder, and its name
static void spec_of(const char *path, FSSpec *spec)
{
	char dir[512];
	FSRef folder;
	FSCatalogInfo info;
	strlcpy(dir, path, sizeof(dir));
	*strrchr(dir, '/') = 0;
	const char *leaf = strrchr(path, '/') + 1;
	Str255 name;
	name[0] = (unsigned char) strlen(leaf);
	memcpy(name + 1, leaf, name[0]);
	if (FSPathMakeRef((const UInt8 *) dir, &folder, NULL) != noErr
	    || FSGetCatalogInfo(&folder, kFSCatInfoVolume | kFSCatInfoNodeID, &info, NULL, NULL, NULL) != noErr
	    || FSMakeFSSpec(info.volume, (SInt32) info.nodeID, name, spec) != noErr) {
		printf("FAIL: no FSSpec for %s\n", path);
		exit(2);
	}
}

int main(void)
{
	CHECK(typeFSS == 'fss ');
	char path[] = "/tmp/darling-aefss-XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0)
		return 2;
	close(fd);

	FSRef ref;
	FSSpec spec;
	CHECK(FSPathMakeRef((const UInt8 *) path, &ref, NULL) == noErr);
	spec_of(path, &spec);

	// FSSpec to FSRef
	AEDesc desc;
	CHECK(AECreateDesc(typeFSS, &spec, sizeof(spec), &desc) == noErr);
	FSRef back;
	DescType actual = 0;
	Size size = 0;
	CHECK(AEGetDescData(&desc, &spec, sizeof(spec)) == noErr);
	AEDesc coerced;
	CHECK(AECoerceDesc(&desc, typeFSRef, &coerced) == noErr && coerced.descriptorType == typeFSRef);
	CHECK(AEGetDescDataSize(&coerced) == sizeof(FSRef));
	CHECK(AEGetDescData(&coerced, &back, sizeof(back)) == noErr);
	UInt8 backPath[1024];
	CHECK(FSRefMakePath(&back, backPath, sizeof(backPath)) == noErr && strcmp((char *) backPath, path) == 0);
	AEDisposeDesc(&coerced);

	// FSRef to FSSpec: the specification names the same file
	AEDesc refDesc;
	CHECK(AECreateDesc(typeFSRef, &ref, sizeof(ref), &refDesc) == noErr);
	CHECK(AECoerceDesc(&refDesc, typeFSS, &coerced) == noErr && coerced.descriptorType == typeFSS);
	FSSpec spec2;
	CHECK(AEGetDescDataSize(&coerced) == sizeof(FSSpec));
	CHECK(AEGetDescData(&coerced, &spec2, sizeof(spec2)) == noErr);
	CHECK(spec2.vRefNum == spec.vRefNum && spec2.parID == spec.parID && spec2.name[0] == spec.name[0]
		&& memcmp(spec2.name + 1, spec.name + 1, spec.name[0]) == 0);
	AEDisposeDesc(&coerced);
	AEDisposeDesc(&refDesc);

	// a list of one specification: its items coerce through AEGetNthPtr
	AEDescList list;
	CHECK(AECreateList(NULL, 0, false, &list) == noErr);
	CHECK(AEPutPtr(&list, 1, typeFSS, &spec, sizeof(spec)) == noErr);
	AEKeyword keyword;
	FSRef item;
	CHECK(AEGetNthPtr(&list, 1, typeFSRef, &keyword, &actual, &item, sizeof(item), &size) == noErr);
	CHECK(actual == typeFSRef && size == sizeof(FSRef));
	FSSpec itemSpec;
	CHECK(AEGetNthPtr(&list, 1, typeFSS, &keyword, &actual, &itemSpec, sizeof(itemSpec), &size) == noErr);
	CHECK(actual == typeFSS && size == sizeof(FSSpec) && itemSpec.parID == spec.parID);
	AEDisposeDesc(&list);

	// a specification of a file that is gone, and of the wrong size and type
	unlink(path);
	CHECK(AECoerceDesc(&desc, typeFSRef, &coerced) == errAECoercionFail);
	AEDisposeDesc(&desc);
	char little[8] = { 0 };
	CHECK(AECreateDesc(typeFSS, little, sizeof(little), &desc) == noErr);
	CHECK(AECoerceDesc(&desc, typeFSRef, &coerced) == errAECoercionFail);
	AEDisposeDesc(&desc);
	CHECK(AECreateDesc(typeChar, "abc", 3, &desc) == noErr);
	CHECK(AECoerceDesc(&desc, typeFSS, &coerced) == errAECoercionFail);
	CHECK(AECoerceDesc(&desc, typeFSRef, &coerced) == errAECoercionFail);
	AEDisposeDesc(&desc);

	printf(g_failures ? "FAIL ae_fsspec_coercion\n" : "PASS ae_fsspec_coercion\n");
	return g_failures != 0;
}
#endif
