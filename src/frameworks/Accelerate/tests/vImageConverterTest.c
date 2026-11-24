// Copyright (C) 2026 Lubos Dolezel
//
// Guest test of the vImage buffer utilities, vImageConverter / vImageConvert_AnyToAny between CGImage-style pixel formats, and the
// conversion between vImage_Buffers and CGImages. Build it for the guest with the guest cross-compiler:
//   guest-cc.sh -o vImageConverterTest tests/vImageConverterTest.c -framework Accelerate -framework CoreGraphics -framework CoreFoundation
// The expected values are black-box observations of the original on macOS 10.14.4 (x86_64) and 26.7.3 made with the probes in the
// research notes (mac-probes/vimage-converter/vimage-*.c, results *.10.14.4-x86_64.txt); where the two systems differ Mojave is followed.
// Integer conversions between sRGB-based formats are expected to match exactly (one step for half and float sources); colour-managed
// conversions, gray conversions, floating point results and the 1555/565 formats have the tolerances the original's own rounding
// needs (see tolerance()). Documented differences from the original: CMYK and other colour spaces need ColorSync (the converter
// is not created, kvImageColorSyncIsAbsent); kvImageGetTempBufferSize answers 0.
#include <Accelerate/Accelerate.h>
#include <CoreGraphics/CoreGraphics.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures, checks;
#define EXPECT(cond, ...) do { checks++; if (!(cond)) { failures++; printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define FLOAT_COMPONENTS 0x100u

static vImage_CGImageFormat mk(uint32_t bpc, uint32_t bpp, CGColorSpaceRef cs, CGBitmapInfo bi) {
    vImage_CGImageFormat f;
    memset(&f, 0, sizeof f);
    f.bitsPerComponent = bpc; f.bitsPerPixel = bpp; f.colorSpace = cs; f.bitmapInfo = bi; f.renderingIntent = kCGRenderingIntentDefault;
    return f;
}

// ---- vImageBuffer_Init, GetSize, GetComponentCount, IsEqual

static void testBufferUtilities(void) {
    // rowBytes and the preferred alignment for 8-bit pixels by row length (a row of 16 bytes or less is padded to 16, up to 31 to 32, more to a multiple of 64)
    static const struct { unsigned w; size_t rowBytes; ssize_t alignment; } rows[] = {
        { 1, 16, 16 }, { 10, 16, 16 }, { 16, 16, 16 }, { 17, 32, 16 }, { 31, 32, 16 }, { 32, 64, 64 }, { 33, 64, 64 }, { 64, 64, 64 }, { 65, 128, 64 }, { 128, 128, 64 }, { 129, 192, 64 },
    };
    for (size_t i = 0; i < sizeof rows / sizeof rows[0]; i++) {
        vImage_Buffer b;
        memset(&b, 0xAA, sizeof b);
        ssize_t r = vImageBuffer_Init(&b, 3, rows[i].w, 8, kvImageNoAllocate);
        EXPECT(r == rows[i].alignment && b.rowBytes == rows[i].rowBytes && b.data == NULL && b.height == 3 && b.width == rows[i].w, "Init NoAllocate width %u: %zd rowBytes %zu", rows[i].w, r, b.rowBytes);
        r = vImageBuffer_Init(&b, 3, rows[i].w, 8, kvImageNoFlags);
        EXPECT(r == kvImageNoError && b.rowBytes == rows[i].rowBytes && b.data != NULL && ((uintptr_t)b.data & 15) == 0, "Init allocate width %u: %zd", rows[i].w, r);
        free(b.data);
    }
    static const struct { unsigned long h, w; uint32_t bits; size_t rowBytes; } sizes[] = {
        { 10, 10, 32, 64 }, { 7, 13, 24, 64 }, { 100, 33, 32, 192 }, { 3, 5, 1, 16 }, { 3, 5, 4, 16 }, { 3, 5, 16, 16 }, { 3, 5, 64, 64 }, { 3, 5, 128, 128 }, { 1000, 1000, 32, 4032 }, { 2, 4097, 32, 16448 },
        { 0, 10, 32, 64 }, { 10, 0, 32, 0 }, { 3, 3, 64, 32 },
    };
    for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
        vImage_Buffer b;
        ssize_t r = vImageBuffer_Init(&b, sizes[i].h, sizes[i].w, sizes[i].bits, kvImageNoAllocate);
        EXPECT(r > 0 && b.rowBytes == sizes[i].rowBytes, "Init %lux%lu at %u bits: rowBytes %zu (expected %zu)", sizes[i].h, sizes[i].w, sizes[i].bits, b.rowBytes, sizes[i].rowBytes);
    }
    vImage_Buffer b;
    EXPECT(vImageBuffer_Init(&b, 4, 4, 8, kvImagePrintDiagnosticsToConsole | kvImageNoAllocate) > 0, "Init accepts kvImagePrintDiagnosticsToConsole");
    EXPECT(vImageBuffer_Init(&b, 4, 4, 8, kvImageDoNotTile) == kvImageUnknownFlagsBit, "Init rejects kvImageDoNotTile");
    EXPECT(vImageBuffer_Init(&b, 4, 4, 8, kvImageLeaveAlphaUnchanged) == kvImageUnknownFlagsBit, "Init rejects kvImageLeaveAlphaUnchanged");
    EXPECT(vImageBuffer_Init(&b, 4, 4, 8, 0x10000) == kvImageUnknownFlagsBit, "Init rejects an unknown flag");
    EXPECT(vImageBuffer_Init(&b, 4, 4, 0, kvImageNoAllocate) == 16, "Init with 0 bits per pixel");
    // an allocation of a size that cannot exist fails (the original answers kvImageNoError for 2^40 x 2^40 pixels and no usable memory; not followed)
    EXPECT(vImageBuffer_Init(&b, ~0ul, ~0ul, 128, kvImageNoFlags) == kvImageMemoryAllocationError && b.data == NULL, "Init of an impossible size");

    vImage_Buffer s = { NULL, 123, 456, 0 };
    CGSize z = vImageBuffer_GetSize(&s);
    EXPECT(z.width == 456 && z.height == 123, "GetSize");
#if __LP64__
    s.width = (1ul << 53) + 1; s.height = (1ul << 53) + 3;
    z = vImageBuffer_GetSize(&s);
    EXPECT(z.width == 9007199254740992.0 && z.height == 9007199254740994.0, "GetSize rounds down: %.0f x %.0f", z.width, z.height);
#endif

    CGColorSpaceRef gray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2), cmyk = CGColorSpaceCreateDeviceCMYK(), srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB), dev = CGColorSpaceCreateDeviceRGB();
    struct { const char *n; vImage_CGImageFormat f; uint32_t count; } c[] = {
        { "RGBA8", mk(8, 32, NULL, kCGImageAlphaLast), 4 }, { "RGB888", mk(8, 24, NULL, kCGImageAlphaNone), 3 }, { "Gray8", mk(8, 8, gray, kCGImageAlphaNone), 1 },
        { "GrayAlpha88", mk(8, 16, gray, kCGImageAlphaLast), 2 }, { "CMYK8", mk(8, 32, cmyk, kCGImageAlphaNone), 4 }, { "RGBA16F", mk(16, 64, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder16Little), 4 },
        { "RGB565", mk(5, 16, NULL, kCGImageAlphaNone | kCGBitmapByteOrder16Little), 3 }, { "xRGB8", mk(8, 32, NULL, kCGImageAlphaNoneSkipFirst), 4 }, { "AlphaOnly8", mk(8, 8, NULL, kCGImageAlphaOnly), 3 },
    };
    for (size_t i = 0; i < sizeof c / sizeof c[0]; i++)
        EXPECT(vImageCGImageFormat_GetComponentCount(&c[i].f) == c[i].count, "component count of %s: %u", c[i].n, vImageCGImageFormat_GetComponentCount(&c[i].f));
    vImage_CGImageFormat a = mk(8, 32, NULL, kCGImageAlphaLast), b2 = mk(8, 32, srgb, kCGImageAlphaLast), d = mk(8, 32, dev, kCGImageAlphaLast), e = mk(8, 32, NULL, kCGImageAlphaFirst), g = a;
    g.renderingIntent = kCGRenderingIntentPerceptual;
    EXPECT(vImageCGImageFormat_IsEqual(&a, &a), "IsEqual of a format with itself");
    EXPECT(!vImageCGImageFormat_IsEqual(&a, &b2), "a NULL colour space is not sRGB for IsEqual");
    EXPECT(vImageCGImageFormat_IsEqual(&a, &d), "a NULL colour space is device RGB for IsEqual");
    EXPECT(!vImageCGImageFormat_IsEqual(&a, &e), "different alpha");
    EXPECT(vImageCGImageFormat_IsEqual(&a, &g), "the rendering intent is not compared");
    EXPECT(!vImageCGImageFormat_IsEqual(NULL, &a) && !vImageCGImageFormat_IsEqual(&a, NULL) && !vImageCGImageFormat_IsEqual(NULL, NULL), "a NULL operand is never equal");
    CGColorSpaceRelease(gray); CGColorSpaceRelease(cmyk); CGColorSpaceRelease(srgb); CGColorSpaceRelease(dev);
}

// ---- converter creation, errors and in-place verdicts

static void testConverterCreation(void) {
    CGColorSpaceRef gray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2), p3 = CGColorSpaceCreateWithName(kCGColorSpaceDisplayP3);
    vImage_CGImageFormat rgba8 = mk(8, 32, NULL, kCGImageAlphaLast), rgba8p = mk(8, 32, NULL, kCGImageAlphaPremultipliedLast), rgb888 = mk(8, 24, NULL, kCGImageAlphaNone);
    struct { const char *n; vImage_CGImageFormat s, d; vImage_Flags fl; vImage_Error err; vImage_Error inPlace; } c[] = {
        { "RGBA8->RGBA8P", rgba8, rgba8p, 0, 0, 0 },
        { "RGBA8->RGB888 without a background", rgba8, rgb888, 0, 0, 0 },
        { "RGB888->RGBA8", rgb888, rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "flag DoNotTile", rgba8, rgba8p, kvImageDoNotTile, 0, 0 },
        { "flag Print", rgba8, rgba8p, kvImagePrintDiagnosticsToConsole, 0, 0 },
        { "flag NoAllocate", rgba8, rgba8p, kvImageNoAllocate, kvImageUnknownFlagsBit, 0 },
        { "flag LeaveAlphaUnchanged", rgba8, rgba8p, kvImageLeaveAlphaUnchanged, kvImageUnknownFlagsBit, 0 },
        { "bpc 3", mk(3, 24, NULL, kCGImageAlphaNone), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "bpc 7", mk(7, 28, NULL, kCGImageAlphaNone), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "bpc 12", mk(12, 48, NULL, kCGImageAlphaNone), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "float with 8 bits", mk(8, 32, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "half float", mk(16, 64, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder16Little), rgba8, 0, 0, 0 },
        { "float", mk(32, 128, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder32Little), rgba8, 0, 0, 0 },
        { "32-bit integer", mk(32, 128, NULL, kCGImageAlphaLast | kCGBitmapByteOrder32Little), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "10 bits", mk(10, 32, NULL, kCGImageAlphaNoneSkipLast), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "alpha value 15", mk(8, 32, NULL, 15), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "alpha only source", mk(8, 8, NULL, kCGImageAlphaOnly), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "alpha only destination", rgba8, mk(8, 8, NULL, kCGImageAlphaOnly), 0, kvImageInvalidImageFormat, 0 },
        { "unknown bitmapInfo bit", mk(8, 32, NULL, kCGImageAlphaLast | 0x40000), rgba8, 0, 0, 0 },
        { "bits per pixel not a multiple of 8", mk(8, 20, NULL, kCGImageAlphaNone), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "bits per pixel too small", mk(8, 16, NULL, kCGImageAlphaLast), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "gray->RGBA8", mk(8, 8, gray, kCGImageAlphaNone), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "RGBA8 sRGB->P3", rgba8, mk(8, 32, p3, kCGImageAlphaLast), 0, 0, 0 },
        { "565->RGBA8", mk(5, 16, NULL, kCGImageAlphaNone | kCGBitmapByteOrder16Little), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "565 with the default byte order", mk(5, 16, NULL, kCGImageAlphaNone), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "ARGB1555->RGBA8", mk(5, 16, NULL, kCGImageAlphaFirst | kCGBitmapByteOrder16Little), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "gray 1 bit", mk(1, 1, gray, kCGImageAlphaNone), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "gray 4 bits", mk(4, 4, gray, kCGImageAlphaNone), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "gray 2 bits with alpha", mk(2, 4, gray, kCGImageAlphaLast), rgba8, 0, 0, kvImageOutOfPlaceOperationRequired },
        { "BGRA->RGBA", mk(8, 32, NULL, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little), rgba8, 0, 0, 0 },
        { "byte order 16 on 8-bit 32-bit pixels", mk(8, 32, NULL, kCGImageAlphaLast | kCGBitmapByteOrder16Little), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "byte order 32 on 24-bit pixels", mk(8, 24, NULL, kCGImageAlphaNone | kCGBitmapByteOrder32Little), rgba8, 0, kvImageInvalidImageFormat, 0 },
        { "RGBA8->RGBA16", rgba8, mk(16, 64, NULL, kCGImageAlphaLast), 0, 0, kvImageOutOfPlaceOperationRequired },
        { "RGBA16->RGBA8", mk(16, 64, NULL, kCGImageAlphaLast), rgba8, 0, 0, 0 },
        { "RGBA8->float", rgba8, mk(32, 128, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder32Little), 0, 0, kvImageOutOfPlaceOperationRequired },
        { "RGBA8->Gray8", rgba8, mk(8, 8, gray, kCGImageAlphaNone), 0, 0, 0 },
    };
    for (size_t i = 0; i < sizeof c / sizeof c[0]; i++) {
        vImage_Error err = 12345;
        vImageConverterRef cv = vImageConverter_CreateWithCGImageFormat(&c[i].s, &c[i].d, NULL, c[i].fl, &err);
        EXPECT((cv != NULL) == (c[i].err == 0) && err == c[i].err, "Create %s: %s, error %zd (expected %zd)", c[i].n, cv ? "ok" : "NULL", err, c[i].err);
        if (cv) {
            const vImageBufferTypeCode *so = vImageConverter_GetSourceBufferOrder(cv), *dof = vImageConverter_GetDestinationBufferOrder(cv);
            EXPECT(vImageConverter_GetNumberOfSourceBuffers(cv) == 1 && vImageConverter_GetNumberOfDestinationBuffers(cv) == 1, "%s: one buffer each side", c[i].n);
            EXPECT(so[0] == kvImageBufferTypeCode_CGFormat && so[1] == kvImageBufferTypeCode_EndOfList && dof[0] == kvImageBufferTypeCode_CGFormat && dof[1] == kvImageBufferTypeCode_EndOfList, "%s: buffer order lists", c[i].n);
            EXPECT(vImageConverter_MustOperateOutOfPlace(cv, NULL, NULL, 0) == c[i].inPlace, "%s: in place verdict %zd (expected %zd)", c[i].n, vImageConverter_MustOperateOutOfPlace(cv, NULL, NULL, 0), c[i].inPlace);
            vImage_Buffer sb = { (void *)1, 4, 4, 16 }, db = { (void *)1, 4, 4, 16 };
            EXPECT(vImageConverter_MustOperateOutOfPlace(cv, &sb, &db, 0) == c[i].inPlace, "%s: in place verdict for buffers", c[i].n);
            vImageConverter_Release(cv);
        }
    }
    // reference counting and the other arguments
    vImage_Error err;
    vImageConverterRef cv = vImageConverter_CreateWithCGImageFormat(&rgba8, &rgba8p, NULL, 0, NULL);
    EXPECT(cv != NULL, "Create with no error pointer");
    vImage_Buffer one = { (void *)1, 1, 1, 4 };
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, &one, NULL, 0) == kvImageNullPointerArgument, "srcs without dests");
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, NULL, &one, 0) == kvImageNullPointerArgument, "dests without srcs");
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, NULL, NULL, 0x1000) == kvImageUnknownFlagsBit, "unknown flag");
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, NULL, NULL, kvImagePrintDiagnosticsToConsole) == 0 && vImageConverter_MustOperateOutOfPlace(cv, NULL, NULL, kvImageDoNotTile) == 0, "known flags");
    EXPECT(vImageConverter_MustOperateOutOfPlace(NULL, NULL, NULL, 0) == kvImageNullPointerArgument, "NULL converter");
    EXPECT(vImageConverter_GetNumberOfSourceBuffers(NULL) == 0, "no buffers for a NULL converter");
    vImage_Buffer a = { (void *)16, 4, 4, 16 }, b = { (void *)16, 4, 4, 16 }, b2 = { (void *)16, 4, 4, 32 }, b3 = { (void *)16, 8, 8, 32 };
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, &a, &b, 0) == 0, "same rowBytes");
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, &a, &b2, 0) == kvImageOutOfPlaceOperationRequired, "different rowBytes");
    EXPECT(vImageConverter_MustOperateOutOfPlace(cv, &a, &b3, 0) == kvImageOutOfPlaceOperationRequired, "different size");
    vImageConverter_Retain(cv);
    vImageConverter_Release(cv);
    vImageConverter_Release(cv);
    vImageConverter_Release(NULL);
    // the ColorSync entry points cannot work without ColorSync
    err = 0;
    EXPECT(vImageConverter_CreateWithColorSyncCodeFragment((CFTypeRef)gray, &rgba8, &rgba8p, NULL, 0, &err) == NULL && err == kvImageColorSyncIsAbsent, "ColorSync code fragment: %zd", err);
    // CMYK needs a colour management module
    CGColorSpaceRef cmyk = CGColorSpaceCreateDeviceCMYK();
    vImage_CGImageFormat fc = mk(8, 32, cmyk, kCGImageAlphaNone);
    err = 0;
    EXPECT(vImageConverter_CreateWithCGImageFormat(&fc, &rgba8, NULL, 0, &err) == NULL && err == kvImageColorSyncIsAbsent, "CMYK needs ColorSync: %zd", err);
    CGColorSpaceRelease(cmyk);
    CGColorSpaceRelease(gray); CGColorSpaceRelease(p3);
}

