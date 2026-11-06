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
#include <string.h>
#include <math.h>
#include <limits.h>

#define KVIMAGE_ALL_VALID_FLAGS ( \
    kvImageLeaveAlphaUnchanged | \
    kvImageCopyInPlace | \
    kvImageBackgroundColorFill | \
    kvImageEdgeExtend | \
    kvImageDoNotTile | \
    kvImageHighQualityResampling | \
    kvImageTruncateKernel | \
    kvImageGetTempBufferSize | \
    kvImagePrintDiagnosticsToConsole | \
    kvImageNoAllocate | \
    kvImageHDRContent | \
    kvImageDoNotClamp \
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

vImage_Error vImageBuffer_InitWithCGImage(
    vImage_Buffer *buf,
    const vImage_CGImageFormat *format,
    const void *backgroundColor,
    void *image,
    vImage_Flags flags
) {
    if (!buf) {
        return kvImageNullPointerArgument;
    }
    buf->data = NULL;
    buf->height = 0;
    buf->width = 0;
    buf->rowBytes = 0;
    return kvImageUnsupportedConversion;
}

vImage_Error vImagePermuteChannels_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    const uint8_t permuteMap[4],
    vImage_Flags flags
) {
    // Not implemented. Report that instead of claiming the pixels were permuted.
    return kvImageUnsupportedConversion;
}

vImage_Error vImageScale_ARGB8888(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    void *tempBuffer,
    vImage_Flags flags
) {
    if (!src || !dest) {
        return kvImageNullPointerArgument;
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
        return kvImageNullPointerArgument;
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
    // Not implemented. Report that instead of claiming the pixels were converted.
    return kvImageUnsupportedConversion;
}

/* Flag and error values of the SDK's vImage_Types.h, which callers are compiled with. The enum in
   this directory's header numbers its flags differently. */
enum {
    sdkLeaveAlphaUnchanged = 1,
    sdkCopyInPlace = 2,
    sdkBackgroundColorFill = 4,
    sdkEdgeExtend = 8,
    sdkTruncateKernel = 64,
    sdkGetTempBufferSize = 128
};
enum {
    sdkRoiLargerThanInputBuffer = -21766,
    sdkInvalidKernelSize = -21767,
    sdkInvalidEdgeStyle = -21768,
    sdkMemoryAllocationError = -21771,
    sdkNullPointerArgument = -21772
};

/* One line of one channel: the mean over a window of `size` around each sample. */
static void boxLine(const uint8_t *in, size_t inStep, size_t count, uint8_t *out, size_t outStep,
                    size_t first, size_t outCount, uint32_t size, uint8_t background, vImage_Flags edge,
                    uint64_t *sums)
{
    sums[0] = 0;
    for (size_t i = 0; i < count; i++)
        sums[i + 1] = sums[i] + in[i * inStep];
    int64_t reach = size / 2;
    for (size_t i = 0; i < outCount; i++) {
        int64_t center = (int64_t)(first + i);
        int64_t low = center - reach, high = center + reach;
        int64_t left = low < 0 ? -low : 0, right = high >= (int64_t)count ? high - (int64_t)count + 1 : 0;
        low = low < 0 ? 0 : low;
        high = high >= (int64_t)count ? (int64_t)count - 1 : high;
        uint64_t sum = sums[high + 1] - sums[low], divisor = size;
        if (edge & sdkEdgeExtend)
            sum += (uint64_t)left * in[0] + (uint64_t)right * in[(count - 1) * inStep];
        else if (edge & sdkBackgroundColorFill)
            sum += (uint64_t)(left + right) * background;
        else if (edge & sdkTruncateKernel)
            divisor = (uint64_t)(high - low + 1);
        else if ((left || right) && (edge & sdkCopyInPlace))
            sum = (uint64_t)in[center * inStep] * size;
        out[i * outStep] = (uint8_t)((sum + divisor / 2) / divisor);
    }
}

/* From the SDK's Convolution.h: a box convolution of each channel, with exactly one edge style and
   odd kernel sizes. It needs no caller-supplied scratch memory, so the size it reports is 0. */
vImage_Error vImageBoxConvolve_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer,
    vImagePixelCount srcOffsetToROI_X, vImagePixelCount srcOffsetToROI_Y, uint32_t kernel_height, uint32_t kernel_width,
    const uint8_t *backgroundColor, vImage_Flags flags)
{
    if (flags & sdkGetTempBufferSize)
        return 0;
    if (src == NULL || dest == NULL || src->data == NULL || dest->data == NULL)
        return sdkNullPointerArgument;
    if (!(kernel_height & 1) || !(kernel_width & 1))
        return sdkInvalidKernelSize;
    vImage_Flags edge = flags & (sdkCopyInPlace | sdkBackgroundColorFill | sdkEdgeExtend | sdkTruncateKernel);
    if (edge == 0 || (edge & (edge - 1)) != 0)
        return sdkInvalidEdgeStyle;
    if (edge == sdkBackgroundColorFill && backgroundColor == NULL)
        return sdkNullPointerArgument;
    if (srcOffsetToROI_X + dest->width > src->width || srcOffsetToROI_Y + dest->height > src->height)
        return sdkRoiLargerThanInputBuffer;

    /* Horizontal into a copy of the ROI's columns over every source row, then vertical into dest, so
       that the source and destination may be the same buffer. */
    size_t width = dest->width, height = src->height, longest = src->width > src->height ? src->width : src->height;
    uint8_t *rows = malloc(width * height * 4);
    uint64_t *sums = malloc((longest + 1) * sizeof(uint64_t));
    if (rows == NULL || sums == NULL) {
        free(rows);
        free(sums);
        return sdkMemoryAllocationError;
    }
    for (size_t channel = 0; channel < 4; channel++) {
        uint8_t background = backgroundColor != NULL ? backgroundColor[channel] : 0;
        for (size_t y = 0; y < height; y++)
            boxLine((const uint8_t *)src->data + y * src->rowBytes + channel, 4, src->width,
                    rows + y * width * 4 + channel, 4, srcOffsetToROI_X, width, kernel_width, background, edge, sums);
        for (size_t x = 0; x < width; x++) {
            size_t destRowBytes = dest->rowBytes;
            boxLine(rows + x * 4 + channel, width * 4, height,
                    (uint8_t *)dest->data + x * 4 + channel, destRowBytes, srcOffsetToROI_Y, dest->height,
                    kernel_height, background, edge, sums);
        }
    }
    free(rows);
    free(sums);
    (void)tempBuffer;
    return kvImageNoError;
}
