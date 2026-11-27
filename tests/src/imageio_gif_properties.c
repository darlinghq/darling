// CFLAGS: -framework ImageIO -framework CoreFoundation -framework CoreGraphics
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// CGImageSourceCopyPropertiesAtIndex on a GIF reports the image's pixel size
// (kCGImagePropertyPixelWidth and kCGImagePropertyPixelHeight), as it does for
// other formats, and it matches the image CGImageSourceCreateImageAtIndex makes.
// Callers that size an image from its properties (WebKit's image decoder) need
// both. An index past the last image has no properties.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

// A 3x2 GIF89a, one frame, every pixel red.
static const UInt8 gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x03, 0x00, 0x02, 0x00, 0xf0, 0x00,
    0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00,
    0x00, 0x02, 0x02, 0x84, 0x5f, 0x00, 0x3b
};

static long number(CFDictionaryRef properties, CFStringRef key) {
    CFNumberRef value = CFDictionaryGetValue(properties, key);
    long result = -1;
    if (value == NULL || CFGetTypeID(value) != CFNumberGetTypeID() ||
        !CFNumberGetValue(value, kCFNumberLongType, &result))
        return -1;
    return result;
}

int main(void) {
    CFDataRef data = CFDataCreate(NULL, gif, sizeof gif);
    CGImageSourceRef source = CGImageSourceCreateWithData(data, NULL);
    CHECK(source != NULL);
    if (source == NULL)
        return 1;
    CHECK(CFEqual(CGImageSourceGetType(source), CFSTR("com.compuserve.gif")));
    CHECK(CGImageSourceGetCount(source) == 1);

    CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, 0, NULL);
    CHECK(properties != NULL);
    if (properties != NULL) {
        CHECK(number(properties, kCGImagePropertyPixelWidth) == 3);
        CHECK(number(properties, kCGImagePropertyPixelHeight) == 2);
        CFRelease(properties);
    }

    CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CHECK(image != NULL);
    if (image != NULL) {
        CHECK(CGImageGetWidth(image) == 3);
        CHECK(CGImageGetHeight(image) == 2);
        CGImageRelease(image);
    }

    CHECK(CGImageSourceCopyPropertiesAtIndex(source, 1, NULL) == NULL);

    CFRelease(source);
    CFRelease(data);
    if (failures == 0)
        printf("PASS\n");
    return failures != 0;
}
