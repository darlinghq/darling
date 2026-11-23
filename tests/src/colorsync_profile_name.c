// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCreateWithName() creates a profile from a predefined profile name, or returns NULL
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatewithname(_:)); the names are the
// kColorSync...Profile constants.
// The pages give no profile data, so the profiles are built from the public definitions of the colour spaces
// (IEC 61966-2-1 for sRGB, the Adobe RGB (1998) specification, Recommendation ITU-R BT.2020) in the layout of
// ICC.1:2022 (https://www.color.org/specification/ICC.1-2022-05.pdf). The test checks the profile's structure and, for sRGB,
// the colorants computed from the standard's primaries by the Bradford transform (the integers are s15Fixed16 values).
// GUESS items pinned here: the generic Lab, XYZ and CMYK names, an unknown name, a NULL and a non-string give NULL.
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static int32_t fixedAt(CFDataRef data, CFIndex offset)
{
	const uint8_t* p = CFDataGetBytePtr(data) + offset;
	return (int32_t) (((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3]);
}

static void checkNamed(CFStringRef name, const char* colorSpace, bool gray)
{
	ColorSyncProfileRef profile = ColorSyncProfileCreateWithName(name);
	CHECK(profile != NULL);
	if (!profile)
		return;

	CFDataRef data = ColorSyncProfileCopyData(profile, NULL);
	CHECK(data != NULL && CFDataGetLength(data) > 132);
	if (data)
	{
		const uint8_t* bytes = CFDataGetBytePtr(data);
		CHECK(memcmp(bytes + 12, "mntr", 4) == 0 && memcmp(bytes + 16, colorSpace, 4) == 0 && memcmp(bytes + 20, "XYZ ", 4) == 0);
		CHECK(memcmp(bytes + 36, "acsp", 4) == 0);
		CFRelease(data);
	}
	CHECK(ColorSyncProfileContainsTag(profile, CFSTR("desc")) && ColorSyncProfileContainsTag(profile, CFSTR("cprt")) && ColorSyncProfileContainsTag(profile, CFSTR("wtpt")));
	CHECK(ColorSyncProfileContainsTag(profile, CFSTR("kTRC")) == gray);
	CHECK(ColorSyncProfileContainsTag(profile, CFSTR("rXYZ")) == !gray);
	CHECK(ColorSyncProfileContainsTag(profile, CFSTR("bTRC")) == !gray);

	// A second call gives a new profile with the same data.
	ColorSyncProfileRef again = ColorSyncProfileCreateWithName(name);
	CHECK(again != NULL && again != profile);
	CFDataRef first = ColorSyncProfileCopyData(profile, NULL);
	CFDataRef second = ColorSyncProfileCopyData(again, NULL);
	CHECK(first && second && CFEqual(first, second));
	if (first)
		CFRelease(first);
	if (second)
		CFRelease(second);
	if (again)
		CFRelease(again);
	CFRelease(profile);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	checkNamed(kColorSyncSRGBProfile, "RGB ", false);
	checkNamed(kColorSyncAdobeRGB1998Profile, "RGB ", false);
	checkNamed(kColorSyncITUR2020Profile, "RGB ", false);
	checkNamed(kColorSyncGenericGrayGamma22Profile, "GRAY", true);

	// The colorants of sRGB for the D50 PCS: X, Y and Z of each primary as s15Fixed16 numbers.
	ColorSyncProfileRef srgb = ColorSyncProfileCreateWithName(kColorSyncSRGBProfile);
	static const struct { const char* tag; int32_t xyz[3]; } colorants[3] = {
		{ "rXYZ", { 28576, 14581, 912 } }, { "gXYZ", { 25239, 46983, 6361 } }, { "bXYZ", { 9375, 3972, 46787 } },
	};
	for (int i = 0; srgb && i < 3; i++)
	{
		CFStringRef signature = CFStringCreateWithCString(NULL, colorants[i].tag, kCFStringEncodingASCII);
		CFDataRef tag = ColorSyncProfileCopyTag(srgb, signature);
		CHECK(tag != NULL && CFDataGetLength(tag) == 20 && memcmp(CFDataGetBytePtr(tag), "XYZ ", 4) == 0);
		for (int k = 0; tag && k < 3; k++)
			CHECK(fixedAt(tag, 8 + 4 * k) == colorants[i].xyz[k]);
		if (tag)
			CFRelease(tag);
		CFRelease(signature);
	}
	if (srgb)
		CFRelease(srgb);

	// GUESS: no definition, an unknown name, NULL and a non-string give NULL.
	CHECK(ColorSyncProfileCreateWithName(kColorSyncGenericLabProfile) == NULL);
	CHECK(ColorSyncProfileCreateWithName(kColorSyncGenericXYZProfile) == NULL);
	CHECK(ColorSyncProfileCreateWithName(kColorSyncGenericCMYKProfile) == NULL);
	CHECK(ColorSyncProfileCreateWithName(CFSTR("not a profile name")) == NULL);
	CHECK(ColorSyncProfileCreateWithName(NULL) == NULL);
	const int number = 7;
	CFNumberRef notAString = CFNumberCreate(NULL, kCFNumberIntType, &number);
	CHECK(ColorSyncProfileCreateWithName((CFStringRef) notAString) == NULL);
	CFRelease(notAString);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
