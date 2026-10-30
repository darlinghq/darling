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
#include <string.h>
#include <math.h>
#include <limits.h>

#define KVIMAGE_ALL_VALID_FLAGS ( \
    kvImageLeaveAlphaUnchanged | \
    kvImageCopyRestriction | \
    kvImageDoNotTile | \
    kvImageGetTempBufferSize | \
    kvImageHighQualityResampling | \
    kvImageTruncateKernel | \
    kvImageEdgeExtend | \
    kvImageBackgroundColorFill \
)

typedef struct {
    vImagePixelCount ix0;
    vImagePixelCount ix1;
    float wx;
    float one_minus_wx;
} vImage_ColumnSample;

static inline uint8_t clamp_u8(float val) {
    if (val <= 0.0f) return 0;
    if (val >= 255.0f) return 255;
    return (uint8_t)(val + 0.5f);
}

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
        fprintf(stderr, "vImageScale_ARGB8888 called: src=(%lu,%lu,rb=%zu) dest=(%lu,%lu,rb=%zu) flags=0x%x\n",
                src ? src->width : 0, src ? src->height : 0, src ? src->rowBytes : 0,
                dest ? dest->width : 0, dest ? dest->height : 0, dest ? dest->rowBytes : 0,
                flags);
    }

    if (!src || !dest) {
        return kvImageNullPointerErr;
    }

    if (src->width == 0 || src->height == 0 || dest->width == 0 || dest->height == 0 ||
        src->width > INT_MAX || src->height > INT_MAX ||
        dest->width > INT_MAX || dest->height > INT_MAX ||
        src->width > SIZE_MAX / 4 || dest->width > SIZE_MAX / sizeof(vImage_ColumnSample)) {
        return kvImageInvalidParameter;
    }

    if (src->rowBytes < (size_t)src->width * 4 || dest->rowBytes < (size_t)dest->width * 4 ||
        src->rowBytes > SIZE_MAX / src->height || dest->rowBytes > SIZE_MAX / dest->height) {
        return kvImageInvalidRowBytes;
    }

    if (flags & ~KVIMAGE_ALL_VALID_FLAGS) {
        return kvImageUnknownFlagsBit;
    }

    /* Temporary buffer size query */
    if (flags & kvImageGetTempBufferSize) {
        return 0; /* Direct scaling requires 0 extra scratch memory */
    }

    if (!src->data || !dest->data) {
        return kvImageNullPointerErr;
    }

    /* Fast path: 1:1 scale */
    if (src->width == dest->width && src->height == dest->height) {
        if (flags & kvImageLeaveAlphaUnchanged) {
            for (vImagePixelCount y = 0; y < dest->height; y++) {
                const uint8_t *s = (const uint8_t *)src->data + y * src->rowBytes;
                uint8_t *d = (uint8_t *)dest->data + y * dest->rowBytes;
                for (vImagePixelCount x = 0; x < dest->width; x++) {
                    d[x * 4 + 1] = s[x * 4 + 1];
                    d[x * 4 + 2] = s[x * 4 + 2];
                    d[x * 4 + 3] = s[x * 4 + 3];
                }
            }
        } else {
            for (vImagePixelCount y = 0; y < dest->height; y++) {
                const uint8_t *s = (const uint8_t *)src->data + y * src->rowBytes;
                uint8_t *d = (uint8_t *)dest->data + y * dest->rowBytes;
                if (d != s) {
                    memmove(d, s, dest->width * 4);
                }
            }
        }
        return kvImageNoError;
    }

    /* Precalculate horizontal column resampling positions & weights */
    vImage_ColumnSample stack_cols[256];
    vImage_ColumnSample *cols = NULL;
    if (dest->width <= 256) {
        cols = stack_cols;
    } else {
        cols = (vImage_ColumnSample *)malloc(dest->width * sizeof(vImage_ColumnSample));
    }
    float scale_x = (float)src->width / (float)dest->width;
    float scale_y = (float)src->height / (float)dest->height;

    if (cols != NULL) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            float sx = ((float)x + 0.5f) * scale_x - 0.5f;
            int x0 = (int)floorf(sx);
            int x1 = x0 + 1;
            float wx = sx - (float)x0;
            if (wx < 0.0f) wx = 0.0f;
            else if (wx > 1.0f) wx = 1.0f;

            if (x0 < 0) x0 = 0;
            if (x0 >= (int)src->width) x0 = (int)src->width - 1;
            if (x1 < 0) x1 = 0;
            if (x1 >= (int)src->width) x1 = (int)src->width - 1;

            if (x0 == x1) {
                wx = 0.0f;
            }

            cols[x].ix0 = (vImagePixelCount)x0;
            cols[x].ix1 = (vImagePixelCount)x1;
            cols[x].wx = wx;
            cols[x].one_minus_wx = 1.0f - wx;
        }
    }

    if (cols != NULL) {
        if (!(flags & kvImageLeaveAlphaUnchanged)) {
            for (vImagePixelCount y = 0; y < dest->height; y++) {
                float sy = ((float)y + 0.5f) * scale_y - 0.5f;
                int y0 = (int)floorf(sy);
                int y1 = y0 + 1;
                float wy = sy - (float)y0;
                if (wy < 0.0f) wy = 0.0f;
                else if (wy > 1.0f) wy = 1.0f;

                if (y0 < 0) y0 = 0;
                if (y0 >= (int)src->height) y0 = (int)src->height - 1;
                if (y1 < 0) y1 = 0;
                if (y1 >= (int)src->height) y1 = (int)src->height - 1;

                if (y0 == y1) {
                    wy = 0.0f;
                }
                float one_minus_wy = 1.0f - wy;

                const uint8_t *row0 = (const uint8_t *)src->data + (vImagePixelCount)y0 * src->rowBytes;
                const uint8_t *row1 = (const uint8_t *)src->data + (vImagePixelCount)y1 * src->rowBytes;
                uint8_t *dst_row = (uint8_t *)dest->data + y * dest->rowBytes;

                for (vImagePixelCount x = 0; x < dest->width; x++) {
                    const vImage_ColumnSample *col = &cols[x];
                    const uint8_t *p00 = row0 + col->ix0 * 4;
                    const uint8_t *p10 = row0 + col->ix1 * 4;
                    const uint8_t *p01 = row1 + col->ix0 * 4;
                    const uint8_t *p11 = row1 + col->ix1 * 4;

                    float w00 = col->one_minus_wx * one_minus_wy;
                    float w10 = col->wx * one_minus_wy;
                    float w01 = col->one_minus_wx * wy;
                    float w11 = col->wx * wy;

                    uint8_t *dst_pixel = dst_row + x * 4;
                    dst_pixel[0] = clamp_u8(p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11);
                    dst_pixel[1] = clamp_u8(p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11);
                    dst_pixel[2] = clamp_u8(p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11);
                    dst_pixel[3] = clamp_u8(p00[3] * w00 + p10[3] * w10 + p01[3] * w01 + p11[3] * w11);
                }
            }
        } else {
            for (vImagePixelCount y = 0; y < dest->height; y++) {
                float sy = ((float)y + 0.5f) * scale_y - 0.5f;
                int y0 = (int)floorf(sy);
                int y1 = y0 + 1;
                float wy = sy - (float)y0;
                if (wy < 0.0f) wy = 0.0f;
                else if (wy > 1.0f) wy = 1.0f;

                if (y0 < 0) y0 = 0;
                if (y0 >= (int)src->height) y0 = (int)src->height - 1;
                if (y1 < 0) y1 = 0;
                if (y1 >= (int)src->height) y1 = (int)src->height - 1;

                if (y0 == y1) {
                    wy = 0.0f;
                }
                float one_minus_wy = 1.0f - wy;

                const uint8_t *row0 = (const uint8_t *)src->data + (vImagePixelCount)y0 * src->rowBytes;
                const uint8_t *row1 = (const uint8_t *)src->data + (vImagePixelCount)y1 * src->rowBytes;
                uint8_t *dst_row = (uint8_t *)dest->data + y * dest->rowBytes;

                for (vImagePixelCount x = 0; x < dest->width; x++) {
                    const vImage_ColumnSample *col = &cols[x];
                    const uint8_t *p00 = row0 + col->ix0 * 4;
                    const uint8_t *p10 = row0 + col->ix1 * 4;
                    const uint8_t *p01 = row1 + col->ix0 * 4;
                    const uint8_t *p11 = row1 + col->ix1 * 4;

                    float w00 = col->one_minus_wx * one_minus_wy;
                    float w10 = col->wx * one_minus_wy;
                    float w01 = col->one_minus_wx * wy;
                    float w11 = col->wx * wy;

                    uint8_t *dst_pixel = dst_row + x * 4;
                    dst_pixel[1] = clamp_u8(p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11);
                    dst_pixel[2] = clamp_u8(p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11);
                    dst_pixel[3] = clamp_u8(p00[3] * w00 + p10[3] * w10 + p01[3] * w01 + p11[3] * w11);
                }
            }
        }
    } else {
        /* Inline fallback if malloc failed */
        for (vImagePixelCount y = 0; y < dest->height; y++) {
            float sy = ((float)y + 0.5f) * scale_y - 0.5f;
            int y0 = (int)floorf(sy);
            int y1 = y0 + 1;
            float wy = sy - (float)y0;
            if (wy < 0.0f) wy = 0.0f;
            else if (wy > 1.0f) wy = 1.0f;

            if (y0 < 0) y0 = 0;
            if (y0 >= (int)src->height) y0 = (int)src->height - 1;
            if (y1 < 0) y1 = 0;
            if (y1 >= (int)src->height) y1 = (int)src->height - 1;

            if (y0 == y1) {
                wy = 0.0f;
            }
            float one_minus_wy = 1.0f - wy;

            const uint8_t *row0 = (const uint8_t *)src->data + (vImagePixelCount)y0 * src->rowBytes;
            const uint8_t *row1 = (const uint8_t *)src->data + (vImagePixelCount)y1 * src->rowBytes;
            uint8_t *dst_row = (uint8_t *)dest->data + y * dest->rowBytes;

            for (vImagePixelCount x = 0; x < dest->width; x++) {
                float sx = ((float)x + 0.5f) * scale_x - 0.5f;
                int x0 = (int)floorf(sx);
                int x1 = x0 + 1;
                float wx = sx - (float)x0;
                if (wx < 0.0f) wx = 0.0f;
                else if (wx > 1.0f) wx = 1.0f;

                if (x0 < 0) x0 = 0;
                if (x0 >= (int)src->width) x0 = (int)src->width - 1;
                if (x1 < 0) x1 = 0;
                if (x1 >= (int)src->width) x1 = (int)src->width - 1;

                if (x0 == x1) {
                    wx = 0.0f;
                }
                float one_minus_wx = 1.0f - wx;

                const uint8_t *p00 = row0 + (vImagePixelCount)x0 * 4;
                const uint8_t *p10 = row0 + (vImagePixelCount)x1 * 4;
                const uint8_t *p01 = row1 + (vImagePixelCount)x0 * 4;
                const uint8_t *p11 = row1 + (vImagePixelCount)x1 * 4;

                float w00 = one_minus_wx * one_minus_wy;
                float w10 = wx * one_minus_wy;
                float w01 = one_minus_wx * wy;
                float w11 = wx * wy;

                uint8_t *dst_pixel = dst_row + x * 4;
                if (!(flags & kvImageLeaveAlphaUnchanged)) {
                    dst_pixel[0] = clamp_u8(p00[0] * w00 + p10[0] * w10 + p01[0] * w01 + p11[0] * w11);
                }
                dst_pixel[1] = clamp_u8(p00[1] * w00 + p10[1] * w10 + p01[1] * w01 + p11[1] * w11);
                dst_pixel[2] = clamp_u8(p00[2] * w00 + p10[2] * w10 + p01[2] * w01 + p11[2] * w11);
                dst_pixel[3] = clamp_u8(p00[3] * w00 + p10[3] * w10 + p01[3] * w01 + p11[3] * w11);
            }
        }
    }

    if (cols != NULL && cols != stack_cols) {
        free(cols);
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
