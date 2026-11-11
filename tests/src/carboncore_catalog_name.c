// Copyright (C) 2026 Lubos Dolezel
//
// FSGetCatalogInfo returns the item's own name (last path component) in
// UTF-16 units, and the OSType <-> string helpers keep all four bytes and work
// with strings that have no direct 8-bit storage.
#include <CoreServices/CoreServices.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { printf("FAIL: %s (", #cond); printf(__VA_ARGS__); printf(")\n"); failures++; } \
} while (0)

static void test_catalog_name(void)
{
	char dir[] = "/tmp/catname.XXXXXX";
	if (!mkdtemp(dir)) { perror("mkdtemp"); failures++; return; }

	// "h" U+00E9 "llo " U+4E16 U+754C ".txt": non-ASCII, so UTF-8 bytes != UTF-16 units
	const char* leaf = "h\xc3\xa9llo \xe4\xb8\x96\xe7\x95\x8c.txt";
	const UniChar expected[] = { 'h', 0xE9, 'l', 'l', 'o', ' ', 0x4E16, 0x754C, '.', 't', 'x', 't' };
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", dir, leaf);
	int fd = open(path, O_CREAT | O_WRONLY, 0600);
	if (fd < 0) { perror("open"); failures++; return; }
	close(fd);

	FSRef ref;
	OSStatus status = FSPathMakeRef((const UInt8*) path, &ref, NULL);
	CHECK(status == noErr, "status %d", (int) status);

	HFSUniStr255 name;
	memset(&name, 0xaa, sizeof(name));
	status = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, &name, NULL, NULL);
	CHECK(status == noErr, "status %d", (int) status);
	CHECK(name.length == sizeof(expected) / sizeof(expected[0]), "length %u", name.length);
	CHECK(name.length == sizeof(expected) / sizeof(expected[0]) &&
		memcmp(name.unicode, expected, sizeof(expected)) == 0, "name is not the last path component");

	unlink(path);
	rmdir(dir);
}

static void test_ostype(void)
{
	CFStringRef tag = UTCreateStringForOSType(0x54455854);
	CHECK(tag && CFStringCompare(tag, CFSTR("TEXT"), 0) == kCFCompareEqualTo, "four characters expected");
	if (tag)
	{
		OSType value = UTGetOSTypeFromString(tag);
		CHECK(value == 0x54455854, "0x%08x", (unsigned) value);
		CFRelease(tag);
	}

	OSType literal = UTGetOSTypeFromString(CFSTR("TEXT"));
	CHECK(literal == 0x54455854, "0x%08x", (unsigned) literal);

	// A string backed by UTF-16 storage has no direct ASCII pointer.
	UniChar characters[] = { 'T', 'E', 'X', 'T' };
	CFMutableStringRef external = CFStringCreateMutableWithExternalCharactersNoCopy(
		NULL, characters, 4, 4, kCFAllocatorNull);
	CHECK(external != NULL, "no string");
	if (external)
	{
		OSType value = UTGetOSTypeFromString(external);
		CHECK(value == 0x54455854, "0x%08x", (unsigned) value);
		CFRelease(external);
	}
}

int main(void)
{
	test_catalog_name();
	test_ostype();
	return failures ? 1 : 0;
}
