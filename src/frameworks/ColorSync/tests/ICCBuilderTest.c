// Native test: compile with the ICC layout and builder code and run on the host.
//   cc -I src -I tests/host tests/ICCBuilderTest.c src/ICCBuilder.c src/ICCProfile.c -lcrypto -lm -o ICCBuilderTest
// Also run it with -fsanitize=address,undefined.
// The profiles are checked against the layout of ICC.1:2022 (https://www.color.org/specification/ICC.1-2022-05.pdf) and
// against the definitions of the colour spaces: the colorants are the primaries and D65 white adapted to D50 with the
// linear Bradford transform, computed independently in Python (the integers below are the s15Fixed16 values, rounded);
// the curves are checked against the formulas of their standards.
#include "ICCBuilder.h"
#include "ICCProfile.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void expect(bool condition, const char* what)
{
	if (!condition)
	{
		printf("FAIL %s\n", what);
		g_failures++;
	}
}

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

static uint32_t get32(const uint8_t* p)
{
	return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
}

static int32_t getFixed(const uint8_t* p)
{
	return (int32_t) get32(p);
}

static bool findTag(const uint8_t* profile, uint32_t signature, ICCTag* tag)
{
	return ICCProfileFindTag(profile, signature, tag);
}

static void expectColorant(const uint8_t* profile, uint32_t signature, int32_t x, int32_t y, int32_t z, const char* what)
{
	ICCTag tag;
	if (!findTag(profile, signature, &tag) || tag.size != 20 || memcmp(profile + tag.offset, "XYZ ", 4) != 0)
	{
		expect(false, what);
		return;
	}
	const uint8_t* p = profile + tag.offset + 8;
	if (getFixed(p) != x || getFixed(p + 4) != y || getFixed(p + 8) != z)
	{
		printf("FAIL %s: got %d %d %d\n", what, getFixed(p), getFixed(p + 4), getFixed(p + 8));
		g_failures++;
	}
}

// The curve of a tag as the function and parameters it encodes, evaluated at x (10.18, Table 68).
static double evaluateCurve(const uint8_t* profile, ICCTag tag, double x)
{
	const uint8_t* p = profile + tag.offset;
	const unsigned function = (unsigned) (p[8] << 8 | p[9]);
	double v[7];
	for (unsigned i = 0; i < 7 && 12 + 4 * i < tag.size; i++)
		v[i] = getFixed(p + 12 + 4 * i) / 65536.0;
	switch (function)
	{
		case 0: return pow(x, v[0]);
		case 3: return x >= v[4] ? pow(v[1] * x + v[2], v[0]) : v[3] * x;
		default: return -1;
	}
}

static void checkProfile(ICCNamedProfile which, const char* name)
{
	char what[96];
	const ICCDisplayProfileSpec spec = ICCNamedProfileSpec(which);
	size_t length = 0;
	uint8_t* profile = ICCBuildDisplayProfile(&spec, &length);
	snprintf(what, sizeof what, "%s builds", name);
	expect(profile != NULL, what);
	if (!profile)
		return;
	snprintf(what, sizeof what, "%s validates", name);
	expect(ICCProfileValidate(profile, length), what);
	snprintf(what, sizeof what, "%s size field", name);
	expect(get32(profile) == length && length % 4 == 0, what);
	snprintf(what, sizeof what, "%s header", name);
	expect(get32(profile + 8) == 0x04300000u && get32(profile + 12) == SIG('m', 'n', 't', 'r') &&
		get32(profile + 16) == (spec.isGray ? SIG('G', 'R', 'A', 'Y') : SIG('R', 'G', 'B', ' ')) && get32(profile + 20) == SIG('X', 'Y', 'Z', ' ') &&
		get32(profile + 36) == SIG('a', 'c', 's', 'p') && get32(profile + 68) == 0xF6D6 && get32(profile + 72) == 0x10000 &&
		get32(profile + 76) == 0xD32D, what);
	snprintf(what, sizeof what, "%s header fields left zero", name);
	expect(get32(profile + 4) == 0 && get32(profile + 40) == 0 && get32(profile + 44) == 0 && get32(profile + 64) == 0 &&
		get32(profile + 80) == 0, what);

	ICCTag tag;
	snprintf(what, sizeof what, "%s description", name);
	expect(findTag(profile, SIG('d', 'e', 's', 'c'), &tag) && memcmp(profile + tag.offset, "mluc", 4) == 0 &&
		get32(profile + tag.offset + 8) == 1 && get32(profile + tag.offset + 20) == 2 * strlen(spec.description) &&
		profile[tag.offset + 28 + 1] == (uint8_t) spec.description[0] && profile[tag.offset + 28] == 0, what);
	snprintf(what, sizeof what, "%s copyright and white point", name);
	expect(findTag(profile, SIG('c', 'p', 'r', 't'), &tag) && findTag(profile, SIG('w', 't', 'p', 't'), &tag) && tag.size == 20, what);
	expectColorant(profile, SIG('w', 't', 'p', 't'), 0xF6D6, 0x10000, 0xD32D, "white point is D50");

	free(profile);
}

