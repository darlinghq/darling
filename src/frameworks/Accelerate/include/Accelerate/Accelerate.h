/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#ifndef _Accelerate_H_
#define _Accelerate_H_

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

typedef unsigned long vImagePixelCount;

typedef struct vImage_Buffer {
    void *data;
    vImagePixelCount height;
    vImagePixelCount width;
    size_t rowBytes;
} vImage_Buffer;

typedef uint32_t vImage_Flags;
typedef ssize_t vImage_Error;
typedef struct vImage_CGImageFormat vImage_CGImageFormat;

enum {
    kvImageNoError = 0,
    kvImageRoiLargerThanInputBuffer = -21766,
    kvImageInvalidKernelSize = -21767,
    kvImageInvalidEdgeStyle = -21768,
    kvImageInvalidOffset_X = -21769,
    kvImageInvalidOffset_Y = -21770,
    kvImageMemoryAllocationError = -21771,
    kvImageNullPointerArgument = -21772,
    kvImageInvalidParameter = -21773,
    kvImageBufferSizeMismatch = -21774,
    kvImageUnknownFlagsBit = -21775,
    kvImageInternalError = -21776,
    kvImageInvalidRowBytes = -21777,
    kvImageInvalidImageFormat = -21778,
    kvImageColorSyncIsAbsent = -21779,
    kvImageOutOfPlaceOperationRequired = -21780,
    kvImageInvalidImageObject = -21781,
    kvImageInvalidCVImageFormat = -21782,
    kvImageUnsupportedConversion = -21783,
    kvImageCoreVideoIsAbsent = -21784
};

enum {
    kvImageNoFlags = 0,
    kvImageLeaveAlphaUnchanged = 1,
    kvImageCopyInPlace = 2,
    kvImageBackgroundColorFill = 4,
    kvImageEdgeExtend = 8,
    kvImageDoNotTile = 16,
    kvImageHighQualityResampling = 32,
    kvImageTruncateKernel = 64,
    kvImageGetTempBufferSize = 128,
    kvImagePrintDiagnosticsToConsole = 256,
    kvImageNoAllocate = 512,
    kvImageHDRContent = 1024,
    kvImageDoNotClamp = 2048
};

vImage_Error vImageBuffer_InitWithCGImage(
    vImage_Buffer *buf,
    const vImage_CGImageFormat *format,
    const void *backgroundColor,
    void *image,
    vImage_Flags flags
);

vImage_Error vImagePermuteChannels_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    const uint8_t permuteMap[4],
    vImage_Flags flags
);

vImage_Error vImageScale_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    void *tempBuffer,
    vImage_Flags flags
);

vImage_Error vImageUnpremultiplyData_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    vImage_Flags flags
);

#endif
