#include <ImageIO/ImageIO.h>
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdint.h>
#include <unistd.h>

// kCGImageDestinationImageMaxPixelSize, a CFNumber in the per-image properties of CGImageDestinationAddImage and
// AddImageFromSource. Apple documents it as: if present, the destination rescales the image to fit within the maximum width
// and height; if absent, the native size is kept. The other expectations are observations of macOS 26.7.3 and 10.14.4 (see
// CGImageDestination.m): the longer side becomes the limit (an image is enlarged as well as reduced), the other side is
// truncated with a minimum of one pixel, the value must be a number of at least 1, the key is read from the per-image
// properties and not from the destination's options. Failing the destination when the image cannot be scaled is the choice
// of this implementation, where macOS gives no answer.
//   clang -framework CoreFoundation -framework ImageIO -framework CoreGraphics tests/imageio-destination-max-pixel-size.c -o /tmp/imageio-destination-max-pixel-size
// Exits non-zero on failure.
#define EXPECT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)
static int failures;

// A 40x20 RGBA PNG made for this test: the left half is red, the right half is blue.
static const unsigned char png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x14, 0x08, 0x06, 0x00, 0x00, 0x00, 0xff, 0x46, 0x7f,
    0xbb, 0x00, 0x00, 0x00, 0x2c, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xce, 0xb1, 0x0d, 0x00,
    0x00, 0x0c, 0xc2, 0x30, 0xfe, 0x7f, 0x9a, 0x3e, 0xc1, 0x54, 0x79, 0xc8, 0x1a, 0x39, 0x4d, 0xba,
    0x6c, 0xbc, 0x6b, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x3f, 0x03,
    0x0f, 0x6c, 0x6e, 0x3a, 0x1b, 0x4a, 0xb4, 0x79, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82};

// The red and blue channels of the pixel at (x, y) of an image, drawn into an RGBA bitmap of its own size.
static int channels(CGImageRef image, size_t x, size_t y, unsigned char *red, unsigned char *blue) {
    size_t width = CGImageGetWidth(image), height = CGImageGetHeight(image);
    unsigned char pixels[64 * 64 * 4] = {0};
    if (width > 64 || height > 64) return 0;
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pixels, width, height, 8, width * 4, space, kCGImageAlphaPremultipliedLast);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    *red = pixels[(y * width + x) * 4];
    *blue = pixels[(y * width + x) * 4 + 2];
    CGContextRelease(context);
    CGColorSpaceRelease(space);
    return 1;
}


static CFDictionaryRef limitDictionary(CFTypeRef value) {
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (value) CFDictionarySetValue(dictionary, kCGImageDestinationImageMaxPixelSize, value);
    return dictionary;
}

static CFDictionaryRef intLimit(int limit) {
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &limit);
    CFDictionaryRef dictionary = limitDictionary(number);
    CFRelease(number);
    return dictionary;
}

// Encodes either `image` (AddImage) or image 0 of `source` (AddImageFromSource) into a PNG with `properties` as the per-image
// properties and `destinationOptions` as the destination's, and reads the PNG back.
static CGImageRef roundTrip(CGImageRef image, CGImageSourceRef source, CFDictionaryRef properties, CFDictionaryRef destinationOptions) {
    CFMutableDataRef encoded = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef destination = CGImageDestinationCreateWithData(encoded, CFSTR("public.png"), 1, destinationOptions);
    if (image) CGImageDestinationAddImage(destination, image, properties);
    else CGImageDestinationAddImageFromSource(destination, source, 0, properties);
    CGImageRef back = NULL;
    if (CGImageDestinationFinalize(destination)) {
        CGImageSourceRef again = CGImageSourceCreateWithData(encoded, NULL);
        back = again ? CGImageSourceCreateImageAtIndex(again, 0, NULL) : NULL;
    }
    CFRelease(destination);
    return back;
}

#define SIZE_IS(image, w, h) ((image) != NULL && CGImageGetWidth(image) == (w) && CGImageGetHeight(image) == (h))

