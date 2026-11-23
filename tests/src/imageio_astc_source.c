// CFLAGS: -framework ImageIO -framework CoreFoundation -framework CoreGraphics
// ARGS: /Volumes/SystemRoot$PWD/src/libtexturecodec/tests/vectors
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// CGImageSourceCreateWithData and CGImageSourceCreateWithURL on ASTC texture files (the .astc
// container, 2D LDR). The behaviour pinned here was observed on macOS 26.7.3 and 10.14.4 (the same
// on both unless noted; vd-upstream cristim-mac-evidence.md section 16):
//  - the type identifier is org.khronos.astc and CGImageSourceCopyTypeIdentifiers lists it;
//  - the image is RGBA8 (32 bits per pixel), kCGImageAlphaLast (bitmap info 3), sRGB colour space,
//    and its properties are ColorModel RGB, Depth 8, HasAlpha, Orientation 4, PixelWidth and
//    PixelHeight;
//  - data with the magic number and at least a header and one block is a source with one image and
//    status complete, also when no image can be made (3D, a footprint outside the specification,
//    zero size, missing blocks, a side of 1048576 or more); its image and properties are then NULL;
//    (macOS 26.7.3 gives an empty or orientation-only properties dictionary there, 10.14.4 NULL,
//    which is followed);
//    shorter data and data without the magic number give no ASTC source;
//  - an illegal block anywhere gives an image of the right size that draws nothing;
//  - the pixels of the astcenc vectors in src/libtexturecodec/tests/vectors are the astcenc output
//    for sRGB vectors; for two linear vectors, where astcenc differs by one in some bytes, they are
//    the 32-bit FNV-1a hashes of what macOS gives; and a void-extent colour is rounded to 8 bits
//    (0xC000 gives 0xBF), also with its reserved bits cleared.
// Usage: imageio_astc_source <vector directory>. Prints PASS, or the failures with a non-zero exit.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static CFDataRef readFile(const char *dir, const char *name, const char *suffix) {
    char path[1024];
    snprintf(path, sizeof path, "%s/%s%s", dir, name, suffix);
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        printf("FAIL: cannot open %s\n", path);
        failures++;
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *bytes = malloc(length);
    size_t got = fread(bytes, 1, length, file);
    fclose(file);
    CFDataRef data = got == (size_t)length ? CFDataCreate(NULL, bytes, length) : NULL;
    free(bytes);
    return data;
}

// A .astc header: magic, block footprint (bw x bh x bd), image size (w x h x d, 24 bits each).
static void putHeader(uint8_t *out, int bw, int bh, int bd, uint32_t w, uint32_t h, uint32_t d) {
    static const uint8_t magic[4] = { 0x13, 0xAB, 0xA1, 0x5C };
    memcpy(out, magic, 4);
    out[4] = bw; out[5] = bh; out[6] = bd;
    for (int i = 0; i < 3; i++) {
        out[7 + i] = w >> (8 * i);
        out[10 + i] = h >> (8 * i);
        out[13 + i] = d >> (8 * i);
    }
}

