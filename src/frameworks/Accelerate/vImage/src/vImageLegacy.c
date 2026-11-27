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


#include <Accelerate/vImage.h>
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

vImage_Error vImageBufferFill_ARGB8888(
    const vImage_Buffer *dest,
    const Pixel_8888 color,
    vImage_Flags flags
) {
    if (!dest || !color) {
        return kvImageNullPointerArgument;
    }

    // An empty buffer has no pixels to write, so it is not an error and its data pointer is unused.
    if (dest->width == 0 || dest->height == 0) {
        return kvImageNoError;
    }

    if (!dest->data) {
        return kvImageNullPointerArgument;
    }

    if (dest->width > SIZE_MAX / 4 || dest->rowBytes < (size_t)dest->width * 4 ||
        dest->rowBytes > SIZE_MAX / dest->height) {
        return kvImageInvalidRowBytes;
    }

    // QUIRK: the flags are not validated. kvImageNoFlags and kvImageDoNotTile are the documented
    // values, but unknown bits and the other kvImage flags are accepted and ignored (alpha is never
    // preserved) instead of returning kvImageUnknownFlagsBit. Evidence: black-box observation on
    // macOS 26.7 (tier 2). Test: vimage_buffer_fill_argb8888 (unknown flags, "quirk" checks).
    (void)flags;

    uint8_t* row = (uint8_t*)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            memcpy(row + x * 4, color, 4);
        }
    }
    return kvImageNoError;
}

