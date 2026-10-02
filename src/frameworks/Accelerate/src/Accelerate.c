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


#include <Accelerate/Accelerate.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

vImage_Error vImageBuffer_InitWithCGImage(
    vImage_Buffer *buf,
    const vImage_CGImageFormat *format,
    const void *backgroundColor,
    void *image,
    vImage_Flags flags
) {
    if (verbose) {
        fprintf(stderr, "stub: vImageBuffer_InitWithCGImage called\n");
    }
    if (!buf) {
        return kvImageNullPointerErr;
    }
    buf->data = NULL;
    buf->height = 0;
    buf->width = 0;
    buf->rowBytes = 0;
    return kvImageInvalidImageFormat;
}

vImage_Error vImagePermuteChannels_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    const uint8_t permuteMap[4],
    vImage_Flags flags
) {
    if (verbose) {
        fprintf(stderr, "stub: vImagePermuteChannels_ARGB8888 called\n");
    }
    return kvImageNoError;
}

vImage_Error vImageScale_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    void *tempBuffer,
    vImage_Flags flags
) {
    if (verbose) {
        fprintf(stderr, "stub: vImageScale_ARGB8888 called\n");
    }
    return kvImageNoError;
}

vImage_Error vImageUnpremultiplyData_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    vImage_Flags flags
) {
    if (verbose) {
        fprintf(stderr, "stub: vImageUnpremultiplyData_ARGB8888 called\n");
    }
    return kvImageNoError;
}