// A void-extent block (sections 23.22 and 23.23): block mode 0x1FC, LDR, the extent all ones
// (ignored), then R, G, B, A as 16 bits, little endian.
static void putVoidBlock(uint8_t *out, uint16_t r, uint16_t g, uint16_t b, uint16_t a) {
    static const uint8_t head[8] = { 0xFC, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    memcpy(out, head, 8);
    uint16_t c[4] = { r, g, b, a };
    for (int i = 0; i < 4; i++) {
        out[8 + 2 * i] = c[i] & 0xFF;
        out[9 + 2 * i] = c[i] >> 8;
    }
}

// The pixels of an RGBA8 image as a tightly packed width * height * 4 buffer, or NULL.
static CFDataRef copyPixels(CGImageRef image) {
    CFDataRef raw = CGDataProviderCopyData(CGImageGetDataProvider(image));
    size_t width = CGImageGetWidth(image), height = CGImageGetHeight(image);
    size_t stride = CGImageGetBytesPerRow(image);
    if (raw == NULL || CFDataGetLength(raw) < (CFIndex)(stride * height) || stride < width * 4) {
        if (raw != NULL)
            CFRelease(raw);
        return NULL;
    }
    CFMutableDataRef packed = CFDataCreateMutable(NULL, width * height * 4);
    CFDataSetLength(packed, width * height * 4);
    for (size_t y = 0; y < height; y++)
        memcpy(CFDataGetMutableBytePtr(packed) + y * width * 4, CFDataGetBytePtr(raw) + y * stride, width * 4);
    CFRelease(raw);
    return packed;
}

// The image the source makes must be RGBA8, sRGB, straight alpha, of the given size.
static CGImageRef checkedImage(CGImageSourceRef source, size_t width, size_t height) {
    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CHECK(image != NULL);
    if (image == NULL)
        return NULL;
    CHECK(CGImageGetWidth(image) == width);
    CHECK(CGImageGetHeight(image) == height);
    CHECK(CGImageGetBitsPerComponent(image) == 8);
    CHECK(CGImageGetBitsPerPixel(image) == 32);
    CHECK(CGImageGetAlphaInfo(image) == kCGImageAlphaLast);
    CHECK(CGImageGetBitmapInfo(image) == (CGBitmapInfo)kCGImageAlphaLast);
    CFStringRef name = CGColorSpaceCopyName(CGImageGetColorSpace(image));
    CHECK(name != NULL && CFEqual(name, kCGColorSpaceSRGB));
    if (name != NULL)
        CFRelease(name);
    return image;
}

static void checkVector(const char *dir, const char *name, size_t width, size_t height) {
    CFDataRef astc = readFile(dir, name, ".astc");
    CFDataRef expected = readFile(dir, name, ".rgba");
    if (astc == NULL || expected == NULL)
        return;

    CGImageSourceRef source = CGImageSourceCreateWithData(astc, NULL);
    CHECK(source != NULL);
    if (source != NULL) {
        CHECK(CGImageSourceGetCount(source) == 1);
        CHECK(CGImageSourceGetStatus(source) == kCGImageStatusComplete);
        CHECK(CGImageSourceGetType(source) != NULL && CFEqual(CGImageSourceGetType(source), CFSTR("org.khronos.astc")));
        CHECK(CGImageSourceCreateImageAtIndex(source, 1, NULL) == NULL);

        CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
        CHECK(properties != NULL);
        if (properties != NULL) {
            int value = 0;
            CFNumberRef number = CFDictionaryGetValue(properties, kCGImagePropertyPixelWidth);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && (size_t)value == width);
            number = CFDictionaryGetValue(properties, kCGImagePropertyPixelHeight);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && (size_t)value == height);
            number = CFDictionaryGetValue(properties, kCGImagePropertyDepth);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && value == 8);
            number = CFDictionaryGetValue(properties, kCGImagePropertyOrientation);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && value == 4);
            CFTypeRef alpha = CFDictionaryGetValue(properties, kCGImagePropertyHasAlpha);
            CHECK(alpha == kCFBooleanTrue);
            CFTypeRef model = CFDictionaryGetValue(properties, kCGImagePropertyColorModel);
            CHECK(model != NULL && CFEqual(model, kCGImagePropertyColorModelRGB));
            CHECK(CFDictionaryGetCount(properties) == 6);
            CFRelease(properties);
        }

        CGImageRef image = checkedImage(source, width, height);
        if (image != NULL) {
            CFDataRef pixels = copyPixels(image);
            CHECK(pixels != NULL);
            if (pixels != NULL) {
                CHECK(CFDataGetLength(pixels) == CFDataGetLength(expected));
                CHECK(CFDataGetLength(pixels) == CFDataGetLength(expected) &&
                      memcmp(CFDataGetBytePtr(pixels), CFDataGetBytePtr(expected), CFDataGetLength(expected)) == 0);
                CFRelease(pixels);
            }
            CGImageRelease(image);
        }
        CFRelease(source);
    }
    CFRelease(astc);
    CFRelease(expected);
}

// The same file through a URL gives the same image.
static void checkURL(const char *dir, const char *name, size_t width, size_t height) {
    char path[1024];
    snprintf(path, sizeof path, "%s/%s.astc", dir, name);
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), false);
    CGImageSourceRef source = CGImageSourceCreateWithURL(url, NULL);
    CHECK(source != NULL);
    if (source != NULL) {
        CGImageRef image = checkedImage(source, width, height);
        if (image != NULL)
            CGImageRelease(image);
        CFRelease(source);
    }
    CFRelease(url);
}

