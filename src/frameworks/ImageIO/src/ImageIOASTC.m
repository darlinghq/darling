// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Darling developers
//
// Decoding of ASTC texture files (the .astc container: a 16-byte header followed by 128-bit blocks)
// to an RGBA8 image, on top of libtexturecodec. Only 2D images in the LDR profile are decoded.
//
// What the public ImageIO documentation does not settle is decided here, conservatively, and listed
// in src/libtexturecodec/GUESS.tsv (ids I1 to I6) and maintainer-question.md:
//  - the container carries no transfer function, so the stored RGB values are taken as sRGB-encoded
//    and the image has the sRGB colour space; alpha is stored straight (not premultiplied);
//  - no uniform type identifier for the format is documented, so the source reports none (NULL from
//    CGImageSourceGetType) and the type is not in CGImageSourceCopyTypeIdentifiers; the data is
//    recognised by its magic number and header;
//  - a texture that decoded with any illegal block (the specification's error colour, which an
//    encoder never writes, and which an HDR image decoded as LDR produces) yields no image;
//  - an image above kMaxDimension on a side or kMaxPixels in total yields no image;
//  - truncated data (fewer blocks than the header needs) yields no image, checked before anything is
//    allocated, so a header cannot make this allocate more than the data it comes with justifies.
// Test: tests/src/imageio_astc_source.c

#import "ImageIOASTC.h"
#import <Foundation/Foundation.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Image.h>
#include <stdint.h>
#include <stdlib.h>
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

+ (BOOL) isPresentInDataProvider: (O2DataProvider *) provider {
    uint8_t head[DTC_ASTC_FILE_HEADER_SIZE];
    NSInteger size = [provider getBytes: head range: NSMakeRange(0, sizeof head)];
    dtc_astc_header header;

    return size == (NSInteger) sizeof head &&
           dtc_astc_file_parse_2d(head, sizeof head, &header) == DTC_OK;
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
    // The O2 and CG colour space names are the same strings.
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

- (NSUInteger) count {
    return 1;
}

- (CFDictionaryRef) copyPropertiesAtIndex: (NSUInteger) index
                                  options: (CFDictionaryRef) options
{
    dtc_astc_header header;

    if (index != 0 || dtc_astc_file_parse_2d([_data bytes], [_data length], &header) != DTC_OK)
        return NULL;
    return (CFDictionaryRef) [[NSDictionary alloc] initWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: header.width], kO2ImagePropertyPixelWidth,
        [NSNumber numberWithUnsignedInt: header.height], kO2ImagePropertyPixelHeight, nil];
}

- (O2Image *) createImageAtIndex: (NSUInteger) index
                         options: (CFDictionaryRef) options
{
    if (index != 0)
        return nil;
    if (!_decoded) {
        _decoded = YES;
        _image = [self newImage];
    }
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
