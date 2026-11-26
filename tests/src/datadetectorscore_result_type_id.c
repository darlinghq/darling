// Copyright (C) 2026 Lubos Dolezel
//
// A DDResult of the private DataDetectorsCore framework is a CoreFoundation
// object; DDResultGetCFTypeID gives its type ID, which WebKit compares with
// CFGetTypeID to tell results from other CF objects. Checked: the type ID is
// a registered one, the same on every call, distinct from the IDs of other CF
// types, and described as "DDResult".
// build: guest-cc.sh -o t datadetectorscore_result_type_id.c -framework CoreFoundation
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL: %s\n", what); failures++; } else printf("PASS: %s\n", what); } while (0)

int main(void)
{
	void *handle = dlopen("/System/Library/PrivateFrameworks/DataDetectorsCore.framework/DataDetectorsCore", RTLD_NOW);
	CHECK(handle != NULL, "DataDetectorsCore loads");
	if (!handle)
		return 1;
	CFTypeID (*getTypeID)(void) = (CFTypeID (*)(void))dlsym(handle, "DDResultGetCFTypeID");
	CHECK(getTypeID != NULL, "DDResultGetCFTypeID is exported");
	if (!getTypeID)
		return 1;

	CFTypeID typeID = getTypeID();
	CHECK(typeID != 0, "the type ID is a registered one");
	CHECK(getTypeID() == typeID, "the type ID is the same on every call");
	CHECK(typeID != CFStringGetTypeID() && typeID != CFArrayGetTypeID()
		&& typeID != CFDictionaryGetTypeID() && typeID != CFDataGetTypeID(),
		"the type ID differs from other CF types'");

	CFStringRef description = CFCopyTypeIDDescription(typeID);
	CHECK(description != NULL && CFEqual(description, CFSTR("DDResult")), "the type is described as DDResult");
	if (description)
		CFRelease(description);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