// IEEE 754-2008 binary16 (1 sign, 5 exponent with bias 15, 10 fraction bits) to binary32
// (1, 8 with bias 127, 23). Every finite binary16 value is exactly representable in binary32.
static float half_to_float(uint16_t half) {
    uint32_t sign = (uint32_t)(half & 0x8000) << 16;
    uint32_t exponent = (half >> 10) & 0x1f;
    uint32_t fraction = half & 0x3ff;
    uint32_t bits;

    if (exponent == 0x1f) {
        // Infinity, or a NaN whose payload moves up unchanged. IEEE 754-2008 convertFormat returns
        // a quiet NaN for a signalling input, so the quiet bit is set when the fraction is not zero.
        bits = sign | 0x7f800000u | (fraction << 13);
        if (fraction != 0) {
            bits |= 0x400000u;
        }
    } else if (exponent != 0) {
        bits = sign | ((exponent + 112) << 23) | (fraction << 13);
    } else if (fraction == 0) {
        bits = sign;
    } else {
        // Subnormal: fraction * 2^-24. Renormalize so the leading set bit becomes the implicit one.
        int lead = 9;
        while (!(fraction & (1u << lead))) {
            lead--;
        }
        bits = sign | ((uint32_t)(lead + 103) << 23) | ((fraction << (23 - lead)) & 0x7fffffu);
    }

    float result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

// binary32 to binary16, rounding to nearest with ties to even (the default IEEE 754 rounding).
// Magnitudes that round past the largest finite half become infinity, magnitudes below half of the
// smallest subnormal become a signed zero, and a NaN stays a NaN (quiet bit set, top payload bits kept).
static uint16_t float_to_half(float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    uint32_t sign = (bits >> 16) & 0x8000u;
    uint32_t exponent = (bits >> 23) & 0xff;
    uint32_t fraction = bits & 0x7fffffu;

    if (exponent == 0xff) {
        if (fraction == 0) {
            return (uint16_t)(sign | 0x7c00u);
        }
        return (uint16_t)(sign | 0x7c00u | 0x200u | (fraction >> 13));
    }

    // Exponent field of the result before rounding: unbiased exponent plus 15.
    int32_t half_exponent = (int32_t)exponent - 112;
    if (half_exponent >= 0x1f) {
        return (uint16_t)(sign | 0x7c00u);
    }

    uint32_t magnitude;
    uint32_t shift;
    if (half_exponent >= 1) {
        magnitude = ((uint32_t)half_exponent << 10) | (fraction >> 13);
        shift = 13;
    } else if (half_exponent >= -10) {
        // Subnormal result: shift the significand (with its implicit one) down to units of 2^-24.
        magnitude = (fraction | 0x800000u) >> (14 - half_exponent);
        shift = 14 - (uint32_t)half_exponent;
        fraction |= 0x800000u;
    } else {
        return (uint16_t)sign;
    }

    uint32_t discarded = fraction & ((1u << shift) - 1);
    uint32_t halfway = 1u << (shift - 1);
    if (discarded > halfway || (discarded == halfway && (magnitude & 1))) {
        // A carry out of the fraction raises the exponent, which is the correct rounded result,
        // including the step from the largest finite half to infinity.
        magnitude++;
    }
    return (uint16_t)(sign | magnitude);
}

// Flag bits the two planar half/single converters refuse with kvImageUnknownFlagsBit.
// Observed on macOS 26.7.3 and 10.14.4 (black-box observation through the public API): kvImageNoFlags,
// kvImageDoNotTile and kvImageGetTempBufferSize are accepted; kvImageLeaveAlphaUnchanged,
// kvImageHighQualityResampling and a bit outside the defined flags (0x40000000) give
// kvImageUnknownFlagsBit. The other defined flags were not observed and are accepted.
// Test: tests/src/accelerate_vimage_half_float.c
#define PLANAR_CONVERT_REFUSED_FLAGS \
    ((vImage_Flags)(kvImageLeaveAlphaUnchanged | kvImageHighQualityResampling | ~(vImage_Flags)0xfff))

// The checks shared by the two planar half/single converters, in the order macOS applies them (observed
// on macOS 26.7.3 and 10.14.4, identical on both; the documentation lists no errors for these functions):
//  1. a NULL buffer pointer gives kvImageNullPointerArgument, even with kvImageGetTempBufferSize;
//  2. a flag bit from PLANAR_CONVERT_REFUSED_FLAGS gives kvImageUnknownFlagsBit (the order of this check
//     against the one above is not observed);
//  3. kvImageGetTempBufferSize returns 0 without looking at the buffers: no temporary buffer is needed and
//     nothing is written;
//  4. a destination wider or taller than the source gives kvImageRoiLargerThanInputBuffer (also for an
//     empty source and a destination with pixels). A narrower or shorter destination is not an error; its
//     width and height are what gets converted, and the source rowBytes and dimensions beyond that are not
//     looked at;
//  5. an empty destination succeeds without touching either data pointer.
// There is no rowBytes validation: a rowBytes smaller than a row succeeds on macOS, and so here.
// The NULL data pointer of a non-empty buffer crashes macOS; kvImageNullPointerArgument is returned
// instead, the documented error for a NULL pointer. Returns kvImageNoError when there is something to do.
static vImage_Error check_planar_convert_buffers(const vImage_Buffer *src, const vImage_Buffer *dest,
                                                 vImage_Flags flags, int *has_pixels) {
    *has_pixels = 0;
    if (!src || !dest) {
        return kvImageNullPointerArgument;
    }
    if (flags & PLANAR_CONVERT_REFUSED_FLAGS) {
        return kvImageUnknownFlagsBit;
    }
    if (flags & kvImageGetTempBufferSize) {
        return kvImageNoError;
    }
    if (dest->width > src->width || dest->height > src->height) {
        return kvImageRoiLargerThanInputBuffer;
    }
    if (dest->width == 0 || dest->height == 0) {
        return kvImageNoError;
    }
    if (!src->data || !dest->data) {
        return kvImageNullPointerArgument;
    }
    *has_pixels = 1;
    return kvImageNoError;
}

// The planar half to single converter. The rows are addressed by each buffer's own rowBytes and the
// destination's width and height are converted. The buffers must not overlap (an in-place
// conversion is not documented).
vImage_Error vImageConvert_Planar16FtoPlanarF(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    vImage_Flags flags
) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            uint16_t half;
            memcpy(&half, src_row + x * sizeof(half), sizeof(half));
            float value = half_to_float(half);
            memcpy(dest_row + x * sizeof(value), &value, sizeof(value));
        }
    }
    return kvImageNoError;
}

