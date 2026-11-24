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

// vImage_Buffer and vImage_CGImageFormat utilities, and the conversion between vImage_Buffers and CGImages.
#include <Accelerate/vImage.h>
#include "vImageInternal.h"
#include "vImageConverterInternal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- vImage_Buffer

vImage_Error vImageBuffer_Init(vImage_Buffer *buf, vImagePixelCount height, vImagePixelCount width, uint32_t pixelBits, vImage_Flags flags) {
    if (flags & ~(vImage_Flags)(kvImageNoAllocate | kvImagePrintDiagnosticsToConsole))
        return kvImageUnknownFlagsBit;
    // Scanlines start at a whole byte. A scanline of up to 16 bytes is padded to 16, up to 31 bytes to 32, and a longer one to a
    // multiple of 64 bytes (these sizes are the original's, found by observation).
    size_t bits = 0, bytes = 0, rowBytes = 0, size = 0;
    bool overflow = __builtin_mul_overflow((size_t)width, (size_t)pixelBits, &bits) || bits > SIZE_MAX - 7;
    if (!overflow) {
        bytes = (bits + 7) / 8;
        if (bytes == 0) {
            rowBytes = 0;
        } else if (bytes <= 16) {
            rowBytes = 16;
        } else if (bytes <= 31) {
            rowBytes = 32;
        } else {
            overflow = bytes > SIZE_MAX - 63;
            rowBytes = overflow ? 0 : (bytes + 63) / 64 * 64;
        }
        overflow = overflow || __builtin_mul_overflow((size_t)height, rowBytes, &size);
    }
    size_t alignment = (!overflow && rowBytes <= 32) ? 16 : 64;
    buf->height = height;
    buf->width = width;
    buf->rowBytes = overflow ? 0 : rowBytes;
    buf->data = NULL;
    if (flags & kvImageNoAllocate)
        return (vImage_Error)alignment;
    if (overflow)
        return kvImageMemoryAllocationError;
    void *p = NULL;
    if (posix_memalign(&p, alignment, size ? size : alignment) != 0)
        return kvImageMemoryAllocationError;
    buf->data = p;
    return kvImageNoError;
}

CGSize vImageBuffer_GetSize(const vImage_Buffer *buf) {
    // Rounded down, so that a size too large for a CGFloat never names rows or columns that do not exist.
    CGFloat w = (CGFloat)buf->width, h = (CGFloat)buf->height;
    while ((long double)w > (long double)buf->width)
        w = nextafter(w, 0);
    while ((long double)h > (long double)buf->height)
        h = nextafter(h, 0);
    return CGSizeMake(w, h);
}

// ---- vImage_CGImageFormat

uint32_t vImageCGImageFormat_GetComponentCount(const vImage_CGImageFormat *format) {
    uint32_t n = format->colorSpace ? (uint32_t)CGColorSpaceGetNumberOfComponents(format->colorSpace) : 3;
    uint32_t alpha = format->bitmapInfo & kCGBitmapAlphaInfoMask;
    return n + (alpha != kCGImageAlphaNone && alpha != kCGImageAlphaOnly ? 1 : 0);
}

Boolean vImageCGImageFormat_IsEqual(const vImage_CGImageFormat *f1, const vImage_CGImageFormat *f2) {
    if (f1 == NULL || f2 == NULL)
        return false;
    if (f1->bitsPerComponent != f2->bitsPerComponent || f1->bitsPerPixel != f2->bitsPerPixel || f1->bitmapInfo != f2->bitmapInfo || f1->version != f2->version)
        return false;
    // The rendering intent is not compared. A NULL colour space is the device RGB space.
    CGColorSpaceRef a = f1->colorSpace, b = f2->colorSpace, device = NULL;
    if (a == NULL || b == NULL) {
        device = CGColorSpaceCreateDeviceRGB();
        if (a == NULL)
            a = device;
        if (b == NULL)
            b = device;
    }
    bool same = a == b || CFEqual(a, b);
    if (device)
        CGColorSpaceRelease(device);
    if (!same)
        return false;
    if ((f1->decode == NULL) != (f2->decode == NULL))
        return false;
    if (f1->decode != NULL) {
        size_t n = 2 * vImageCGImageFormat_GetComponentCount(f1);
        if (f1->bitmapInfo & kCGBitmapAlphaInfoMask)
            n = 2 * (f1->colorSpace ? CGColorSpaceGetNumberOfComponents(f1->colorSpace) : 3);
        for (size_t i = 0; i < n; i++)
            if (f1->decode[i] != f2->decode[i])
                return false;
    }
    return true;
}

