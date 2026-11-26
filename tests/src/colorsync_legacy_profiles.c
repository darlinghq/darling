// Copyright (C) 2026 Lubos Dolezel
//
// The legacy ColorSync profile references (CMOpenProfile, CMCloneProfileRef, CMCloseProfile) and the platform
// colour space made from one. The error codes are from black-box observation on macOS 10.14.4 i386.
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
#ifdef __LP64__
	printf("SKIP colorsync_legacy_profiles (32-bit only)\n");
	return 0;
#else
	CMProfileRef profile;
	CMProfileLocation location;

	// A profile in memory: the one of the ColorSync framework for sRGB
	CFTypeRef srgb = ColorSyncProfileCreateWithName(kColorSyncSRGBProfile);
	CFDataRef data = srgb ? ColorSyncProfileCopyData(srgb, NULL) : NULL;

	CHECK(data != NULL);
	memset(&location, 0, sizeof(location));
	location.locType = cmBufferBasedProfile;
	location.u.bufferLoc.buffer = (void*) CFDataGetBytePtr(data);
	location.u.bufferLoc.size = (UInt32) CFDataGetLength(data);
	profile = (CMProfileRef) 1;
	CHECK(CMOpenProfile(&profile, &location) == noErr && profile);

	CGColorSpaceRef space = CGColorSpaceCreateWithPlatformColorSpace(profile);
	CHECK(space != NULL && CGColorSpaceGetNumberOfComponents(space) == 3);
	if (space)
		CGColorSpaceRelease(space);

	// One reference per open and per clone
	CMCloneProfileRef(profile);
	CHECK(CMCloseProfile(profile) == noErr);
	CHECK(CMCloseProfile(profile) == noErr);

	// QUIRK: the open fails with fnfErr for a file that is not there, cmInvalidProfile (-4204) for a location
	// type that is not supported, paramErr for NULL; the reference is NULL after a failed open; closing
	// NULL gives cmInvalidProfile. Not documented. Evidence: black-box observation on macOS 10.14.4 i386
	// (tier 2). Test: this test (test_quirk_open_errors).
	location.locType = cmPathBasedProfile;
	strcpy(location.u.pathLoc.path, "/nonexistent/profile.icc");
	profile = (CMProfileRef) 1;
	CHECK(CMOpenProfile(&profile, &location) == fnfErr && profile == NULL);
	location.locType = 99;
	profile = (CMProfileRef) 1;
	CHECK(CMOpenProfile(&profile, &location) == cmInvalidProfile && profile == NULL);
	CHECK(CMOpenProfile(NULL, &location) == paramErr);
	CHECK(CMOpenProfile(&profile, NULL) == paramErr);
	CHECK(CMCloseProfile(NULL) == cmInvalidProfile);
	CHECK(CGColorSpaceCreateWithPlatformColorSpace(NULL) == NULL);

	if (data)
		CFRelease(data);
	if (srgb)
		CFRelease(srgb);
	printf(g_failures ? "FAIL colorsync_legacy_profiles\n" : "PASS colorsync_legacy_profiles\n");
	return g_failures != 0;
#endif
}