// The planar single to half converter; the checks and addressing are those of the function above. The
// rounding is to nearest with ties to even, as observed on macOS 26.7.3 and 10.14.4 (black-box
// observation; the values are listed in tests/src/accelerate_vimage_half_float.c).
vImage_Error vImageConvert_PlanarFtoPlanar16F(
    const vImage_Buffer *src,
    const vImage_Buffer *dest,
    vImage_Flags flags
) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            float value;
            memcpy(&value, src_row + x * sizeof(value), sizeof(value));
            uint16_t half = float_to_half(value);
            memcpy(dest_row + x * sizeof(half), &half, sizeof(half));
        }
    }
    return kvImageNoError;
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

/* The sum of the `size` samples centred on sample `center` of a line of `count` samples, given its
   prefix sums. Samples outside the line follow the edge style: kvImageEdgeExtend repeats the nearest
   end sample, kvImageBackgroundColorFill uses `outside`; the other styles leave them out. */
static uint64_t boxWindowSum(const uint32_t *line, const uint64_t *prefix, size_t count, int64_t center,
                             uint32_t size, vImage_Flags edge, uint64_t outside)
{
    int64_t reach = size / 2;
    int64_t low = center - reach, high = center + reach;
    uint64_t before = low < 0 ? (uint64_t)-low : 0;
    uint64_t after = high >= (int64_t)count ? (uint64_t)(high - (int64_t)count + 1) : 0;
    low = low < 0 ? 0 : low;
    high = high >= (int64_t)count ? (int64_t)count - 1 : high;
    uint64_t sum = prefix[high + 1] - prefix[low];
    if (edge == kvImageEdgeExtend)
        sum += before * line[0] + after * line[count - 1];
    else if (edge == kvImageBackgroundColorFill)
        sum += (before + after) * outside;
    return sum;
}

/* How many of the `size` samples centred on sample `center` of a line of `count` samples lie on it. */
static uint64_t boxSamplesInside(int64_t center, size_t count, uint32_t size)
{
    int64_t low = center - size / 2, high = center + size / 2;
    low = low < 0 ? 0 : low;
    high = high >= (int64_t)count ? (int64_t)count - 1 : high;
    return (uint64_t)(high - low + 1);
}

static int boxWindowLeaves(int64_t center, size_t count, uint32_t size)
{
    return center - size / 2 < 0 || center + size / 2 >= (int64_t)count;
}

static void boxPrefix(const uint32_t *line, size_t count, uint64_t *prefix)
{
    prefix[0] = 0;
    for (size_t i = 0; i < count; i++)
        prefix[i + 1] = prefix[i] + line[i];
}

/* From the public Convolution.h: the mean of each channel over a kernel_height x kernel_width window.
   The window is summed exactly and divided once, rounding to the nearest integer. It is placed in the
   source image, and samples beyond the source's edges follow the one edge style in `flags`:
   kvImageEdgeExtend repeats the edge samples, kvImageBackgroundColorFill uses backgroundColor (zero
   when it is NULL), kvImageTruncateKernel averages the samples inside the image and kvImageCopyInPlace
   copies the source pixel where the window leaves the image. With kvImageLeaveAlphaUnchanged the
   first channel (alpha) is copied from the source instead of being averaged. No scratch memory is needed, so the
   temporary buffer size reported is 0. */
