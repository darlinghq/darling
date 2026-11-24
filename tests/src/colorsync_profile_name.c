// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCreateWithName() creates a profile from a predefined profile name, or returns NULL
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatewithname(_:)); the names are the
// kColorSync...Profile constants.
// The pages give no profile data, so the profiles are built from the public definitions of the colour spaces
// (IEC 61966-2-1 for sRGB, the Adobe RGB (1998) specification, Recommendation ITU-R BT.2020) in the layout of
// ICC.1:2022 (https://www.color.org/specification/ICC.1-2022-05.pdf). The test checks the profile's structure and, for sRGB,
// the colorants computed from the standard's primaries by the Bradford transform (the integers are s15Fixed16 values).
// Observed on macOS 26.7.3 and 10.14.4 and pinned here: a name gives the same profile object on every call (QUIRK), and an unknown
// name, a NULL and a non-string give NULL. The descriptions are those of macOS's profiles of these names. macOS also has profiles
// for the generic Lab, XYZ and CMYK names, which this implementation does not make (no public definition to build them from).
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

static void checkNamed(CFStringRef name, const char* colorSpace, bool gray, const char* description)
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

	CFStringRef text = ColorSyncProfileCopyDescriptionString(profile);
	CFStringRef wanted = CFStringCreateWithCString(NULL, description, kCFStringEncodingASCII);
	CHECK(text != NULL && CFEqual(text, wanted));
	if (text)
		CFRelease(text);
	CFRelease(wanted);

	// QUIRK: a second call gives the same profile.
	ColorSyncProfileRef again = ColorSyncProfileCreateWithName(name);
	CHECK(again != NULL && again == profile);
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

	checkNamed(kColorSyncSRGBProfile, "RGB ", false, "sRGB IEC61966-2.1");
	checkNamed(kColorSyncAdobeRGB1998Profile, "RGB ", false, "Adobe RGB (1998)");
	checkNamed(kColorSyncITUR2020Profile, "RGB ", false, "Rec. ITU-R BT.2020-1");
	checkNamed(kColorSyncGenericGrayGamma22Profile, "GRAY", true, "Generic Gray Gamma 2.2 Profile");

	// The colorants for the D50 PCS: X, Y and Z of each primary as s15Fixed16 numbers, computed from the primaries and the
	// white of each standard by the Bradford transform. macOS 26.7.3 and 10.14.4 give these numbers to within 12 (the sRGB blue
	// Z, which its profile rounds differently), 1 (Adobe RGB) and 0 (BT.2020), so the check allows 16.
	static const struct { CFStringRef* name; const char* tag; int32_t xyz[3]; } colorants[] = {
		{ &kColorSyncSRGBProfile, "rXYZ", { 28576, 14581, 912 } }, { &kColorSyncSRGBProfile, "gXYZ", { 25239, 46983, 6361 } },
		{ &kColorSyncSRGBProfile, "bXYZ", { 9375, 3972, 46787 } },
		{ &kColorSyncAdobeRGB1998Profile, "rXYZ", { 39960, 20389, 1276 } }, { &kColorSyncAdobeRGB1998Profile, "gXYZ", { 13453, 41004, 3989 } },
		{ &kColorSyncAdobeRGB1998Profile, "bXYZ", { 9777, 4143, 48795 } },
		{ &kColorSyncITUR2020Profile, "rXYZ", { 44137, 18287, -127 } }, { &kColorSyncITUR2020Profile, "gXYZ", { 10857, 44259, 1965 } },
		{ &kColorSyncITUR2020Profile, "bXYZ", { 8195, 2989, 52222 } },
	};
	for (int i = 0; i < 9; i++)
	{
		ColorSyncProfileRef profile = ColorSyncProfileCreateWithName(*colorants[i].name);
		CFStringRef signature = CFStringCreateWithCString(NULL, colorants[i].tag, kCFStringEncodingASCII);
		CFDataRef tag = profile ? ColorSyncProfileCopyTag(profile, signature) : NULL;
		CHECK(tag != NULL && CFDataGetLength(tag) == 20 && memcmp(CFDataGetBytePtr(tag), "XYZ ", 4) == 0);
		for (int k = 0; tag && k < 3; k++)
			CHECK(abs(fixedAt(tag, 8 + 4 * k) - colorants[i].xyz[k]) <= 16);
		if (tag)
			CFRelease(tag);
		CFRelease(signature);
		if (profile)
			CFRelease(profile);
	}

	// An unknown name, NULL and a non-string give NULL.
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
