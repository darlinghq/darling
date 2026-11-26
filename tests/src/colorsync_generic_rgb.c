// Copyright (C) 2026 Darling developers
//
// The Generic RGB and Generic Gray profiles of ColorSync: kColorSyncGenericRGBProfile and kColorSyncGenericGrayProfile name
// predefined profiles (https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatewithname(_:)). No standard
// defines these spaces, so every number below is from black-box observation (tier 2) of macOS 26.7.3 and 10.14.4 (x86_64 and
// i386) through the public API: ColorSyncProfileCopyTag numbers, the header fields, and CGColorCreateCopyByMatchingToColorSpace
// results (relative colorimetric). The test pins:
//  - the profile of the name: class 'mntr', colour space 'RGB ' (or 'GRAY'), PCS 'XYZ ', description "Generic RGB Profile"
//    ("Generic Gray Profile"), the D50-adapted colorants (the primaries 0.630/0.340, 0.295/0.605, 0.155/0.077 and the D65
//    white adapted to D50), and a power of 1.8 as the curve of each channel;
//  - the profile file /System/Library/ColorSync/Profiles/Generic RGB Profile.icc, which opens and gives the profile of the name
//    (observed: the same MD5 digest);
//  - CGColorSpaceCopyICCData of the colour spaces kCGColorSpaceGenericRGB and kCGColorSpaceGenericGray gives that profile
//    (observed: the same MD5 digest);
//  - the conversions of CGColor between the generic spaces and sRGB.
// The profile files of Darling are written from the same definition; the header version and the white point tag differ from
// those of macOS (4.3 with the PCS white, against 2.2 with the D65 media white), which this test does not pin.
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>
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

static uint32_t be32(const uint8_t* p)
{
	return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
}

static double s15(const uint8_t* p)
{
	return (int32_t) be32(p) / 65536.0;
}

static CFDataRef copyTag(ColorSyncProfileRef profile, const char* signature)
{
	CFStringRef s = CFStringCreateWithCString(NULL, signature, kCFStringEncodingASCII);
	CFDataRef tag = ColorSyncProfileCopyTag(profile, s);
	CFRelease(s);
	return tag;
}

// The value of a colorant-curve tag at x: a curveType with one entry is a power as u8Fixed8; a parametricCurveType of function 0
// is a power as s15Fixed16. Returns false for any other curve.
static bool curveAt(CFDataRef tag, double x, double* out)
{
	const uint8_t* b = CFDataGetBytePtr(tag);
	CFIndex length = CFDataGetLength(tag);
	if (length >= 14 && memcmp(b, "curv", 4) == 0 && be32(b + 8) == 1)
	{
		*out = pow(x, ((b[12] << 8) | b[13]) / 256.0);
		return true;
	}
	if (length >= 16 && memcmp(b, "para", 4) == 0 && b[8] == 0 && b[9] == 0)
	{
		*out = pow(x, s15(b + 12));
		return true;
	}
	return false;
}

static void checkHeader(ColorSyncProfileRef profile, const char* colorSpace, const char* description)
{
	CFDataRef data = ColorSyncProfileCopyData(profile, NULL);
	CHECK(data != NULL && CFDataGetLength(data) > 132);
	if (data)
	{
		const uint8_t* b = CFDataGetBytePtr(data);
		CHECK(memcmp(b + 12, "mntr", 4) == 0 && memcmp(b + 16, colorSpace, 4) == 0 && memcmp(b + 20, "XYZ ", 4) == 0);
		CHECK(memcmp(b + 36, "acsp", 4) == 0);
		CHECK(fabs(s15(b + 68) - 0.9642) < 0.001 && fabs(s15(b + 72) - 1.0) < 0.001 && fabs(s15(b + 76) - 0.8249) < 0.001);
		CFRelease(data);
	}
	CFStringRef text = ColorSyncProfileCopyDescriptionString(profile);
	CFStringRef wanted = CFStringCreateWithCString(NULL, description, kCFStringEncodingASCII);
	CHECK(text != NULL && CFEqual(text, wanted));
	if (text)
		CFRelease(text);
	CFRelease(wanted);
}

static void checkGamma18(ColorSyncProfileRef profile, const char* signature)
{
	CFDataRef tag = copyTag(profile, signature);
	CHECK(tag != NULL);
	if (!tag)
		return;
	static const double xs[] = { 0.1, 0.25, 0.5, 0.75, 1.0 };
	for (int i = 0; i < 5; i++)
	{
		double y = -1;
		// 1.8 is stored as 1.80078 in the u8Fixed8 curve of macOS: the values differ by less than 0.001.
		CHECK(curveAt(tag, xs[i], &y) && fabs(y - pow(xs[i], 1.8)) < 0.002);
	}
	CFRelease(tag);
}