vImage_Error vImageBoxConvolve_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer,
    vImagePixelCount srcOffsetToROI_X, vImagePixelCount srcOffsetToROI_Y, uint32_t kernel_height, uint32_t kernel_width,
    const uint8_t *backgroundColor, vImage_Flags flags)
{
    if (flags & kvImageGetTempBufferSize)
        return 0;
    if (src == NULL || dest == NULL || src->data == NULL || dest->data == NULL)
        return kvImageNullPointerArgument;
    if (!(kernel_height & 1) || !(kernel_width & 1))
        return kvImageInvalidKernelSize;
    vImage_Flags edge = flags & (kvImageCopyInPlace | kvImageBackgroundColorFill | kvImageEdgeExtend |
                                 kvImageTruncateKernel);
    if (edge == 0 || (edge & (edge - 1)) != 0)
        return kvImageInvalidEdgeStyle;
    if (srcOffsetToROI_X > src->width || srcOffsetToROI_Y > src->height ||
        dest->width > src->width - srcOffsetToROI_X || dest->height > src->height - srcOffsetToROI_Y)
        return kvImageBufferSizeMismatch;
    if (dest->width == 0 || dest->height == 0)
        return kvImageNoError;

    size_t srcWidth = src->width, srcHeight = src->height, width = dest->width, height = dest->height;
    size_t longest = srcWidth > srcHeight ? srcWidth : srcHeight;
    /* The horizontal sums of every source row for each output column, a line of samples, its prefix
       sums and the vertical sums of one output column. */
    uint32_t *rows = malloc(srcHeight * width * sizeof(uint32_t));
    uint32_t *line = malloc(longest * sizeof(uint32_t));
    uint64_t *prefix = malloc((longest + 1) * sizeof(uint64_t));
    /* Output pixels are written after the source may have been overwritten (in place), so what is
       copied from the source (copy in place, an unchanged alpha channel) comes from a copy of the ROI. */
    int needsROI = edge == kvImageCopyInPlace || (flags & kvImageLeaveAlphaUnchanged);
    uint8_t *roi = needsROI ? malloc(width * height * 4) : NULL;
    if (rows == NULL || line == NULL || prefix == NULL || (needsROI && roi == NULL)) {
        free(rows);
        free(line);
        free(prefix);
        free(roi);
        return kvImageMemoryAllocationError;
    }
    for (size_t y = 0; roi != NULL && y < height; y++)
        memcpy(roi + y * width * 4, (const uint8_t *)src->data + (srcOffsetToROI_Y + y) * src->rowBytes +
               srcOffsetToROI_X * 4, width * 4);

    for (size_t channel = 0; channel < 4; channel++) {
        if (channel == 0 && (flags & kvImageLeaveAlphaUnchanged)) {
            for (size_t y = 0; y < height; y++)
                for (size_t x = 0; x < width; x++)
                    ((uint8_t *)dest->data)[y * dest->rowBytes + x * 4] = roi[(y * width + x) * 4];
            continue;
        }
        uint64_t background = backgroundColor != NULL ? backgroundColor[channel] : 0;
        for (size_t y = 0; y < srcHeight; y++) {
            const uint8_t *in = (const uint8_t *)src->data + y * src->rowBytes + channel;
            for (size_t x = 0; x < srcWidth; x++)
                line[x] = in[x * 4];
            boxPrefix(line, srcWidth, prefix);
            for (size_t x = 0; x < width; x++)
                rows[y * width + x] = (uint32_t)boxWindowSum(line, prefix, srcWidth,
                    (int64_t)(srcOffsetToROI_X + x), kernel_width, edge, background);
        }
        for (size_t x = 0; x < width; x++) {
            for (size_t y = 0; y < srcHeight; y++)
                line[y] = rows[y * width + x];
            boxPrefix(line, srcHeight, prefix);
            for (size_t y = 0; y < height; y++) {
                int64_t cx = (int64_t)(srcOffsetToROI_X + x), cy = (int64_t)(srcOffsetToROI_Y + y);
                uint8_t *out = (uint8_t *)dest->data + y * dest->rowBytes + x * 4 + channel;
                if (edge == kvImageCopyInPlace && (boxWindowLeaves(cx, srcWidth, kernel_width) ||
                                                   boxWindowLeaves(cy, srcHeight, kernel_height))) {
                    *out = roi[(y * width + x) * 4 + channel];
                    continue;
                }
                uint64_t sum = boxWindowSum(line, prefix, srcHeight, cy, kernel_height, edge,
                                            (uint64_t)kernel_width * background);
                uint64_t count = (uint64_t)kernel_width * kernel_height;
                if (edge == kvImageTruncateKernel)
                    count = boxSamplesInside(cx, srcWidth, kernel_width) * boxSamplesInside(cy, srcHeight, kernel_height);
                *out = (uint8_t)((sum + count / 2) / count);
            }
        }
    }
    free(rows);
    free(line);
    free(prefix);
    free(roi);
    (void)tempBuffer;
    return kvImageNoError;
}

