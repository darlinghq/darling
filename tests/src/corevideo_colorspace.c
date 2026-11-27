// CFLAGS: -framework CoreVideo -framework CoreGraphics
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    CHECK(CVImageBufferGetColorSpace(NULL) == NULL);

    CVPixelBufferRef pb = NULL;
    CHECK(CVPixelBufferCreate(NULL, 4, 4, kCVPixelFormatType_32BGRA, NULL, &pb) == kCVReturnSuccess);
    CHECK(pb != NULL);
    CHECK(CVImageBufferGetColorSpace(pb) == NULL);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CHECK(cs != NULL);
    CVBufferSetAttachment(pb, kCVImageBufferCGColorSpaceKey, cs, kCVAttachmentMode_ShouldPropagate);
    CFIndex rcBefore = CFGetRetainCount(cs);
    CHECK(CVImageBufferGetColorSpace(pb) == cs);
    CHECK(CFGetRetainCount(cs) == rcBefore);

    CGColorSpaceRelease(cs);
    CHECK(CVImageBufferGetColorSpace(pb) == cs);

    CVBufferRemoveAttachment(pb, kCVImageBufferCGColorSpaceKey);
    CHECK(CVImageBufferGetColorSpace(pb) == NULL);

    CVPixelBufferRelease(pb);
    puts("PASS");
    return 0;
}