// ---- vImageConvert_AnyToAny: errors, sizes and flags

static void testAnyToAnyErrors(void) {
    vImage_CGImageFormat rgba8 = mk(8, 32, NULL, kCGImageAlphaLast), rgba8p = mk(8, 32, NULL, kCGImageAlphaPremultipliedLast), rgb888 = mk(8, 24, NULL, kCGImageAlphaNone);
    CGFloat bg[4] = { 0, 0, 0, 0 };
    vImage_Error err;
    vImageConverterRef c88p = vImageConverter_CreateWithCGImageFormat(&rgba8, &rgba8p, bg, 0, &err), c24 = vImageConverter_CreateWithCGImageFormat(&rgb888, &rgba8, bg, 0, &err);
    uint8_t s[64 * 8], d[64 * 8];
    memset(s, 0x40, sizeof s);
    memset(d, 0xEE, sizeof d);
    vImage_Buffer sb = { s, 4, 4, 16 }, db = { d, 4, 4, 16 };
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, 0) == 0 && d[0] == 0x10 && d[1] == 0x10 && d[2] == 0x10 && d[3] == 0x40 && d[16] == 0x10, "converts: %02x %02x %02x %02x", d[0], d[1], d[2], d[3]);
    EXPECT(vImageConvert_AnyToAny(NULL, &sb, &db, NULL, 0) == kvImageNullPointerArgument, "NULL converter");
    vImage_Buffer nb = { NULL, 4, 4, 16 };
    EXPECT(vImageConvert_AnyToAny(c88p, &nb, &db, NULL, 0) == kvImageInvalidParameter, "source data NULL");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &nb, NULL, 0) == kvImageInvalidParameter, "destination data NULL");
    vImage_Buffer small = { s, 3, 4, 16 };
    EXPECT(vImageConvert_AnyToAny(c88p, &small, &db, NULL, 0) == kvImageRoiLargerThanInputBuffer, "source with fewer rows");
    small = (vImage_Buffer){ s, 4, 3, 16 };
    EXPECT(vImageConvert_AnyToAny(c88p, &small, &db, NULL, 0) == kvImageRoiLargerThanInputBuffer, "source with fewer columns");
    {   // a source larger than the destination converts the part the destination covers
        vImage_Buffer big = { s, 8, 8, 32 }, dd = { d, 2, 2, 8 };
        memset(d, 0xEE, sizeof d);
        EXPECT(vImageConvert_AnyToAny(c88p, &big, &dd, NULL, 0) == 0 && d[0] == 0x10 && d[7] == 0x40 && d[15] == 0x40 && d[16] == 0xEE, "source larger than destination");
    }
    vImage_Buffer z = { s, 0, 0, 0 }, dz = { d, 0, 0, 0 };
    EXPECT(vImageConvert_AnyToAny(c88p, &z, &dz, NULL, 0) == 0, "empty images");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImageGetTempBufferSize) >= 0, "temp buffer size is not an error");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImageDoNotTile) == 0 && vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImagePrintDiagnosticsToConsole) == 0, "known flags");
    uint8_t temp[4096];
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, temp, 0) == 0, "a temp buffer may be passed");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImageNoAllocate) == kvImageUnknownFlagsBit, "kvImageNoAllocate");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImageLeaveAlphaUnchanged) == kvImageUnknownFlagsBit, "kvImageLeaveAlphaUnchanged");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, kvImageHDRContent) == kvImageUnknownFlagsBit, "kvImageHDRContent");
    EXPECT(vImageConvert_AnyToAny(c88p, &sb, &db, NULL, 0x10000) == kvImageUnknownFlagsBit, "an unknown flag");
    {   // in place
        memset(s, 0x40, sizeof s);
        vImage_Buffer in = { s, 4, 4, 16 };
        EXPECT(vImageConvert_AnyToAny(c88p, &in, &in, NULL, 0) == 0 && s[0] == 0x10 && s[3] == 0x40 && s[16] == 0x10, "in place RGBA8 -> RGBA8P: %02x", s[0]);
    }
    vImageConverter_Release(c88p);
    vImageConverter_Release(c24);
}

// ---- vImageConvert_AnyToAny: pixel values

typedef struct { const char *name; vImage_CGImageFormat f; } Fmt;
static const float base[8][4] = { {0,0,0,0}, {1,1,1,1}, {1,0,0,1}, {0.0392f,0.0784f,0.1176f,0.5019f}, {0.7843f,0.3922f,0.1961f,0.2510f}, {0.0039f,0.0078f,0.0118f,0.0039f}, {0.1234f,0.5678f,0.9012f,0.3456f}, {1,0.5020f,0,0.7843f} };

static uint16_t toHalf(float v) {
    union { float f; uint32_t u; } x = { v };
    uint32_t sign = (x.u >> 16) & 0x8000, e = (x.u >> 23) & 0xff, m = x.u & 0x7fffff;
    int exp = (int)e - 127 + 15;
    if (exp <= 0) return (uint16_t)sign;
    if (exp >= 31) return (uint16_t)(sign | 0x7c00);
    uint32_t h = sign | (exp << 10) | (m >> 13);
    if ((m & 0x1fff) > 0x1000 || ((m & 0x1fff) == 0x1000 && (h & 1))) h++;
    return (uint16_t)h;
}
static float fromHalf(uint16_t h) {
    uint32_t sign = (uint32_t)(h & 0x8000) << 16, exp = (h >> 10) & 0x1f, man = h & 0x3ff, u;
    if (exp == 0) { float f = ldexpf((float)man, -24); return sign ? -f : f; }
    if (exp == 31) u = sign | 0x7f800000 | (man << 13);
    else u = sign | ((exp - 15 + 127) << 23) | (man << 13);
    float f; memcpy(&f, &u, 4); return f;
}

// The reference pixels in a format (the colour values are written as they are, in that format's colour space, without conversion).
static size_t pack(const vImage_CGImageFormat *f, const float *rgba, uint8_t *out) {
    CGBitmapInfo bi = f->bitmapInfo;
    int alphaInfo = bi & kCGBitmapAlphaInfoMask, order = bi & kCGBitmapByteOrderMask, isFloat = (bi & 0xF00) != 0;
    int gray = f->colorSpace && CGColorSpaceGetNumberOfComponents(f->colorSpace) == 1;
    float a = rgba[3];
    int hasAlpha = alphaInfo == kCGImageAlphaPremultipliedLast || alphaInfo == kCGImageAlphaPremultipliedFirst || alphaInfo == kCGImageAlphaLast || alphaInfo == kCGImageAlphaFirst;
    int premult = alphaInfo == kCGImageAlphaPremultipliedFirst || alphaInfo == kCGImageAlphaPremultipliedLast;
    int first = alphaInfo == kCGImageAlphaFirst || alphaInfo == kCGImageAlphaPremultipliedFirst || alphaInfo == kCGImageAlphaNoneSkipFirst;
    int skip = alphaInfo == kCGImageAlphaNoneSkipFirst || alphaInfo == kCGImageAlphaNoneSkipLast;
    float v[3]; int nc = gray ? 1 : 3;
    for (int i = 0; i < nc; i++) v[i] = rgba[i] * (premult ? a : 1.0f);
    float chans[4]; int total = 0;
    if ((hasAlpha || skip) && first) chans[total++] = skip ? 1.0f : a;
    for (int i = 0; i < nc; i++) chans[total++] = v[i];
    if ((hasAlpha || skip) && !first) chans[total++] = skip ? 1.0f : a;
    uint32_t bpc = f->bitsPerComponent;
    if (bpc == 5) {
        uint16_t w;
        if (hasAlpha) w = (uint16_t)(((a >= 0.5f ? 1 : 0) << 15) | ((int)lroundf(v[0] * 31) << 10) | ((int)lroundf(v[1] * 31) << 5) | (int)lroundf(v[2] * 31));
        else w = (uint16_t)(((int)lroundf(v[0] * 31) << 11) | ((int)lroundf(v[1] * 63) << 5) | (int)lroundf(v[2] * 31));
        out[0] = (uint8_t)w; out[1] = w >> 8;
        return 2;
    }
    if (bpc == 1 || bpc == 4) { out[0] = (uint8_t)(lroundf(chans[0] * ((1 << bpc) - 1)) << (8 - bpc)); return 1; }
    size_t pos = 0;
    for (int i = 0; i < total; i++) {
        if (bpc == 8) out[pos++] = (uint8_t)lroundf(chans[i] * 255);
        else if (bpc == 16) {
            uint16_t w = isFloat ? toHalf(chans[i]) : (uint16_t)lroundf(chans[i] * 65535);
            if (order == kCGBitmapByteOrder16Little) { out[pos++] = (uint8_t)w; out[pos++] = w >> 8; } else { out[pos++] = w >> 8; out[pos++] = (uint8_t)w; }
        } else {
            union { float f; uint32_t u; } x = { chans[i] };
            out[pos++] = (uint8_t)x.u; out[pos++] = x.u >> 8; out[pos++] = x.u >> 16; out[pos++] = x.u >> 24;
        }
    }
    if (bpc == 8 && order == kCGBitmapByteOrder32Little && pos == 4) { uint8_t t0 = out[0], t1 = out[1]; out[0] = out[3]; out[3] = t0; out[1] = out[2]; out[2] = t1; }
    return pos;
}

static int sameSpace(const Fmt *a, const Fmt *b) {
    if (a->f.colorSpace == b->f.colorSpace)
        return 1;
    return a->f.colorSpace && b->f.colorSpace && CFEqual(a->f.colorSpace, b->f.colorSpace);
}

// How far a result may be from the original's: exact for integer formats of one colour space, one step for 8-bit results otherwise, a few
// steps where the original's colour management differs in the last bits, and relative for floating point.
// Whether a format's colour space is one whose conversions the original does through ColorSync profiles that differ from the plain
// matrices and curves in the darkest values and for colours outside the other space's gamut (wide gamut and the generic profiles).
static int profiled(const Fmt *f) {
    if (!f->f.colorSpace)
        return 0;
    CFStringRef n = CGColorSpaceCopyName(f->f.colorSpace);
    int r = n && (CFEqual(n, kCGColorSpaceDisplayP3) || CFEqual(n, kCGColorSpaceAdobeRGB1998) || CFEqual(n, kCGColorSpaceGenericRGB) || CFEqual(n, kCGColorSpaceGenericGray));
    if (n) CFRelease(n);
    return r;
}

static int closeEnough(const Fmt *s, const Fmt *d, int pixel, const uint8_t *got, const uint8_t *want, size_t bytesPerPixel) {
    uint32_t sAlpha = s->f.bitmapInfo & kCGBitmapAlphaInfoMask;
    int sPremult = sAlpha == kCGImageAlphaPremultipliedFirst || sAlpha == kCGImageAlphaPremultipliedLast;
    int sFloat = (s->f.bitmapInfo & 0xF00) != 0, dFloat = (d->f.bitmapInfo & 0xF00) != 0;
    if (pixel == 5 && sPremult && sFloat)
        return 1;   // alpha of 1/255 in a premultiplied float source: the original's result for it is coarse
    int colourManaged = !sameSpace(s, d);
    if (colourManaged && (profiled(s) || profiled(d)) && pixel != 0 && pixel != 1 && pixel != 4)
        return 1;   // dark or out of gamut colours: only black, white and one mid colour are compared
    uint32_t bpc = d->f.bitsPerComponent;
    if (bpc == 1)
        return 1;
    if (bpc == 4) {
        for (size_t i = 0; i < bytesPerPixel; i++) if (abs((int)got[i] - (int)want[i]) > 16) return 0;
        return 1;
    }
    if (bpc == 5) {
        uint32_t a = got[0] | (got[1] << 8), b = want[0] | (want[1] << 8);
        int hasAlpha = (d->f.bitmapInfo & kCGBitmapAlphaInfoMask) != kCGImageAlphaNone;
        int shift[3] = { hasAlpha ? 10 : 11, 5, 0 }, mask[3] = { 31, hasAlpha ? 31 : 63, 31 };
        for (int k = 0; k < 3; k++) if (abs((int)((a >> shift[k]) & mask[k]) - (int)((b >> shift[k]) & mask[k])) > 1) return 0;
        return 1;
    }
    if (dFloat) {
        size_t step = bpc / 8;
        for (size_t i = 0; i + step <= bytesPerPixel; i += step) {
            float x, y;
            if (step == 4) { memcpy(&x, got + i, 4); memcpy(&y, want + i, 4); }
            else { x = fromHalf((uint16_t)(got[i] | (got[i + 1] << 8))); y = fromHalf((uint16_t)(want[i] | (want[i + 1] << 8))); }
            if (fabsf(x - y) > 0.004f + 0.01f * fabsf(y)) return 0;
        }
        return 1;
    }
    int tol;
    if (bpc == 16) {
        tol = colourManaged ? 80 : 8;
        int little = (d->f.bitmapInfo & kCGBitmapByteOrderMask) == kCGBitmapByteOrder16Little;
        for (size_t i = 0; i + 2 <= bytesPerPixel; i += 2) {
            int x = little ? (got[i] | (got[i + 1] << 8)) : ((got[i] << 8) | got[i + 1]), y = little ? (want[i] | (want[i + 1] << 8)) : ((want[i] << 8) | want[i + 1]);
            if (abs(x - y) > tol) return 0;
        }
        return 1;
    }
    tol = colourManaged || (s->f.bitsPerComponent == 5) ? 8 : 1;
    if (sFloat && sPremult)
        tol = tol < 3 ? 3 : tol;   // colour divided by a small alpha
    else if (sFloat || sPremult || s->f.bitsPerComponent != 8)
        tol = tol < 1 ? 1 : tol;
    for (size_t i = 0; i < bytesPerPixel; i++) if (abs((int)got[i] - (int)want[i]) > tol) return 0;
    return 1;
}

static int hexByte(const char **p) {
    while (**p == ' ') (*p)++;
    char *end;
    long v = strtol(*p, &end, 16);
    if (end == *p) return -1;
    *p = end;
    return (int)v;
}