static inline uint8_t expand5to8(uint16_t v) {
    return (uint8_t)((v * 255u + 15u) / 31u);
}

static inline uint8_t expand6to8(uint16_t v) {
    return (uint8_t)((v * 255u + 31u) / 63u);
}

static inline uint16_t quantize8to5(uint8_t c) {
    return (uint16_t)((c * 31u + 127u) / 255u);
}

static inline uint16_t quantize8to6(uint8_t c) {
    return (uint16_t)((c * 63u + 127u) / 255u);
}

vImage_Error vImageConvert_ARGB1555toARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            uint16_t p;
            memcpy(&p, src_row + x * 2, sizeof(p));
            uint8_t *out = dest_row + x * 4;
            out[0] = (p & 0x8000u) ? 0xFFu : 0x00u;
            out[1] = expand5to8((p >> 10) & 0x1Fu);
            out[2] = expand5to8((p >> 5) & 0x1Fu);
            out[3] = expand5to8(p & 0x1Fu);
        }
    }
    return kvImageNoError;
}

vImage_Error vImageConvert_ARGB8888toARGB1555(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            const uint8_t *in = src_row + x * 4;
            uint16_t a = (in[0] >= 128) ? 1u : 0u;
            uint16_t r = quantize8to5(in[1]);
            uint16_t g = quantize8to5(in[2]);
            uint16_t b = quantize8to5(in[3]);
            uint16_t p = (uint16_t)((a << 15) | (r << 10) | (g << 5) | b);
            memcpy(dest_row + x * 2, &p, sizeof(p));
        }
    }
    return kvImageNoError;
}

static vImage_Error convert_rgb565_to_8888(uint8_t alpha, const vImage_Buffer *src,
                                           const vImage_Buffer *dest, vImage_Flags flags, int bgra) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            uint16_t p;
            memcpy(&p, src_row + x * 2, sizeof(p));
            uint8_t r = expand5to8((p >> 11) & 0x1Fu);
            uint8_t g = expand6to8((p >> 5) & 0x3Fu);
            uint8_t b = expand5to8(p & 0x1Fu);
            uint8_t *out = dest_row + x * 4;
            if (bgra) {
                out[0] = b; out[1] = g; out[2] = r; out[3] = alpha;
            } else {
                out[0] = alpha; out[1] = r; out[2] = g; out[3] = b;
            }
        }
    }
    return kvImageNoError;
}

vImage_Error vImageConvert_RGB565toARGB8888(uint8_t alpha, const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    return convert_rgb565_to_8888(alpha, src, dest, flags, 0);
}

vImage_Error vImageConvert_RGB565toBGRA8888(uint8_t alpha, const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    return convert_rgb565_to_8888(alpha, src, dest, flags, 1);
}

