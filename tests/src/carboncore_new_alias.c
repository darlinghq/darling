// Copyright (C) 2026 Lubos Dolezel
//
// NewAlias of the 32-bit Alias Manager makes an alias record of the item an FSSpec names, like FSNewAlias does
// of an FSRef: the record has a size and FSCopyAliasInfo reads the target name from it. A NULL result or target is paramErr, a missing target is fnfErr and
// the result is NULL after an error. Built with -fpascal-strings.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
#ifdef __LP64__
	printf("SKIP carboncore_new_alias (32-bit only)\n");
	return 0;
#else
	char dir[] = "/tmp/carboncore_new_alias.XXXXXX";
	char path[300];
	FSSpec target, from, missing;
	AliasHandle alias = (AliasHandle) 1;
	HFSUniStr255 name;
	FSAliasInfoBitmap which = 0;
	FSAliasInfo info;
	Str255 pathName;

	CHECK(mkdtemp(dir) != NULL);
	snprintf(path, sizeof(path), "%s/target.txt", dir);
	pathName[0] = strlen(path);
	memcpy(pathName + 1, path, pathName[0]);
	CHECK(FSMakeFSSpec(0, 0, pathName, &target) == fnfErr);
	CHECK(FSpCreate(&target, 'ttxt', 'TEXT', 0) == noErr);
	CHECK(FSMakeFSSpec(0, 0, pathName, &target) == noErr);

	CHECK(NewAlias(NULL, &target, &alias) == noErr);
	CHECK(alias != NULL && *alias != NULL && GetAliasSize(alias) >= 6);
	memset(&name, 0, sizeof(name));
	CHECK(FSCopyAliasInfo(alias, &name, NULL, NULL, &which, &info) == noErr);
	CHECK(name.length == 10 && name.unicode[0] == 't' && name.unicode[9] == 't');
	CHECK(!(which & kFSAliasInfoIsDirectory) || !info.isDirectory);
	DisposeHandle((Handle) alias);

	// A from-file that exists is accepted
	snprintf(path, sizeof(path), "%s", dir);
	pathName[0] = strlen(path);
	memcpy(pathName + 1, path, pathName[0]);
	CHECK(FSMakeFSSpec(0, 0, pathName, &from) == noErr);
	alias = (AliasHandle) 1;
	CHECK(NewAlias(&from, &target, &alias) == noErr && alias != NULL);
	DisposeHandle((Handle) alias);

	// Errors
	alias = (AliasHandle) 1;
	CHECK(NewAlias(NULL, NULL, &alias) == paramErr && alias == NULL);
	CHECK(NewAlias(NULL, &target, NULL) == paramErr);
	snprintf(path, sizeof(path), "%s/none.txt", dir);
	pathName[0] = strlen(path);
	memcpy(pathName + 1, path, pathName[0]);
	CHECK(FSMakeFSSpec(0, 0, pathName, &missing) == fnfErr);
	alias = (AliasHandle) 1;
	CHECK(NewAlias(NULL, &missing, &alias) == fnfErr && alias == NULL);

	CHECK(FSpDelete(&target) == noErr);
	rmdir(dir);
	if (g_failures) {
		printf("%d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS carboncore_new_alias\n");
	return 0;
#endif
}