static const struct { const char *src, *dst, *bytes; } conversions[] = {
    { "RGBA8", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA8", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA8", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "RGBA8", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA8", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBA8", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBA8", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA8", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBA8", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBA8", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "RGBA8", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "RGBA8", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "RGBA8", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 29 04 2d 88 2f 04 38 46 3a 46 36 46 32 04 34 00 1c 00 20 00 22 00 1c c8 2f 8c 38 37 3b 86 35 00 3c 04 38 00 00 46 3a" },
    { "RGBA8", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 81 80 00 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 81 80 80 3e 81 80 80 3b 81 80 00 3c c2 c0 40 3c 81 80 80 3b fa f8 f8 3d 92 91 11 3f e8 e6 66 3f b1 b0 b0 3e 00 00 80 3f 81 80 00 3f 00 00 00 00 ca c8 48 3f" },
    { "RGBA8", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f e4 41 a1 3c e4 41 21 3d d6 e2 71 3d 81 80 00 3f 5d 92 49 3e 5d 92 c9 3d 5d 92 49 3d 81 80 80 3e 83 01 81 37 83 01 01 38 45 82 41 38 81 80 80 3b 03 d7 2b 3d 1a f1 48 3e 1e 5e 9f 3e b1 b0 b0 3e ca c8 48 3f 5d 92 c9 3e 00 00 00 00 ca c8 48 3f" },
    { "RGBA8", "RGB565", "f7 3b ff ff 00 f8 6e 2a b3 5b 17 44 39 34 05 d4" },
    { "RGBA8", "ARGB1555", "00 00 ff ff 00 fc 43 04 86 65 00 00 5c 12 00 fe" },
    { "RGBA8", "Gray1", "00 80 80 00 00 00 80 00" },
    { "RGBA8", "Gray4", "40 f0 80 20 50 40 50 80" },
    { "RGBA8", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 80 26 26 3e 18 39 a1 3e 80 06 7f 3e e0 6f b3 3e 39 dd 0f 3f" },
    { "RGBA8", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBA8", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 20 08 40 00 00 00 01 03 48 ca 58 ff 37 00 c8" },
    { "RGBA8", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 01 02 04 01 56 90 e2 58 e3 7f 1e c8" },
    { "RGBA8", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1c 7c e0 58 fc 6a 08 c8" },
    { "RGBA8", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBA8P", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c7 64 34 40 00 00 00 01 20 91 e5 58 ff 80 00 c8" },
    { "RGBA8P", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA8P", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c7 64 34 01 00 00 00 58 20 91 e5 c8 ff 80 00" },
    { "RGBA8P", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA8P", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 7f be 35 86 cc d6 80 29" },
    { "RGBA8P", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 7f be ff 35 86 cc ff d6 80 29" },
    { "RGBA8P", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 7f be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA8P", "Gray8", "40 ff 82 2a 50 40 5a 8f" },
    { "RGBA8P", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 81 40 00 01 8a 58 a5 c8" },
    { "RGBA8P", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 00 14 00 1e 00 80 80 c7 ff 64 00 34 00 40 40 00 00 00 00 00 00 01 01 20 00 91 74 e5 d1 58 58 ff ff 80 00 00 00 c8 c8" },
    { "RGBA8P", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 00 0a 00 14 00 1e 80 80 ff c7 00 64 00 34 40 40 00 00 00 00 00 00 01 01 00 20 74 91 d1 e5 58 58 ff ff 00 80 00 00 c8 c8" },
    { "RGBA8P", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 00 14 00 1e 00 80 80 c7 ff 64 00 34 00 40 40 00 00 00 00 00 00 01 01 20 00 91 74 e5 d1 58 58 ff ff 80 00 00 00 c8 c8" },
    { "RGBA8P", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c f8 28 04 2d 80 2f 04 38 40 3a 42 36 82 32 04 34 00 00 00 00 00 00 00 1c 02 30 8b 38 2e 3b 86 35 00 3c ff 37 00 00 46 3a" },
    { "RGBA8P", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 20 3d 00 00 a0 3d 00 00 f0 3d 81 80 00 3f 00 00 48 3f 00 00 c8 3e 00 00 50 3e 81 80 80 3e 00 00 00 00 00 00 00 00 00 00 00 00 81 80 80 3b 00 00 00 3e 5e 74 11 3f 75 d1 65 3f b1 b0 b0 3e 00 00 80 3f 00 00 00 3f 00 00 00 00 ca c8 48 3f" },
    { "RGBA8P", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 a0 3c a1 a0 20 3d f2 f0 70 3d 81 80 00 3f ca c8 48 3e ca c8 c8 3d d2 d0 50 3d 81 80 80 3e 00 00 00 00 00 00 00 00 00 00 00 00 81 80 80 3b b1 b0 30 3d ca c8 48 3e 9f 9e 9e 3e b1 b0 b0 3e ca c8 48 3f ca c8 c8 3e 00 00 00 00 ca c8 48 3f" },
    { "RGBA8P", "RGB565", "18 44 ff ff 00 f8 4d 22 d3 63 f7 43 39 34 e5 d3" },
    { "RGBA8P", "ARGB1555", "00 00 ff ff 00 fc 44 04 86 61 00 80 3c 12 00 fe" },
    { "RGBA8P", "Gray1", "00 80 80 00 00 00 80 00" },
    { "RGBA8P", "Gray4", "40 f0 80 30 40 40 50 80" },
    { "RGBA8P", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 80 26 26 3e e0 00 a1 3e 00 00 7f 3e d4 64 b3 3e 39 dd 0f 3f" },
    { "RGBA8P", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 ba 69 3f 40 00 00 00 01 47 8f df 58 ef 87 33 c8" },
    { "RGBA8P", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 92 20 09 40 00 00 00 01 04 48 c8 58 ff 37 00 c8" },
    { "RGBA8P", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b0 64 3a 40 00 00 00 01 56 90 e2 58 e3 7f 1e c8" },
    { "RGBA8P", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 b9 4f 28 40 00 00 00 01 1d 7c de 58 fc 6a 08 c8" },
    { "RGBA8P", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "ARGB8", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "ARGB8", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "ARGB8", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "ARGB8", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "ARGB8", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "ARGB8", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "ARGB8", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "ARGB8", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "ARGB8", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "ARGB8", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "ARGB8", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "ARGB8", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e 80 80 c8 c8 64 64 32 32 40 40 01 01 02 02 03 03 01 01 1f 1f 91 91 e6 e6 58 58 ff ff 80 80 00 00 c8 c8" },
    { "ARGB8", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 29 04 2d 88 2f 04 38 46 3a 46 36 46 32 04 34 00 1c 00 20 00 22 00 1c c8 2f 8c 38 37 3b 86 35 00 3c 04 38 00 00 46 3a" },
    { "ARGB8", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 81 80 00 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 81 80 80 3e 81 80 80 3b 81 80 00 3c c2 c0 40 3c 81 80 80 3b fa f8 f8 3d 92 91 11 3f e8 e6 66 3f b1 b0 b0 3e 00 00 80 3f 81 80 00 3f 00 00 00 00 ca c8 48 3f" },
    { "ARGB8", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f e4 41 a1 3c e4 41 21 3d d6 e2 71 3d 81 80 00 3f 5d 92 49 3e 5d 92 c9 3d 5d 92 49 3d 81 80 80 3e 83 01 81 37 83 01 01 38 45 82 41 38 81 80 80 3b 03 d7 2b 3d 1a f1 48 3e 1e 5e 9f 3e b1 b0 b0 3e ca c8 48 3f 5d 92 c9 3e 00 00 00 00 ca c8 48 3f" },
    { "ARGB8", "RGB565", "18 44 ff ff 00 f8 4e 2a b3 63 17 44 39 3c e5 d3" },
    { "ARGB8", "ARGB1555", "00 00 ff ff 00 fc 43 04 86 e5 00 00 5c 12 00 fe" },
    { "ARGB8", "Gray1", "00 80 00 00 00 00 00 80" },
    { "ARGB8", "Gray4", "40 f0 80 20 50 40 50 90" },
    { "ARGB8", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 80 26 26 3e 18 39 a1 3e 80 06 7f 3e e0 6f b3 3e 39 dd 0f 3f" },
    { "ARGB8", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "ARGB8", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 20 08 40 00 00 00 01 03 48 ca 58 ff 37 00 c8" },
    { "ARGB8", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 01 02 04 01 56 90 e2 58 e3 7f 1e c8" },
    { "ARGB8", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1c 7c e0 58 fc 6a 08 c8" },
    { "ARGB8", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "BGRA8P", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c7 64 34 40 00 00 00 01 20 91 e5 58 ff 80 00 c8" },
    { "BGRA8P", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "BGRA8P", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c7 64 34 01 00 00 00 58 20 91 e5 c8 ff 80 00" },
    { "BGRA8P", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "BGRA8P", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 7f be 35 86 cc d6 80 29" },
    { "BGRA8P", "Gray8", "40 ff 82 2a 50 40 5a 8f" },
    { "BGRA8P", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 81 40 00 01 8a 58 a5 c8" },
    { "BGRA8P", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 00 14 00 1e 00 80 80 c7 ff 64 00 34 00 40 40 00 00 00 00 00 00 01 01 20 00 91 74 e5 d1 58 58 ff ff 80 00 00 00 c8 c8" },
    { "BGRA8P", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 00 0a 00 14 00 1e 80 80 ff c7 00 64 00 34 40 40 00 00 00 00 00 00 01 01 00 20 74 91 d1 e5 58 58 ff ff 00 80 00 00 c8 c8" },
    { "BGRA8P", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 00 14 00 1e 00 80 80 c7 ff 64 00 34 00 40 40 00 00 00 00 00 00 01 01 20 00 91 74 e5 d1 58 58 ff ff 80 00 00 00 c8 c8" },
    { "BGRA8P", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c f8 28 04 2d 80 2f 04 38 40 3a 42 36 82 32 04 34 00 00 00 00 00 00 00 1c 02 30 8b 38 2e 3b 86 35 00 3c ff 37 00 00 46 3a" },
    { "BGRA8P", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 20 3d 00 00 a0 3d 00 00 f0 3d 81 80 00 3f 00 00 48 3f 00 00 c8 3e 00 00 50 3e 81 80 80 3e 00 00 00 00 00 00 00 00 00 00 00 00 81 80 80 3b 00 00 00 3e 5e 74 11 3f 75 d1 65 3f b1 b0 b0 3e 00 00 80 3f 00 00 00 3f 00 00 00 00 ca c8 48 3f" },
    { "BGRA8P", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 a0 3c a1 a0 20 3d f2 f0 70 3d 81 80 00 3f ca c8 48 3e ca c8 c8 3d d2 d0 50 3d 81 80 80 3e 00 00 00 00 00 00 00 00 00 00 00 00 81 80 80 3b b1 b0 30 3d ca c8 48 3e 9f 9e 9e 3e b1 b0 b0 3e ca c8 48 3f ca c8 c8 3e 00 00 00 00 ca c8 48 3f" },
    { "BGRA8P", "ARGB1555", "00 00 ff ff 00 fc 44 04 87 61 00 00 3c 12 00 fe" },
    { "BGRA8P", "Gray1", "00 80 80 00 80 00 00 00" },
    { "BGRA8P", "Gray4", "40 f0 80 30 50 40 50 80" },
    { "BGRA8P", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 80 26 26 3e e0 00 a1 3e 00 00 7f 3e d4 64 b3 3e 39 dd 0f 3f" },
    { "BGRA8P", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 ba 69 3f 40 00 00 00 01 47 8f df 58 ef 87 33 c8" },
    { "BGRA8P", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 92 20 09 40 00 00 00 01 04 48 c8 58 ff 37 00 c8" },
    { "BGRA8P", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b0 64 3a 40 00 00 00 01 56 90 e2 58 e3 7f 1e c8" },
    { "BGRA8P", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 b9 4f 28 40 00 00 00 01 1d 7c de 58 fc 6a 08 c8" },
    { "BGRA8P", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGB888", "RGBA8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGB888", "RGBA8P", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGB888", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "RGB888", "BGRA8P", "00 00 00 ff ff ff ff ff 00 00 ff ff 1e 14 0a ff 32 64 c8 ff 03 02 01 ff e6 91 1f ff 00 80 ff ff" },
    { "RGB888", "RGB888", "00 00 00 ff ff ff ff 00 00 0a 14 1e c8 64 32 01 02 03 1f 91 e6 ff 80 00" },
    { "RGB888", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "RGB888", "RGBx8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGB888", "Gray8", "00 ff 82 13 82 02 8a a5" },
    { "RGB888", "GrayAlpha88", "00 ff ff ff 82 ff 13 ff 82 ff 02 ff 8a ff a5 ff" },
    { "RGB888", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGB888", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGB888", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGB888", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 29 04 2d 88 2f 00 3c 46 3a 46 36 46 32 00 3c 00 1c 00 20 00 22 00 3c c8 2f 8c 38 37 3b 00 3c 00 3c 04 38 00 00 00 3c" },
    { "RGB888", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "RGB888", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "RGB888", "RGB565", "00 00 ff ff 00 f8 a4 08 26 cb 21 00 9c 24 00 fc" },
    { "RGB888", "ARGB1555", "00 80 ff ff 00 fc 64 84 86 e1 00 80 5c 92 00 fe" },
    { "RGB888", "Gray1", "00 80 00 00 00 00 80 00" },
    { "RGB888", "Gray4", "00 f0 80 10 80 00 80 a0" },
    { "RGB888", "GrayF32", "00 00 00 00 00 00 80 3f 00 50 02 3f 00 00 9a 3d 00 30 02 3f 00 00 d0 3b 00 80 0a 3f 00 d0 25 3f" },
    { "RGB888", "RGBA8-P3", "00 00 00 ff ff ff ff ff ea 33 23 ff 0c 14 1d ff bb 69 3e ff 01 02 03 ff 46 8f df ff ef 87 33 ff" },
    { "RGB888", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff 00 00 ff 01 02 03 ff 93 20 08 ff 00 00 00 ff 03 48 ca ff ff 37 00 ff" },
    { "RGB888", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff db 00 00 ff 11 1b 23 ff b1 64 38 ff 01 02 04 ff 56 90 e2 ff e3 7f 1e ff" },
    { "RGB888", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff fb 00 07 ff 0a 10 17 ff ba 4f 26 ff 01 02 04 ff 1c 7c e0 ff fc 6a 08 ff" },
    { "RGB888", "Gray8-Generic18", "00 ff 6f 10 6f 02 77 94" },
    { "xRGB8", "RGBA8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "xRGB8", "RGBA8P", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "xRGB8", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "xRGB8", "BGRA8P", "00 00 00 ff ff ff ff ff 00 00 ff ff 1e 14 0a ff 32 64 c8 ff 03 02 01 ff e6 91 1f ff 00 80 ff ff" },
    { "xRGB8", "RGB888", "00 00 00 ff ff ff ff 00 00 0a 14 1e c8 64 32 01 02 03 1f 91 e6 ff 80 00" },
    { "xRGB8", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "xRGB8", "RGBx8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "xRGB8", "Gray8", "00 ff 82 13 82 02 8a a5" },
    { "xRGB8", "GrayAlpha88", "00 ff ff ff 82 ff 13 ff 82 ff 02 ff 8a ff a5 ff" },
    { "xRGB8", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "xRGB8", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "xRGB8", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "xRGB8", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 29 04 2d 88 2f 00 3c 46 3a 46 36 46 32 00 3c 00 1c 00 20 00 22 00 3c c8 2f 8c 38 37 3b 00 3c 00 3c 04 38 00 00 00 3c" },
    { "xRGB8", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "xRGB8", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "xRGB8", "RGB565", "00 00 ff ff 00 f8 a4 08 26 cb 00 00 9c 24 e0 fb" },
    { "xRGB8", "ARGB1555", "00 80 ff ff 00 fc 43 84 a7 e5 00 80 3c 92 e0 fd" },
    { "xRGB8", "Gray1", "00 80 80 00 80 00 80 80" },
    { "xRGB8", "Gray4", "00 f0 80 10 80 00 80 a0" },
    { "xRGB8", "GrayF32", "00 00 00 00 00 00 80 3f 00 50 02 3f 00 00 9a 3d 00 30 02 3f 00 00 d0 3b 00 80 0a 3f 00 d0 25 3f" },
    { "xRGB8", "RGBA8-P3", "00 00 00 ff ff ff ff ff ea 33 23 ff 0c 14 1d ff bb 69 3e ff 01 02 03 ff 46 8f df ff ef 87 33 ff" },
    { "xRGB8", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff 00 00 ff 01 02 03 ff 93 20 08 ff 00 00 00 ff 03 48 ca ff ff 37 00 ff" },
    { "xRGB8", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff db 00 00 ff 11 1b 23 ff b1 64 38 ff 01 02 04 ff 56 90 e2 ff e3 7f 1e ff" },
    { "xRGB8", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff fb 00 07 ff 0a 10 17 ff ba 4f 26 ff 01 02 04 ff 1c 7c e0 ff fc 6a 08 ff" },
    { "xRGB8", "Gray8-Generic18", "00 ff 6f 10 6f 02 77 94" },
    { "RGBx8", "RGBA8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGBx8", "RGBA8P", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGBx8", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "RGBx8", "BGRA8P", "00 00 00 ff ff ff ff ff 00 00 ff ff 1e 14 0a ff 32 64 c8 ff 03 02 01 ff e6 91 1f ff 00 80 ff ff" },
    { "RGBx8", "RGB888", "00 00 00 ff ff ff ff 00 00 0a 14 1e c8 64 32 01 02 03 1f 91 e6 ff 80 00" },
    { "RGBx8", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00" },
    { "RGBx8", "RGBx8", "00 00 00 ff ff ff ff ff ff 00 00 ff 0a 14 1e ff c8 64 32 ff 01 02 03 ff 1f 91 e6 ff ff 80 00 ff" },
    { "RGBx8", "Gray8", "00 ff 82 13 82 02 8a a5" },
    { "RGBx8", "GrayAlpha88", "00 ff ff ff 82 ff 13 ff 82 ff 02 ff 8a ff a5 ff" },
    { "RGBx8", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGBx8", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGBx8", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1e ff ff c8 c8 64 64 32 32 ff ff 01 01 02 02 03 03 ff ff 1f 1f 91 91 e6 e6 ff ff ff ff 80 80 00 00 ff ff" },
    { "RGBx8", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 29 04 2d 88 2f 00 3c 46 3a 46 36 46 32 00 3c 00 1c 00 20 00 22 00 3c c8 2f 8c 38 37 3b 00 3c 00 3c 04 38 00 00 00 3c" },
    { "RGBx8", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "RGBx8", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 a0 20 3d a1 a0 a0 3d f2 f0 f0 3d 00 00 80 3f ca c8 48 3f ca c8 c8 3e ca c8 48 3e 00 00 80 3f 81 80 80 3b 81 80 00 3c c2 c0 40 3c 00 00 80 3f fa f8 f8 3d 92 91 11 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 81 80 00 3f 00 00 00 00 00 00 80 3f" },
    { "RGBx8", "RGB565", "00 00 ff ff 00 f8 a4 08 26 cb 00 00 9c 24 e0 fb" },
    { "RGBx8", "ARGB1555", "00 80 ff ff 00 fc 44 84 86 e1 21 84 3c 8e 00 fe" },
    { "RGBx8", "Gray1", "00 80 00 00 80 00 80 00" },
    { "RGBx8", "Gray4", "00 f0 80 10 80 00 80 a0" },
    { "RGBx8", "GrayF32", "00 00 00 00 00 00 80 3f 00 50 02 3f 00 00 9a 3d 00 30 02 3f 00 00 d0 3b 00 80 0a 3f 00 d0 25 3f" },
    { "RGBx8", "RGBA8-P3", "00 00 00 ff ff ff ff ff ea 33 23 ff 0c 14 1d ff bb 69 3e ff 01 02 03 ff 46 8f df ff ef 87 33 ff" },
    { "RGBx8", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff 00 00 ff 01 02 03 ff 93 20 08 ff 00 00 00 ff 03 48 ca ff ff 37 00 ff" },
    { "RGBx8", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff db 00 00 ff 11 1b 23 ff b1 64 38 ff 01 02 04 ff 56 90 e2 ff e3 7f 1e ff" },
    { "RGBx8", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff fb 00 07 ff 0a 10 17 ff ba 4f 26 ff 01 02 04 ff 1c 7c e0 ff fc 6a 08 ff" },
    { "RGBx8", "Gray8-Generic18", "00 ff 6f 10 6f 02 77 94" },
    { "Gray8", "RGBA8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8", "RGBA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff" },
    { "Gray8", "BGRA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8", "RGB888", "00 00 00 ff ff ff ff ff ff 0a 0a 0a c8 c8 c8 01 01 01 1f 1f 1f ff ff ff" },
    { "Gray8", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff" },
    { "Gray8", "RGBx8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8", "Gray8", "00 ff ff 0a c8 01 1f ff" },
    { "Gray8", "GrayAlpha88", "00 ff ff ff ff ff 0a ff c8 ff 01 ff 1f ff ff ff" },
    { "Gray8", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 10 0a 10 0a 10 ff ff c8 cf c8 cf c8 cf ff ff 01 00 01 00 01 00 ff ff 1f 20 1f 20 1f 20 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 10 0a 10 0a 10 0a ff ff cf c8 cf c8 cf c8 ff ff 00 01 00 01 00 01 ff ff 20 1f 20 1f 20 1f ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 10 0a 10 0a 10 ff ff c8 cf c8 cf c8 cf ff ff 01 00 01 00 01 00 ff ff 1f 20 1f 20 1f 20 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 08 29 08 29 08 29 00 3c 46 3a 46 3a 46 3a 00 3c 00 1c 00 1c 00 1c 00 3c c8 2f c8 2f c8 2f 00 3c 00 3c 00 3c 00 3c 00 3c" },
    { "Gray8", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 21 3d 00 00 21 3d 00 00 21 3d 00 00 80 3f 00 d0 48 3f 00 d0 48 3f 00 d0 48 3f 00 00 80 3f 00 00 80 3b 00 00 80 3b 00 00 80 3b 00 00 80 3f 00 00 f9 3d 00 00 f9 3d 00 00 f9 3d 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray8", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 21 3d 00 00 21 3d 00 00 21 3d 00 00 80 3f 00 d0 48 3f 00 d0 48 3f 00 d0 48 3f 00 00 80 3f 00 00 80 3b 00 00 80 3b 00 00 80 3b 00 00 80 3f 00 00 f9 3d 00 00 f9 3d 00 00 f9 3d 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray8", "RGB565", "00 00 ff ff ff ff 61 08 38 c6 00 00 04 21 ff ff" },
    { "Gray8", "ARGB1555", "00 80 ff ff ff ff 21 84 18 e3 00 80 84 90 ff ff" },
    { "Gray8", "Gray1", "00 80 80 00 80 00 00 80" },
    { "Gray8", "Gray4", "00 f0 f0 10 c0 00 20 f0" },
    { "Gray8", "GrayF32", "00 00 00 00 00 00 80 3f 00 00 80 3f a1 a0 20 3d ca c8 48 3f 81 80 80 3b fa f8 f8 3d 00 00 80 3f" },
    { "Gray8", "RGBA8-P3", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 01 01 01 ff 93 93 93 ff 00 00 00 ff 03 03 03 ff ff ff ff ff" },
    { "Gray8", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff ff ff ff ff 0c 0c 0c ff c7 c7 c7 ff 01 01 01 ff 24 24 24 ff ff ff ff ff" },
    { "Gray8", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff bc bc bc ff 01 01 01 ff 17 17 17 ff ff ff ff ff" },
    { "Gray8", "Gray8-Generic18", "00 ff ff 0a bc 01 17 ff" },
    { "GrayAlpha88", "RGBA8", "00 00 00 00 ff ff ff ff ff ff ff ff 0a 0a 0a 80 c8 c8 c8 40 01 01 01 01 1f 1f 1f 58 ff ff ff c8" },
    { "GrayAlpha88", "RGBA8P", "00 00 00 00 ff ff ff ff ff ff ff ff 05 05 05 80 32 32 32 40 00 00 00 01 0b 0b 0b 58 c8 c8 c8 c8" },
    { "GrayAlpha88", "ARGB8", "00 00 00 00 ff ff ff ff ff ff ff ff 80 0a 0a 0a 40 c8 c8 c8 01 01 01 01 58 1f 1f 1f c8 ff ff ff" },
    { "GrayAlpha88", "BGRA8P", "00 00 00 00 ff ff ff ff ff ff ff ff 05 05 05 80 32 32 32 40 00 00 00 01 0b 0b 0b 58 c8 c8 c8 c8" },
    { "GrayAlpha88", "RGB888", "40 80 bf ff ff ff ff ff ff 25 45 64 62 92 c1 40 80 be 35 5f 88 d6 e4 f1" },
    { "GrayAlpha88", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff ff ff ff 25 45 64 ff 62 92 c1 ff 40 80 be ff 35 5f 88 ff d6 e4 f1" },
    { "GrayAlpha88", "RGBx8", "40 80 bf ff ff ff ff ff ff ff ff ff 25 45 64 ff 62 92 c1 ff 40 80 be ff 35 5f 88 ff d6 e4 f1 ff" },
    { "GrayAlpha88", "Gray8", "40 ff ff 26 62 40 36 d6" },
    { "GrayAlpha88", "GrayAlpha88", "00 00 ff ff ff ff 0a 80 c8 40 01 01 1f 58 ff c8" },
    { "GrayAlpha88", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 10 0a 10 0a 10 80 7f c8 cf c8 cf c8 cf 40 40 01 00 01 00 01 00 01 00 1f 20 1f 20 1f 20 58 60 ff ff ff ff ff ff c8 cf" },
    { "GrayAlpha88", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 10 0a 10 0a 10 0a 7f 80 cf c8 cf c8 cf c8 40 40 00 01 00 01 00 01 00 01 20 1f 20 1f 20 1f 60 58 ff ff ff ff ff ff cf c8" },
    { "GrayAlpha88", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 10 0a 10 0a 10 80 7f c8 cf c8 cf c8 cf 40 40 01 00 01 00 01 00 01 00 1f 20 1f 20 1f 20 58 60 ff ff ff ff ff ff c8 cf" },
    { "GrayAlpha88", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 08 29 08 29 08 29 04 38 46 3a 46 3a 46 3a 04 34 00 1c 00 1c 00 1c 00 1c c8 2f c8 2f c8 2f 86 35 00 3c 00 3c 00 3c 46 3a" },
    { "GrayAlpha88", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 21 3d 00 00 21 3d 00 00 21 3d 00 80 00 3f 00 d0 48 3f 00 d0 48 3f 00 d0 48 3f 00 80 80 3e 00 00 80 3b 00 00 80 3b 00 00 80 3b 00 00 80 3b 00 00 f9 3d 00 00 f9 3d 00 00 f9 3d 00 c0 b0 3e 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 d0 48 3f" },
    { "GrayAlpha88", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 a1 a1 3c 00 a1 a1 3c 00 a1 a1 3c 00 80 00 3f d0 98 49 3e d0 98 49 3e d0 98 49 3e 00 80 80 3e 00 00 80 37 00 00 80 37 00 00 80 37 00 00 80 3b c0 ea 2b 3d c0 ea 2b 3d c0 ea 2b 3d 00 c0 b0 3e 00 d0 48 3f 00 d0 48 3f 00 d0 48 3f 00 d0 48 3f" },
    { "GrayAlpha88", "RGB565", "18 44 ff ff ff ff 2d 2a 97 64 17 44 f0 32 1d d7" },
    { "GrayAlpha88", "ARGB1555", "00 00 ff ff ff ff 21 84 18 63 00 00 63 0c ff ff" },
    { "GrayAlpha88", "Gray1", "00 80 80 00 80 00 00 80" },
    { "GrayAlpha88", "Gray4", "40 f0 f0 30 60 40 30 d0" },
    { "GrayAlpha88", "GrayF32", "00 00 80 3e 00 00 80 3f 00 00 80 3f bc a7 13 3e 0e a9 c4 3e 07 03 7f 3e 68 9d 52 3e 98 96 56 3f" },
    { "GrayAlpha88", "RGBA8-P3", "00 00 00 00 ff ff ff ff ff ff ff ff 0a 0a 0a 80 c8 c8 c8 40 01 01 01 01 1f 1f 1f 58 ff ff ff c8" },
    { "GrayAlpha88", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff ff ff ff 01 01 01 80 93 93 93 40 00 00 00 01 03 03 03 58 ff ff ff c8" },
    { "GrayAlpha88", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff ff ff ff ff 0c 0c 0c 80 c7 c7 c7 40 01 01 01 01 24 24 24 58 ff ff ff c8" },
    { "GrayAlpha88", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff ff ff ff ff 0a 0a 0a 80 bc bc bc 40 01 01 01 01 17 17 17 58 ff ff ff c8" },
    { "GrayAlpha88", "Gray8-Generic18", "40 ff ff 26 5f 40 33 d6" },
    { "RGBA16", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA16", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA16", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "RGBA16", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA16", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBA16", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBA16", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA16", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBA16", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBA16", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 09 0a 12 14 1b 1e 7c 80 c7 c8 67 64 33 32 41 40 00 01 ff 01 05 03 00 01 97 1f 5b 91 b4 e6 79 58 ff ff 83 80 00 00 c7 c8" },
    { "RGBA16", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 00 1c fc 1f 0a 22 00 1c e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBA16", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 90 20 3d a1 90 a0 3d f1 d8 f0 3d 80 7c 00 3f c9 c7 48 3f c9 ce c8 3e c9 cc 48 3e 81 82 80 3e 80 00 80 3b 00 81 ff 3b c1 40 41 3c 80 00 80 3b fd b8 fc 3d 91 5b 11 3f e7 b4 66 3f b1 f2 b0 3e 00 00 80 3f 81 83 00 3f 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f ce 2c a1 3c ce 2c 21 3d 34 c3 71 3d 80 7c 00 3f 7e 94 49 3e 85 9b c9 3d 83 99 49 3d 81 82 80 3e 00 01 80 37 00 82 ff 37 82 41 41 38 80 00 80 3b c4 ae 2e 3d 81 f1 48 3e 15 77 9f 3e b1 f2 b0 3e c9 c7 48 3f 10 96 c9 3e 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16", "RGB565", "18 44 ff ff 00 f8 4d 22 d3 63 17 44 39 34 05 d4" },
    { "RGBA16", "ARGB1555", "00 00 ff ff 00 fc 64 84 86 61 21 04 5c 12 00 fe" },
    { "RGBA16", "Gray1", "00 80 00 00 00 00 00 80" },
    { "RGBA16", "Gray4", "40 f0 80 20 50 40 50 80" },
    { "RGBA16", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 9a a4 25 3e f5 31 a1 3e 58 07 7f 3e 84 61 b3 3e ba e0 0f 3f" },
    { "RGBA16", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBA16", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 21 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBA16", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 02 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBA16", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBA16", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBA16L", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA16L", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA16L", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "RGBA16L", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA16L", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBA16L", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBA16L", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA16L", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBA16L", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBA16L", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16L", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 09 0a 12 14 1b 1e 7c 80 c7 c8 67 64 33 32 41 40 00 01 ff 01 05 03 00 01 97 1f 5b 91 b4 e6 79 58 ff ff 83 80 00 00 c7 c8" },
    { "RGBA16L", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16L", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 00 1c fc 1f 0a 22 00 1c e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBA16L", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 90 20 3d a1 90 a0 3d f1 d8 f0 3d 80 7c 00 3f c9 c7 48 3f c9 ce c8 3e c9 cc 48 3e 81 82 80 3e 80 00 80 3b 00 81 ff 3b c1 40 41 3c 80 00 80 3b fd b8 fc 3d 91 5b 11 3f e7 b4 66 3f b1 f2 b0 3e 00 00 80 3f 81 83 00 3f 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16L", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f ce 2c a1 3c ce 2c 21 3d 34 c3 71 3d 80 7c 00 3f 7e 94 49 3e 85 9b c9 3d 83 99 49 3d 81 82 80 3e 00 01 80 37 00 82 ff 37 82 41 41 38 80 00 80 3b c4 ae 2e 3d 81 f1 48 3e 15 77 9f 3e b1 f2 b0 3e c9 c7 48 3f 10 96 c9 3e 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16L", "RGB565", "f7 43 ff ff 00 f8 4d 2a d3 63 f7 3b 39 3c 05 d4" },
    { "RGBA16L", "ARGB1555", "00 00 ff ff 00 fc 64 88 86 61 00 00 5c 92 00 fe" },
    { "RGBA16L", "Gray1", "00 80 80 00 80 00 00 00" },
    { "RGBA16L", "Gray4", "40 f0 80 30 50 40 50 80" },
    { "RGBA16L", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 9a a4 25 3e f5 31 a1 3e 58 07 7f 3e 84 61 b3 3e ba e0 0f 3f" },
    { "RGBA16L", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBA16L", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 21 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBA16L", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 02 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBA16L", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBA16L", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBA16B", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA16B", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA16B", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "RGBA16B", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA16B", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBA16B", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBA16B", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA16B", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBA16B", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBA16B", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16B", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 09 0a 12 14 1b 1e 7c 80 c7 c8 67 64 33 32 41 40 00 01 ff 01 05 03 00 01 97 1f 5b 91 b4 e6 79 58 ff ff 83 80 00 00 c7 c8" },
    { "RGBA16B", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBA16B", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 00 1c fc 1f 0a 22 00 1c e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBA16B", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a1 90 20 3d a1 90 a0 3d f1 d8 f0 3d 80 7c 00 3f c9 c7 48 3f c9 ce c8 3e c9 cc 48 3e 81 82 80 3e 80 00 80 3b 00 81 ff 3b c1 40 41 3c 80 00 80 3b fd b8 fc 3d 91 5b 11 3f e7 b4 66 3f b1 f2 b0 3e 00 00 80 3f 81 83 00 3f 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16B", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f ce 2c a1 3c ce 2c 21 3d 34 c3 71 3d 80 7c 00 3f 7e 94 49 3e 85 9b c9 3d 83 99 49 3d 81 82 80 3e 00 01 80 37 00 82 ff 37 82 41 41 38 80 00 80 3b c4 ae 2e 3d 81 f1 48 3e 15 77 9f 3e b1 f2 b0 3e c9 c7 48 3f 10 96 c9 3e 00 00 00 00 c9 c7 48 3f" },
    { "RGBA16B", "RGB565", "17 44 ff ff 00 f8 4d 22 d3 63 f7 3b 59 3c e5 d3" },
    { "RGBA16B", "ARGB1555", "00 00 ff ff 00 fc 43 04 86 e5 00 00 5c 12 00 fe" },
    { "RGBA16B", "Gray1", "00 80 80 00 00 00 80 00" },
    { "RGBA16B", "Gray4", "40 f0 80 30 50 40 50 80" },
    { "RGBA16B", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 9a a4 25 3e f5 31 a1 3e 58 07 7f 3e 84 61 b3 3e ba e0 0f 3f" },
    { "RGBA16B", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBA16B", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 21 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBA16B", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 02 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBA16B", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBA16B", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBA16F", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 20 91 e6 58 ff 80 00 c8" },
    { "RGBA16F", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBA16F", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 20 91 e6 c8 ff 80 00" },
    { "RGBA16F", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBA16F", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBA16F", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBA16F", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBA16F", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBA16F", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBA16F", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1c 80 80 c8 bf 64 60 32 30 40 40 01 00 01 ff 03 05 01 00 1f 98 91 5f e6 bf 58 80 ff ff 80 80 00 00 c8 bf" },
    { "RGBA16F", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1c 1e 80 80 bf c8 60 64 30 32 40 40 00 01 ff 01 05 03 00 01 98 1f 5f 91 bf e6 80 58 ff ff 80 80 00 00 bf c8" },
    { "RGBA16F", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 0a 14 14 1e 1c 80 80 c8 bf 64 60 32 30 40 40 01 00 01 ff 03 05 01 00 1f 98 91 5f e6 bf 58 80 ff ff 80 80 00 00 c8 bf" },
    { "RGBA16F", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 fd 1b fd 1f 0b 22 fd 1b e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBA16F", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 a0 20 3d 00 a0 a0 3d 00 e0 f0 3d 00 80 00 3f 00 c0 48 3f 00 c0 c8 3e 00 c0 48 3e 00 80 80 3e 00 a0 7f 3b 00 a0 ff 3b 00 60 41 3c 00 a0 7f 3b 00 c0 fc 3d 00 60 11 3f 00 c0 66 3f 00 00 b1 3e 00 00 80 3f 00 80 00 3f 00 00 00 00 00 c0 48 3f" },
    { "RGBA16F", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f a0 40 a1 3c a0 40 21 3d e0 d0 71 3d 00 80 00 3f c0 88 49 3e c0 88 c9 3d c0 88 49 3d 00 80 80 3e 24 40 7f 37 24 40 ff 37 7c 17 41 38 00 a0 7f 3b c0 c0 2e 3d c0 06 49 3e c0 8a 9f 3e 00 00 b1 3e 00 c0 48 3f c0 88 c9 3e 00 00 00 00 00 c0 48 3f" },
    { "RGBA16F", "RGB565", "17 44 ff ff 00 f8 4d 22 d3 63 17 44 38 34 05 dc" },
    { "RGBA16F", "ARGB1555", "00 00 ff ff 00 fc 64 88 86 61 21 00 5c 92 e0 7d" },
    { "RGBA16F", "Gray1", "00 80 80 00 00 00 00 00" },
    { "RGBA16F", "Gray4", "40 f0 80 30 50 40 50 90" },
    { "RGBA16F", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 78 a5 25 3e d5 2d a1 3e b6 07 7f 3e a6 68 b3 3e 34 dc 0f 3f" },
    { "RGBA16F", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBA16F", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 20 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBA16F", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 01 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBA16F", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBA16F", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBAF32", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBAF32", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBAF32", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c8 64 32 01 01 02 03 58 1f 91 e6 c8 ff 80 00" },
    { "RGBAF32", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBAF32", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 80 be 35 86 cc d6 80 29" },
    { "RGBAF32", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29" },
    { "RGBAF32", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 80 be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBAF32", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBAF32", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBAF32", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBAF32", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 09 0a 12 14 1b 1e 7c 80 c7 c8 67 64 33 32 41 40 00 01 ff 01 05 03 00 01 97 1f 5b 91 b4 e6 79 58 ff ff 83 80 00 00 c7 c8" },
    { "RGBAF32", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 09 14 12 1e 1b 80 7c c8 c7 64 67 32 33 40 41 01 00 01 ff 03 05 01 00 1f 97 91 5b e6 b4 58 79 ff ff 80 83 00 00 c8 c7" },
    { "RGBAF32", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 fd 1b fd 1f 0b 22 fd 1b e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBAF32", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 2e 90 20 3d 2e 90 a0 3d 45 d8 f0 3d 85 7c 00 3f e3 c7 48 3f 70 ce c8 3e 70 ce 48 3e 12 83 80 3e 24 97 7f 3b 24 97 ff 3b ca 54 41 3c 24 97 7f 3b 24 b9 fc 3d 57 5b 11 3f 0b b5 66 3f 7c f2 b0 3e 00 00 80 3f 12 83 00 3f 00 00 00 00 e3 c7 48 3f" },
    { "RGBAF32", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 61 2c a1 3c 61 2c 21 3d 91 c2 71 3d 85 7c 00 3f 7c 95 49 3e 10 9c c9 3d 10 9c 49 3d 12 83 80 3e 73 2e 7f 37 73 2e ff 37 99 05 41 38 24 97 7f 3b aa ae 2e 3d f5 f0 48 3e fe 76 9f 3e 7c f2 b0 3e e3 c7 48 3f 7c 95 c9 3e 00 00 00 00 e3 c7 48 3f" },
    { "RGBAF32", "RGB565", "18 44 ff ff 00 f8 4e 2a d3 63 17 44 39 34 e5 d3" },
    { "RGBAF32", "ARGB1555", "00 00 ff ff 00 fc 43 04 a6 e5 00 00 5c 12 00 fe" },
    { "RGBAF32", "Gray1", "00 80 80 00 00 00 00 80" },
    { "RGBAF32", "Gray4", "40 f0 70 30 50 40 50 80" },
    { "RGBAF32", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 7a a4 25 3e 15 32 a1 3e bf 07 7f 3e 5a 61 b3 3e 97 e0 0f 3f" },
    { "RGBAF32", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBAF32", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 21 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBAF32", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 02 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBAF32", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBAF32", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGBAF32P", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c7 64 34 40 00 00 00 01 20 91 e5 58 ff 80 00 c8" },
    { "RGBAF32P", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0a 0f 80 32 19 0d 40 00 00 00 01 0b 32 4f 58 c8 64 00 c8" },
    { "RGBAF32P", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 0a 14 1e 40 c7 64 34 01 00 00 00 58 20 91 e5 c8 ff 80 00" },
    { "RGBAF32P", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0f 0a 05 80 0d 19 32 40 00 00 00 01 4f 32 0b 58 00 64 c8 c8" },
    { "RGBAF32P", "RGB888", "40 80 bf ff ff ff ff 00 00 25 4a 6e 62 79 9c 40 7f be 35 86 cc d6 80 29" },
    { "RGBAF32P", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 7f be ff 35 86 cc ff d6 80 29" },
    { "RGBAF32P", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 25 4a 6e ff 62 79 9c ff 40 7f be ff 35 86 cc ff d6 80 29 ff" },
    { "RGBAF32P", "Gray8", "40 ff 82 2a 51 40 5a 8f" },
    { "RGBAF32P", "GrayAlpha88", "00 00 ff ff 82 ff 13 80 82 40 02 01 8a 58 a5 c8" },
    { "RGBAF32P", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 08 14 12 1e 1b 80 7c c8 c7 64 66 32 35 40 41 01 00 02 00 03 00 01 00 1f 97 91 5a e6 b3 58 79 ff ff 80 82 00 00 c8 c7" },
    { "RGBAF32P", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 0a 12 14 1b 1e 7c 80 c7 c8 66 64 35 32 41 40 00 01 00 02 00 03 00 01 97 1f 5a 91 b3 e6 79 58 ff ff 82 80 00 00 c7 c8" },
    { "RGBAF32P", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 0a 08 14 12 1e 1b 80 7c c8 c7 64 66 32 35 40 41 01 00 02 00 03 00 01 00 1f 97 91 5a e6 b3 58 79 ff ff 80 82 00 00 c8 c7" },
    { "RGBAF32P", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 05 29 05 2d 87 2f 04 38 46 3a 46 36 46 32 04 34 fd 1b fd 1f 0b 22 fd 1b e6 2f 8b 38 36 3b 88 35 00 3c 04 38 00 00 46 3a" },
    { "RGBAF32P", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 2e 90 20 3d 2e 90 a0 3d 45 d8 f0 3d 85 7c 00 3f e3 c7 48 3f 70 ce c8 3e 70 ce 48 3e 12 83 80 3e 24 97 7f 3b 24 97 ff 3b ca 54 41 3c 24 97 7f 3b 24 b9 fc 3d 57 5b 11 3f 0a b5 66 3f 7c f2 b0 3e 00 00 80 3f 12 83 00 3f 00 00 00 00 e3 c7 48 3f" },
    { "RGBAF32P", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 61 2c a1 3c 61 2c 21 3d 91 c2 71 3d 85 7c 00 3f 7c 95 49 3e 10 9c c9 3d 10 9c 49 3d 12 83 80 3e 73 2e 7f 37 73 2e ff 37 99 05 41 38 24 97 7f 3b aa ae 2e 3d f5 f0 48 3e fe 76 9f 3e 7c f2 b0 3e e3 c7 48 3f 7c 95 c9 3e 00 00 00 00 e3 c7 48 3f" },
    { "RGBAF32P", "RGB565", "17 44 ff ff 00 f8 4d 22 d3 63 17 44 39 3c 05 d4" },
    { "RGBAF32P", "ARGB1555", "00 00 ff ff 00 fc 64 88 86 61 00 00 3b 0e 00 fe" },
    { "RGBAF32P", "Gray1", "00 80 80 00 00 00 80 80" },
    { "RGBAF32P", "Gray4", "40 f0 80 20 50 40 50 90" },
    { "RGBAF32P", "GrayF32", "00 00 80 3e 1f ff 7f 3f 5d 4f 02 3f 7a a4 25 3e 15 32 a1 3e bf 07 7f 3e 5a 61 b3 3e 97 e0 0f 3f" },
    { "RGBAF32P", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0c 14 1d 80 bb 69 3e 40 01 02 03 01 46 8f df 58 ef 87 33 c8" },
    { "RGBAF32P", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 02 03 80 93 21 08 40 00 00 00 01 04 48 c9 58 ff 37 00 c8" },
    { "RGBAF32P", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 11 1b 23 80 b1 64 38 40 02 02 04 01 56 8f e2 58 e3 7f 1e c8" },
    { "RGBAF32P", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 0a 10 17 80 ba 4f 26 40 01 02 04 01 1d 7b df 58 fc 6a 08 c8" },
    { "RGBAF32P", "Gray8-Generic18", "40 ff 6f 29 4c 40 53 82" },
    { "RGB565", "RGBA8", "00 00 00 ff ff ff ff ff ff 00 00 ff 08 14 21 ff c5 65 31 ff 00 00 00 ff 21 92 e6 ff ff 82 00 ff" },
    { "RGB565", "RGBA8P", "00 00 00 ff ff ff ff ff ff 00 00 ff 08 14 21 ff c5 65 31 ff 00 00 00 ff 21 92 e6 ff ff 82 00 ff" },
    { "RGB565", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 08 14 21 ff c5 65 31 ff 00 00 00 ff 21 92 e6 ff ff 82 00" },
    { "RGB565", "BGRA8P", "00 00 00 ff ff ff ff ff 00 00 ff ff 21 14 08 ff 31 65 c5 ff 00 00 00 ff e6 92 21 ff 00 82 ff ff" },
    { "RGB565", "RGB888", "00 00 00 ff ff ff ff 00 00 08 14 21 c5 65 31 00 00 00 21 92 e6 ff 82 00" },
    { "RGB565", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff 00 00 ff 08 14 21 ff c5 65 31 ff 00 00 00 ff 21 92 e6 ff ff 82 00" },
    { "RGB565", "RGBx8", "00 00 00 ff ff ff ff ff ff 00 00 ff 08 14 21 ff c5 65 31 ff 00 00 00 ff 21 92 e6 ff ff 82 00 ff" },
    { "RGB565", "Gray8", "00 ff 82 13 81 00 8b a6" },
    { "RGB565", "GrayAlpha88", "00 ff ff ff 82 ff 13 ff 81 ff 00 ff 8b ff a6 ff" },
    { "RGB565", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 14 14 21 21 ff ff c5 c5 65 65 31 31 ff ff 00 00 00 00 00 00 ff ff 21 21 92 92 e6 e6 ff ff ff ff 82 82 00 00 ff ff" },
    { "RGB565", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 14 14 21 21 ff ff c5 c5 65 65 31 31 ff ff 00 00 00 00 00 00 ff ff 21 21 92 92 e6 e6 ff ff ff ff 82 82 00 00 ff ff" },
    { "RGB565", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 14 14 21 21 ff ff c5 c5 65 65 31 31 ff ff 00 00 00 00 00 00 ff ff 21 21 92 92 e6 e6 ff ff ff ff 82 82 00 00 ff ff" },
    { "RGB565", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 28 04 2d 24 30 00 3c 2e 3a 56 36 26 32 00 3c 00 00 00 00 00 00 00 3c 24 30 94 38 37 3b 00 3c 00 3c 14 38 00 00 00 3c" },
    { "RGB565", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 81 80 00 3d a1 a0 a0 3d 85 84 04 3e 00 00 80 3f c7 c5 45 3f cc ca ca 3e c6 c4 44 3e 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 85 84 04 3e 93 92 12 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 83 82 02 3f 00 00 00 00 00 00 80 3f" },
    { "RGB565", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 81 80 00 3d a1 a0 a0 3d 85 84 04 3e 00 00 80 3f c7 c5 45 3f cc ca ca 3e c6 c4 44 3e 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 85 84 04 3e 93 92 12 3f e8 e6 66 3f 00 00 80 3f 00 00 80 3f 83 82 02 3f 00 00 00 00 00 00 80 3f" },
    { "RGB565", "RGB565", "00 00 ff ff 00 f8 a4 08 26 c3 00 00 9c 24 00 fc" },
    { "RGB565", "ARGB1555", "00 80 ff ff 00 fc 64 84 86 e1 00 80 5c 92 e0 fd" },
    { "RGB565", "Gray1", "00 80 80 00 00 00 80 00" },
    { "RGB565", "Gray4", "00 f0 80 10 80 00 80 a0" },
    { "RGB565", "GrayF32", "00 00 00 00 00 00 80 3f 00 50 02 3f 00 00 9a 3d 00 80 01 3f 00 00 00 00 00 50 0b 3f 00 e0 26 3f" },
    { "RGB565", "RGBA8-P3", "00 00 00 ff ff ff ff ff ea 33 23 ff 0a 14 20 ff b9 6a 3d ff 00 00 00 ff 47 90 df ff ef 89 33 ff" },
    { "RGB565", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff 00 00 ff 01 02 04 ff 8e 21 08 ff 00 00 00 ff 04 49 ca ff ff 39 00 ff" },
    { "RGB565", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff db 00 00 ff 0f 1b 26 ff af 65 37 ff 00 00 00 ff 57 91 e3 ff e4 81 1e ff" },
    { "RGB565", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff fb 00 07 ff 09 10 19 ff b7 50 25 ff 00 00 00 ff 1e 7d e0 ff fc 6d 08 ff" },
    { "RGB565", "Gray8-Generic18", "00 ff 6f 10 6e 00 78 95" },
    { "ARGB1555", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 08 10 21 ff c5 63 31 00 00 00 00 00 21 94 e6 00 ff 84 00 ff" },
    { "ARGB1555", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 08 10 21 ff 00 00 00 00 00 00 00 00 00 00 00 00 ff 84 00 ff" },
    { "ARGB1555", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 ff 08 10 21 00 c5 63 31 00 00 00 00 00 21 94 e6 ff ff 84 00" },
    { "ARGB1555", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 21 10 08 ff 00 00 00 00 00 00 00 00 00 00 00 00 00 84 ff ff" },
    { "ARGB1555", "RGB888", "40 80 bf ff ff ff ff 00 00 08 10 21 40 80 bf 40 80 bf 40 80 bf ff 84 00" },
    { "ARGB1555", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 08 10 21 ff 40 80 bf ff 40 80 bf ff 40 80 bf ff ff 84 00" },
    { "ARGB1555", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 08 10 21 ff 40 80 bf ff 40 80 bf ff 40 80 bf ff ff 84 00 ff" },
    { "ARGB1555", "Gray8", "40 ff 82 11 40 40 40 a7" },
    { "ARGB1555", "GrayAlpha88", "00 00 ff ff 82 ff 10 ff 80 00 00 00 8c 00 a7 ff" },
    { "ARGB1555", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 10 10 21 21 ff ff c5 c5 63 63 31 31 00 00 00 00 00 00 00 00 00 00 21 21 94 94 e6 e6 00 00 ff ff 84 84 00 00 ff ff" },
    { "ARGB1555", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 10 10 21 21 ff ff c5 c5 63 63 31 31 00 00 00 00 00 00 00 00 00 00 21 21 94 94 e6 e6 00 00 ff ff 84 84 00 00 ff ff" },
    { "ARGB1555", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 08 08 10 10 21 21 ff ff c5 c5 63 63 31 31 00 00 00 00 00 00 00 00 00 00 21 21 94 94 e6 e6 00 00 ff ff 84 84 00 00 ff ff" },
    { "ARGB1555", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 08 28 04 2c 24 30 00 3c 2e 3a 36 36 26 32 00 00 00 00 00 00 00 00 00 00 24 30 a4 38 37 3b 00 00 00 3c 24 38 00 00 00 3c" },
    { "ARGB1555", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 81 80 00 3d 81 80 80 3d 85 84 04 3e 00 00 80 3f c7 c5 45 3f c8 c6 c6 3e c6 c4 44 3e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 85 84 04 3e 95 94 14 3f e8 e6 66 3f 00 00 00 00 00 00 80 3f 85 84 04 3f 00 00 00 00 00 00 80 3f" },
    { "ARGB1555", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 81 80 00 3d 81 80 80 3d 85 84 04 3e 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 85 84 04 3f 00 00 00 00 00 00 80 3f" },
    { "ARGB1555", "RGB565", "18 44 ff ff 00 f8 84 08 17 44 f7 3b 17 44 00 fc" },
    { "ARGB1555", "ARGB1555", "00 00 ff ff 00 fc 44 84 86 61 00 00 5c 12 00 fe" },
    { "ARGB1555", "Gray1", "00 80 00 00 00 00 00 80" },
    { "ARGB1555", "Gray4", "40 f0 80 10 40 40 40 a0" },
    { "ARGB1555", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 00 00 7f 3d 00 00 80 3e 00 00 80 3e 00 00 80 3e 00 f0 27 3f" },
    { "ARGB1555", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 0a 10 20 ff b8 68 3d 00 00 00 00 00 48 92 e0 00 ef 8a 33 ff" },
    { "ARGB1555", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 01 01 04 ff 8e 20 08 00 00 00 00 00 04 4c ca 00 ff 3b 00 ff" },
    { "ARGB1555", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 0d 15 25 ff af 63 37 00 00 00 00 00 58 93 e3 00 e4 83 1f ff" },
    { "ARGB1555", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 09 0d 19 ff b6 4e 25 00 00 00 00 00 1e 7f e0 00 fc 6f 08 ff" },
    { "ARGB1555", "Gray8-Generic18", "40 ff 6f 0f 40 40 40 97" },
    { "Gray1", "RGBA8", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "RGBA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff" },
    { "Gray1", "BGRA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "RGB888", "00 00 00 ff ff ff ff ff ff 00 00 00 ff ff ff 00 00 00 00 00 00 ff ff ff" },
    { "Gray1", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff" },
    { "Gray1", "RGBx8", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "Gray8", "00 ff ff 00 ff 00 00 ff" },
    { "Gray1", "GrayAlpha88", "00 ff ff ff ff ff 00 ff ff ff 00 ff 00 ff ff ff" },
    { "Gray1", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray1", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray1", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray1", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 00 00 3c 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c" },
    { "Gray1", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray1", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray1", "RGB565", "00 00 ff ff ff ff 00 00 ff ff 00 00 00 00 ff ff" },
    { "Gray1", "ARGB1555", "00 80 ff ff ff ff 00 80 ff ff 00 80 00 80 ff ff" },
    { "Gray1", "Gray1", "00 80 80 00 80 00 00 80" },
    { "Gray1", "Gray4", "00 f0 f0 00 f0 00 00 f0" },
    { "Gray1", "GrayF32", "00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f" },
    { "Gray1", "RGBA8-P3", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 00 00 00 ff ff ff ff ff 00 00 00 ff 00 00 00 ff ff ff ff ff" },
    { "Gray1", "Gray8-Generic18", "00 ff ff 00 ff 00 00 ff" },
    { "Gray4", "RGBA8", "00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff ff" },
    { "Gray4", "RGBA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff ff" },
    { "Gray4", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff" },
    { "Gray4", "BGRA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff ff" },
    { "Gray4", "RGB888", "00 00 00 ff ff ff ff ff ff 11 11 11 cc cc cc 00 00 00 22 22 22 ff ff ff" },
    { "Gray4", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff" },
    { "Gray4", "RGBx8", "00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff ff" },
    { "Gray4", "Gray8", "00 ff ff 11 cc 00 22 ff" },
    { "Gray4", "GrayAlpha88", "00 ff ff ff ff ff 11 ff cc ff 00 ff 22 ff ff ff" },
    { "Gray4", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 11 10 11 10 11 10 ff ff cc cf cc cf cc cf ff ff 00 00 00 00 00 00 ff ff 22 20 22 20 22 20 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray4", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 10 11 10 11 10 11 ff ff cf cc cf cc cf cc ff ff 00 00 00 00 00 00 ff ff 20 22 20 22 20 22 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray4", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 11 10 11 10 11 10 ff ff cc cf cc cf cc cf ff ff 00 00 00 00 00 00 ff ff 22 20 22 20 22 20 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray4", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 44 2c 44 2c 44 2c 00 3c 66 3a 66 3a 66 3a 00 3c 00 00 00 00 00 00 00 3c 44 30 44 30 44 30 00 3c 00 3c 00 3c 00 3c 00 3c" },
    { "Gray4", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 80 88 3d 00 80 88 3d 00 80 88 3d 00 00 80 3f 00 d0 4c 3f 00 d0 4c 3f 00 d0 4c 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 80 08 3e 00 80 08 3e 00 80 08 3e 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray4", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 80 88 3d 00 80 88 3d 00 80 88 3d 00 00 80 3f 00 d0 4c 3f 00 d0 4c 3f 00 d0 4c 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 80 08 3e 00 80 08 3e 00 80 08 3e 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray4", "RGB565", "00 00 ff ff ff ff a2 10 58 c6 00 00 24 21 ff ff" },
    { "Gray4", "ARGB1555", "00 80 ff ff ff ff 42 88 39 e7 00 80 84 90 ff ff" },
    { "Gray4", "Gray1", "00 80 80 00 80 00 00 80" },
    { "Gray4", "Gray4", "00 f0 f0 10 c0 00 20 f0" },
    { "Gray4", "GrayF32", "00 00 00 00 00 00 80 3f 00 00 80 3f 89 88 88 3d ce cc 4c 3f 00 00 00 00 89 88 08 3e 00 00 80 3f" },
    { "Gray4", "RGBA8-P3", "00 00 00 ff ff ff ff ff ff ff ff ff 11 11 11 ff cc cc cc ff 00 00 00 ff 22 22 22 ff ff ff ff ff" },
    { "Gray4", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 01 01 01 ff 9a 9a 9a ff 00 00 00 ff 04 04 04 ff ff ff ff ff" },
    { "Gray4", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff ff ff ff ff 17 17 17 ff cb cb cb ff 00 00 00 ff 27 27 27 ff ff ff ff ff" },
    { "Gray4", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 0e 0e 0e ff c1 c1 c1 ff 00 00 00 ff 1a 1a 1a ff ff ff ff ff" },
    { "Gray4", "Gray8-Generic18", "00 ff ff 0e c1 00 1a ff" },
    { "GrayF32", "RGBA8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "GrayF32", "RGBA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "GrayF32", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff" },
    { "GrayF32", "BGRA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "GrayF32", "RGB888", "00 00 00 ff ff ff ff ff ff 0a 0a 0a c8 c8 c8 01 01 01 1f 1f 1f ff ff ff" },
    { "GrayF32", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff" },
    { "GrayF32", "RGBx8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "GrayF32", "Gray8", "00 ff ff 0a c8 01 1f ff" },
    { "GrayF32", "GrayAlpha88", "00 ff ff ff ff ff 0a ff c8 ff 01 ff 1f ff ff ff" },
    { "GrayF32", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 09 0a 09 0a 09 ff ff c8 c7 c8 c7 c8 c7 ff ff 01 00 01 00 01 00 ff ff 1f 97 1f 97 1f 97 ff ff ff ff ff ff ff ff ff ff" },
    { "GrayF32", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 09 0a 09 0a 09 0a ff ff c7 c8 c7 c8 c7 c8 ff ff 00 01 00 01 00 01 ff ff 97 1f 97 1f 97 1f ff ff ff ff ff ff ff ff ff ff" },
    { "GrayF32", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 0a 09 0a 09 0a 09 ff ff c8 c7 c8 c7 c8 c7 ff ff 01 00 01 00 01 00 ff ff 1f 97 1f 97 1f 97 ff ff ff ff ff ff ff ff ff ff" },
    { "GrayF32", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 05 29 05 29 05 29 00 3c 46 3a 46 3a 46 3a 00 3c fd 1b fd 1b fd 1b 00 3c e6 2f e6 2f e6 2f 00 3c 00 3c 00 3c 00 3c 00 3c" },
    { "GrayF32", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 2e 90 20 3d 2e 90 20 3d 2e 90 20 3d 00 00 80 3f e3 c7 48 3f e3 c7 48 3f e3 c7 48 3f 00 00 80 3f 24 97 7f 3b 24 97 7f 3b 24 97 7f 3b 00 00 80 3f 24 b9 fc 3d 24 b9 fc 3d 24 b9 fc 3d 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "GrayF32", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 2e 90 20 3d 2e 90 20 3d 2e 90 20 3d 00 00 80 3f e3 c7 48 3f e3 c7 48 3f e3 c7 48 3f 00 00 80 3f 24 97 7f 3b 24 97 7f 3b 24 97 7f 3b 00 00 80 3f 24 b9 fc 3d 24 b9 fc 3d 24 b9 fc 3d 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "GrayF32", "RGB565", "00 00 ff ff ff ff 41 08 59 ce 21 08 e3 18 ff ff" },
    { "GrayF32", "ARGB1555", "00 80 ff ff ff ff 42 88 18 e3 00 80 84 90 ff ff" },
    { "GrayF32", "Gray1", "00 80 80 00 80 00 00 80" },
    { "GrayF32", "Gray4", "00 f0 f0 10 c0 00 20 f0" },
    { "GrayF32", "GrayF32", "00 00 00 00 00 00 80 3f 00 00 80 3f 2e 90 20 3d e3 c7 48 3f 24 97 7f 3b 24 b9 fc 3d 00 00 80 3f" },
    { "GrayF32", "RGBA8-P3", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "GrayF32", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 01 01 01 ff 93 93 93 ff 00 00 00 ff 04 04 04 ff ff ff ff ff" },
    { "GrayF32", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff ff ff ff ff 0c 0c 0c ff c7 c7 c7 ff 01 01 01 ff 25 25 25 ff ff ff ff ff" },
    { "GrayF32", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff bc bc bc ff 01 01 01 ff 18 18 18 ff ff ff ff ff" },
    { "GrayF32", "Gray8-Generic18", "00 ff ff 0a bc 01 18 ff" },
    { "RGBA8-P3", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 06 15 1f 80 d7 5d 1f 40 01 02 03 01 00 94 ed 58 ff 77 00 c8" },
    { "RGBA8-P3", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 03 0b 10 80 36 17 08 40 00 00 00 01 00 33 52 58 c8 5d 00 c8" },
    { "RGBA8-P3", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 06 15 1f 40 d7 5d 1f 01 01 02 03 58 00 94 ed c8 ff 77 00" },
    { "RGBA8-P3", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 10 0b 03 80 08 17 36 40 00 00 00 01 52 33 00 58 00 5d c8 c8" },
    { "RGBA8-P3", "RGB888", "40 80 bf ff ff ff ff 00 00 23 4a 6f 66 77 97 40 80 be 2a 87 cf d6 79 29" },
    { "RGBA8-P3", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 23 4a 6f ff 66 77 97 ff 40 80 be ff 2a 87 cf ff d6 79 29" },
    { "RGBA8-P3", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 23 4a 6f ff 66 77 97 ff 40 80 be ff 2a 87 cf ff d6 79 29 ff" },
    { "RGBA8-P3", "Gray8", "40 ff 87 2a 51 40 59 92" },
    { "RGBA8-P3", "GrayAlpha88", "00 00 ff ff 87 ff 13 80 84 40 02 01 89 58 a8 c8" },
    { "RGBA8-P3", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 06 70 14 c0 1f 10 80 7f d7 bf 5d 00 1f 10 40 40 00 d0 01 a0 03 40 01 00 00 00 94 3f ed df 58 60 ff ff 77 00 00 00 c8 cf" },
    { "RGBA8-P3", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 70 06 c0 14 10 1f 7f 80 bf d7 00 5d 10 1f 40 40 d0 00 a0 01 40 03 00 01 00 00 3f 94 df ed 60 58 ff ff 00 77 00 00 cf c8" },
    { "RGBA8-P3", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 06 70 14 c0 1f 10 80 7f d7 bf 5d 00 1f 10 40 40 00 d0 01 a0 03 40 01 00 00 00 94 3f ed df 58 60 ff ff 77 00 00 00 c8 cf" },
    { "RGBA8-P3", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 5f 3c 40 b3 ca b0 00 3c 70 26 30 2d c4 2f 04 38 be 3a d0 35 c4 2f 04 34 80 1a 80 1e 80 22 00 1c b2 b3 a2 38 6f 3b 86 35 4c 3c 70 37 c0 b2 46 3a" },
    { "RGBA8-P3", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 e8 8b 3f 00 00 68 be 00 40 19 be 00 00 80 3f 00 00 ce 3c 00 00 a6 3d 00 80 f8 3d 00 80 00 3f 00 c0 57 3f 00 00 ba 3e 00 80 f8 3d 00 80 80 3e 00 00 50 3b 00 00 d0 3b 00 00 50 3c 00 00 80 3b 00 40 76 be 00 40 14 3f 00 e0 6d 3f 00 c0 b0 3e 00 70 89 3f 00 00 ee 3e 00 00 58 be 00 d0 48 3f" },
    { "RGBA8-P3", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 e8 8b 3f 00 00 68 be 00 40 19 be 00 00 80 3f 00 ce 4e 3c 00 a6 26 3d 80 78 79 3d 00 80 00 3f c0 97 58 3e 00 ba ba 3d 80 78 f9 3c 00 80 80 3e 00 00 50 37 00 00 d0 37 00 00 50 38 00 00 80 3b b0 04 aa bd 60 b6 4c 3e 68 3c a4 3e 00 c0 b0 3e 56 9e 57 3f 60 b1 ba 3e 80 6f 29 be 00 d0 48 3f" },
    { "RGBA8-P3", "RGB565", "18 44 ff ff 00 f8 4d 22 b2 63 17 44 39 2c c5 d3" },
    { "RGBA8-P3", "ARGB1555", "00 00 ff ff 00 fc 43 00 64 69 21 00 5d 02 c0 7d" },
    { "RGBA8-P3", "Gray1", "00 80 80 00 00 00 00 80" },
    { "RGBA8-P3", "Gray4", "40 f0 80 30 50 40 50 90" },
    { "RGBA8-P3", "GrayF32", "00 00 80 3e 00 00 80 3f 00 40 07 3f 80 25 25 3e 20 42 a2 3e 80 06 7f 3e 80 17 b3 3e e8 f8 11 3f" },
    { "RGBA8-P3", "RGBA8-P3", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA8-P3", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 00 02 03 80 ad 1c 03 40 00 00 00 01 00 4b d8 58 ff 2f 00 c8" },
    { "RGBA8-P3", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff ef 00 00 ff 0e 1b 24 80 bd 5d 29 40 01 02 04 01 41 92 e9 58 f2 76 00 c8" },
    { "RGBA8-P3", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 09 10 17 80 cb 47 19 40 01 02 04 01 00 7e e8 58 ff 5f 00 c8" },
    { "RGBA8-P3", "Gray8-Generic18", "40 ff 74 28 4c 40 53 84" },
    { "RGBA8-LinSRGB", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 38 4f 60 80 e5 a8 7a 40 0d 16 1c 01 62 c7 f4 58 ff bc 00 c8" },
    { "RGBA8-LinSRGB", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 1c 28 30 80 39 2a 1f 40 00 00 00 01 22 45 54 58 c8 93 00 c8" },
    { "RGBA8-LinSRGB", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 38 4f 60 40 e5 a8 7a 01 0d 16 1c 58 62 c7 f4 c8 ff bc 00" },
    { "RGBA8-LinSRGB", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 30 28 1c 80 1f 2a 39 40 00 00 00 01 54 45 22 58 00 93 c8 c8" },
    { "RGBA8-LinSRGB", "RGB888", "40 80 bf ff ff ff ff 00 00 3c 67 8f 69 8a ae 40 80 be 4c 99 d1 d6 af 29" },
    { "RGBA8-LinSRGB", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 3c 67 8f ff 69 8a ae ff 40 80 be ff 4c 99 d1 ff d6 af 29" },
    { "RGBA8-LinSRGB", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 3c 67 8f ff 69 8a ae ff 40 80 be ff 4c 99 d1 ff d6 af 29 ff" },
    { "RGBA8-LinSRGB", "Gray8", "40 ff 82 46 5e 40 6a ab" },
    { "RGBA8-LinSRGB", "GrayAlpha88", "00 00 ff ff 82 ff 4c 80 b6 40 14 01 ba 58 c9 c8" },
    { "RGBA8-LinSRGB", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 38 00 4f 70 60 a0 80 7f e5 ff a8 bf 7a e0 40 40 0c c0 15 c0 1c 50 01 00 62 30 c7 5f f4 9f 58 60 ff ff bc 8f 00 00 c8 cf" },
    { "RGBA8-LinSRGB", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 00 38 70 4f a0 60 7f 80 ff e5 bf a8 e0 7a 40 40 c0 0c c0 15 50 1c 00 01 30 62 5f c7 9f f4 60 58 ff ff 8f bc 00 00 cf c8" },
    { "RGBA8-LinSRGB", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 38 00 4f 70 60 a0 80 7f e5 ff a8 bf 7a e0 40 40 0c c0 15 c0 1c 50 01 00 62 30 c7 5f f4 9f 58 60 ff ff bc 8f 00 00 c8 cf" },
    { "RGBA8-LinSRGB", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 00 00 00 00 3c 00 33 f7 34 0a 36 04 38 30 3b 46 39 ae 37 04 34 60 2a 70 2d 14 2f 00 1c 23 36 3b 3a a5 3b 86 35 00 3c e4 39 00 00 46 3a" },
    { "RGBA8-LinSRGB", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 60 3e 00 e0 9e 3e 00 40 c1 3e 00 80 00 3f 00 00 66 3f 00 c0 28 3f 00 c0 f5 3e 00 80 80 3e 00 00 4c 3d 00 00 ae 3d 00 80 e2 3d 00 00 80 3b 00 60 c4 3e 00 60 47 3f 00 a0 74 3f 00 c0 b0 3e 00 00 80 3f 00 90 3c 3f 00 00 00 00 00 d0 48 3f" },
    { "RGBA8-LinSRGB", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 e0 e0 3d e0 7e 1f 3e 40 01 42 3e 00 80 00 3f 00 e6 66 3e c0 68 29 3e c0 b5 f6 3d 00 80 80 3e 00 00 4c 39 00 00 ae 39 00 80 e2 39 00 00 80 3b 48 95 07 3e 88 a7 89 3e 78 e5 a8 3e 00 c0 b0 3e 00 d0 48 3f b5 e9 13 3f 00 00 00 00 00 d0 48 3f" },
    { "RGBA8-LinSRGB", "RGB565", "18 44 ff ff 00 f8 52 3b 55 64 17 44 da 4c 65 d5" },
    { "RGBA8-LinSRGB", "ARGB1555", "00 00 ff ff 00 fc 4c 9d 8f 72 64 08 1e b3 c0 7e" },
    { "RGBA8-LinSRGB", "Gray1", "00 80 80 00 00 00 00 80" },
    { "RGBA8-LinSRGB", "Gray4", "40 f0 80 40 60 40 60 a0" },
    { "RGBA8-LinSRGB", "GrayF32", "00 00 80 3e 00 00 80 3f 00 50 02 3f 30 3c 8c 3e 58 93 bb 3e 00 51 7f 3e d4 88 d4 3e b8 db 2b 3f" },
    { "RGBA8-LinSRGB", "RGBA8-P3", "00 00 00 00 ff ff ff ff ea 33 23 ff 3d 4f 5f 80 dc ab 81 40 0f 15 1b 01 7c c4 ef 58 f5 bf 41 c8" },
    { "RGBA8-LinSRGB", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA8-LinSRGB", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff db 00 00 ff 42 50 60 80 d5 a7 7c 40 15 1c 21 01 88 c5 f2 58 ee ba 2c c8" },
    { "RGBA8-LinSRGB", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff fb 00 07 ff 2b 3e 4e 80 dd 97 67 40 0c 11 16 01 53 ba f0 58 fd af 09 c8" },
    { "RGBA8-LinSRGB", "Gray8-Generic18", "40 ff 6f 3e 5a 40 65 a2" },
    { "RGBA8-Adobe", "RGBA8", "00 00 00 00 ff ff ff ff ff 00 00 ff 05 0f 18 80 e3 64 2a 40 01 02 02 01 00 92 e9 58 ff 81 00 c8" },
    { "RGBA8-Adobe", "RGBA8P", "00 00 00 00 ff ff ff ff ff 00 00 ff 03 08 0c 80 39 19 0b 40 00 00 00 01 00 32 50 58 c8 65 00 c8" },
    { "RGBA8-Adobe", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 00 00 80 05 0f 18 40 e3 64 2a 01 01 02 02 58 00 92 e9 c8 ff 81 00" },
    { "RGBA8-Adobe", "BGRA8P", "00 00 00 00 ff ff ff ff 00 00 ff ff 0c 08 03 80 0b 19 39 40 00 00 00 01 50 32 00 58 00 65 c8 c8" },
    { "RGBA8-Adobe", "RGB888", "40 80 bf ff ff ff ff 00 00 22 47 6b 69 79 9a 40 80 be 2a 86 cd d6 81 29" },
    { "RGBA8-Adobe", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 00 00 ff 22 47 6b ff 69 79 9a ff 40 80 be ff 2a 86 cd ff d6 81 29" },
    { "RGBA8-Adobe", "RGBx8", "40 80 bf ff ff ff ff ff ff 00 00 ff 22 47 6b ff 69 79 9a ff 40 80 be ff 2a 86 cd ff d6 81 29 ff" },
    { "RGBA8-Adobe", "Gray8", "40 ff 97 28 53 40 58 9a" },
    { "RGBA8-Adobe", "GrayAlpha88", "00 00 ff ff 97 ff 0e 80 8c 40 02 01 85 58 b3 c8" },
    { "RGBA8-Adobe", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 04 e0 0f 50 18 50 80 7f e3 ef 64 60 2a 70 40 40 00 d0 01 a0 02 70 01 00 00 00 92 df ea 5f 58 60 ff ff 81 7f 00 00 c8 cf" },
    { "RGBA8-Adobe", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff e0 04 50 0f 50 18 7f 80 ef e3 60 64 70 2a 40 40 d0 00 a0 01 70 02 00 01 00 00 df 92 5f ea 60 58 ff ff 7f 81 00 00 cf c8" },
    { "RGBA8-Adobe", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 00 00 00 00 ff ff 04 e0 0f 50 18 50 80 7f e3 ef 64 60 2a 70 40 40 00 d0 01 a0 02 70 01 00 00 00 92 df ea 5f 58 60 ff ff 81 7f 00 00 c8 cf" },
    { "RGBA8-Adobe", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c a2 3c 00 00 00 00 00 3c e0 24 a8 2b 14 2e 04 38 20 3b 46 36 4e 31 04 34 80 1a 80 1e e0 20 00 1c 9f b5 97 38 53 3b 86 35 81 3c 0c 38 30 ae 46 3a" },
    { "RGBA8-Adobe", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 40 94 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 9c 3c 00 00 75 3d 00 80 c2 3d 00 80 00 3f 00 f0 63 3f 00 c0 c8 3e 00 c0 29 3e 00 80 80 3e 00 00 50 3b 00 00 d0 3b 00 00 1c 3c 00 00 80 3b 00 e0 b3 be 00 e0 12 3f 00 60 6a 3f 00 c0 b0 3e 00 20 90 3f 00 80 01 3f 00 00 c6 bd 00 d0 48 3f" },
    { "RGBA8-Adobe", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 40 94 3f 00 00 00 00 00 00 00 00 00 00 80 3f 00 9c 1c 3c 00 f5 f5 3c 80 42 43 3d 00 80 00 3f f0 d3 64 3e c0 88 c9 3d c0 69 2a 3d 00 80 80 3e 00 00 50 37 00 00 d0 37 00 00 1c 38 00 00 80 3b d0 61 f8 bd 50 d0 4a 3e c8 d1 a1 3e 00 c0 b0 3e 34 1c 62 3f 70 2a cb 3e e0 50 9b bd 00 d0 48 3f" },
    { "RGBA8-Adobe", "RGB565", "17 44 ff ff 00 f8 4d 2a b2 63 17 44 59 34 e5 d3" },
    { "RGBA8-Adobe", "ARGB1555", "00 00 ff ff 00 fc 43 84 85 6d 00 00 5d 82 e0 7d" },
    { "RGBA8-Adobe", "Gray1", "00 80 00 00 00 00 00 00" },
    { "RGBA8-Adobe", "Gray4", "40 f0 90 20 50 40 50 90" },
    { "RGBA8-Adobe", "GrayF32", "00 00 80 3e 00 00 80 3f 00 f0 17 3f 20 bc 1b 3e 70 96 a6 3e 80 06 7f 3e 14 f1 af 3e 31 74 1a 3f" },
    { "RGBA8-Adobe", "RGBA8-P3", "00 00 00 00 ff ff ff ff ff 3d 2b ff 06 0f 17 80 d3 6c 3b 40 01 02 02 01 00 8f e3 58 ff 8a 30 c8" },
    { "RGBA8-Adobe", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 00 01 02 80 c4 20 06 40 00 00 00 01 00 4a d1 58 ff 38 00 c8" },
    { "RGBA8-Adobe", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA8-Adobe", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff ff 00 09 ff 06 0d 13 80 da 4e 21 40 01 02 03 01 00 7d e4 58 ff 6a 00 c8" },
    { "RGBA8-Adobe", "Gray8-Generic18", "40 ff 85 27 4f 40 51 8e" },
    { "RGBA8-GenericRGB", "RGBA8", "00 00 00 00 ff ff ff ff ff 26 00 ff 09 1a 28 80 d4 79 40 40 01 02 02 01 20 a4 eb 58 ff 93 00 c8" },
    { "RGBA8-GenericRGB", "RGBA8P", "00 00 00 00 ff ff ff ff ff 26 00 ff 05 0d 14 80 35 1e 10 40 00 00 00 01 0b 39 51 58 c8 73 00 c8" },
    { "RGBA8-GenericRGB", "ARGB8", "00 00 00 00 ff ff ff ff ff ff 26 00 80 09 1a 28 40 d4 79 40 01 01 02 02 58 20 a4 eb c8 ff 93 00" },
    { "RGBA8-GenericRGB", "BGRA8P", "00 00 00 00 ff ff ff ff 00 26 ff ff 14 0d 05 80 10 1e 35 40 00 00 00 01 51 39 0b 58 00 73 c8 c8" },
    { "RGBA8-GenericRGB", "RGB888", "40 80 bf ff ff ff ff 26 00 24 4d 73 65 7e 9f 40 80 be 35 8c ce d6 8f 29" },
    { "RGBA8-GenericRGB", "xRGB8", "ff 40 80 bf ff ff ff ff ff ff 26 00 ff 24 4d 73 ff 65 7e 9f ff 40 80 be ff 35 8c ce ff d6 8f 29" },
    { "RGBA8-GenericRGB", "RGBx8", "40 80 bf ff ff ff ff ff ff 26 00 ff 24 4d 73 ff 65 7e 9f ff 40 80 be ff 35 8c ce ff d6 8f 29 ff" },
    { "RGBA8-GenericRGB", "Gray8", "40 ff 87 2d 55 40 5f 98" },
    { "RGBA8-GenericRGB", "GrayAlpha88", "00 00 ff ff 87 ff 19 80 92 40 02 01 99 58 b0 c8" },
    { "RGBA8-GenericRGB", "RGBA16", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 26 50 00 00 ff ff 08 e0 1a 60 28 30 80 7f d4 af 79 30 40 b0 40 40 00 d0 01 a0 02 70 01 00 20 10 a4 8f ec 2f 58 60 ff ff 93 ff 00 00 c8 cf" },
    { "RGBA8-GenericRGB", "RGBA16L", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 50 26 00 00 ff ff e0 08 60 1a 30 28 7f 80 af d4 30 79 b0 40 40 40 d0 00 a0 01 70 02 00 01 10 20 8f a4 2f ec 60 58 ff ff ff 93 00 00 cf c8" },
    { "RGBA8-GenericRGB", "RGBA16B", "00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff 26 50 00 00 ff ff 08 e0 1a 60 28 30 80 7f d4 af 79 30 40 b0 40 40 00 d0 01 a0 02 70 01 00 20 10 a4 8f ec 2f 58 60 ff ff 93 ff 00 00 c8 cf" },
    { "RGBA8-GenericRGB", "RGBA16F", "00 00 00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 0b 3c ca 30 a0 a5 00 3c 70 28 98 2e 06 31 04 38 a6 3a 93 37 0b 34 04 34 80 1a 80 1e e0 20 00 1c 02 30 24 39 62 3b 86 35 08 3c a0 38 40 a7 46 3a" },
    { "RGBA8-GenericRGB", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 68 81 3f 00 40 19 3e 00 00 b4 bc 00 00 80 3f 00 00 0e 3d 00 00 d3 3d 00 c0 20 3e 00 80 00 3f 00 b0 54 3f 00 60 f2 3e 00 60 81 3e 00 80 80 3e 00 00 50 3b 00 00 d0 3b 00 00 1c 3c 00 00 80 3b 00 40 00 3e 00 90 24 3f 00 30 6c 3f 00 c0 b0 3e 00 f8 80 3f 00 00 14 3f 00 00 e8 bc 00 d0 48 3f" },
    { "RGBA8-GenericRGB", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 68 81 3f 00 40 19 3e 00 00 b4 bc 00 00 80 3f 00 8e 8e 3c 00 d3 53 3d c0 60 a1 3d 00 80 00 3f b0 84 55 3e 60 52 f3 3d 60 e1 81 3d 00 80 80 3e 00 00 50 37 00 00 d0 37 00 00 1c 38 00 00 80 3b 60 18 31 3d d8 3c 63 3e 24 12 a3 3e 00 c0 b0 3e 13 55 4a 3f 80 30 e8 3e 80 fc b5 bc 00 d0 48 3f" },
    { "RGBA8-GenericRGB", "RGB565", "17 44 ff ff 40 f9 6e 22 f3 63 17 44 79 3c 85 d4" },
    { "RGBA8-GenericRGB", "ARGB1555", "00 00 ff ff 80 fc 65 84 c7 65 00 00 9c 12 40 fe" },
    { "RGBA8-GenericRGB", "Gray1", "00 80 80 00 80 00 00 80" },
    { "RGBA8-GenericRGB", "Gray4", "40 f0 80 30 50 40 60 90" },
    { "RGBA8-GenericRGB", "GrayF32", "00 00 80 3e 00 00 80 3f 00 70 07 3f 80 31 31 3e 10 39 a9 3e 80 06 7f 3e 5c 02 be 3e 77 c9 18 3f" },
    { "RGBA8-GenericRGB", "RGBA8-P3", "00 00 00 00 ff ff ff ff ed 41 24 ff 0d 1a 27 80 c8 7d 4c 40 01 02 02 01 4f a1 e5 58 f2 99 36 c8" },
    { "RGBA8-GenericRGB", "RGBA8-LinSRGB", "00 00 00 00 ff ff ff ff ff 05 00 ff 01 03 05 80 a8 30 0d 40 00 00 00 01 04 5f d4 58 ff 4b 00 c8" },
    { "RGBA8-GenericRGB", "RGBA8-Adobe", "00 00 00 00 ff ff ff ff de 2b 00 ff 14 20 2c 80 be 78 46 40 01 02 03 01 60 a2 e8 58 e8 92 1f c8" },
    { "RGBA8-GenericRGB", "RGBA8-GenericRGB", "00 00 00 00 ff ff ff ff ff 00 00 ff 0a 14 1e 80 c8 64 32 40 01 02 03 01 1f 91 e6 58 ff 80 00 c8" },
    { "RGBA8-GenericRGB", "Gray8-Generic18", "40 ff 74 2a 50 40 59 8c" },
    { "Gray8-Generic18", "RGBA8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff ff" },
    { "Gray8-Generic18", "ARGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff" },
    { "Gray8-Generic18", "BGRA8P", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff ff" },
    { "Gray8-Generic18", "RGB888", "00 00 00 ff ff ff ff ff ff 0a 0a 0a d2 d2 d2 01 01 01 29 29 29 ff ff ff" },
    { "Gray8-Generic18", "xRGB8", "ff 00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff" },
    { "Gray8-Generic18", "RGBx8", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff ff" },
    { "Gray8-Generic18", "Gray8", "00 ff ff 0a d2 01 29 ff" },
    { "Gray8-Generic18", "GrayAlpha88", "00 ff ff ff ff ff 0a ff d2 ff 01 ff 29 ff ff ff" },
    { "Gray8-Generic18", "RGBA16", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 09 b0 09 b0 09 b0 ff ff d2 ff d2 ff d2 ff ff ff 00 d0 00 d0 00 d0 ff ff 29 70 29 70 29 70 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA16L", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff b0 09 b0 09 b0 09 ff ff ff d2 ff d2 ff d2 ff ff d0 00 d0 00 d0 00 ff ff 70 29 70 29 70 29 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA16B", "00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 09 b0 09 b0 09 b0 ff ff d2 ff d2 ff d2 ff ff ff 00 d0 00 d0 00 d0 ff ff 29 70 29 70 29 70 ff ff ff ff ff ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA16F", "00 00 00 00 00 00 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c 00 3c d8 28 d8 28 d8 28 00 3c 98 3a 98 3a 98 3a 00 3c 80 1a 80 1a 80 1a 00 3c 2e 31 2e 31 2e 31 00 3c 00 3c 00 3c 00 3c 00 3c" },
    { "Gray8-Generic18", "RGBAF32", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 1b 3d 00 00 1b 3d 00 00 1b 3d 00 00 80 3f 00 00 53 3f 00 00 53 3f 00 00 53 3f 00 00 80 3f 00 00 50 3b 00 00 50 3b 00 00 50 3b 00 00 80 3f 00 c0 25 3e 00 c0 25 3e 00 c0 25 3e 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray8-Generic18", "RGBAF32P", "00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 1b 3d 00 00 1b 3d 00 00 1b 3d 00 00 80 3f 00 00 53 3f 00 00 53 3f 00 00 53 3f 00 00 80 3f 00 00 50 3b 00 00 50 3b 00 00 50 3b 00 00 80 3f 00 c0 25 3e 00 c0 25 3e 00 c0 25 3e 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00 80 3f" },
    { "Gray8-Generic18", "RGB565", "00 00 ff ff ff ff 62 10 99 ce 00 00 65 29 ff ff" },
    { "Gray8-Generic18", "ARGB1555", "00 80 ff ff ff ff 21 84 5a eb 00 80 a5 94 ff ff" },
    { "Gray8-Generic18", "Gray1", "00 80 80 00 80 00 00 80" },
    { "Gray8-Generic18", "Gray4", "00 f0 f0 10 c0 00 20 f0" },
    { "Gray8-Generic18", "GrayF32", "00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 1b 3d 00 00 53 3f 00 00 50 3b 00 c0 25 3e 00 00 80 3f" },
    { "Gray8-Generic18", "RGBA8-P3", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff d2 d2 d2 ff 01 01 01 ff 29 29 29 ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA8-LinSRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 01 01 01 ff a5 a5 a5 ff 00 00 00 ff 06 06 06 ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA8-Adobe", "00 00 00 ff ff ff ff ff ff ff ff ff 0c 0c 0c ff d1 d1 d1 ff 01 01 01 ff 2d 2d 2d ff ff ff ff ff" },
    { "Gray8-Generic18", "RGBA8-GenericRGB", "00 00 00 ff ff ff ff ff ff ff ff ff 0a 0a 0a ff c8 c8 c8 ff 01 01 01 ff 1f 1f 1f ff ff ff ff ff" },
    { "Gray8-Generic18", "Gray8-Generic18", "00 ff ff 0a c8 01 1f ff" },
};

static void testConversions(void) {
    CGColorSpaceRef gray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2), genGray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray), p3 = CGColorSpaceCreateWithName(kCGColorSpaceDisplayP3),
                    lin = CGColorSpaceCreateWithName(kCGColorSpaceLinearSRGB), adobe = CGColorSpaceCreateWithName(kCGColorSpaceAdobeRGB1998), genRGB = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    Fmt F[] = {
        { "RGBA8", mk(8, 32, NULL, kCGImageAlphaLast) }, { "RGBA8P", mk(8, 32, NULL, kCGImageAlphaPremultipliedLast) }, { "ARGB8", mk(8, 32, NULL, kCGImageAlphaFirst) },
        { "BGRA8P", mk(8, 32, NULL, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little) }, { "RGB888", mk(8, 24, NULL, kCGImageAlphaNone) },
        { "xRGB8", mk(8, 32, NULL, kCGImageAlphaNoneSkipFirst) }, { "RGBx8", mk(8, 32, NULL, kCGImageAlphaNoneSkipLast) }, { "Gray8", mk(8, 8, gray, kCGImageAlphaNone) },
        { "GrayAlpha88", mk(8, 16, gray, kCGImageAlphaLast) }, { "RGBA16", mk(16, 64, NULL, kCGImageAlphaLast) },
        { "RGBA16L", mk(16, 64, NULL, kCGImageAlphaLast | kCGBitmapByteOrder16Little) }, { "RGBA16B", mk(16, 64, NULL, kCGImageAlphaLast | kCGBitmapByteOrder16Big) },
        { "RGBA16F", mk(16, 64, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder16Little) },
        { "RGBAF32", mk(32, 128, NULL, kCGImageAlphaLast | FLOAT_COMPONENTS | kCGBitmapByteOrder32Little) },
        { "RGBAF32P", mk(32, 128, NULL, kCGImageAlphaPremultipliedLast | FLOAT_COMPONENTS | kCGBitmapByteOrder32Little) },
        { "RGB565", mk(5, 16, NULL, kCGImageAlphaNone | kCGBitmapByteOrder16Little) }, { "ARGB1555", mk(5, 16, NULL, kCGImageAlphaFirst | kCGBitmapByteOrder16Little) },
        { "Gray1", mk(1, 1, gray, kCGImageAlphaNone) }, { "Gray4", mk(4, 4, gray, kCGImageAlphaNone) }, { "GrayF32", mk(32, 32, gray, kCGImageAlphaNone | FLOAT_COMPONENTS | kCGBitmapByteOrder32Little) },
        { "RGBA8-P3", mk(8, 32, p3, kCGImageAlphaLast) }, { "RGBA8-LinSRGB", mk(8, 32, lin, kCGImageAlphaLast) }, { "RGBA8-Adobe", mk(8, 32, adobe, kCGImageAlphaLast) },
        { "RGBA8-GenericRGB", mk(8, 32, genRGB, kCGImageAlphaLast) }, { "Gray8-Generic18", mk(8, 8, genGray, kCGImageAlphaNone) },
    };
    size_t nf = sizeof F / sizeof F[0];
    CGFloat bg[4] = { 0.25, 0.5, 0.75, 0 };
    int compared = 0;
    for (size_t t = 0; t < sizeof conversions / sizeof conversions[0]; t++) {
        const Fmt *s = NULL, *d = NULL;
        for (size_t i = 0; i < nf; i++) { if (!strcmp(F[i].name, conversions[t].src)) s = &F[i]; if (!strcmp(F[i].name, conversions[t].dst)) d = &F[i]; }
        if (!s || !d) { EXPECT(0, "unknown format in table: %s -> %s", conversions[t].src, conversions[t].dst); continue; }
        uint8_t srcData[8 * 16], px[16];
        memset(srcData, 0, sizeof srcData);
        size_t sp = 1;
        for (int i = 0; i < 8; i++) {
            size_t n = pack(&s->f, base[i], px);
            sp = n ? n : 1;
            memcpy(srcData + i * sp, px, sp);
        }
        vImage_Error err = 0;
        vImageConverterRef cv = vImageConverter_CreateWithCGImageFormat(&s->f, &d->f, bg, kvImageNoFlags, &err);
        if (!cv) { EXPECT(0, "%s -> %s: converter not created (%zd)", s->name, d->name, err); continue; }
        uint8_t dst[8 * 16 + 16];
        memset(dst, 0xEE, sizeof dst);
        size_t dp = d->f.bitsPerPixel < 8 ? 1 : d->f.bitsPerPixel / 8;
        vImage_Buffer sb = { srcData, 8, 1, sp }, db = { dst, 8, 1, dp };
        err = vImageConvert_AnyToAny(cv, &sb, &db, NULL, kvImageNoFlags);
        vImageConverter_Release(cv);
        EXPECT(err == kvImageNoError, "%s -> %s: error %zd", s->name, d->name, err);
        const char *p = conversions[t].bytes;
        int bad = 0;
        for (int i = 0; i < 8 && !bad; i++) {
            uint8_t want[16];
            for (size_t k = 0; k < dp; k++) { int v = hexByte(&p); want[k] = (uint8_t)v; }
            if (!closeEnough(s, d, i, dst + i * dp, want, dp)) {
                bad = 1;
                printf("FAIL %s -> %s pixel %d: got", s->name, d->name, i);
                for (size_t k = 0; k < dp; k++) printf(" %02x", dst[i * dp + k]);
                printf(", expected");
                for (size_t k = 0; k < dp; k++) printf(" %02x", want[k]);
                printf("\n");
            }
        }
        checks++;
        failures += bad;
        compared++;
    }
    printf("%d conversions compared\n", compared);
    EXPECT(compared > 600, "the table was compared");
    CGColorSpaceRelease(gray); CGColorSpaceRelease(genGray); CGColorSpaceRelease(p3); CGColorSpaceRelease(lin); CGColorSpaceRelease(adobe); CGColorSpaceRelease(genRGB);
}

// ---- vImageBuffer_InitWithCGImage and vImageCreateCGImageFromBuffer

static int gReleased;
static void *gReleasedUser;
static void onRelease(void *userData, void *buf) { gReleased++; gReleasedUser = userData; free(buf); }

static CGImageRef makeImage(void) {
    static const uint8_t px[2][3][4] = { { {255,0,0,255}, {0,128,0,128}, {10,20,30,64} }, { {255,255,255,255}, {0,0,0,0}, {100,50,25,200} } };
    CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    uint8_t *data = malloc(sizeof px);
    memcpy(data, px, sizeof px);
    CGDataProviderRef prov = CGDataProviderCreateWithData(NULL, data, sizeof px, NULL);
    CGImageRef img = CGImageCreate(3, 2, 8, 32, 12, cs, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault, prov, NULL, false, kCGRenderingIntentDefault);
    CGDataProviderRelease(prov);
    CGColorSpaceRelease(cs);
    return img;
}

static int bytesNear(const uint8_t *a, const uint8_t *b, size_t n, int tol) {
    for (size_t i = 0; i < n; i++) if (abs((int)a[i] - (int)b[i]) > tol) return 0;
    return 1;
}

static void testCGImage(void) {
    CGImageRef img = makeImage();
    EXPECT(img != NULL && CGImageGetWidth(img) == 3 && CGImageGetHeight(img) == 2, "test image");
    CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB), gray = CGColorSpaceCreateWithName(kCGColorSpaceGenericGrayGamma2_2);
    CGFloat bg[3] = { 0.25, 0.5, 0.75 };
    struct { const char *n; vImage_CGImageFormat f; size_t rowBytes; size_t pixelBytes; const char *row0, *row1; int tol; } T[] = {
        { "RGBA8P", mk(8, 32, srgb, kCGImageAlphaPremultipliedLast), 12, 4, "ff 00 00 ff 00 80 00 80 0a 14 1e 40", "ff ff ff ff 00 00 00 00 64 32 19 c8", 0 },
        { "RGBA8", mk(8, 32, srgb, kCGImageAlphaLast), 16, 4, "ff 00 00 ff 00 ff 00 80 28 50 78 40", "ff ff ff ff 00 00 00 00 80 40 20 c8", 1 },
        { "ARGB8", mk(8, 32, srgb, kCGImageAlphaFirst), 16, 4, "ff ff 00 00 80 00 ff 00 40 28 50 78", "ff ff ff ff 00 00 00 00 c8 80 40 20", 1 },
        { "BGRA8P", mk(8, 32, srgb, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little), 16, 4, "00 00 ff ff 00 80 00 80 1e 14 0a 40", "ff ff ff ff 00 00 00 00 19 32 64 c8", 0 },
        { "RGB888", mk(8, 24, srgb, kCGImageAlphaNone), 16, 3, "ff 00 00 20 c0 5f 3a 74 ad", "ff ff ff 40 80 bf 72 4e 42", 1 },
        { "Gray8", mk(8, 8, gray, kCGImageAlphaNone), 16, 1, "82 8e 43", "ff 40 4f", 1 },
        { "RGBA16", mk(16, 64, srgb, kCGImageAlphaLast | kCGBitmapByteOrder16Little), 32, 8, "ff ff 00 00 00 00 ff ff 00 00 ff ff 00 00 80 80 00 28 00 50 00 78 40 40", "ff ff ff ff ff ff ff ff 00 00 00 00 00 00 00 00 00 80 00 40 00 20 c8 c8", 1 },
        { "RGBA8 NULL colour space", mk(8, 32, NULL, kCGImageAlphaLast), 16, 4, "ff 00 00 ff 00 ff 00 80 28 50 78 40", "ff ff ff ff 00 00 00 00 80 40 20 c8", 1 },
    };
    for (size_t t = 0; t < sizeof T / sizeof T[0]; t++) {
        vImage_Buffer b;
        memset(&b, 0, sizeof b);
        vImage_Error e = vImageBuffer_InitWithCGImage(&b, &T[t].f, bg, img, kvImageNoFlags);
        EXPECT(e == 0 && b.width == 3 && b.height == 2 && b.rowBytes == T[t].rowBytes && b.data != NULL, "InitWithCGImage %s: %zd %lux%lu rowBytes %zu", T[t].n, e, b.width, b.height, b.rowBytes);
        if (e == 0 && b.data) {
            uint8_t want[32];
            const char *p = T[t].row0;
            for (size_t i = 0; i < 3 * T[t].pixelBytes; i++) want[i] = (uint8_t)hexByte(&p);
            EXPECT(bytesNear(b.data, want, 3 * T[t].pixelBytes, T[t].tol), "InitWithCGImage %s row 0", T[t].n);
            p = T[t].row1;
            for (size_t i = 0; i < 3 * T[t].pixelBytes; i++) want[i] = (uint8_t)hexByte(&p);
            EXPECT(bytesNear((uint8_t *)b.data + b.rowBytes, want, 3 * T[t].pixelBytes, T[t].tol), "InitWithCGImage %s row 1", T[t].n);
            free(b.data);
        }
    }
    {   // kvImageNoAllocate: the caller's buffer and rowBytes are used as they are
        uint8_t store[2 * 32];
        memset(store, 0xEE, sizeof store);
        vImage_Buffer b = { store, 0, 0, 32 };
        vImage_CGImageFormat f = T[0].f;
        vImage_Error e = vImageBuffer_InitWithCGImage(&b, &f, NULL, img, kvImageNoAllocate);
        static const uint8_t r0[12] = { 0xff, 0, 0, 0xff, 0, 0x80, 0, 0x80, 0x0a, 0x14, 0x1e, 0x40 };
        EXPECT(e == 0 && b.data == store && b.rowBytes == 32 && b.width == 3 && b.height == 2 && memcmp(store, r0, 12) == 0 && store[12] == 0xEE && store[32] == 0xff, "InitWithCGImage NoAllocate: %zd", e);
    }
    {
        vImage_Buffer b;
        vImage_CGImageFormat f = T[0].f, g;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &f, NULL, img, kvImageDoNotTile) == kvImageUnknownFlagsBit, "InitWithCGImage rejects kvImageDoNotTile");
        EXPECT(vImageBuffer_InitWithCGImage(&b, &f, NULL, img, kvImageHighQualityResampling) == kvImageUnknownFlagsBit, "InitWithCGImage rejects kvImageHighQualityResampling");
        EXPECT(vImageBuffer_InitWithCGImage(&b, &f, NULL, img, 0x10000) == kvImageUnknownFlagsBit && b.data == NULL, "InitWithCGImage rejects an unknown flag");
        vImage_Error e = vImageBuffer_InitWithCGImage(&b, &f, NULL, img, kvImagePrintDiagnosticsToConsole);
        EXPECT(e == 0, "InitWithCGImage accepts kvImagePrintDiagnosticsToConsole"); if (e == 0) free(b.data);
        g = f; g.bitmapInfo |= 0x40000;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidParameter, "unknown bitmapInfo bit");
        g = f; g.bitmapInfo = 15;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidParameter, "alpha value 15");
        g = f; g.bitsPerComponent = 3;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidParameter, "3 bits per component");
        g = f; g.bitsPerComponent = 12;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidImageFormat, "12 bits per component");
        g = f; g.version = 2;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidImageFormat, "version 2");
        g = f; g.bitmapInfo = kCGImageAlphaOnly; g.bitsPerPixel = 8;
        EXPECT(vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0) == kvImageInvalidImageFormat, "alpha only");
        CGFloat dec[6] = { 0, 1, 0, 1, 0, 1 };
        g = f; g.decode = dec;
        e = vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0);
        EXPECT(e == 0, "a decode array is accepted"); if (e == 0) free(b.data);
        g = f; g.renderingIntent = (CGColorRenderingIntent)99;
        e = vImageBuffer_InitWithCGImage(&b, &g, NULL, img, 0);
        EXPECT(e == 0, "an unknown rendering intent is accepted"); if (e == 0) free(b.data);
    }
    {   // CGImage from a buffer: a copy (the buffer may change), read back
        uint8_t px[2 * 16];
        static const uint8_t src[2][3][4] = { { {255,0,0,255}, {0,128,0,128}, {10,20,30,64} }, { {255,255,255,255}, {0,0,0,0}, {100,50,25,200} } };
        for (int y = 0; y < 2; y++) { memset(px + y * 16, 0, 16); memcpy(px + y * 16, src[y], 12); }
        vImage_Buffer b = { px, 2, 3, 16 };
        vImage_Error e = 123;
        CGImageRef c = vImageCreateCGImageFromBuffer(&b, &T[0].f, NULL, NULL, kvImageNoFlags, &e);
        EXPECT(c != NULL && e == 0 && CGImageGetWidth(c) == 3 && CGImageGetHeight(c) == 2, "CreateCGImageFromBuffer: %zd", e);
        memset(px, 0x11, sizeof px);
        if (c) {
            vImage_Buffer back;
            vImage_CGImageFormat rf = T[0].f;
            e = vImageBuffer_InitWithCGImage(&back, &rf, NULL, c, 0);
            static const uint8_t r0[12] = { 0xff, 0, 0, 0xff, 0, 0x80, 0, 0x80, 0x0a, 0x14, 0x1e, 0x40 };
            EXPECT(e == 0 && memcmp(back.data, r0, 12) == 0, "the image has the buffer's pixels as they were");
            if (e == 0) free(back.data);
            CGImageRelease(c);
        }
        for (int y = 0; y < 2; y++) memcpy(px + y * 16, src[y], 12);
        c = vImageCreateCGImageFromBuffer(&b, &T[0].f, NULL, NULL, kvImageNoFlags, NULL);
        EXPECT(c != NULL, "no error pointer"); if (c) CGImageRelease(c);
        // buffers in the other formats make images too
        for (size_t t = 0; t < sizeof T / sizeof T[0]; t++) {
            vImage_Buffer fb;
            vImage_CGImageFormat f = T[t].f;
            if (vImageBuffer_InitWithCGImage(&fb, &f, bg, img, 0) != 0)
                continue;
            vImage_Error ee = 0;
            c = vImageCreateCGImageFromBuffer(&fb, &f, NULL, NULL, 0, &ee);
            EXPECT(c != NULL && ee == 0, "CreateCGImageFromBuffer %s: %zd", T[t].n, ee);
            if (c) CGImageRelease(c);
            free(fb.data);
        }
        // no copy: the image owns the memory and the callback gets it back when the image is released
        uint8_t *own = malloc(32);
        memcpy(own, px, 32);
        gReleased = 0;
        vImage_Buffer nb = { own, 2, 3, 16 };
        c = vImageCreateCGImageFromBuffer(&nb, &T[1].f, onRelease, (void *)0x1234, kvImageNoAllocate, &e);
        EXPECT(c != NULL && e == 0 && gReleased == 0, "NoAllocate image: %zd", e);
        if (c) {
            CGImageRelease(c);
            EXPECT(gReleased == 1 && gReleasedUser == (void *)0x1234, "the release callback ran once with its user data");
        } else {
            free(own);
        }
        vImage_CGImageFormat f = T[0].f, g;
        EXPECT(vImageCreateCGImageFromBuffer(&b, &f, NULL, NULL, 0x10000, &e) == NULL && e == kvImageUnknownFlagsBit, "unknown flag");
        c = vImageCreateCGImageFromBuffer(&b, &f, NULL, NULL, kvImageDoNotTile, &e);
        EXPECT(c != NULL && e == 0, "kvImageDoNotTile is accepted"); if (c) CGImageRelease(c);
        c = vImageCreateCGImageFromBuffer(&b, &f, NULL, NULL, kvImageHighQualityResampling, &e);
        EXPECT(c != NULL && e == 0, "kvImageHighQualityResampling is accepted"); if (c) CGImageRelease(c);
        g = f; g.bitsPerComponent = 3;
        EXPECT(vImageCreateCGImageFromBuffer(&b, &g, NULL, NULL, 0, &e) == NULL && e == kvImageInvalidImageFormat, "3 bits per component");
        g = f; g.renderingIntent = (CGColorRenderingIntent)99;
        EXPECT(vImageCreateCGImageFromBuffer(&b, &g, NULL, NULL, 0, &e) == NULL && e == kvImageInvalidImageFormat, "unknown rendering intent");
        g = f; g.colorSpace = NULL;
        c = vImageCreateCGImageFromBuffer(&b, &g, NULL, NULL, 0, &e);
        EXPECT(c != NULL && e == 0, "a NULL colour space is sRGB"); if (c) CGImageRelease(c);
    }
    CGImageRelease(img);
    CGColorSpaceRelease(srgb); CGColorSpaceRelease(gray);
}

int main(void) {
    testBufferUtilities();
    testConverterCreation();
    testAnyToAnyErrors();
    testConversions();
    testCGImage();
    printf("%d checks, %d failures\n", checks, failures);
    printf(failures ? "FAIL\n" : "PASS\n");
    return failures != 0;
}