static vImage_Error convert_8888_to_rgb565(const vImage_Buffer *src, const vImage_Buffer *dest,
                                           vImage_Flags flags, int bgra) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            const uint8_t *in = src_row + x * 4;
            uint8_t r = bgra ? in[2] : in[0];
            uint8_t g = in[1];
            uint8_t b = bgra ? in[0] : in[2];
            uint16_t p = (uint16_t)((quantize8to5(r) << 11) |
                                    (quantize8to6(g) << 5) |
                                    quantize8to5(b));
            memcpy(dest_row + x * 2, &p, sizeof(p));
        }
    }
    return kvImageNoError;
}

vImage_Error vImageConvert_BGRA8888toRGB565(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    return convert_8888_to_rgb565(src, dest, flags, 1);
}

vImage_Error vImageConvert_RGBA8888toRGB565(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    return convert_8888_to_rgb565(src, dest, flags, 0);
}

vImage_Error vImageConvert_Planar4toPlanar8(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags) {
    int has_pixels;
    vImage_Error error = check_planar_convert_buffers(src, dest, flags, &has_pixels);
    if (error != kvImageNoError || !has_pixels) {
        return error;
    }

    const uint8_t *src_row = (const uint8_t *)src->data;
    uint8_t *dest_row = (uint8_t *)dest->data;
    for (vImagePixelCount y = 0; y < dest->height; y++, src_row += src->rowBytes, dest_row += dest->rowBytes) {
        for (vImagePixelCount x = 0; x < dest->width; x++) {
            uint8_t byte = src_row[x >> 1];
            uint8_t nibble = (x & 1) ? (byte & 0x0Fu) : (byte >> 4);
            dest_row[x] = (uint8_t)((nibble << 4) | nibble);
        }
    }
    return kvImageNoError;
}

