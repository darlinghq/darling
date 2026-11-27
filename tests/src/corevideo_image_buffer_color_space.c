// Copyright (C) 2026 Lubos Dolezel
//
// CVImageBufferGetColorSpace returns the colour space attached to the buffer under kCVImageBufferCGColorSpaceKey
// (the very object), and NULL for a buffer that has none.
#include <CoreVideo/CoreVideo.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	CVPixelBufferRef pb = NULL;
	CHECK(CVPixelBufferCreate(NULL, 16, 16, kCVPixelFormatType_32BGRA, NULL, &pb) == kCVReturnSuccess && pb);
	if (!pb)
		return 1;
	CHECK(CVImageBufferGetColorSpace(pb) == NULL);

	CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	CVBufferSetAttachment(pb, kCVImageBufferCGColorSpaceKey, srgb, kCVAttachmentMode_ShouldPropagate);
	CHECK(CVImageBufferGetColorSpace(pb) == srgb);

	CGColorSpaceRef gray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2);
	CVBufferSetAttachment(pb, kCVImageBufferCGColorSpaceKey, gray, kCVAttachmentMode_ShouldPropagate);
	CHECK(CVImageBufferGetColorSpace(pb) == gray);

	CVBufferRemoveAttachment(pb, kCVImageBufferCGColorSpaceKey);
	CHECK(CVImageBufferGetColorSpace(pb) == NULL);

	CGColorSpaceRelease(gray);
	CGColorSpaceRelease(srgb);
	CVPixelBufferRelease(pb);
	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
