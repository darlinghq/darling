// Copyright (C) 2026 Lubos Dolezel
//
// vImageBufferFill_ARGB8888: every pixel of the destination gets the four
// color bytes in memory order, bytes beyond the row's pixels (row padding) are
// left alone, and empty buffers succeed. Flags are not validated (QUIRK, as
// observed on macOS 26.7: unknown flag bits and kvImageLeaveAlphaUnchanged are
// accepted and alpha is not preserved).
#include <Accelerate/Accelerate.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	Pixel_8888 color = { 1, 2, 3, 4 };

	// 3x2 pixels in 16-byte rows (4 bytes of padding each).
	uint8_t padded[32];
	memset(padded, 0xEE, sizeof(padded));
	vImage_Buffer buffer = { padded, 2, 3, 16 };
	CHECK(vImageBufferFill_ARGB8888(&buffer, color, kvImageNoFlags) == kvImageNoError);
	for (int row = 0; row < 2; row++) {
		for (int pixel = 0; pixel < 3; pixel++) {
			const uint8_t* p = padded + row * 16 + pixel * 4;
			CHECK(p[0] == 1 && p[1] == 2 && p[2] == 3 && p[3] == 4);
		}
		for (int pad = 12; pad < 16; pad++)
			CHECK(padded[row * 16 + pad] == 0xEE);
	}

	CHECK(vImageBufferFill_ARGB8888(&buffer, (Pixel_8888){ 9, 8, 7, 6 }, kvImageDoNotTile) == kvImageNoError);
	CHECK(padded[0] == 9 && padded[1] == 8 && padded[2] == 7 && padded[3] == 6);

	// QUIRK pinned here: unknown flags and kvImageLeaveAlphaUnchanged are accepted, alpha is overwritten.
	CHECK(vImageBufferFill_ARGB8888(&buffer, color, 0x40000000) == kvImageNoError);
	CHECK(vImageBufferFill_ARGB8888(&buffer, (Pixel_8888){ 5, 6, 7, 8 }, kvImageLeaveAlphaUnchanged) == kvImageNoError);
	CHECK(padded[0] == 5);

	// Empty buffers have nothing to write and succeed, whatever the data pointer.
	vImage_Buffer noHeight = { padded, 0, 3, 16 };
	vImage_Buffer noWidth = { padded, 2, 0, 16 };
	vImage_Buffer noData = { NULL, 0, 0, 0 };
	memset(padded, 0xEE, sizeof(padded));
	CHECK(vImageBufferFill_ARGB8888(&noHeight, color, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageBufferFill_ARGB8888(&noWidth, color, kvImageNoFlags) == kvImageNoError);
	CHECK(vImageBufferFill_ARGB8888(&noData, color, kvImageNoFlags) == kvImageNoError);
	CHECK(padded[0] == 0xEE && padded[16] == 0xEE);

	// A one-pixel buffer in one row.
	uint8_t one[4] = { 0, 0, 0, 0 };
	vImage_Buffer single = { one, 1, 1, 4 };
	CHECK(vImageBufferFill_ARGB8888(&single, color, kvImageNoFlags) == kvImageNoError);
	CHECK(memcmp(one, color, 4) == 0);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
