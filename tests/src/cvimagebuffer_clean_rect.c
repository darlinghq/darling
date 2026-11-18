// CFLAGS: -framework CoreVideo -framework CoreFoundation
// Copyright (C) 2026 Lubos Dolezel
//
// CVImageBufferGetCleanRect: the encoded rectangle without a clean aperture or with an incomplete
// one, otherwise the aperture centered in the buffer and moved by its offsets (vertical offset
// upwards). Observed on macOS 26.7.3; the program passes unchanged there.
#include <CoreVideo/CoreVideo.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

static void setAperture(CVPixelBufferRef buffer, int width, int height, int dx, int dy, int count)
{
	CFStringRef keys[4] = { kCVImageBufferCleanApertureWidthKey, kCVImageBufferCleanApertureHeightKey,
		kCVImageBufferCleanApertureHorizontalOffsetKey, kCVImageBufferCleanApertureVerticalOffsetKey };
	int values[4] = { width, height, dx, dy };
	CFNumberRef numbers[4];
	for (int i = 0; i < 4; i++)
		numbers[i] = CFNumberCreate(NULL, kCFNumberIntType, &values[i]);
	CFDictionaryRef aperture = CFDictionaryCreate(NULL, (const void **) keys, (const void **) numbers, count,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CVBufferSetAttachment(buffer, kCVImageBufferCleanApertureKey, aperture, kCVAttachmentMode_ShouldPropagate);
	CFRelease(aperture);
	for (int i = 0; i < 4; i++)
		CFRelease(numbers[i]);
}

static int rectIs(CGRect r, double x, double y, double w, double h)
{
	return r.origin.x == x && r.origin.y == y && r.size.width == w && r.size.height == h;
}

int main(void)
{
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreate(NULL, 64, 48, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnSuccess);

	CHECK(rectIs(CVImageBufferGetCleanRect(buffer), 0, 0, 64, 48));
	setAperture(buffer, 32, 24, 0, 0, 4);
	CHECK(rectIs(CVImageBufferGetCleanRect(buffer), 16, 12, 32, 24));
	setAperture(buffer, 32, 24, 8, -4, 4);
	CHECK(rectIs(CVImageBufferGetCleanRect(buffer), 24, 16, 32, 24));
	setAperture(buffer, 32, 24, 0, 0, 1);   // only the width: incomplete
	CHECK(rectIs(CVImageBufferGetCleanRect(buffer), 0, 0, 64, 48));
	CHECK(rectIs(CVImageBufferGetCleanRect(NULL), 0, 0, 0, 0));

	CVPixelBufferRelease(buffer);
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures != 0;
}