// Convolves four-channel 8-bit pixels (in any channel order) with an odd-sized kernel, as the public
// vImage documentation describes it: each channel of a destination pixel is
// (sum(kernel * source) + divisor / 2) / divisor, clamped to 0..255, where a divisor of 0 stands for 1. A
// kernel pixel outside the source takes the nearest source pixel (kvImageEdgeExtend), the background
// colour (kvImageBackgroundColorFill), or makes the destination pixel a copy of its source pixel
// (kvImageCopyInPlace); with kvImageTruncateKernel missing pixels are left out and the divisor is
// scaled to the share of the kernel's weight that remains.
// kvImageLeaveAlphaUnchanged copies the first channel of each source pixel.
vImage_Error vImageConvolve_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer,
                                     vImagePixelCount srcOffsetToROI_X, vImagePixelCount srcOffsetToROI_Y,
                                     const int16_t *kernel, uint32_t kernel_height, uint32_t kernel_width,
                                     int32_t divisor, const Pixel_8888 backgroundColor, vImage_Flags flags)
{
    (void)tempBuffer;
    if (flags & kvImageGetTempBufferSize)
        return 0;
    if (src == NULL || dest == NULL || kernel == NULL)
        return kvImageNullPointerArgument;
    if (!(kernel_height & 1) || !(kernel_width & 1))
        return kvImageInvalidKernelSize;
    vImage_Flags edge = flags & (kvImageCopyInPlace | kvImageBackgroundColorFill | kvImageEdgeExtend |
                                 kvImageTruncateKernel);
    if (edge == 0 || (edge & (edge - 1)) != 0)
        return kvImageInvalidEdgeStyle;
    if (dest->width > src->width || dest->height > src->height)
        return kvImageRoiLargerThanInputBuffer;
    if (srcOffsetToROI_X > src->width || dest->width > src->width - srcOffsetToROI_X)
        return kvImageInvalidOffset_X;
    if (srcOffsetToROI_Y > src->height || dest->height > src->height - srcOffsetToROI_Y)
        return kvImageInvalidOffset_Y;
    if (dest->width == 0 || dest->height == 0)
        return kvImageNoError;
    if (src->data == NULL || dest->data == NULL)
        return kvImageNullPointerArgument;

    int64_t full_sum = 0;
    for (size_t i = 0; i < (size_t)kernel_height * kernel_width; i++)
        full_sum += kernel[i];
    int64_t div = divisor != 0 ? divisor : 1;

    int64_t srcWidth = (int64_t)src->width, srcHeight = (int64_t)src->height;
    size_t width = dest->width, height = dest->height;
    if (width > SIZE_MAX / 4 / height)
        return kvImageMemoryAllocationError;
    // The destination may not alias the source, but the result is staged anyway so that a failure
    // leaves the destination whole.
    uint8_t *roi = malloc(width * height * 4);
    if (roi == NULL)
        return kvImageMemoryAllocationError;

    int64_t half_h = (int64_t)(kernel_height / 2);
    int64_t half_w = (int64_t)(kernel_width / 2);
    const uint8_t *src_data = (const uint8_t *)src->data;

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            int64_t cx = (int64_t)(srcOffsetToROI_X + x);
            int64_t cy = (int64_t)(srcOffsetToROI_Y + y);
            const uint8_t *center = src_data + cy * src->rowBytes + cx * 4;
            uint8_t *out = roi + (y * width + x) * 4;

            for (size_t c = 0; c < 4; c++) {
                if (c == 0 && (flags & kvImageLeaveAlphaUnchanged)) {
                    out[c] = center[c];
                    continue;
                }
                int missing = 0;
                int64_t acc = 0;
                int64_t kept_weight = 0;
                for (uint32_t ky = 0; ky < kernel_height; ky++) {
                    int64_t sy = cy + (int64_t)ky - half_h;
                    for (uint32_t kx = 0; kx < kernel_width; kx++) {
                        int64_t sx = cx + (int64_t)kx - half_w;
                        int16_t k = kernel[(size_t)ky * kernel_width + kx];
                        if (sx < 0 || sx >= srcWidth || sy < 0 || sy >= srcHeight) {
                            missing = 1;
                            if (edge == kvImageEdgeExtend) {
                                int64_t ex = sx < 0 ? 0 : (sx >= srcWidth ? srcWidth - 1 : sx);
                                int64_t ey = sy < 0 ? 0 : (sy >= srcHeight ? srcHeight - 1 : sy);
                                acc += (int64_t)k * src_data[ey * src->rowBytes + ex * 4 + c];
                            } else if (edge == kvImageBackgroundColorFill) {
                                acc += (int64_t)k * (backgroundColor ? backgroundColor[c] : 0);
                            }
                        } else {
                            acc += (int64_t)k * src_data[sy * src->rowBytes + sx * 4 + c];
                            kept_weight += k;
                        }
                    }
                }
                if (missing && edge == kvImageCopyInPlace) {
                    out[c] = center[c];
                    continue;
                }
                int64_t num = acc, d = div;
                if (missing && edge == kvImageTruncateKernel) {
                    // The documentation leaves the rounding of the adjusted divisor open ("adjusted
                    // accordingly"). This computes the exactly rounded result, which was observed to
                    // differ from macOS 26.7.3 by at most 1 (7200 edge samples, three kernels). A
                    // kernel that keeps no weight, or whose full weight is zero, gives 0, as observed.
                    // Test: vimage_legacy_convert.c (truncate cases, tolerance 1).
                    if (kept_weight == 0 || full_sum == 0) {
                        out[c] = 0;
                        continue;
                    }
                    num = acc * full_sum;
                    d = div * kept_weight;
                }
                // The documented rounding, with C integer division.
                int64_t val = (num + d / 2) / d;
                out[c] = (uint8_t)(val < 0 ? 0 : (val > 255 ? 255 : val));
            }
        }
    }

    for (size_t y = 0; y < height; y++)
        memcpy((uint8_t *)dest->data + y * dest->rowBytes, roi + y * width * 4, width * 4);
    free(roi);
    return kvImageNoError;
}
