// CFLAGS: -framework Accelerate
// Copyright (C) 2026 Darling developers
//
// vImageConvert_Planar16FtoPlanarF and vImageConvert_PlanarFtoPlanar16F convert between IEEE 754-2008
// binary16 (half) and binary32 (single) planar buffers. The conversion values come from the standard
// (and agree with macOS, see below): the half bit patterns of the named values (1.0 is 0x3C00, -2.0 is
// 0xC000, the largest finite half 65504 is 0x7BFF, the smallest subnormal 2^-24 is 0x0001, infinity
// is 0x7C00), the exhaustive round trip over all 65536 patterns, and round to nearest with ties to
// even, checked at every midpoint between two adjacent finite halves (computed here with ldexpf,
// independently of the implementation).
//
// NaN: IEEE 754-2008 convertFormat returns a quiet NaN for a signalling input, so both directions
// set the quiet bit (half fraction bit 9, single fraction bit 22) and keep the sign and the other
// payload bits (the half fraction moves to the top of the single fraction). A quiet half NaN comes
// back from half -> float -> half with the same bits; a signalling half NaN (bit 9 clear, fraction
// non-zero) comes back as the quiet NaN with the same sign and lower fraction bits, the only
// deviation from an identical round trip.
//
// Error results and flags: pinned to what macOS 26.7.3 and 10.14.4 return for the same calls
// (black-box observation through the public API, identical on both): a NULL buffer pointer gives
// kvImageNullPointerArgument (also with kvImageGetTempBufferSize); kvImageGetTempBufferSize returns 0
// without looking at the buffers; kvImageLeaveAlphaUnchanged, kvImageHighQualityResampling and an unknown
// bit give kvImageUnknownFlagsBit while kvImageDoNotTile is accepted; a destination wider or taller than
// the source gives kvImageRoiLargerThanInputBuffer (also for an empty source), a narrower one is
// converted at its own size; rowBytes smaller than a row is not an error; empty buffers succeed.
// A NULL data pointer crashes macOS; here it gives kvImageNullPointerArgument. The rounding values (ties
// to even at 1+2^-11 and 1+3*2^-11, 2^-25 to zero, 65520 to infinity, quiet NaN with the top ten payload
// bits kept) were observed on both as well.
// Build as in vImageTest.c:
//   cc -I src/frameworks/Accelerate/include tests/src/accelerate_vimage_half_float.c \
//      src/frameworks/Accelerate/src/Accelerate.c -lm -o accelerate_vimage_half_float
#include <Accelerate/Accelerate.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static uint32_t floatBits(float value)
{
	uint32_t bits;
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static float bitsFloat(uint32_t bits)
{
	float value;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

// One-pixel conversions through the public functions.
static float halfToFloat(uint16_t half)
{
	float out = 0;
	vImage_Buffer src = { &half, 1, 1, sizeof(half) };
	vImage_Buffer dest = { &out, 1, 1, sizeof(out) };
	CHECK(vImageConvert_Planar16FtoPlanarF(&src, &dest, kvImageNoFlags) == kvImageNoError);
	return out;
}

static uint16_t floatToHalf(float value)
{
	uint16_t out = 0;
	vImage_Buffer src = { &value, 1, 1, sizeof(value) };
	vImage_Buffer dest = { &out, 1, 1, sizeof(out) };
	CHECK(vImageConvert_PlanarFtoPlanar16F(&src, &dest, kvImageNoFlags) == kvImageNoError);
	return out;
}

// The value of a half bit pattern with exponent field 0..31 (31 read as 2^16, the first value past
// the finite range), from the definition of binary16: subnormal m * 2^-24, normal (1024 + m) * 2^(e-25).
static float referenceHalfValue(unsigned pattern)
{
	unsigned exponent = pattern >> 10;
	unsigned fraction = pattern & 0x3ff;
	return exponent == 0 ? ldexpf((float)fraction, -24) : ldexpf((float)(1024 + fraction), (int)exponent - 25);
}

static void testKnownPairs(void)
{
	static const struct { uint16_t half; float value; } pairs[] = {
		{ 0x3C00, 1.0f }, { 0xBC00, -1.0f }, { 0xC000, -2.0f }, { 0x4000, 2.0f }, { 0x3800, 0.5f },
		{ 0x7BFF, 65504.0f }, { 0xFBFF, -65504.0f },
		{ 0x0001, 0x1p-24f }, { 0x8001, -0x1p-24f },
		{ 0x03FF, 0x1.ff8p-15f },	// largest subnormal, 1023 * 2^-24
		{ 0x0400, 0x1p-14f },		// smallest normal
		{ 0x3555, 0x1.554p-2f },	// the half nearest 1/3
		{ 0x0000, 0.0f },
	};
	for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
		CHECK(floatBits(halfToFloat(pairs[i].half)) == floatBits(pairs[i].value));
		CHECK(floatToHalf(pairs[i].value) == pairs[i].half);
	}

	// Signed zeros keep their sign in both directions.
	CHECK(floatBits(halfToFloat(0x8000)) == 0x80000000u);
	CHECK(floatToHalf(-0.0f) == 0x8000);

	CHECK(floatBits(halfToFloat(0x7C00)) == 0x7F800000u);
	CHECK(floatBits(halfToFloat(0xFC00)) == 0xFF800000u);
	CHECK(floatToHalf(INFINITY) == 0x7C00);
	CHECK(floatToHalf(-INFINITY) == 0xFC00);

	CHECK(isnan(halfToFloat(0x7E00)));
	CHECK(isnan(halfToFloat(0xFE00)) && (floatBits(halfToFloat(0xFE00)) >> 31) == 1);
	CHECK(isnan(halfToFloat(0x7C01)));
	// Quiet NaN: unchanged payload. Signalling NaN: quiet bit set, sign and payload bits kept.
	CHECK(floatBits(halfToFloat(0x7E00)) == 0x7FC00000u);
	CHECK(floatBits(halfToFloat(0x7C01)) == 0x7FC02000u);
	CHECK(floatBits(halfToFloat(0xFC01)) == 0xFFC02000u);
	CHECK(floatBits(halfToFloat(0x7DFF)) == 0x7FFFE000u);
	uint16_t nan = floatToHalf(NAN);
	CHECK((nan & 0x7C00) == 0x7C00 && (nan & 0x03FF) != 0);
	// A float NaN whose payload sits only in the low bits (all of them dropped) is still a NaN, not infinity.
	uint16_t lowPayload = floatToHalf(bitsFloat(0x7F800001u));
	CHECK((lowPayload & 0x7C00) == 0x7C00 && (lowPayload & 0x03FF) != 0);
	uint16_t negNan = floatToHalf(bitsFloat(0xFFC00000u));
	CHECK((negNan & 0xFC00) == 0xFC00 && (negNan & 0x03FF) != 0);
}

static void testRoundTripAllHalves(void)
{
	uint16_t halves[65536];
	float singles[65536];
	uint16_t back[65536];
	for (unsigned i = 0; i < 65536; i++)
		halves[i] = (uint16_t)i;

	vImage_Buffer halfBuffer = { halves, 1, 65536, sizeof(halves) };
	vImage_Buffer singleBuffer = { singles, 1, 65536, sizeof(singles) };
	vImage_Buffer backBuffer = { back, 1, 65536, sizeof(back) };
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &backBuffer, kvImageNoFlags) == kvImageNoError);

	int widened = 0, identical = 0, quieted = 0;
	for (unsigned i = 0; i < 65536; i++) {
		unsigned exponent = (i >> 10) & 0x1f;
		unsigned fraction = i & 0x3ff;
		if (exponent != 0x1f) {
			// Finite values: the single is exactly the half's value, and the round trip is identical.
			float expected = referenceHalfValue(i & 0x7fff);
			if (i & 0x8000)
				expected = -expected;
			widened += floatBits(singles[i]) == floatBits(expected);
			identical += back[i] == i;
		} else if (fraction == 0) {
			widened += floatBits(singles[i]) == (((i & 0x8000u) << 16) | 0x7F800000u);
			identical += back[i] == i;
		} else {
			widened += isnan(singles[i]) != 0;
			if (fraction & 0x200)
				identical += back[i] == i;
			else
				quieted += back[i] == (i | 0x200);
		}
	}
	CHECK(widened == 65536);
	// 2 * 1023 NaN patterns, half of each sign's fractions are signalling (bit 9 clear).
	CHECK(quieted == 2 * 511);
	CHECK(identical == 65536 - 2 * 511);
}

