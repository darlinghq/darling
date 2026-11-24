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

// vImageConverter: conversion between vImage_CGImageFormat pixel formats (vImageConvert_AnyToAny). The pixel work is done by
// vImageConvertCore.c; this file describes the formats to it, using CoreGraphics only to classify colour spaces, and implements
// the documented checks and errors.
#include <Accelerate/vImage.h>
#include "vImageInternal.h"
#include "vImageConverterInternal.h"
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maps a CoreGraphics colour space to one the conversion core knows; VISpaceInvalid for anything else (CMYK, Lab, indexed, ICC
// based spaces, other named spaces: converting those needs ColorSync).
VISpace VIClassifyColorSpace(CGColorSpaceRef cs) {
    if (cs == NULL)
        return VISpaceSRGB;
    CGColorSpaceModel model = CGColorSpaceGetModel(cs);
    CFStringRef name = CGColorSpaceCopyName(cs);
    VISpace result = VISpaceInvalid;
#define IS(n) (name != NULL && CFEqual(name, n))
    if (model == kCGColorSpaceModelRGB) {
        if (name == NULL || IS(kCGColorSpaceSRGB) || IS(kCGColorSpaceExtendedSRGB))
            result = VISpaceSRGB;
        else if (IS(kCGColorSpaceLinearSRGB) || IS(kCGColorSpaceExtendedLinearSRGB))
            result = VISpaceLinearSRGB;
        else if (IS(kCGColorSpaceDisplayP3) || IS(kCGColorSpaceExtendedDisplayP3))
            result = VISpaceP3;
        else if (IS(kCGColorSpaceLinearDisplayP3) || IS(kCGColorSpaceExtendedLinearDisplayP3))
            result = VISpaceLinearP3;
        else if (IS(kCGColorSpaceAdobeRGB1998))
            result = VISpaceAdobeRGB;
        else if (IS(kCGColorSpaceGenericRGB))
            result = VISpaceGenericRGB;
        else if (IS(kCGColorSpaceGenericRGBLinear))
            result = VISpaceGenericRGBLinear;
    } else if (model == kCGColorSpaceModelMonochrome) {
        if (name == NULL || IS(kCGColorSpaceGenericGrayGamma2_2) || IS(kCGColorSpaceExtendedGray))
            result = VISpaceGray22;
        else if (IS(kCGColorSpaceGenericGray))
            result = VISpaceGray18;
        else if (IS(kCGColorSpaceLinearGray) || IS(kCGColorSpaceExtendedLinearGray))
            result = VISpaceLinearGray;
    }
#undef IS
    if (name != NULL)
        CFRelease(name);
    return result;
}

// Describes a vImage_CGImageFormat to the core. Returns 0, kvImageInvalidImageFormat or kvImageColorSyncIsAbsent.
vImage_Error VIFormatFromCG(const vImage_CGImageFormat *f, VIFormat *out) {
    memset(out, 0, sizeof *out);
    out->bpc = f->bitsPerComponent;
    out->bpp = f->bitsPerPixel;
    out->alphaInfo = f->bitmapInfo & kCGBitmapAlphaInfoMask;
    out->order = f->bitmapInfo & kCGBitmapByteOrderMask;
    out->isFloat = (f->bitmapInfo & VI_BITMAP_FLOAT_INFO_MASK) != 0;
    out->space = VIClassifyColorSpace(f->colorSpace);
    if (f->version != 0)
        return kvImageInvalidImageFormat;
    if (out->space == VISpaceInvalid) {
        // Reject what is invalid by itself first: a bad format is a format error whatever its colour space.
        VIFormat probe = *out;
        probe.space = VISpaceSRGB;
        if (VIFormatCheck(&probe) != 0)
            return kvImageInvalidImageFormat;
        return kvImageColorSyncIsAbsent;
    }
    if (VIFormatCheck(out) != 0)
        return kvImageInvalidImageFormat;
    if (f->decode != NULL) {
        int n = VIFormatIsGray(out) ? 1 : 3;
        out->hasDecode = true;
        for (int i = 0; i < 2 * n; i++)
            out->decode[i] = f->decode[i];
    }
    return kvImageNoError;
}

static void converterFinalize(CFTypeRef cf) {
    struct vImageConverter *c = (struct vImageConverter *)cf;
    VIPlanDestroy(c->plan);
    if (c->srcSpace)
        CGColorSpaceRelease(c->srcSpace);
    if (c->dstSpace)
        CGColorSpaceRelease(c->dstSpace);
}

static CFTypeID converterTypeID(void) {
    static CFTypeID typeID;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        static const CFRuntimeClass cls = { .version = 0, .className = "vImageConverter", .finalize = converterFinalize };
        typeID = _CFRuntimeRegisterClass(&cls);
    });
    return typeID;
}

static void setError(vImage_Error *error, vImage_Error value) {
    if (error)
        *error = value;
}

