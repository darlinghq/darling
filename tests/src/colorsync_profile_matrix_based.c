// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileIsMatrixBased() says whether a profile is matrix-based
// (https://developer.apple.com/documentation/colorsync/colorsyncprofileismatrixbased(_:)). ICC.1:2022, 8.3.3 and
// 8.4.3 (https://www.color.org/specification/ICC.1-2022-05.pdf) define a three-component matrix-based profile as one
// with the red, green and blue matrix columns (rXYZ, gXYZ, bXYZ) and tone reproduction curves (rTRC, gTRC, bTRC), and
// let it have AToB tables as well.
// The profiles are hand-built from the layout in ICC.1:2022, 7.2 and 7.3, so they hold no vendor data.
// The expectations are observations of macOS 26.7.3 (the QUIRK on ICCProfileHasMatrixTRC; the function is not in 10.14.4):
// an RGB profile with the six tags, or a gray profile with kTRC, is matrix-based unless it also has an A2B0, A2B1 or A2B2 table;
// any other colour space is not; a NULL or non-profile argument gives false.
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

static void put32(uint8_t* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

// A profile of the given colour space whose tags are the given signatures, each with 4 bytes of data.
static ColorSyncProfileRef makeProfile(const char* colorSpace, const char* const* signatures, uint32_t count)
{
	uint8_t bytes[512];
	const uint32_t dataStart = 128 + 4 + count * 12;
	memset(bytes, 0, sizeof bytes);
	put32(bytes, dataStart + count * 4);
	memcpy(bytes + 16, colorSpace, 4);
	memcpy(bytes + 36, "acsp", 4);
	put32(bytes + 128, count);
	for (uint32_t i = 0; i < count; i++)
	{
		memcpy(bytes + 132 + i * 12, signatures[i], 4);
		put32(bytes + 136 + i * 12, dataStart + i * 4);
		put32(bytes + 140 + i * 12, 4);
		memcpy(bytes + dataStart + i * 4, "DATA", 4);
	}
	CFDataRef data = CFDataCreate(NULL, bytes, dataStart + count * 4);
	ColorSyncProfileRef profile = ColorSyncProfileCreate(data, NULL);
	CFRelease(data);
	return profile;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	const char* const six[] = { "rXYZ", "gXYZ", "bXYZ", "rTRC", "gTRC", "bTRC" };
	ColorSyncProfileRef matrix = makeProfile("RGB ", six, 6);
	CHECK(matrix != NULL && ColorSyncProfileIsMatrixBased(matrix));

	// Any one tag missing: not matrix-based.
	for (int skipped = 0; skipped < 6; skipped++)
	{
		const char* five[5];
		for (int i = 0, n = 0; i < 6; i++)
			if (i != skipped)
				five[n++] = six[i];
		ColorSyncProfileRef partial = makeProfile("RGB ", five, 5);
		CHECK(partial != NULL && !ColorSyncProfileIsMatrixBased(partial));
		CFRelease(partial);
	}

	// QUIRK: an AToB table beside the six tags makes the profile not matrix-based; other tables do not; and so does a
	// colour space other than RGB.
	const char* const withLut[] = { "rXYZ", "gXYZ", "bXYZ", "rTRC", "gTRC", "bTRC", "A2B0" };
	ColorSyncProfileRef lut = makeProfile("RGB ", withLut, 7);
	CHECK(lut != NULL && !ColorSyncProfileIsMatrixBased(lut));
	const char* const withB2A[] = { "rXYZ", "gXYZ", "bXYZ", "rTRC", "gTRC", "bTRC", "B2A0" };
	ColorSyncProfileRef backTable = makeProfile("RGB ", withB2A, 7);
	CHECK(backTable != NULL && ColorSyncProfileIsMatrixBased(backTable));
	ColorSyncProfileRef otherSpace = makeProfile("Lab ", six, 6);
	CHECK(otherSpace != NULL && !ColorSyncProfileIsMatrixBased(otherSpace));

	// Profiles of other models, and gray profiles with kTRC.
	const char* const lutOnly[] = { "wtpt", "A2B0", "B2A0" };
	ColorSyncProfileRef table = makeProfile("RGB ", lutOnly, 3);
	CHECK(table != NULL && !ColorSyncProfileIsMatrixBased(table));
	const char* const gray[] = { "wtpt", "kTRC" };
	ColorSyncProfileRef grayProfile = makeProfile("GRAY", gray, 2);
	CHECK(grayProfile != NULL && ColorSyncProfileIsMatrixBased(grayProfile));
	const char* const grayLut[] = { "kTRC", "A2B0" };
	ColorSyncProfileRef grayTable = makeProfile("GRAY", grayLut, 2);
	CHECK(grayTable != NULL && !ColorSyncProfileIsMatrixBased(grayTable));
	const char* const trcOnly[] = { "kTRC" };
	ColorSyncProfileRef rgbGrayCurve = makeProfile("RGB ", trcOnly, 1);
	CHECK(rgbGrayCurve != NULL && !ColorSyncProfileIsMatrixBased(rgbGrayCurve));

	CHECK(!ColorSyncProfileIsMatrixBased(NULL));
	CHECK(!ColorSyncProfileIsMatrixBased((ColorSyncProfileRef) CFSTR("not a profile")));

	CFRelease(matrix);
	CFRelease(lut);
	CFRelease(backTable);
	CFRelease(grayTable);
	CFRelease(rgbGrayCurve);
	CFRelease(otherSpace);
	CFRelease(table);
	CFRelease(grayProfile);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