// One void-extent block, 4x4 texels, the colour r, g, b, a (16 bits each): the RGBA8 the image must have.
static void checkVoid(uint16_t r, uint16_t g, uint16_t b, uint16_t a, bool clearReserved,
                      uint8_t er, uint8_t eg, uint8_t eb, uint8_t ea) {
    uint8_t file[16 + 16];
    putHeader(file, 4, 4, 1, 4, 4, 1);
    putVoidBlock(file + 16, r, g, b, a);
    if (clearReserved)
        file[16 + 1] &= ~0x0C;  // bits 10 and 11 of the block: reserved, must be 1 in the specification
    CFDataRef data = CFDataCreate(NULL, file, sizeof file);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CHECK(source != NULL);
    if (source != NULL) {
        CGImageRef image = checkedImage(source, 4, 4);
        CFDataRef pixels = image != NULL ? copyPixels(image) : NULL;
        CHECK(pixels != NULL);
        if (pixels != NULL) {
            const uint8_t *p = CFDataGetBytePtr(pixels);
            for (int i = 0; i < 16; i++)
                CHECK(p[i * 4] == er && p[i * 4 + 1] == eg && p[i * 4 + 2] == eb && p[i * 4 + 3] == ea);
            CFRelease(pixels);
        }
        if (image != NULL)
            CGImageRelease(image);
        CFRelease(source);
    }
    CFRelease(data);
}

// The image drawn into a premultiplied RGBA8 sRGB bitmap gives the colour of an opaque void block
// (what the raw pixels say must also be what drawing shows).
static void checkDrawn(void) {
    uint8_t file[16 + 16];
    putHeader(file, 4, 4, 1, 4, 4, 1);
    putVoidBlock(file + 16, 0x1234, 0x5678, 0x9ABC, 0xFFFF);
    CFDataRef data = CFDataCreate(NULL, file, sizeof file);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CGImageRef image = source != NULL ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
    CHECK(image != NULL);
    if (image != NULL) {
        uint8_t pixels[4 * 4 * 4];
        memset(pixels, 0, sizeof pixels);
        CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        CGContextRef context = CGBitmapContextCreate(pixels, 4, 4, 8, 16, space, kCGImageAlphaPremultipliedLast);
        CGContextDrawImage(context, CGRectMake(0, 0, 4, 4), image);
        for (int i = 0; i < 16; i++)
            CHECK(pixels[i * 4] == 0x12 && pixels[i * 4 + 1] == 0x56 && pixels[i * 4 + 2] == 0x9A && pixels[i * 4 + 3] == 0xFF);
        CGContextRelease(context);
        CGColorSpaceRelease(space);
        CGImageRelease(image);
    }
    if (source != NULL)
        CFRelease(source);
    CFRelease(data);
}

static void checkVoidBlocks(void) {
    checkDrawn();
    // the colour is rounded to the nearest of 255 steps, and the alpha stays straight
    checkVoid(0x1234, 0x5678, 0x9ABC, 0x8000, false, 0x12, 0x56, 0x9A, 0x80);
    // 0xC000 is 191.25 of 255: 0xBF, not the top byte 0xC0; 0x0081 is 0.50 of one step: 0x01, 0x0080 0x00
    checkVoid(0xFFFF, 0x0080, 0x0081, 0xC000, false, 0xFF, 0x00, 0x01, 0xBF);
    // the reserved bits cleared do not matter
    checkVoid(0xFFFF, 0x0080, 0x0081, 0xC000, true, 0xFF, 0x00, 0x01, 0xBF);
}

// What a case has to give: no ASTC source (no source at all, or on macOS one of unknown type, count 0
// and status -4; either way no image); an ASTC source (type org.khronos.astc, count 1, status
// complete) with no image and no properties; with properties but no image; with both.
enum { NOT_ASTC, NO_IMAGE, PROPERTIES_ONLY, IMAGE };