vImageConverterRef vImageConverter_CreateWithCGImageFormat(const vImage_CGImageFormat *srcFormat, const vImage_CGImageFormat *destFormat,
                                                           const CGFloat *backgroundColor, vImage_Flags flags, vImage_Error *error) {
    if (srcFormat == NULL || destFormat == NULL) {
        setError(error, kvImageNullPointerArgument);
        return NULL;
    }
    if (flags & ~(vImage_Flags)(kvImagePrintDiagnosticsToConsole | kvImageDoNotTile)) {
        setError(error, kvImageUnknownFlagsBit);
        return NULL;
    }
    VIFormat s, d;
    vImage_Error e = VIFormatFromCG(srcFormat, &s);
    if (e == kvImageNoError)
        e = VIFormatFromCG(destFormat, &d);
    if (e != kvImageNoError) {
        if ((flags & kvImagePrintDiagnosticsToConsole) != 0)
            fprintf(stderr, "vImageConverter_CreateWithCGImageFormat: %s\n", e == kvImageColorSyncIsAbsent ? "the colour space needs ColorSync" : "invalid image format");
        setError(error, e);
        return NULL;
    }
    struct vImageConverter *c = (struct vImageConverter *)_CFRuntimeCreateInstance(NULL, converterTypeID(), sizeof(struct vImageConverter) - sizeof(CFRuntimeBase), NULL);
    if (c == NULL) {
        setError(error, kvImageMemoryAllocationError);
        return NULL;
    }
    memset((char *)c + sizeof(CFRuntimeBase), 0, sizeof(struct vImageConverter) - sizeof(CFRuntimeBase));
    double bg[3] = { 0, 0, 0 };
    if (backgroundColor != NULL)
        for (int i = 0; i < (VIFormatIsGray(&d) ? 1 : 3); i++)
            bg[i] = backgroundColor[i];
    c->s = s;
    c->d = d;
    c->flags = flags;
    c->plan = VIPlanCreate(&s, &d, bg);
    if (c->plan == NULL) {
        CFRelease(c);
        setError(error, kvImageMemoryAllocationError);
        return NULL;
    }
    if (srcFormat->colorSpace)
        c->srcSpace = CGColorSpaceRetain(srcFormat->colorSpace);
    if (destFormat->colorSpace)
        c->dstSpace = CGColorSpaceRetain(destFormat->colorSpace);
    c->order[0] = kvImageBufferTypeCode_CGFormat;
    c->order[1] = kvImageBufferTypeCode_EndOfList;
    setError(error, kvImageNoError);
    return c;
}

vImageConverterRef vImageConverter_CreateWithColorSyncCodeFragment(CFTypeRef codeFragment, const vImage_CGImageFormat *srcFormat,
                                                                   const vImage_CGImageFormat *destFormat, const CGFloat *backgroundColor,
                                                                   vImage_Flags flags, vImage_Error *error) {
    setError(error, srcFormat == NULL || destFormat == NULL || codeFragment == NULL ? kvImageNullPointerArgument : kvImageColorSyncIsAbsent);
    return NULL;
}

vImageConverterRef vImageConverter_CreateWithCGColorConversionInfo(CGColorConversionInfoRef info, const vImage_CGImageFormat *sFormat,
                                                                   const vImage_CGImageFormat *dFormat, const CGFloat *bg, vImage_Flags flags,
                                                                   vImage_Error *error) {
    setError(error, info == NULL || sFormat == NULL || dFormat == NULL ? kvImageNullPointerArgument : kvImageColorSyncIsAbsent);
    return NULL;
}

void vImageConverter_Retain(vImageConverterRef converter) {
    if (converter)
        CFRetain(converter);
}

void vImageConverter_Release(vImageConverterRef converter) {
    if (converter)
        CFRelease(converter);
}

unsigned long vImageConverter_GetNumberOfSourceBuffers(const vImageConverterRef converter) { return converter ? 1 : 0; }
unsigned long vImageConverter_GetNumberOfDestinationBuffers(const vImageConverterRef converter) { return converter ? 1 : 0; }
const vImageBufferTypeCode *vImageConverter_GetSourceBufferOrder(vImageConverterRef converter) { return converter ? converter->order : NULL; }
const vImageBufferTypeCode *vImageConverter_GetDestinationBufferOrder(vImageConverterRef converter) { return converter ? converter->order : NULL; }

vImage_Error vImageConverter_MustOperateOutOfPlace(const vImageConverterRef converter, const vImage_Buffer *srcs, const vImage_Buffer *dests, vImage_Flags flags) {
    if (converter == NULL)
        return kvImageNullPointerArgument;
    if ((srcs == NULL) != (dests == NULL))
        return kvImageNullPointerArgument;
    if (flags & ~(vImage_Flags)(kvImagePrintDiagnosticsToConsole | kvImageDoNotTile))
        return kvImageUnknownFlagsBit;
    // Pixels that grow cannot be written over the pixels still to be read.
    if (converter->d.bpp > converter->s.bpp)
        return kvImageOutOfPlaceOperationRequired;
    if (srcs != NULL && (srcs->rowBytes != dests->rowBytes || srcs->width != dests->width || srcs->height != dests->height))
        return kvImageOutOfPlaceOperationRequired;
    return kvImageNoError;
}

vImage_Error vImageConvert_AnyToAny(const vImageConverterRef converter, const vImage_Buffer *srcs, const vImage_Buffer *dests, void *tempBuffer, vImage_Flags flags) {
    if (converter == NULL || srcs == NULL || dests == NULL)
        return kvImageNullPointerArgument;
    if (flags & ~(vImage_Flags)(kvImagePrintDiagnosticsToConsole | kvImageDoNotTile | kvImageGetTempBufferSize))
        return kvImageUnknownFlagsBit;
    if (flags & kvImageGetTempBufferSize)
        return 0;   // no scratch space is needed
    if (srcs->height < dests->height || srcs->width < dests->width)
        return kvImageRoiLargerThanInputBuffer;
    if (dests->height == 0 || dests->width == 0)
        return kvImageNoError;
    if (srcs->data == NULL || dests->data == NULL)
        return kvImageInvalidParameter;
    VIPlanRun(converter->plan, srcs->data, srcs->rowBytes, dests->data, dests->rowBytes, dests->width, dests->height);
    return kvImageNoError;
}