static void testRoundingToNearestEven(void)
{
	// Every pair of adjacent non-negative halves 0x0000..0x7BFF, with 0x7C00 standing for 2^16:
	// the midpoint is exactly representable in single; the tie goes to the even pattern, a hair
	// above goes up, a hair below goes down. The negative side mirrors it.
	int checked = 0;
	for (unsigned h = 0; h <= 0x7BFF; h++) {
		float lower = referenceHalfValue(h);
		float upper = referenceHalfValue(h + 1);
		float midpoint = (float)(((double)lower + (double)upper) / 2);
		uint16_t evenPattern = (h & 1) ? (uint16_t)(h + 1) : (uint16_t)h;
		uint16_t negative = 0x8000;

		CHECK(floatToHalf(lower) == h);
		CHECK(floatToHalf(-lower) == (uint16_t)(h | negative));
		CHECK(floatToHalf(midpoint) == evenPattern);
		CHECK(floatToHalf(-midpoint) == (uint16_t)(evenPattern | negative));
		CHECK(floatToHalf(nextafterf(midpoint, INFINITY)) == (uint16_t)(h + 1));
		CHECK(floatToHalf(nextafterf(midpoint, 0.0f)) == (uint16_t)h);
		checked++;
	}
	CHECK(checked == 0x7C00);

	// Named boundary cases of the same rule.
	CHECK(floatToHalf(1.0f + 0x1p-11f) == 0x3C00);			// tie between 0x3C00 (even) and 0x3C01
	CHECK(floatToHalf(1.0f + 3 * 0x1p-11f) == 0x3C02);		// tie between 0x3C01 and 0x3C02 (even)
	CHECK(floatToHalf(0x1p-25f) == 0x0000);				// tie between 0 and 2^-24: zero is even
	CHECK(floatToHalf(nextafterf(0x1p-25f, 1.0f)) == 0x0001);
	CHECK(floatToHalf(0x1.8p-25f) == 0x0001);			// tie between 2^-24 and 2^-23: 0x0001 is odd, so up
	CHECK(floatToHalf(0x1.ffcp-15f + 0x1p-26f) == 0x0400);		// rounds up out of the subnormals into 2^-14
}

