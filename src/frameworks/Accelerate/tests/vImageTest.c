// Copyright (C) 2026 Lubos Dolezel
//
// Native test: compile with the vImage sources it covers and run on the host.
//   cc -I include tests/vImageTest.c src/Accelerate.c -lm -o vImageTest
// Error codes and flag values are those of Apple's vImage_Types.h. The scaling
// expectations follow bilinear sampling at pixel centres, edge pixels repeated.
#include <Accelerate/Accelerate.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static vImage_Buffer makeBuffer(void* data, vImagePixelCount width, vImagePixelCount height, size_t rowBytes)
{
	vImage_Buffer buffer = { data, height, width, rowBytes };
	return buffer;
}

static void testConstants(void)
{
	CHECK(kvImageNoError == 0);
	CHECK(kvImageRoiLargerThanInputBuffer == -21766);
	CHECK(kvImageMemoryAllocationError == -21771);
	CHECK(kvImageNullPointerArgument == -21772);
	CHECK(kvImageInvalidParameter == -21773);
	CHECK(kvImageUnknownFlagsBit == -21775);
	CHECK(kvImageInvalidRowBytes == -21777);
	CHECK(kvImageInvalidImageFormat == -21778);
	CHECK(kvImageUnsupportedConversion == -21783);
	CHECK(kvImageCoreVideoIsAbsent == -21784);

	CHECK(kvImageLeaveAlphaUnchanged == 1);
	CHECK(kvImageCopyInPlace == 2);
	CHECK(kvImageBackgroundColorFill == 4);
	CHECK(kvImageEdgeExtend == 8);
	CHECK(kvImageDoNotTile == 16);
	CHECK(kvImageHighQualityResampling == 32);
	CHECK(kvImageTruncateKernel == 64);
	CHECK(kvImageGetTempBufferSize == 128);

	CHECK(sizeof(vImage_Buffer) == 4 * sizeof(void*));
}

static void testValidation(void)
{
	uint8_t pixels[16] = { 0 };
	vImage_Buffer src = makeBuffer(pixels, 2, 2, 8);
	vImage_Buffer dest = makeBuffer(pixels, 2, 2, 8);

	CHECK(vImageScale_ARGB8888(NULL, &dest, NULL, 0) == kvImageNullPointerArgument);
	CHECK(vImageScale_ARGB8888(&src, NULL, NULL, 0) == kvImageNullPointerArgument);

	vImage_Buffer noData = makeBuffer(NULL, 2, 2, 8);
	CHECK(vImageScale_ARGB8888(&noData, &dest, NULL, 0) == kvImageNullPointerArgument);
	CHECK(vImageScale_ARGB8888(&src, &noData, NULL, 0) == kvImageNullPointerArgument);

	vImage_Buffer empty = makeBuffer(pixels, 0, 2, 8);
	CHECK(vImageScale_ARGB8888(&empty, &dest, NULL, 0) == kvImageInvalidParameter);
	empty = makeBuffer(pixels, 2, 0, 8);
	CHECK(vImageScale_ARGB8888(&src, &empty, NULL, 0) == kvImageInvalidParameter);

	vImage_Buffer shortRows = makeBuffer(pixels, 2, 2, 4);
	CHECK(vImageScale_ARGB8888(&shortRows, &dest, NULL, 0) == kvImageInvalidRowBytes);
	CHECK(vImageScale_ARGB8888(&src, &shortRows, NULL, 0) == kvImageInvalidRowBytes);

	CHECK(vImageScale_ARGB8888(&src, &dest, NULL, 0x10000) == kvImageUnknownFlagsBit);

	// The temporary buffer size query needs no pixel memory and reports no scratch need
	vImage_Buffer q1 = makeBuffer(NULL, 4, 4, 16), q2 = makeBuffer(NULL, 2, 2, 8);
	CHECK(vImageScale_ARGB8888(&q1, &q2, NULL, kvImageGetTempBufferSize) == 0);
}