static void checkCase(const char *what, const uint8_t *bytes, size_t length, int expect) {
    CFDataRef data = CFDataCreate(NULL, bytes, length);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    if (expect == NOT_ASTC) {
        if (source != NULL && (CGImageSourceGetType(source) != NULL || CGImageSourceGetCount(source) != 0 ||
                               CGImageSourceCreateImageAtIndex(source, 0, NULL) != NULL)) {
            printf("FAIL: %s: a source with a type, a count or an image\n", what);
            failures++;
        }
    } else if (source == NULL) {
        printf("FAIL: %s: source is NULL\n", what);
        failures++;
    } else {
        if (CGImageSourceGetType(source) == NULL || !CFEqual(CGImageSourceGetType(source), CFSTR("org.khronos.astc"))) {
            printf("FAIL: %s: type is not org.khronos.astc\n", what);
            failures++;
        }
        if (CGImageSourceGetCount(source) != 1) {
            printf("FAIL: %s: count is %zu\n", what, CGImageSourceGetCount(source));
            failures++;
        }
        if (CGImageSourceGetStatus(source) != kCGImageStatusComplete) {
            printf("FAIL: %s: status is %d\n", what, (int)CGImageSourceGetStatus(source));
            failures++;
        }
        CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        if ((image != NULL) != (expect == IMAGE)) {
            printf("FAIL: %s: image %s\n", what, image != NULL ? "was made" : "is NULL");
            failures++;
        }
        CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
        if ((properties != NULL) != (expect == IMAGE || expect == PROPERTIES_ONLY)) {
            printf("FAIL: %s: properties %s\n", what, properties != NULL ? "exist" : "are NULL");
            failures++;
        }
        if (properties != NULL)
            CFRelease(properties);
        if (image != NULL)
            CGImageRelease(image);
    }
    if (source != NULL)
        CFRelease(source);
    CFRelease(data);
}

