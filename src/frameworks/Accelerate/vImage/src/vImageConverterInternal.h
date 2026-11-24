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

#ifndef VIMAGE_CONVERTER_INTERNAL_H
#define VIMAGE_CONVERTER_INTERNAL_H

#include <Accelerate/vImage.h>
#include <CoreFoundation/CFRuntime.h>
#include "vImageInternal.h"

// A CoreFoundation object (the header declares vImageConverterRef as a CoreFoundation type, which Swift retains and releases with CFRetain
// and CFRelease).
struct vImageConverter {
    CFRuntimeBase base;
    VIFormat s, d;
    vImage_Flags flags;
    VIColorPlan *plan;
    CGColorSpaceRef srcSpace, dstSpace;
    vImageBufferTypeCode order[2];
};

VI_PRIVATE VISpace VIClassifyColorSpace(CGColorSpaceRef cs);
VI_PRIVATE vImage_Error VIFormatFromCG(const vImage_CGImageFormat *f, VIFormat *out);

#endif