static void testOverflowAndUnderflow(void)
{
	CHECK(floatToHalf(65504.0f) == 0x7BFF);
	CHECK(floatToHalf(nextafterf(65520.0f, 0.0f)) == 0x7BFF);
	CHECK(floatToHalf(65520.0f) == 0x7C00);		// tie between 65504 (odd pattern) and "65536": rounds to infinity
	CHECK(floatToHalf(65536.0f) == 0x7C00);
	CHECK(floatToHalf(70000.0f) == 0x7C00);
	CHECK(floatToHalf(-70000.0f) == 0xFC00);
	CHECK(floatToHalf(131072.0f) == 0x7C00);		// 2^17
	CHECK(floatToHalf(nextafterf(131072.0f, 0.0f)) == 0x7C00);
	CHECK(floatToHalf(-nextafterf(131072.0f, 0.0f)) == 0xFC00);
	CHECK(floatToHalf(1.0e10f) == 0x7C00);
	CHECK(floatToHalf(-1.0e10f) == 0xFC00);
	CHECK(floatToHalf(3.4028235e38f) == 0x7C00);

	CHECK(floatToHalf(0x1p-26f) == 0x0000);
	CHECK(floatToHalf(-0x1p-26f) == 0x8000);
	CHECK(floatToHalf(1.0e-40f) == 0x0000);			// a single subnormal
	CHECK(floatToHalf(bitsFloat(0x80000001u)) == 0x8000);	// smallest negative single subnormal
	CHECK(floatToHalf(1.0e-30f) == 0x0000);
}

