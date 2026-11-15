#include <ImageIO/ImageIO.h>
#include <stdio.h>

#define EXPECT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 1; } } while (0)

// A 1x1 RGBA PNG made for this test.
static const unsigned char png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4,
    0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

// kCGImageSourceShouldCacheImmediately (CGImageSource.h, macOS 10.9): a CFBoolean option of
// CGImageSourceCreateImageAtIndex that asks for the image to be decoded and cached when it is created. The
// key must exist, differ from kCGImageSourceShouldCache, and be accepted without changing the image.
int main(void) {
    EXPECT(kCGImageSourceShouldCacheImmediately != NULL && CFGetTypeID(kCGImageSourceShouldCacheImmediately) == CFStringGetTypeID(), "the key is a CFString");
    EXPECT(!CFEqual(kCGImageSourceShouldCacheImmediately, kCGImageSourceShouldCache), "it is not the lazy-cache key");

    CFDataRef data = CFDataCreate(NULL, png, sizeof png);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    EXPECT(source != NULL, "the PNG opens");
    const void *keys[] = {kCGImageSourceShouldCacheImmediately};
    const void *values[] = {kCFBooleanTrue};
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, options);
    EXPECT(image != NULL, "an image is created with the option");
    EXPECT(CGImageGetWidth(image) == 1 && CGImageGetHeight(image) == 1, "the image is the 1x1 PNG");
    CGImageRelease(image);
    CFRelease(options);
    CFRelease(source);
    CFRelease(data);
    printf("PASS: kCGImageSourceShouldCacheImmediately\n");
    return 0;
}