// ---- vImage_Buffer and CGImage

// False when bits that CoreGraphics gives no meaning are set (an alpha value above kCGImageAlphaOnly counts).
static bool formatBitmapInfoKnown(CGBitmapInfo bi) {
    return (bi & ~(CGBitmapInfo)(kCGBitmapAlphaInfoMask | VI_BITMAP_FLOAT_INFO_MASK | kCGBitmapByteOrderMask)) == 0 && (bi & kCGBitmapAlphaInfoMask) <= kCGImageAlphaOnly;
}

// The image's own pixel format and its pixels (a copy in `data`), or false when it cannot be read that way.
static bool nativeImage(CGImageRef image, vImage_CGImageFormat *format, CFDataRef *data, size_t *rowBytes) {
    CGDataProviderRef provider = CGImageGetDataProvider(image);
    if (provider == NULL || CGImageIsMask(image))
        return false;
    CFDataRef d = CGDataProviderCopyData(provider);
    if (d == NULL)
        return false;
    size_t height = CGImageGetHeight(image), rb = CGImageGetBytesPerRow(image);
    if (height == 0 || (size_t)CFDataGetLength(d) < rb * (height - 1) + (CGImageGetWidth(image) * CGImageGetBitsPerPixel(image) + 7) / 8) {
        CFRelease(d);
        return false;
    }
    memset(format, 0, sizeof *format);
    format->bitsPerComponent = (uint32_t)CGImageGetBitsPerComponent(image);
    format->bitsPerPixel = (uint32_t)CGImageGetBitsPerPixel(image);
    format->colorSpace = CGImageGetColorSpace(image);
    format->bitmapInfo = CGImageGetBitmapInfo(image);
    format->decode = CGImageGetDecode(image);
    format->renderingIntent = CGImageGetRenderingIntent(image);
    *data = d;
    *rowBytes = rb;
    return true;
}

