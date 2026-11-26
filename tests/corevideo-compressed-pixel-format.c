// Copyright (C) 2026 Lubos Dolezel
//
// CVIsCompressedPixelFormatAvailable reports whether the platform supports a
// compressed pixel format. Observed on macOS 26.7.3 x86_64 (no hardware with
// compressed formats): false for the lossless and lossy formats, and also for
// uncompressed and unknown format codes. Darling supports no compressed
// formats, so it answers false everywhere.

#include <CoreVideo/CoreVideo.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void)
{
	CHECK(!CVIsCompressedPixelFormatAvailable(kCVPixelFormatType_Lossless_32BGRA));
	CHECK(!CVIsCompressedPixelFormatAvailable(kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange));
	CHECK(!CVIsCompressedPixelFormatAvailable(kCVPixelFormatType_Lossy_32BGRA));
	CHECK(!CVIsCompressedPixelFormatAvailable(kCVPixelFormatType_32BGRA));
	CHECK(!CVIsCompressedPixelFormatAvailable(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange));
	CHECK(!CVIsCompressedPixelFormatAvailable('zzzz'));
	CHECK(!CVIsCompressedPixelFormatAvailable(0));
	printf(failures ? "FAILED\n" : "PASSED\n");
	return failures != 0;
}
