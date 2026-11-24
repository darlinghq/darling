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

// Private interface between the pixel format conversion core (vImageConvertCore.c, no CoreGraphics) and the vImage API
// (vImageConverter.c, vImageUtilities.c).
#ifndef VIMAGE_INTERNAL_H
#define VIMAGE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// The functions shared between the files of the library are not exported.
#define VI_PRIVATE __attribute__((visibility("hidden")))

// Colour spaces the converter can convert between (by the exact matrices and curves of the named spaces).
typedef enum {
    VISpaceInvalid = 0,
    VISpaceSRGB,            // also device RGB and NULL
    VISpaceLinearSRGB,
    VISpaceP3,              // Display P3
    VISpaceLinearP3,
    VISpaceAdobeRGB,
    VISpaceGenericRGB,
    VISpaceGenericRGBLinear,
    VISpaceGray22,          // device gray and generic gray gamma 2.2 (the sRGB curve, as the original uses)
    VISpaceGray18,          // generic gray
    VISpaceLinearGray
} VISpace;

// kCGBitmapFloatInfoMask of CGImage.h
#define VI_BITMAP_FLOAT_INFO_MASK 0x0F00u

// CGImageAlphaInfo values
enum { VIAlphaNone = 0, VIAlphaPremultipliedLast = 1, VIAlphaPremultipliedFirst = 2, VIAlphaLast = 3, VIAlphaFirst = 4,
       VIAlphaNoneSkipLast = 5, VIAlphaNoneSkipFirst = 6, VIAlphaOnly = 7 };
// CGBitmapInfo byte order values
enum { VIOrderDefault = 0x0000, VIOrder16Little = 0x1000, VIOrder32Little = 0x2000, VIOrder16Big = 0x3000, VIOrder32Big = 0x4000 };

typedef struct {
    uint32_t bpc, bpp;
    uint32_t alphaInfo;     // VIAlpha*
    uint32_t order;         // VIOrder*
    bool isFloat;
    VISpace space;
    bool hasDecode;
    double decode[6];       // min, max per colour component
} VIFormat;

static inline bool VIFormatHasAlpha(const VIFormat *f) {
    return f->alphaInfo == VIAlphaPremultipliedLast || f->alphaInfo == VIAlphaPremultipliedFirst || f->alphaInfo == VIAlphaLast || f->alphaInfo == VIAlphaFirst;
}
static inline bool VIFormatIsGray(const VIFormat *f) { return f->space == VISpaceGray22 || f->space == VISpaceGray18 || f->space == VISpaceLinearGray; }

// The error code for a format that cannot be described (kvImageInvalidImageFormat) or 0.
VI_PRIVATE int VIFormatCheck(const VIFormat *f);

typedef struct VIColorPlan VIColorPlan;
// A conversion between two formats; bg holds the background colour (in the destination space, one value per colour component) or NULL.
VI_PRIVATE VIColorPlan *VIPlanCreate(const VIFormat *src, const VIFormat *dst, const double *bg);
VI_PRIVATE void VIPlanDestroy(VIColorPlan *plan);
// Converts width x height pixels. The rows of src and dst may alias exactly.
VI_PRIVATE void VIPlanRun(const VIColorPlan *plan, const void *src, size_t srcRowBytes, void *dst, size_t dstRowBytes, size_t width, size_t height);

#endif