vImage_Error vImageBuffer_InitWithCGImage(vImage_Buffer *buf, vImage_CGImageFormat *format, const CGFloat *backgroundColor, CGImageRef image, vImage_Flags flags) {
    if (buf == NULL || format == NULL || image == NULL)
        return kvImageNullPointerArgument;
    if (flags & ~(vImage_Flags)(kvImageNoAllocate | kvImagePrintDiagnosticsToConsole)) {
        buf->data = NULL;
        return kvImageUnknownFlagsBit;
    }
    if (!formatBitmapInfoKnown(format->bitmapInfo) || (format->bitsPerComponent != 0 && format->bitsPerComponent != 1 && format->bitsPerComponent != 2 &&
        format->bitsPerComponent != 4 && format->bitsPerComponent != 5 && format->bitsPerComponent != 8 && format->bitsPerComponent != 12 &&
        format->bitsPerComponent != 16 && format->bitsPerComponent != 32)) {
        buf->data = NULL;
        return kvImageInvalidParameter;
    }
    size_t width = CGImageGetWidth(image), height = CGImageGetHeight(image);

    // The pixels in the format the image has, or, when they cannot be had that way, drawn into an 8-bit premultiplied sRGB image.
    vImage_CGImageFormat srcFormat;
    CFDataRef data = NULL;
    size_t srcRowBytes = 0;
    CGColorSpaceRef drawSpace = NULL;
    void *drawn = NULL;
    vImageConverterRef converter = NULL;
    vImage_Error error = kvImageNoError;
    if (nativeImage(image, &srcFormat, &data, &srcRowBytes)) {
        converter = vImageConverter_CreateWithCGImageFormat(&srcFormat, format, backgroundColor, flags & kvImageDoNotTile, &error);
        if (converter == NULL && data != NULL) {
            CFRelease(data);
            data = NULL;
        }
    }
    if (converter == NULL) {
        drawSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        srcRowBytes = width * 4;
        drawn = calloc(height ? height : 1, srcRowBytes ? srcRowBytes : 1);
        CGContextRef context = drawn ? CGBitmapContextCreate(drawn, width, height, 8, srcRowBytes, drawSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault) : NULL;
        if (context == NULL) {
            if (context)
                CGContextRelease(context);
            free(drawn);
            CGColorSpaceRelease(drawSpace);
            buf->data = NULL;
            return kvImageMemoryAllocationError;
        }
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
        CGContextRelease(context);
        memset(&srcFormat, 0, sizeof srcFormat);
        srcFormat.bitsPerComponent = 8;
        srcFormat.bitsPerPixel = 32;
        srcFormat.colorSpace = drawSpace;
        srcFormat.bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault;
        error = kvImageNoError;
        converter = vImageConverter_CreateWithCGImageFormat(&srcFormat, format, backgroundColor, flags & kvImageDoNotTile, &error);
    }
    if (converter == NULL) {
        free(drawn);
        if (drawSpace)
            CGColorSpaceRelease(drawSpace);
        buf->data = NULL;
        return error == kvImageNoError ? kvImageInternalError : error;
    }

    // With kvImageNoAllocate the caller's buffer and rowBytes are used as they are. An image that already has the requested format
    // is copied as it is, with its own rowBytes.
    bool ownsBuffer = !(flags & kvImageNoAllocate);
    bool sameFormat = data != NULL && vImageCGImageFormat_IsEqual(&srcFormat, format);
    if (ownsBuffer && sameFormat) {
        size_t size = srcRowBytes * height;
        void *p = NULL;
        if (posix_memalign(&p, 16, size ? size : 16) != 0) {
            error = kvImageMemoryAllocationError;
        } else {
            memcpy(p, CFDataGetBytePtr(data), size);
            buf->data = p;
            buf->width = width;
            buf->height = height;
            buf->rowBytes = srcRowBytes;
            error = kvImageNoError;
            vImageConverter_Release(converter);
            CFRelease(data);
            return kvImageNoError;
        }
    } else if (ownsBuffer) {
        error = vImageBuffer_Init(buf, height, width, format->bitsPerPixel, kvImageNoFlags);
    } else {
        buf->width = width;
        buf->height = height;
    }
    if (error == kvImageNoError) {
        vImage_Buffer src = { data ? (void *)CFDataGetBytePtr(data) : drawn, height, width, srcRowBytes };
        error = vImageConvert_AnyToAny(converter, &src, buf, NULL, flags & kvImageDoNotTile);
    }
    if (error != kvImageNoError) {
        if (ownsBuffer) {
            free(buf->data);
        }
        buf->data = NULL;
    }
    vImageConverter_Release(converter);
    if (data)
        CFRelease(data);
    free(drawn);
    if (drawSpace)
        CGColorSpaceRelease(drawSpace);
    return error;
}

typedef struct {
    void (*callback)(void *userData, void *data);
    void *userData;
    bool cancelled;     // the image could not be created: the caller keeps the memory
} BufferOwner;

static void releaseBuffer(void *info, const void *data, size_t size) {
    BufferOwner *o = info;
    if (o != NULL && o->cancelled) {
        free(o);
        return;
    }
    if (o != NULL && o->callback != NULL)
        o->callback(o->userData, (void *)data);
    else
        free((void *)data);
    free(o);
}

// Makes a CGImage of `pixels` (rowBytes) in the given format; the CGImage owns the memory when it succeeds, and owner (freed here)
// says how to release it (a NULL callback means free()). When it fails the memory stays with the caller.
static CGImageRef imageFromPixels(void *pixels, size_t rowBytes, size_t width, size_t height, const vImage_CGImageFormat *format,
                                  CGColorSpaceRef space, BufferOwner *owner) {
    CGDataProviderRef provider = CGDataProviderCreateWithData(owner, pixels, height * rowBytes, releaseBuffer);
    if (provider == NULL)
        return NULL;
    CGImageRef image = CGImageCreate(width, height, format->bitsPerComponent, format->bitsPerPixel, rowBytes, space, format->bitmapInfo, provider,
                                     format->decode, false, format->renderingIntent);
    if (image == NULL)
        owner->cancelled = true;
    CGDataProviderRelease(provider);   // releases the owner, and the memory unless it was cancelled
    return image;
}

