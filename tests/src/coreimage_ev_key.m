/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// The exposure parameter key (CIFilter.h) and ImageIO's type identifier hint option must be exported
// strings, distinct from their neighbours; the hint is accepted as an option and does not change decoding.
#import <Foundation/Foundation.h>
#import <CoreImage/CoreImage.h>
#import <ImageIO/ImageIO.h>
#include <stdio.h>

#define EXPECT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 1; } } while (0)

static const unsigned char png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4,
    0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

int main(void) {
    @autoreleasepool {
        EXPECT(kCIInputEVKey != nil && [kCIInputEVKey length] > 0, "kCIInputEVKey exists");
        EXPECT(![kCIInputEVKey isEqual:kCIInputBrightnessKey], "kCIInputEVKey differs from the brightness key");

        EXPECT(kCGImageSourceTypeIdentifierHint != NULL, "kCGImageSourceTypeIdentifierHint exists");
        EXPECT(!CFEqual(kCGImageSourceTypeIdentifierHint, kCGImageSourceShouldCache), "hint key differs from the cache key");

        CFDataRef data = CFDataCreate(NULL, png, sizeof png);
        const void *keys[] = {kCGImageSourceTypeIdentifierHint};
        const void *values[] = {CFSTR("public.png")};
        CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CGImageSourceRef source = CGImageSourceCreateWithData(data, options);
        EXPECT(source != NULL, "a source is created with the hint");
        CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        EXPECT(image != NULL, "the image decodes");
        CGImageRelease(image);
        CFRelease(source);
        CFRelease(options);
        CFRelease(data);
    }
    printf("PASS: coreimage_ev_key\n");
    return 0;
}
