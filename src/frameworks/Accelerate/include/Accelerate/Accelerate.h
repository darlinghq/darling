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
    kvImageRoiLargerThanInputFile = -21766,
    kvImageInvalidRowBytes = -21767,
    kvImageInvalidImageFormat = -21768,
    kvImageMemoryAllocationError = -21769,
    kvImageNullPointerErr = -21770,
    kvImageUnknownFlagsBit = -21771,
    kvImageInvalidParameter = -21772
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