// An image of w x h texels in blocks of bw x bh, every block the same void-extent block, as a file.
static void checkFile(const char *what, int bw, int bh, uint32_t w, uint32_t h, int expect) {
    size_t blocks = (size_t)((w + bw - 1) / bw) * ((h + bh - 1) / bh);
    size_t length = 16 + blocks * 16;
    uint8_t *file = malloc(length);
    if (file == NULL) {
        printf("FAIL: %s: out of memory\n", what);
        failures++;
        return;
    }
    putHeader(file, bw, bh, 1, w, h, 1);
    for (size_t i = 0; i < blocks; i++)
        putVoidBlock(file + 16 + i * 16, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase(what, file, length, expect);
    free(file);
}

// The image of a texture with an illegal block has the size of the header, draws nothing and
// is made for the whole file, wherever the block is.
static void checkIllegalBlock(const char *what, size_t badBlock) {
    uint8_t file[16 + 16 * 4];
    putHeader(file, 4, 4, 1, 8, 8, 1);
    for (int i = 0; i < 4; i++)
        putVoidBlock(file + 16 + 16 * i, 0x1000, 0x2000, 0x3000, 0xFFFF);
    memset(file + 16 + 16 * badBlock, 0, 16);   // block mode 0: reserved
    checkCase(what, file, sizeof file, IMAGE);
    CFDataRef data = CFDataCreate(NULL, file, sizeof file);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CGImageRef image = source != NULL ? checkedImage(source, 8, 8) : NULL;
    CFDataRef pixels = image != NULL ? copyPixels(image) : NULL;
    if (pixels != NULL) {
        bool allZero = true;
        for (CFIndex i = 0; i < CFDataGetLength(pixels); i++)
            if (CFDataGetBytePtr(pixels)[i] != 0)
                allZero = false;
        if (!allZero) {
            printf("FAIL: %s: pixels are not all transparent black\n", what);
            failures++;
        }
        CFRelease(pixels);
    }
    if (image != NULL)
        CGImageRelease(image);
    if (source != NULL)
        CFRelease(source);
    CFRelease(data);
}

static void checkCases(void) {
    uint8_t file[16 + 16 * 4];
    memset(file, 0, sizeof file);

    // the header only is no source; with one block it is, and needs all the blocks it asks for
    putHeader(file, 4, 4, 1, 8, 8, 1);
    checkCase("no blocks", file, 16, NOT_ASTC);
    checkCase("a header and one byte", file, 17, NOT_ASTC);
    checkCase("a header and 15 bytes", file, 31, NOT_ASTC);
    putVoidBlock(file + 16, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase("one of four blocks", file, 32, NO_IMAGE);
    putVoidBlock(file + 32, 0x1000, 0x2000, 0x3000, 0xFFFF);
    putVoidBlock(file + 48, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase("three of four blocks", file, 16 + 3 * 16, NO_IMAGE);
    checkCase("three and a half of four blocks", file, 16 + 3 * 16 + 8, NO_IMAGE);
    putVoidBlock(file + 64, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase("all four blocks (control)", file, sizeof file, IMAGE);
    checkIllegalBlock("an illegal block first", 0);
    checkIllegalBlock("an illegal block in the middle", 2);
    checkIllegalBlock("an illegal block last", 3);

    // sides: 65537 and 524288 texels are images (10.14.4; 26.7.3 makes one that draws nothing from
    // 65536 up, the wider behaviour is followed), 1048576 is not
    checkFile("a wide image", 4, 4, 65537, 4, IMAGE);
    checkFile("at the side limit", 4, 4, 524288, 4, IMAGE);
    checkFile("over the side limit", 4, 4, 1048576, 4, PROPERTIES_ONLY);

    // the magic number with a header that is not a 2D one with a legal footprint: a source with no
    // image (count 1, complete, properties NULL)
    putHeader(file, 4, 4, 1, 4, 4, 1);
    checkCase("magic number only", file, 4, NOT_ASTC);
    checkCase("shorter than a header", file, 15, NOT_ASTC);
    putHeader(file, 7, 7, 1, 4, 4, 1);
    checkCase("footprint not in the specification", file, 32, NO_IMAGE);
    putHeader(file, 3, 3, 1, 4, 4, 1);
    checkCase("footprint 3x3", file, 32, NO_IMAGE);
    putHeader(file, 4, 4, 1, 0, 4, 1);
    checkCase("zero width", file, 32, NO_IMAGE);
    putHeader(file, 4, 4, 4, 4, 4, 4);
    checkCase("3D image", file, 32, NO_IMAGE);
    putHeader(file, 4, 4, 1, 4, 4, 2);
    checkCase("2D footprint, depth 2", file, 48, NO_IMAGE);

    // not an .astc file at all (no magic number): no source, the Onyx2D decoders do not know it either
    putHeader(file, 4, 4, 1, 4, 4, 1);
    file[0] ^= 1;
    checkCase("bad magic", file, 32, NOT_ASTC);
    checkCase("three bytes", file, 3, NOT_ASTC);
    checkCase("empty", file, 0, NOT_ASTC);
}

// 32-bit FNV-1a of the pixels macOS gives for these linear-mode vectors, where astcenc's own
// output differs by one in some bytes (the sRGB vectors are the same as astcenc's).
static uint32_t fnv32(const uint8_t *bytes, size_t length) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < length; i++)
        h = (h ^ bytes[i]) * 16777619u;
    return h;
}

static void checkHash(const char *dir, const char *name, uint32_t expected) {
    CFDataRef astc = readFile(dir, name, ".astc");
    if (astc == NULL)
        return;
    CGImageSourceRef source = CGImageSourceCreateWithData(astc, NULL);
    CGImageRef image = source != NULL ? CGImageSourceCreateImageAtIndex(source, 0, NULL) : NULL;
    CFDataRef pixels = image != NULL ? copyPixels(image) : NULL;
    CHECK(pixels != NULL);
    if (pixels != NULL) {
        uint32_t hash = fnv32(CFDataGetBytePtr(pixels), CFDataGetLength(pixels));
        if (hash != expected) {
            printf("FAIL: %s: pixel hash %08x, expected %08x\n", name, hash, expected);
            failures++;
        }
        CFRelease(pixels);
    }
    if (image != NULL)
        CGImageRelease(image);
    if (source != NULL)
        CFRelease(source);
    CFRelease(astc);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <directory of the astcenc vectors>\n", argv[0]);
        return 2;
    }
    // 30 x 26 texels, so each footprint ends in partial blocks on at least one axis
    checkVector(argv[1], "alpha_4x4_srgb_exh", 30, 26);
    checkVector(argv[1], "alpha_12x12_srgb_exh", 30, 26);
    checkVector(argv[1], "grayalpha_8x6_srgb_exh", 30, 26);
    checkURL(argv[1], "alpha_6x5_srgb_exh", 30, 26);
    checkHash(argv[1], "noise_12x12_lin_fast", 0x8069a871);
    checkHash(argv[1], "normal_8x6_lin_exh", 0x7df22074);
    checkHash(argv[1], "blobs_4x4_lin_exh", 0x463826f3);
    checkVoidBlocks();
    checkCases();

    CFArrayRef types = CGImageSourceCopyTypeIdentifiers();
    CHECK(types != NULL);
    if (types != NULL) {
        CHECK(CFArrayContainsValue(types, CFRangeMake(0, CFArrayGetCount(types)), CFSTR("org.khronos.astc")));
        CFRelease(types);
    }

    if (failures == 0)
        printf("PASS: ImageIO ASTC source\n");
    return failures ? 1 : 0;
}