// The colorants of the D50 PCS observed on macOS (X, Y, Z of each primary). The profiles of this implementation are computed from
// the primaries and the D65 white, so the s15Fixed16 integers may differ in the last digits.
static void checkColorants(ColorSyncProfileRef profile)
{
	static const struct { const char* tag; double xyz[3]; } colorants[] = {
		{ "rXYZ", { 0.454300, 0.241913, 0.014893 } },
		{ "gXYZ", { 0.353348, 0.673630, 0.090637 } },
		{ "bXYZ", { 0.156647, 0.084457, 0.719574 } },
	};
	for (int i = 0; i < 3; i++)
	{
		CFDataRef tag = copyTag(profile, colorants[i].tag);
		CHECK(tag != NULL && CFDataGetLength(tag) == 20 && memcmp(CFDataGetBytePtr(tag), "XYZ ", 4) == 0);
		for (int k = 0; tag && k < 3; k++)
			CHECK(fabs(s15(CFDataGetBytePtr(tag) + 8 + 4 * k) - colorants[i].xyz[k]) < 0.0005);
		if (tag)
			CFRelease(tag);
	}
}

static bool digestsEqual(ColorSyncProfileRef a, ColorSyncProfileRef b)
{
	ColorSyncMD5 x = ColorSyncProfileGetMD5(a), y = ColorSyncProfileGetMD5(b);
	return memcmp(&x, &y, sizeof x) == 0;
}

