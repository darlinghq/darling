// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// Decoding of ASTC texture files (the .astc container: a 16-byte header followed by 128-bit blocks)
// to an RGBA8 image, on top of libtexturecodec. Only 2D images in the LDR profile are decoded.
//
// What the public ImageIO documentation does not settle is decided here, conservatively, and listed
// in src/libtexturecodec/GUESS.tsv (ids I1 to I7) and maintainer-question.md:
//  - the container carries no transfer function, so the stored RGB values are taken as sRGB-encoded
//    and the image has the sRGB colour space; alpha is stored straight (not premultiplied);
//  - no uniform type identifier for the format is documented, so the source reports none (NULL from
//    CGImageSourceGetType) and the type is not in CGImageSourceCopyTypeIdentifiers; the data is
//    recognised by its magic number alone, so that other data goes on to the Onyx2D decoders;
//  - a texture that decoded with any illegal block (the specification's error colour, which an
//    encoder never writes, and which an HDR image decoded as LDR produces) yields no image;
//  - an image above kMaxDimension on a side or kMaxPixels in total yields no image;
//  - truncated data (fewer blocks than the header needs) yields no image, checked before anything is
//    allocated, so a header cannot make this allocate more than the data it comes with justifies;
//  - data that has the magic number but is refused for any of these reasons (or has a header that
//    is not a 2D one with a legal footprint) is a source with no image: its count is 0, as for the
//    PSD source, so CGImageSourceGetStatus reports invalid data for it. Data without the magic
//    number is not an ASTC source at all and goes on to the Onyx2D decoders.
// Test: tests/src/imageio_astc_source.c

#import "ImageIOASTC.h"
#import <Foundation/Foundation.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Image.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <texturecodec/texturecodec.h>

enum {
    kMaxDimension = 16384,
    // 256 MiB of RGBA8, the limit the PSD source uses as well
    kMaxPixels = 1 << 26,
};

@interface _ImageIOASTCSource : O2ImageSource {
    NSData *_data;
    O2Image *_image;
    BOOL _decoded;
}
@end

@implementation _ImageIOASTCSource

// The magic number of the .astc container, bytes 0 to 3 of its header.
static const uint8_t kMagic[4] = { 0x13, 0xAB, 0xA1, 0x5C };

+ (BOOL) isPresentInDataProvider: (O2DataProvider *) provider {
    uint8_t head[sizeof kMagic];
    NSInteger size = [provider getBytes: head range: NSMakeRange(0, sizeof head)];

    return size == (NSInteger) sizeof head && memcmp(head, kMagic, sizeof head) == 0;
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
    if (header.width > kMaxDimension || header.height > kMaxDimension ||
        (uint64_t) header.width * header.height > kMaxPixels)
        return nil;

    uint64_t blocks = (((uint64_t) header.width + header.block_w - 1) / header.block_w) *
                      (((uint64_t) header.height + header.block_h - 1) / header.block_h);
    if ((length - DTC_ASTC_FILE_HEADER_SIZE) / DTC_ASTC_BLOCK_BYTES < blocks)
        return nil;

    size_t width = header.width, height = header.height;
    uint8_t *rgba = malloc(width * height * 4);
    dtc_decode_report report;

    if (rgba == NULL)
        return nil;
    if (dtc_astc_decode_ex(bytes + DTC_ASTC_FILE_HEADER_SIZE,
                           length - DTC_ASTC_FILE_HEADER_SIZE, header.block_w,
                           header.block_h, header.width, header.height, 1, rgba,
                           &report) != DTC_OK ||
        report.error_blocks != 0) {
        free(rgba);
        return nil;
    }

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
                                          bitmapInfo: kO2BitmapByteOrder32Big |
                                                      kO2ImageAlphaLast
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
    // No uniform type identifier for ASTC is documented publicly, so none is invented.
    return NULL;
}

- (void) parseIfNeeded {
    if (!_decoded) {
        _decoded = YES;
        _image = [self newImage];
    }
}

- (NSUInteger) count {
    [self parseIfNeeded];
    return _image != nil ? 1 : 0;
}

- (CFDictionaryRef) copyPropertiesAtIndex: (NSUInteger) index
                                  options: (CFDictionaryRef) options
{
    [self parseIfNeeded];
    if (index != 0 || _image == nil)
        return NULL;
    return (CFDictionaryRef) [[NSDictionary alloc] initWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLong: O2ImageGetWidth(_image)], kO2ImagePropertyPixelWidth,
        [NSNumber numberWithUnsignedLong: O2ImageGetHeight(_image)], kO2ImagePropertyPixelHeight, nil];
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
