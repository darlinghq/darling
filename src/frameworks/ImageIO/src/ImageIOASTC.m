// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// Decoding of ASTC texture files (the .astc container: a 16-byte header followed by 128-bit blocks)
// to an RGBA8 image, on top of libtexturecodec. Only 2D images in the LDR profile are decoded.
//
// The behaviour follows what macOS 26.7.3 and 10.14.4 do with .astc data through the public
// ImageIO functions (black-box observations, vd-upstream cristim-mac-evidence.md section 16,
// pinned by the test below); where the public documentation is silent, that is the contract:
//  - the type identifier is org.khronos.astc and CGImageSourceCopyTypeIdentifiers lists it;
//  - data with the magic number and room for the 16-byte header and one block is an ASTC source
//    with one image and status complete, whether or not the image can be made; shorter data, or
//    data without the magic number, is not an ASTC source (it goes on to the Onyx2D decoders;
//    macOS makes a source of unknown type, count 0 and status -4 of such data, which is not
//    part of this change);
//  - the image is RGBA8 (32 bits per pixel), sRGB colour space, straight alpha; its properties are
//    ColorModel RGB, Depth 8, HasAlpha, Orientation 4, PixelWidth and PixelHeight;
//  - a 2D header with a footprint of the specification's table and all the blocks the header asks
//    for gives properties, and an image when its sides are below kMaxSide; anything else (3D,
//    other footprints, zero size, missing blocks) gives a source whose image and properties are
//    NULL;
//  - a texture with an illegal block gives an image of the right size that draws nothing (below).
// Test: tests/src/imageio_astc_source.c

#import "ImageIOASTC.h"
#import <Foundation/Foundation.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Image.h>
#import <ImageIO/CGImageProperties.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <texturecodec/texturecodec.h>

enum {
    // QUIRK: a side of 1048576 texels or more gives no image; one of 524288 does (checked with a
    // 4 texel other side on 10.14.4; macOS 26.7.3 makes an image that draws nothing from 65536 up,
    // so the wider behaviour is followed). Where the limit lies between 524288 and 1048576 was not
    // probed. Evidence: black-box observation, tier 2 (cristim-mac-evidence.md section 16).
    // Test: tests/src/imageio_astc_source.c, "at the side limit" and "over the side limit".
    kMaxSide = 1 << 20,
    kHeaderSize = 16,
    // the header and one block: with less than that macOS makes no ASTC source
    kMinimumLength = kHeaderSize + 16,
};

@interface _ImageIOASTCSource : O2ImageSource {
    NSData *_data;
    O2Image *_image;
    BOOL _decoded;
    // the size from the header, once it is a 2D header with a legal footprint and all its blocks
    BOOL _headerValid;
    size_t _width, _height;
}
@end

@implementation _ImageIOASTCSource

// The magic number of the .astc container, bytes 0 to 3 of its header.
static const uint8_t kMagic[4] = { 0x13, 0xAB, 0xA1, 0x5C };

+ (BOOL) isPresentInDataProvider: (O2DataProvider *) provider {
    uint8_t head[kMinimumLength];
    NSInteger size = [provider getBytes: head range: NSMakeRange(0, sizeof head)];

    return size == (NSInteger) sizeof head && memcmp(head, kMagic, sizeof kMagic) == 0;
}

- (id) initWithDataProvider: (O2DataProvider *) provider
               options: (NSDictionary *) options
{
    [super initWithDataProvider: provider options: options];
    _data = (NSData *) O2DataProviderCopyData(provider);
    return self;
}

- (void) dealloc {
    [_data release];
    [_image release];
    [super dealloc];
}

