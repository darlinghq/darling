// CFLAGS: -framework ImageIO -framework CoreFoundation -framework CoreGraphics
// ARGS: /Volumes/SystemRoot$PWD/src/libtexturecodec/tests/vectors
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// CGImageSourceCreateWithData and CGImageSourceCreateWithURL on ASTC texture files (the .astc
// container, 2D LDR). The ImageIO documentation names no type identifier and no pixel format for
// them, so what is pinned here is the contract of src/frameworks/ImageIO/src/ImageIOASTC.m, which
// follows the Khronos Data Format Specification 1.4.0 (section 23) and the container description
// of ARM's astc-encoder (Docs/FileFormat.md); none of it is observed on macOS (GUESS items I1 to
// I6 in src/libtexturecodec/GUESS.tsv):
//  - the image is RGBA8, sRGB colour space, straight alpha, as the decoder returns it;
//  - the source has no type identifier and CGImageSourceCopyTypeIdentifiers does not list one;
//  - truncated data, data over the size limits, an illegal block, a 3D file and a bad magic number
//    give no image (or no source, when the header is not an .astc header).
// Expected pixels: the committed vectors in src/libtexturecodec/tests/vectors (each a .astc file
// and the .rgba decoded from it by astcenc 5.7.1, see the NOTICE there), and one block built from
// the specification's void-extent layout (section 23.22), whose colour is its own expectation.
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
        CHECK(CGImageSourceGetType(source) == NULL);
        CHECK(CGImageSourceCreateImageAtIndex(source, 1, NULL) == NULL);

        CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
        CHECK(properties != NULL);
        if (properties != NULL) {
            int value = 0;
            CFNumberRef number = CFDictionaryGetValue(properties, kCGImagePropertyPixelWidth);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && (size_t)value == width);
            number = CFDictionaryGetValue(properties, kCGImagePropertyPixelHeight);
            CHECK(number != NULL && CFNumberGetValue(number, kCFNumberIntType, &value) && (size_t)value == height);
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

static void checkVoidBlock(void) {
    // 4x4 texels in one 4x4 block, all the colour 0x1234, 0x5678, 0x9ABC, 0x8000: sRGB mode uses
    // the top 8 bits of each, and the alpha stays straight (12 56 9A 80, not 09 2B 4D 80).
    uint8_t file[16 + 16];
    putHeader(file, 4, 4, 1, 4, 4, 1);
    putVoidBlock(file + 16, 0x1234, 0x5678, 0x9ABC, 0x8000);
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
                CHECK(p[i * 4] == 0x12 && p[i * 4 + 1] == 0x56 && p[i * 4 + 2] == 0x9A && p[i * 4 + 3] == 0x80);
            CFRelease(pixels);
        }
        if (image != NULL)
            CGImageRelease(image);
        CFRelease(source);
    }
    CFRelease(data);
}

// Each case is bytes, whether a source exists for them and whether it makes an image.
static void checkCase(const char *what, const uint8_t *bytes, size_t length, bool sourceExists, bool imageExists) {
    CFDataRef data = CFDataCreate(NULL, bytes, length);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    if ((source != NULL) != sourceExists) {
        printf("FAIL: %s: source %s\n", what, source != NULL ? "exists" : "is NULL");
        failures++;
    }
    if (source != NULL) {
        CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        if ((image != NULL) != imageExists) {
            printf("FAIL: %s: image %s\n", what, image != NULL ? "was made" : "is NULL");
            failures++;
        }
        if (image != NULL)
            CGImageRelease(image);
        CFRelease(source);
    }
    CFRelease(data);
}

static void checkCases(void) {
    uint8_t file[16 + 16 * 4];
    memset(file, 0, sizeof file);

    // header only, and one block short of the four a 8x8 image of 4x4 blocks needs
    putHeader(file, 4, 4, 1, 8, 8, 1);
    checkCase("no blocks", file, 16, true, false);
    putVoidBlock(file + 16, 0x1000, 0x2000, 0x3000, 0xFFFF);
    putVoidBlock(file + 32, 0x1000, 0x2000, 0x3000, 0xFFFF);
    putVoidBlock(file + 48, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase("three of four blocks", file, 16 + 3 * 16, true, false);
    putVoidBlock(file + 64, 0x1000, 0x2000, 0x3000, 0xFFFF);
    checkCase("all four blocks (control)", file, sizeof file, true, true);

    // an illegal block (block mode 0, the specification's error colour) fails the whole image
    memset(file + 64, 0, 16);
    checkCase("an illegal block", file, sizeof file, true, false);

    // over the limits: 16385 texels wide, 16384 x 16384 in total; the data is a single block, so
    // these also show that nothing is allocated for the size the header claims
    putHeader(file, 4, 4, 1, 16385, 4, 1);
    checkCase("too wide", file, 32, true, false);
    putHeader(file, 12, 12, 1, 16384, 16384, 1);
    checkCase("too many pixels", file, 32, true, false);

    // not an .astc header at all: no source
    putHeader(file, 4, 4, 1, 4, 4, 1);
    file[0] ^= 1;
    checkCase("bad magic", file, 32, false, false);
    putHeader(file, 4, 4, 1, 4, 4, 1);
    checkCase("shorter than a header", file, 15, false, false);
    putHeader(file, 7, 7, 1, 4, 4, 1);
    checkCase("footprint not in the specification", file, 32, false, false);
    putHeader(file, 4, 4, 1, 0, 4, 1);
    checkCase("zero width", file, 32, false, false);
    putHeader(file, 4, 4, 4, 4, 4, 4);
    checkCase("3D image", file, 32, false, false);
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
    checkVoidBlock();
    checkCases();

    CFArrayRef types = CGImageSourceCopyTypeIdentifiers();
    CHECK(types != NULL);
    if (types != NULL) {
        // no type identifier is documented for ASTC, so nothing in the list may name it
        for (CFIndex i = 0; i < CFArrayGetCount(types); i++) {
            CFStringRef type = CFArrayGetValueAtIndex(types, i);
            CHECK(CFStringFind(type, CFSTR("astc"), kCFCompareCaseInsensitive).location == kCFNotFound);
        }
        CFRelease(types);
    }

    if (failures == 0)
        printf("PASS: ImageIO ASTC source\n");
    return failures ? 1 : 0;
}