int main(void) {
    CFDataRef data = CFDataCreate(NULL, png, sizeof png);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CGImageRef image = source ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
    if (image == NULL) { printf("FAIL: the PNG opens\n"); return 1; }

    CGImageRef back = roundTrip(image, NULL, intLimit(10), NULL);
    EXPECT(SIZE_IS(back, 10, 5), "AddImage with a limit of 10 writes the 40x20 image as 10x5 (aspect kept)");
    unsigned char red, blue;
    EXPECT(back && channels(back, 1, 2, &red, &blue) && red > 200 && blue < 50, "the left of the limited image is red");
    EXPECT(back && channels(back, 8, 2, &red, &blue) && red < 50 && blue > 200, "the right of the limited image is blue");

    back = roundTrip(NULL, source, intLimit(20), NULL);
    EXPECT(SIZE_IS(back, 20, 10), "AddImageFromSource with a limit of 20 writes 20x10");

    back = roundTrip(image, NULL, NULL, NULL);
    EXPECT(SIZE_IS(back, 40, 20), "without the key the image keeps its size");
    back = roundTrip(image, NULL, intLimit(100), NULL);
    EXPECT(SIZE_IS(back, 100, 50), "a limit above the image's size enlarges it: 40x20 at 100 is 100x50");
    back = roundTrip(image, NULL, intLimit(41), NULL);
    EXPECT(SIZE_IS(back, 41, 20), "40x20 at a limit of 41 is 41x20: the longer side is the limit, the other side is truncated");
    back = roundTrip(NULL, source, intLimit(100), NULL);
    EXPECT(SIZE_IS(back, 100, 50), "AddImageFromSource enlarges as well: 40x20 at 100 is 100x50");
    back = roundTrip(image, NULL, intLimit(40), NULL);
    EXPECT(SIZE_IS(back, 40, 20), "a limit equal to the longer side adds it unchanged");
    back = roundTrip(image, NULL, intLimit(1), NULL);
    EXPECT(SIZE_IS(back, 1, 1), "the smaller side never drops below one pixel");

    // A tall image: the height is the longer side.
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(NULL, 20, 40, 8, 0, space, kCGImageAlphaPremultipliedLast);
    CGContextDrawImage(context, CGRectMake(0, 0, 20, 40), image);
    CGImageRef tall = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    CGColorSpaceRelease(space);
    EXPECT(SIZE_IS(roundTrip(tall, NULL, intLimit(30), NULL), 15, 30), "a 20x40 image with a limit of 30 is written as 15x30 (the height is the longer side)");
    EXPECT(SIZE_IS(roundTrip(tall, NULL, intLimit(40), NULL), 20, 40), "a 20x40 image with a limit of 40 is unchanged");
    EXPECT(SIZE_IS(roundTrip(image, NULL, intLimit(7), NULL), 7, 3), "40x20 at a limit of 7 is 7x3: the shorter side is truncated (3.5)");

    EXPECT(SIZE_IS(roundTrip(tall, NULL, intLimit(7), NULL), 3, 7), "20x40 at a limit of 7 is 3x7: the shorter side is truncated (3.5)");
    CGColorSpaceRef rowSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef rowContext = CGBitmapContextCreate(NULL, 40, 1, 8, 0, rowSpace, kCGImageAlphaPremultipliedLast);
    CGImageRef row = CGBitmapContextCreateImage(rowContext);
    CGContextRelease(rowContext);
    CGColorSpaceRelease(rowSpace);
    EXPECT(SIZE_IS(roundTrip(row, NULL, intLimit(10), NULL), 10, 1), "a 40x1 image at a limit of 10 is 10x1: the shorter side never drops below one pixel");

    // One image is added once: a TIFF destination holds exactly one image afterwards.
    CFMutableDataRef tiff = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef tiffDestination = CGImageDestinationCreateWithData(tiff, CFSTR("public.tiff"), 1, NULL);
    CGImageDestinationAddImage(tiffDestination, image, intLimit(10));
    EXPECT(CGImageDestinationFinalize(tiffDestination), "a TIFF destination finalizes");
    CGImageSourceRef tiffSource = CGImageSourceCreateWithData(tiff, NULL);
    EXPECT(tiffSource != NULL && CGImageSourceGetCount(tiffSource) == 1, "a limited image is added once, not twice");

    double real = 10.0;
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberDoubleType, &real);
    back = roundTrip(image, NULL, limitDictionary(number), NULL);
    EXPECT(SIZE_IS(back, 10, 5), "a floating point limit counts");
    double fraction = 10.5;
    CFNumberRef fractional = CFNumberCreate(NULL, kCFNumberDoubleType, &fraction);
    EXPECT(SIZE_IS(roundTrip(image, NULL, limitDictionary(fractional), NULL), 10, 5), "a limit of 10.5 is 10 (truncated)");
    double belowOne = 0.5;
    CFNumberRef small = CFNumberCreate(NULL, kCFNumberDoubleType, &belowOne);
    EXPECT(SIZE_IS(roundTrip(image, NULL, limitDictionary(small), NULL), 40, 20), "a limit below 1 is ignored");
    EXPECT(SIZE_IS(roundTrip(image, NULL, intLimit(0), NULL), 40, 20), "a limit of 0 is ignored");
    EXPECT(SIZE_IS(roundTrip(image, NULL, intLimit(-5), NULL), 40, 20), "a negative limit is ignored");
    EXPECT(SIZE_IS(roundTrip(image, NULL, limitDictionary(CFSTR("10")), NULL), 40, 20), "a limit that is not a number is ignored");
    EXPECT(SIZE_IS(roundTrip(image, NULL, limitDictionary(kCFBooleanTrue), NULL), 40, 20), "a boolean is not a limit");

    // Read from the per-image properties, not from the options of the destination.
    EXPECT(SIZE_IS(roundTrip(image, NULL, NULL, intLimit(10)), 40, 20), "the key in the destination's options (not the per-image properties) has no effect");

    // A limit of 1e10 or more, infinity and NaN leave the image alone (macOS 26.7.3 and 10.14.4; 2^31 is the cutoff used here).
    double huge[] = { 1e10, 1e30, INFINITY, NAN };
    for (int i = 0; i < 4; i++) {
        CFNumberRef n = CFNumberCreate(NULL, kCFNumberDoubleType, &huge[i]);
        EXPECT(SIZE_IS(roundTrip(image, NULL, limitDictionary(n), NULL), 40, 20), "a limit of 1e10 or more, infinity or NaN leaves the image unchanged (no undefined cast)");
        CFRelease(n);
    }

    // The image cannot be scaled (the scaled bitmap is far too big to allocate): it is not added at full size and
    // CGImageDestinationFinalize returns false. Run in a child so that a crash is an assertion failure here and not the end
    // of the test.
    {
        fflush(stdout);
        pid_t child = fork();
        if (child == 0) {
            const size_t giant = (size_t)1 << 40;
            CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, NULL, (size_t)1 << 62, NULL);
            CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
            CGImageRef huge = CGImageCreate(giant, (size_t)1 << 20, 8, 32, giant * 4, rgb, kCGImageAlphaPremultipliedLast, provider, NULL, false, kCGRenderingIntentDefault);
            if (huge == NULL) _exit(100);
            int64_t limit = (int64_t)1 << 30; // scaled: 2^30 x 2^10 pixels, 4 TB
            CFNumberRef n = CFNumberCreate(NULL, kCFNumberSInt64Type, &limit);
            CFMutableDataRef out = CFDataCreateMutable(NULL, 0);
            CGImageDestinationRef d = CGImageDestinationCreateWithData(out, CFSTR("public.png"), 1, NULL);
            CGImageDestinationAddImage(d, huge, limitDictionary(n));
            int code = 0;
            if (!CGImageDestinationFinalize(d)) code |= 1;
            if (CFDataGetLength(out) == 0) code |= 2;
            _exit(code);
        }
        int status = 0;
        waitpid(child, &status, 0);
        EXPECT(WIFEXITED(status) && WEXITSTATUS(status) < 100, "a 2^40 x 2^20 image can be described without allocating its pixels, and the scaling then runs to its end (no crash)");
        int code = WIFEXITED(status) ? WEXITSTATUS(status) : 0;
        EXPECT(code & 1, "an image that cannot be scaled fails the destination instead of being added at full size");
        EXPECT(code & 2, "nothing is written for it");
    }

    if (failures == 0) printf("PASS: kCGImageDestinationImageMaxPixelSize limits the added image\n");
    return failures ? 1 : 0;
}