int main(void)
{
	const struct { ICCNamedProfile which; const char* name; } all[] = {
		{ ICCNamedSRGB, "sRGB" }, { ICCNamedAdobeRGB1998, "Adobe RGB" }, { ICCNamedITUR2020, "BT.2020" }, { ICCNamedGrayGamma22, "gray" },
		{ ICCNamedGenericRGB, "Generic RGB" }, { ICCNamedGenericGray, "Generic Gray" },
	};
	for (size_t i = 0; i < sizeof all / sizeof all[0]; i++)
		checkProfile(all[i].which, all[i].name);

	size_t length;
	ICCTag tag, green, blue;

	// sRGB (IEC 61966-2-1): the colorants for D50, and the piecewise curve, which maps 0.5 to 0.2140 and meets at its knee.
	ICCDisplayProfileSpec spec = ICCNamedProfileSpec(ICCNamedSRGB);
	uint8_t* srgb = ICCBuildDisplayProfile(&spec, &length);
	expect(ICCProfileTagCount(srgb) == 10, "sRGB tag count");
	expectColorant(srgb, SIG('r', 'X', 'Y', 'Z'), 28576, 14581, 912, "sRGB red colorant");
	expectColorant(srgb, SIG('g', 'X', 'Y', 'Z'), 25239, 46983, 6361, "sRGB green colorant");
	expectColorant(srgb, SIG('b', 'X', 'Y', 'Z'), 9375, 3972, 46787, "sRGB blue colorant");
	expect(findTag(srgb, SIG('r', 'T', 'R', 'C'), &tag) && findTag(srgb, SIG('g', 'T', 'R', 'C'), &green) && findTag(srgb, SIG('b', 'T', 'R', 'C'), &blue), "sRGB curves");
	expect(tag.offset == green.offset && tag.offset == blue.offset && tag.size == 32 && green.size == 32 && blue.size == 32, "the three curves share data");
	// Function 3 with g = 2.4, a = 1/1.055, b = 0.055/1.055, c = 1/12.92, d = 0.04045 (IEC 61966-2-1, as 10.18 lists it).
	static const int32_t srgbParameters[5] = { 157286, 62119, 3417, 5072, 2651 };
	expect(tag.size == 32 && srgb[tag.offset + 9] == 3, "sRGB curve is function 3");
	for (int i = 0; i < 5; i++)
		expect(getFixed(srgb + tag.offset + 12 + 4 * i) == srgbParameters[i], "sRGB curve parameter");
	expect(fabs(evaluateCurve(srgb, tag, 0.5) - 0.2140) < 5e-4, "sRGB curve at 0.5");
	expect(fabs(evaluateCurve(srgb, tag, 1.0) - 1.0) < 1e-4 && evaluateCurve(srgb, tag, 0.0) == 0.0, "sRGB curve ends");
	expect(fabs(evaluateCurve(srgb, tag, 0.04045) - evaluateCurve(srgb, tag, 0.04045 - 1e-9)) < 1e-4, "sRGB curve is continuous at its knee");
	// The chromatic adaptation matrix takes the D65 white of sRGB to the D50 white of the PCS.
	expect(findTag(srgb, SIG('c', 'h', 'a', 'd'), &tag) && tag.size == 44 && memcmp(srgb + tag.offset, "sf32", 4) == 0, "chad tag");
	double a[9], d65[3] = { 0.3127 / 0.3290, 1.0, (1 - 0.3127 - 0.3290) / 0.3290 }, adapted[3];
	for (int i = 0; i < 9; i++)
		a[i] = getFixed(srgb + tag.offset + 8 + 4 * i) / 65536.0;
	for (int i = 0; i < 3; i++)
		adapted[i] = a[i * 3] * d65[0] + a[i * 3 + 1] * d65[1] + a[i * 3 + 2] * d65[2];
	expect(fabs(adapted[0] - 0.9642) < 2e-4 && fabs(adapted[1] - 1.0) < 2e-4 && fabs(adapted[2] - 0.8249) < 2e-4, "chad takes D65 to D50");
	free(srgb);

	// Adobe RGB (1998): colorants, and the pure power of 563/256.
	spec = ICCNamedProfileSpec(ICCNamedAdobeRGB1998);
	uint8_t* adobe = ICCBuildDisplayProfile(&spec, &length);
	expectColorant(adobe, SIG('r', 'X', 'Y', 'Z'), 39960, 20389, 1276, "Adobe RGB red colorant");
	expectColorant(adobe, SIG('g', 'X', 'Y', 'Z'), 13453, 41004, 3989, "Adobe RGB green colorant");
	expectColorant(adobe, SIG('b', 'X', 'Y', 'Z'), 9777, 4143, 48795, "Adobe RGB blue colorant");
	expect(findTag(adobe, SIG('g', 'T', 'R', 'C'), &tag) && tag.size == 16 && getFixed(adobe + tag.offset + 12) == 0x23300, "Adobe RGB gamma 563/256");
	expect(fabs(evaluateCurve(adobe, tag, 0.5) - pow(0.5, 563.0 / 256.0)) < 1e-9, "Adobe RGB curve");
	free(adobe);

	// BT.2020: colorants (the red one has a small negative Z), and the curve meets at its knee.
	spec = ICCNamedProfileSpec(ICCNamedITUR2020);
	uint8_t* bt2020 = ICCBuildDisplayProfile(&spec, &length);
	expectColorant(bt2020, SIG('r', 'X', 'Y', 'Z'), 44137, 18287, -127, "BT.2020 red colorant");
	expectColorant(bt2020, SIG('g', 'X', 'Y', 'Z'), 10857, 44259, 1965, "BT.2020 green colorant");
	expectColorant(bt2020, SIG('b', 'X', 'Y', 'Z'), 8195, 2989, 52222, "BT.2020 blue colorant");
	expect(findTag(bt2020, SIG('b', 'T', 'R', 'C'), &tag), "BT.2020 curve");
	static const int32_t bt2020Parameters[5] = { 145636, 59616, 5920, 14564, 5308 };
	for (int i = 0; i < 5; i++)
		expect(getFixed(bt2020 + tag.offset + 12 + 4 * i) == bt2020Parameters[i], "BT.2020 curve parameter");
	expect(fabs(evaluateCurve(bt2020, tag, 1.0) - 1.0) < 1e-3 && fabs(evaluateCurve(bt2020, tag, 0.081) - evaluateCurve(bt2020, tag, 0.081 - 1e-9)) < 1e-4, "BT.2020 curve ends and knee");
	free(bt2020);

	// Gray: no colorants, one curve, gamma 2.2.
	spec = ICCNamedProfileSpec(ICCNamedGrayGamma22);
	uint8_t* gray = ICCBuildDisplayProfile(&spec, &length);
	expect(ICCProfileTagCount(gray) == 4 && !findTag(gray, SIG('r', 'X', 'Y', 'Z'), &tag) && !findTag(gray, SIG('r', 'T', 'R', 'C'), &tag), "gray has no colorants");
	expect(findTag(gray, SIG('k', 'T', 'R', 'C'), &tag) && tag.size == 16 && getFixed(gray + tag.offset + 12) == 144179, "gray gamma 2.2");
	free(gray);

	// Generic RGB (primaries 0.630/0.340, 0.295/0.605, 0.155/0.077, D65 white, a power of 1.8): the colorants for D50 are the
	// primaries adapted with the linear Bradford transform, computed independently in Python. Generic Gray is a power of 1.8.
	spec = ICCNamedProfileSpec(ICCNamedGenericRGB);
	uint8_t* genericRGB = ICCBuildDisplayProfile(&spec, &length);
	expectColorant(genericRGB, SIG('r', 'X', 'Y', 'Z'), 29771, 15853, 976, "Generic RGB red colorant");
	expectColorant(genericRGB, SIG('g', 'X', 'Y', 'Z'), 23155, 44148, 5939, "Generic RGB green colorant");
	expectColorant(genericRGB, SIG('b', 'X', 'Y', 'Z'), 10264, 5534, 47146, "Generic RGB blue colorant");
	expect(findTag(genericRGB, SIG('b', 'T', 'R', 'C'), &tag) && tag.size == 16 && getFixed(genericRGB + tag.offset + 12) == 117965, "Generic RGB gamma 1.8");
	free(genericRGB);
	spec = ICCNamedProfileSpec(ICCNamedGenericGray);
	uint8_t* genericGray = ICCBuildDisplayProfile(&spec, &length);
	expect(ICCProfileTagCount(genericGray) == 4 && findTag(genericGray, SIG('k', 'T', 'R', 'C'), &tag) && tag.size == 16 && getFixed(genericGray + tag.offset + 12) == 117965, "Generic Gray gamma 1.8");
	free(genericGray);

	// The same input gives the same bytes, and bad input gives NULL.
	spec = ICCNamedProfileSpec(ICCNamedSRGB);
	uint8_t* first = ICCBuildDisplayProfile(&spec, &length);
	size_t secondLength;
	uint8_t* second = ICCBuildDisplayProfile(&spec, &secondLength);
	expect(length == secondLength && memcmp(first, second, length) == 0, "building is deterministic");
	free(first);
	free(second);
	char longName[200];
	memset(longName, 'x', sizeof longName - 1);
	longName[sizeof longName - 1] = 0;
	spec.description = longName;
	expect(ICCBuildDisplayProfile(&spec, &length) == NULL, "description too long");
	spec.description = "ok";
	spec.curve.function = 5;
	expect(ICCBuildDisplayProfile(&spec, &length) == NULL, "unknown curve function");
	spec = ICCNamedProfileSpec((ICCNamedProfile) 99);
	expect(ICCBuildDisplayProfile(&spec, &length) == NULL, "a profile that is not in the enumeration gives no profile");

	if (g_failures)
	{
		printf("ICCBuilderTest: %d failures\n", g_failures);
		return 1;
	}
	printf("ICCBuilderTest: OK\n");
	return 0;
}