- (O2Image *) newImage {
    const uint8_t *bytes = [_data bytes];
    size_t length = [_data length];
    dtc_astc_header header;

    if (dtc_astc_file_parse_2d(bytes, length, &header) != DTC_OK)
        return nil;

    uint64_t blocks = (((uint64_t) header.width + header.block_w - 1) / header.block_w) *
                      (((uint64_t) header.height + header.block_h - 1) / header.block_h);
    if ((length - DTC_ASTC_FILE_HEADER_SIZE) / DTC_ASTC_BLOCK_BYTES < blocks)
        return nil;
    // The properties exist from here on, as on macOS, also for a size no image is made of.
    _headerValid = YES;
    _width = header.width;
    _height = header.height;
    if (header.width >= kMaxSide || header.height >= kMaxSide)
        return nil;
    if ((uint64_t) header.width * header.height > SIZE_MAX / 4)
        return nil;

    size_t width = header.width, height = header.height;
    uint8_t *rgba = malloc(width * height * 4);
    dtc_decode_report report;

    if (rgba == NULL)
        return nil;
    if (dtc_astc_decode_ex(bytes + DTC_ASTC_FILE_HEADER_SIZE,
                           length - DTC_ASTC_FILE_HEADER_SIZE, header.block_w,
                           header.block_h, header.width, header.height,
                           DTC_SRGB | DTC_COMPAT, rgba,
                           &report) != DTC_OK) {
        free(rgba);
        return nil;
    }
    // QUIRK: one illegal block anywhere in the texture gives an image of the right size, but a
    // transparent black one: macOS answers such a file with an image whose data cannot be copied
    // and which draws nothing (the same for a block of zeros, of ones and for random blocks,
    // for a bad block in the middle or at the end). Whole-image zero pixels stand in for that.
    // Evidence: black-box observation on macOS 26.7.3 and 10.14.4, tier 2.
    // Test: tests/src/imageio_astc_source.c, "an illegal block".
    if (report.error_blocks != 0)
        memset(rgba, 0, width * height * 4);

    NSData *bitmap = [[NSData alloc] initWithBytesNoCopy: rgba
                                                  length: width * height * 4
                                            freeWhenDone: YES];
    O2DataProviderRef provider = O2DataProviderCreateWithCFData((CFDataRef) bitmap);
    // The O2 and CG colour space names are the same strings. kO2ColorSpaceSRGB is not declared
    // in an Onyx2D header, so the name is spelled out.
    O2ColorSpaceRef colorSpace = O2ColorSpaceCreateWithName(CFSTR("kCGColorSpaceSRGB"));
    O2Image *result = [[O2Image alloc] initWithWidth: width
                                              height: height
                                    bitsPerComponent: 8
                                        bitsPerPixel: 32
                                         bytesPerRow: width * 4
                                          colorSpace: colorSpace
                                          bitmapInfo: kO2ImageAlphaLast
                                             decoder: NULL
                                            provider: provider
                                              decode: NULL
                                         interpolate: NO
                                     renderingIntent: kO2RenderingIntentDefault];

    O2ColorSpaceRelease(colorSpace);
    O2DataProviderRelease(provider);
    [bitmap release];
    return result;
}

- (CFStringRef) type {
    return CFSTR("org.khronos.astc");
}

- (void) parseIfNeeded {
    if (!_decoded) {
        _decoded = YES;
        _image = [self newImage];
    }
}

// One image, even when it cannot be made: macOS reports count 1 and status complete for every
// ASTC source (observed on 26.7.3 and 10.14.4).
- (NSUInteger) count {
    return 1;
}

- (CFDictionaryRef) copyPropertiesAtIndex: (NSUInteger) index
                                  options: (CFDictionaryRef) options
{
    [self parseIfNeeded];
    if (index != 0 || !_headerValid)
        return NULL;
    // Orientation 4 is what macOS reports; its image data is not flipped by it.
    return (CFDictionaryRef) [[NSDictionary alloc] initWithObjectsAndKeys:
        (NSString *) kCGImagePropertyColorModelRGB, (NSString *) kCGImagePropertyColorModel,
        [NSNumber numberWithInt: 8], (NSString *) kCGImagePropertyDepth,
        [NSNumber numberWithBool: YES], (NSString *) kCGImagePropertyHasAlpha,
        [NSNumber numberWithInt: 4], (NSString *) kCGImagePropertyOrientation,
        [NSNumber numberWithUnsignedLong: _width], kO2ImagePropertyPixelWidth,
        [NSNumber numberWithUnsignedLong: _height], kO2ImagePropertyPixelHeight, nil];
}

- (O2Image *) createImageAtIndex: (NSUInteger) index
                         options: (CFDictionaryRef) options
{
    [self parseIfNeeded];
    if (index != 0)
        return nil;
    return [_image retain];
}

@end

O2ImageSourceRef ImageIOASTCSourceCreate(O2DataProvider *provider, CFDictionaryRef options) {
    if (![_ImageIOASTCSource isPresentInDataProvider: provider])
        return nil;
    [provider rewind];
    return [[_ImageIOASTCSource alloc] initWithDataProvider: provider
                                                    options: (NSDictionary *) options];
}