static void testScaling(void)
{
	// 1:1 copies, with a row stride larger than the row
	uint8_t src1[2 * 12], dest1[2 * 12];
	for (int i = 0; i < (int) sizeof(src1); i++)
		src1[i] = (uint8_t) (i + 1);
	memset(dest1, 0xEE, sizeof(dest1));
	vImage_Buffer s = makeBuffer(src1, 2, 2, 12), d = makeBuffer(dest1, 2, 2, 12);
	CHECK(vImageScale_ARGB8888(&s, &d, NULL, 0) == kvImageNoError);
	CHECK(memcmp(dest1, src1, 8) == 0 && memcmp(dest1 + 12, src1 + 12, 8) == 0);
	CHECK(dest1[8] == 0xEE && dest1[11] == 0xEE); // padding untouched

	// kvImageLeaveAlphaUnchanged keeps the destination's alpha (channel 0 of ARGB)
	memset(dest1, 0x55, sizeof(dest1));
	CHECK(vImageScale_ARGB8888(&s, &d, NULL, kvImageLeaveAlphaUnchanged) == kvImageNoError);
	CHECK(dest1[0] == 0x55 && dest1[1] == src1[1] && dest1[2] == src1[2] && dest1[3] == src1[3]);

	// Upscale a row of two pixels to four; every channel is 0 then 100
	uint8_t two[8] = { 0, 0, 0, 0, 100, 100, 100, 100 }, four[16];
	vImage_Buffer up1 = makeBuffer(two, 2, 1, 8), up2 = makeBuffer(four, 4, 1, 16);
	CHECK(vImageScale_ARGB8888(&up1, &up2, NULL, 0) == kvImageNoError);
	const int expected[4] = { 0, 25, 75, 100 };
	for (int x = 0; x < 4; x++)
		for (int c = 0; c < 4; c++)
			CHECK(abs((int) four[x * 4 + c] - expected[x]) <= 1);

	// Upscaling and downscaling a flat colour keeps it, in a wide destination too
	enum { W = 300 };
	uint8_t* flat = malloc(4 * 4 * 4);
	uint8_t* wide = malloc((size_t) W * 4 * 3);
	for (int i = 0; i < 16; i++)
		memcpy(flat + i * 4, (uint8_t[]) { 255, 10, 200, 90 }, 4);
	vImage_Buffer f = makeBuffer(flat, 4, 4, 16), w = makeBuffer(wide, W, 3, (size_t) W * 4);
	CHECK(vImageScale_ARGB8888(&f, &w, NULL, 0) == kvImageNoError);
	int wrong = 0;
	for (int i = 0; i < W * 3; i++)
		wrong += memcmp(wide + i * 4, (uint8_t[]) { 255, 10, 200, 90 }, 4) != 0;
	CHECK(wrong == 0);

	vImage_Buffer f2 = makeBuffer(flat, 4, 4, 16), small = makeBuffer(four, 1, 1, 4);
	CHECK(vImageScale_ARGB8888(&f2, &small, NULL, 0) == kvImageNoError);
	CHECK(memcmp(four, (uint8_t[]) { 255, 10, 200, 90 }, 4) == 0);
	free(flat);
	free(wide);
}

static void testUnimplemented(void)
{
	uint8_t pixels[16] = { 0 };
	vImage_Buffer b = makeBuffer(pixels, 2, 2, 8);
	const uint8_t map[4] = { 3, 2, 1, 0 };

	// Entry points that do no work must not report success
	CHECK(vImagePermuteChannels_ARGB8888(&b, &b, map, 0) == kvImageUnsupportedConversion);
	CHECK(vImageUnpremultiplyData_ARGB8888(&b, &b, 0) == kvImageUnsupportedConversion);

	vImage_Buffer out = makeBuffer(pixels, 5, 5, 20);
	CHECK(vImageBuffer_InitWithCGImage(&out, NULL, NULL, NULL, 0) == kvImageUnsupportedConversion);
	CHECK(out.data == NULL && out.width == 0 && out.height == 0 && out.rowBytes == 0);
	CHECK(vImageBuffer_InitWithCGImage(NULL, NULL, NULL, NULL, 0) == kvImageNullPointerArgument);
}

int main(void)
{
	testConstants();
	testValidation();
	testScaling();
	testUnimplemented();

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