static void testRowStrideAndPadding(void)
{
	// 3 x 2 pixels. Half rows are 10 bytes (4 padding), single rows are 16 bytes (4 padding).
	static const uint16_t halves[2][3] = { { 0x3C00, 0xC000, 0x7BFF }, { 0x0001, 0x7C00, 0x3555 } };
	static const float expected[2][3] = { { 1.0f, -2.0f, 65504.0f }, { 0x1p-24f, INFINITY, 0x1.554p-2f } };

	uint8_t halfRows[20];
	uint8_t singleRows[32];
	memset(halfRows, 0xEE, sizeof(halfRows));
	memset(singleRows, 0xEE, sizeof(singleRows));
	for (int row = 0; row < 2; row++)
		memcpy(halfRows + row * 10, halves[row], 6);

	vImage_Buffer halfBuffer = { halfRows, 2, 3, 10 };
	vImage_Buffer singleBuffer = { singleRows, 2, 3, 16 };
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageNoFlags) == kvImageNoError);
	for (int row = 0; row < 2; row++) {
		CHECK(memcmp(singleRows + row * 16, expected[row], 12) == 0);
		for (int pad = 12; pad < 16; pad++)
			CHECK(singleRows[row * 16 + pad] == 0xEE);
	}

	// And back: the padding of the half rows stays untouched.
	uint8_t halfOut[20];
	memset(halfOut, 0xDD, sizeof(halfOut));
	vImage_Buffer halfOutBuffer = { halfOut, 2, 3, 10 };
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfOutBuffer, kvImageDoNotTile) == kvImageNoError);
	for (int row = 0; row < 2; row++) {
		CHECK(memcmp(halfOut + row * 10, halves[row], 6) == 0);
		for (int pad = 6; pad < 10; pad++)
			CHECK(halfOut[row * 10 + pad] == 0xDD);
	}

	// A destination whose rowBytes differs from the source's: rows are addressed independently.
	uint8_t wideSingles[2 * 20];
	memset(wideSingles, 0x11, sizeof(wideSingles));
	vImage_Buffer wideBuffer = { wideSingles, 2, 3, 20 };
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &wideBuffer, kvImageNoFlags) == kvImageNoError);
	CHECK(memcmp(wideSingles, expected[0], 12) == 0);
	CHECK(memcmp(wideSingles + 20, expected[1], 12) == 0);
	CHECK(wideSingles[12] == 0x11 && wideSingles[19] == 0x11);
}