static void checkConversion(CGColorSpaceRef from, CGColorSpaceRef to, const double* in, int inCount, const double* want, int wantCount, const char* what)
{
	CGFloat components[5];
	for (int i = 0; i < inCount; i++)
		components[i] = in[i];
	components[inCount] = 1;
	CGColorRef color = CGColorCreate(from, components);
	CGColorRef result = color ? CGColorCreateCopyByMatchingToColorSpace(to, kCGRenderingIntentRelativeColorimetric, color, NULL) : NULL;
	bool ok = result != NULL && CGColorGetNumberOfComponents(result) == (size_t) wantCount + 1;
	for (int i = 0; ok && i < wantCount; i++)
		ok = fabs(CGColorGetComponents(result)[i] - want[i]) < 0.003;
	if (!ok)
	{
		printf("FAIL: %s: got", what);
		for (size_t i = 0; result && i + 1 < CGColorGetNumberOfComponents(result); i++)
			printf(" %.6f", CGColorGetComponents(result)[i]);
		printf(", wanted");
		for (int i = 0; i < wantCount; i++)
			printf(" %.6f", want[i]);
		printf("\n");
		g_failures++;
	}
	if (result)
		CGColorRelease(result);
	if (color)
		CGColorRelease(color);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	// The constants' values (observed).
	CHECK(CFEqual(kColorSyncGenericRGBProfile, CFSTR("com.apple.ColorSync.GenericRGB")));
	CHECK(CFEqual(kColorSyncGenericGrayProfile, CFSTR("com.apple.ColorSync.GenericGray")));

	ColorSyncProfileRef rgb = ColorSyncProfileCreateWithName(kColorSyncGenericRGBProfile);
	ColorSyncProfileRef gray = ColorSyncProfileCreateWithName(kColorSyncGenericGrayProfile);
	CHECK(rgb != NULL && gray != NULL);
	if (!rgb || !gray)
		return 1;

	checkHeader(rgb, "RGB ", "Generic RGB Profile");
	checkHeader(gray, "GRAY", "Generic Gray Profile");
	CHECK(ColorSyncProfileContainsTag(rgb, CFSTR("chad")) && ColorSyncProfileContainsTag(rgb, CFSTR("wtpt")));
	CHECK(!ColorSyncProfileContainsTag(gray, CFSTR("rXYZ")) && ColorSyncProfileContainsTag(gray, CFSTR("kTRC")));
	checkColorants(rgb);
	checkGamma18(rgb, "rTRC");
	checkGamma18(rgb, "gTRC");
	checkGamma18(rgb, "bTRC");
	checkGamma18(gray, "kTRC");

	// Observed: the same profile object from every call with one name; the profiles of the two names differ.
	ColorSyncProfileRef again = ColorSyncProfileCreateWithName(kColorSyncGenericRGBProfile);
	CHECK(again == rgb);
	if (again)
		CFRelease(again);
	CHECK(!digestsEqual(rgb, gray));

	// The profile file opens by path and is the profile of the name (observed: equal MD5 digests).
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFSTR("/System/Library/ColorSync/Profiles/Generic RGB Profile.icc"), kCFURLPOSIXPathStyle, false);
	CFErrorRef error = NULL;
	ColorSyncProfileRef file = ColorSyncProfileCreateWithURL(url, &error);
	CHECK(file != NULL && digestsEqual(file, rgb));
	if (file)
		CFRelease(file);
	CFRelease(url);
	url = CFURLCreateWithFileSystemPath(NULL, CFSTR("/System/Library/ColorSync/Profiles/Generic Gray Profile.icc"), kCFURLPOSIXPathStyle, false);
	file = ColorSyncProfileCreateWithURL(url, &error);
	CHECK(file != NULL && digestsEqual(file, gray));
	if (file)
		CFRelease(file);
	CFRelease(url);

	// CoreGraphics' generic spaces are these profiles (observed: equal MD5 digests).
	CGColorSpaceRef genericRGB = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGColorSpaceRef genericGray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
	CHECK(genericRGB != NULL && genericGray != NULL);
	if (!genericRGB || !genericGray)
		return 1;
	CFDataRef rgbData = CGColorSpaceCopyICCData(genericRGB);
	CFDataRef grayData = CGColorSpaceCopyICCData(genericGray);
	ColorSyncProfileRef fromSpace = rgbData ? ColorSyncProfileCreate(rgbData, NULL) : NULL;
	CHECK(fromSpace != NULL && digestsEqual(fromSpace, rgb));
	if (fromSpace)
		CFRelease(fromSpace);
	fromSpace = grayData ? ColorSyncProfileCreate(grayData, NULL) : NULL;
	CHECK(fromSpace != NULL && digestsEqual(fromSpace, gray));
	if (fromSpace)
		CFRelease(fromSpace);

	// Conversions, relative colorimetric (observed on macOS 26.7.3; 10.14.4 agrees within 0.0001).
	CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	CGColorSpaceRef extended = CGColorSpaceCreateWithName(kCGColorSpaceExtendedSRGB);
	static const struct { double in[3]; double out[3]; } toExtended[] = {
		{ { 1, 0, 0 }, { 1.011015, 0.149100, -0.022767 } },
		{ { 0, 1, 0 }, { -0.177584, 0.976789, -0.018534 } },
		{ { 0, 0, 1 }, { 0.016842, 0.198341, 1.001404 } },
		{ { 0.5, 0.5, 0.5 }, { 0.572307, 0.572307, 0.572307 } },
		{ { 0.25, 0.5, 0.75 }, { 0.308863, 0.577813, 0.796054 } },
		{ { 1, 0.5, 0 }, { 1.007706, 0.576357, -0.028089 } },
		{ { 0.9, 0.3, 0.6 }, { 0.928800, 0.406674, 0.663966 } },
		{ { 1, 1, 1 }, { 1, 1, 1 } },
	};
	for (int i = 0; i < 8; i++)
		checkConversion(genericRGB, extended, toExtended[i].in, 3, toExtended[i].out, 3, "Generic RGB to extended sRGB");

	static const struct { double in[3]; double out[3]; } fromSRGB[] = {
		{ { 0.5, 0.5, 0.5 }, { 0.424672, 0.424672, 0.424672 } },
		{ { 0.25, 0.5, 0.75 }, { 0.198231, 0.416573, 0.696309 } },
		{ { 0.9, 0.3, 0.6 }, { 0.864179, 0.190488, 0.530022 } },
		{ { 0.8, 0.8, 0.8 }, { 0.755588, 0.755588, 0.755588 } },
	};
	for (int i = 0; i < 4; i++)
		checkConversion(srgb, genericRGB, fromSRGB[i].in, 3, fromSRGB[i].out, 3, "sRGB to Generic RGB");

	static const struct { double in; double out; } grays[] = {
		{ 0.125, 0.166786 }, { 0.25, 0.317999 }, { 0.5, 0.572307 }, { 0.75, 0.795254 }, { 0.875, 0.899462 }, { 1, 1 },
	};
	for (int i = 0; i < 6; i++)
	{
		const double want[3] = { grays[i].out, grays[i].out, grays[i].out };
		checkConversion(genericGray, extended, &grays[i].in, 1, want, 3, "Generic Gray to extended sRGB");
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