// Converts the buffer to 8-bit premultiplied (or gray) pixels in the same colour space, which CoreGraphics can always use, and makes the image.
static CGImageRef imageFromConvertedBuffer(const vImage_Buffer *buf, const vImage_CGImageFormat *format, CGColorSpaceRef space, vImage_Flags flags, vImage_Error *error) {
    bool gray = CGColorSpaceGetNumberOfComponents(space) == 1;
    vImage_CGImageFormat target;
    memset(&target, 0, sizeof target);
    target.bitsPerComponent = 8;
    target.bitsPerPixel = gray ? 16 : 32;
    target.colorSpace = space;
    target.bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault;
    target.renderingIntent = format->renderingIntent;
    vImage_CGImageFormat src = *format;
    src.colorSpace = space;
    vImageConverterRef converter = vImageConverter_CreateWithCGImageFormat(&src, &target, NULL, flags & kvImageDoNotTile, error);
    if (converter == NULL)
        return NULL;
    vImage_Buffer dest;
    vImage_Error e = vImageBuffer_Init(&dest, buf->height, buf->width, target.bitsPerPixel, kvImageNoFlags);
    if (e == kvImageNoError)
        e = vImageConvert_AnyToAny(converter, buf, &dest, NULL, flags & kvImageDoNotTile);
    vImageConverter_Release(converter);
    if (e != kvImageNoError) {
        free(dest.data);
        *error = e;
        return NULL;
    }
    BufferOwner *owner = calloc(1, sizeof *owner);
    CGImageRef image = owner ? imageFromPixels(dest.data, dest.rowBytes, buf->width, buf->height, &target, space, owner) : NULL;
    if (image == NULL) {
        free(dest.data);
        *error = kvImageInvalidImageFormat;
    } else {
        *error = kvImageNoError;
    }
    return image;
}

CGImageRef vImageCreateCGImageFromBuffer(const vImage_Buffer *buf, const vImage_CGImageFormat *format, void (*callback)(void *userData, void *buf_data),
                                         void *userData, vImage_Flags flags, vImage_Error *error) {
    vImage_Error dummy;
    if (error == NULL)
        error = &dummy;
    if (buf == NULL || format == NULL) {
        *error = kvImageNullPointerArgument;
        return NULL;
    }
    if (flags & ~(vImage_Flags)(kvImageNoAllocate | kvImagePrintDiagnosticsToConsole | kvImageHighQualityResampling | kvImageDoNotTile)) {
        *error = kvImageUnknownFlagsBit;
        return NULL;
    }
    uint32_t bpc = format->bitsPerComponent;
    if ((bpc != 5 && bpc != 8 && bpc != 16 && bpc != 32) || format->renderingIntent > kCGRenderingIntentPerceptual + 1) {   // intents: default, absolute, relative, perceptual, saturation
        *error = kvImageInvalidImageFormat;
        return NULL;
    }
    CGColorSpaceRef space = format->colorSpace ? CGColorSpaceRetain(format->colorSpace) : CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGImageRef image = NULL;
    size_t height = buf->height, width = buf->width;
    if (flags & kvImageNoAllocate) {
        // No copy: the CGImage takes the buffer over and uses it as it is.
        BufferOwner *owner = malloc(sizeof *owner);
        if (owner == NULL) {
            *error = kvImageMemoryAllocationError;
        } else {
            owner->callback = callback;
            owner->userData = userData;
            owner->cancelled = false;
            image = imageFromPixels(buf->data, buf->rowBytes, width, height, format, space, owner);
            *error = image ? kvImageNoError : kvImageInvalidImageFormat;
        }
    } else {
        // A copy, so that the buffer may be reused: as it is if CoreGraphics takes the format, else converted.
        size_t size = height * buf->rowBytes;
        void *pixels = malloc(size ? size : 1);
        if (pixels == NULL) {
            *error = kvImageMemoryAllocationError;
        } else {
            if (size)
                memcpy(pixels, buf->data, size);
            BufferOwner *owner = calloc(1, sizeof *owner);
            image = owner ? imageFromPixels(pixels, buf->rowBytes, width, height, format, space, owner) : NULL;
            if (image == NULL) {
                free(pixels);
                image = imageFromConvertedBuffer(buf, format, space, flags, error);
            } else {
                *error = kvImageNoError;
            }
        }
    }
    CGColorSpaceRelease(space);
    return image;
}