static void testErrors(void)
{
	uint16_t halves[6] = { 0 };
	float singles[6] = { 0 };
	vImage_Buffer halfBuffer = { halves, 2, 3, 6 };
	vImage_Buffer singleBuffer = { singles, 2, 3, 12 };

	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfBuffer, kvImageNoFlags) == kvImageNoError);

	// A destination wider or taller than the source: kvImageRoiLargerThanInputBuffer, nothing written.
	vImage_Buffer wider = { singles, 2, 4, 16 };
	vImage_Buffer taller = { singles, 3, 3, 12 };
	singles[0] = 7.0f;
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &wider, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &taller, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(singles[0] == 7.0f);
	{
		vImage_Buffer halfWide = { halves, 2, 4, 8 };
		vImage_Buffer halfTall = { halves, 3, 3, 6 };
		CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfWide, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
		CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfTall, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	}
	// An empty source with a destination that has pixels.
	vImage_Buffer emptyWidth = { halves, 1, 0, 0 };
	vImage_Buffer fourSingles = { singles, 1, 4, 16 };
	CHECK(vImageConvert_Planar16FtoPlanarF(&emptyWidth, &fourSingles, kvImageNoFlags) == kvImageRoiLargerThanInputBuffer);
	CHECK(singles[0] == 7.0f);

	// A narrower destination is converted at its own size (macOS: width 3 into width 4 returns 0).
	{
		uint16_t four[4] = { 0x3C00, 0x4000, 0x4200, 0x4400 };
		float three[4] = { 0, 0, 0, 5.0f };
		vImage_Buffer fourBuffer = { four, 1, 4, 8 };
		vImage_Buffer threeBuffer = { three, 1, 3, 12 };
		CHECK(vImageConvert_Planar16FtoPlanarF(&fourBuffer, &threeBuffer, kvImageNoFlags) == kvImageNoError);
		CHECK(three[0] == 1.0f && three[1] == 2.0f && three[2] == 3.0f && three[3] == 5.0f);
	}

	// Null buffers (checked before the flags, also with kvImageGetTempBufferSize) and null data.
	vImage_Buffer noData = { NULL, 2, 3, 12 };
	CHECK(vImageConvert_Planar16FtoPlanarF(NULL, &singleBuffer, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, NULL, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_PlanarFtoPlanar16F(NULL, &halfBuffer, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, NULL, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_Planar16FtoPlanarF(NULL, NULL, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_Planar16FtoPlanarF(NULL, NULL, kvImageGetTempBufferSize) == kvImageNullPointerArgument);
	CHECK(vImageConvert_PlanarFtoPlanar16F(NULL, NULL, kvImageGetTempBufferSize) == kvImageNullPointerArgument);
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &noData, kvImageNoFlags) == kvImageNullPointerArgument);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&noData, &halfBuffer, kvImageNoFlags) == kvImageNullPointerArgument);

	// rowBytes smaller than a row is not an error (macOS returns 0), one row high so nothing overlaps.
	{
		uint16_t row16[3] = { 0x3C00, 0x4000, 0x4200 };
		float row32[3] = { 0, 0, 0 };
		vImage_Buffer tightHalf = { row16, 1, 3, 2 };
		vImage_Buffer tightSingle = { row32, 1, 3, 4 };
		vImage_Buffer okSingle = { row32, 1, 3, 12 };
		CHECK(vImageConvert_Planar16FtoPlanarF(&tightHalf, &okSingle, kvImageNoFlags) == kvImageNoError);
		CHECK(row32[0] == 1.0f && row32[2] == 3.0f);
		vImage_Buffer okHalf = { row16, 1, 3, 6 };
		CHECK(vImageConvert_Planar16FtoPlanarF(&okHalf, &tightSingle, kvImageNoFlags) == kvImageNoError);
		CHECK(vImageConvert_PlanarFtoPlanar16F(&tightSingle, &okHalf, kvImageNoFlags) == kvImageNoError);
	}

	// Empty buffers have nothing to convert and succeed, whatever their data pointers.
	vImage_Buffer emptyHalf = { NULL, 0, 0, 0 };
	vImage_Buffer emptySingle = { NULL, 0, 0, 0 };
	vImage_Buffer zeroHeight = { NULL, 0, 4, 16 };
	CHECK(vImageConvert_Planar16FtoPlanarF(&emptyHalf, &emptySingle, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&emptySingle, &emptyHalf, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageConvert_Planar16FtoPlanarF(&zeroHeight, &zeroHeight, kvImageNoFlags) == kvImageNoError);

	// kvImageGetTempBufferSize: 0 is returned and nothing is written; the buffers are not checked.
	halves[0] = 0x3C00;
	singles[0] = 7.0f;
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageGetTempBufferSize) == 0);
	CHECK(singles[0] == 7.0f);
	halves[0] = 0x1234;
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfBuffer, kvImageGetTempBufferSize) == 0);
	CHECK(halves[0] == 0x1234);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&wider, &halfBuffer, kvImageGetTempBufferSize) == 0);
	CHECK(vImageConvert_Planar16FtoPlanarF(&noData, &halfBuffer, kvImageGetTempBufferSize) == 0);

	// kvImageDoNotTile is accepted; the flags macOS refuses give kvImageUnknownFlagsBit and write nothing.
	halves[0] = 0x3C00;
	singles[0] = 0;
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageDoNotTile) == kvImageNoError);
	CHECK(singles[0] == 1.0f);
	singles[0] = 7.0f;
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageLeaveAlphaUnchanged) == kvImageUnknownFlagsBit);
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, kvImageHighQualityResampling) == kvImageUnknownFlagsBit);
	CHECK(vImageConvert_Planar16FtoPlanarF(&halfBuffer, &singleBuffer, 0x40000000) == kvImageUnknownFlagsBit);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfBuffer, kvImageLeaveAlphaUnchanged) == kvImageUnknownFlagsBit);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfBuffer, kvImageHighQualityResampling) == kvImageUnknownFlagsBit);
	CHECK(vImageConvert_PlanarFtoPlanar16F(&singleBuffer, &halfBuffer, 0x40000000) == kvImageUnknownFlagsBit);
	CHECK(singles[0] == 7.0f);
}

int main(void)
{
	testKnownPairs();
	testRoundTripAllHalves();
	testRoundingToNearestEven();
	testOverflowAndUnderflow();
	testRowStrideAndPadding();
	testErrors();

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
