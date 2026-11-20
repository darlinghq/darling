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

#ifndef SIMD_CONVERSION_H
#define SIMD_CONVERSION_H

#include <simd/base.h>
#include <simd/vector_types.h>

/*
 * Element-wise vector type conversion functions:
 * simd_char, simd_uchar, simd_short, simd_ushort, simd_int, simd_uint,
 * simd_long, simd_ulong, simd_float, simd_double.
 */

#define SIMD_CONV(DST_NAME, SRC_TYPE, N) \
SIMD_FN simd_##DST_NAME##N simd_##DST_NAME(SRC_TYPE x) { \
    return __builtin_convertvector(x, simd_##DST_NAME##N); \
}

#define SIMD_CONV_ALL_SRC(DST_NAME, N) \
    SIMD_CONV(DST_NAME, simd_char##N, N) \
    SIMD_CONV(DST_NAME, simd_uchar##N, N) \
    SIMD_CONV(DST_NAME, simd_short##N, N) \
    SIMD_CONV(DST_NAME, simd_ushort##N, N) \
    SIMD_CONV(DST_NAME, simd_int##N, N) \
    SIMD_CONV(DST_NAME, simd_uint##N, N) \
    SIMD_CONV(DST_NAME, simd_long##N, N) \
    SIMD_CONV(DST_NAME, simd_ulong##N, N) \
    SIMD_CONV(DST_NAME, simd_float##N, N) \
    SIMD_CONV(DST_NAME, simd_double##N, N)

#define SIMD_CONV_FAMILY(DST_NAME) \
    SIMD_CONV_ALL_SRC(DST_NAME, 2) \
    SIMD_CONV_ALL_SRC(DST_NAME, 3) \
    SIMD_CONV_ALL_SRC(DST_NAME, 4)

SIMD_CONV_FAMILY(char)
SIMD_CONV_FAMILY(uchar)
SIMD_CONV_FAMILY(short)
SIMD_CONV_FAMILY(ushort)
SIMD_CONV_FAMILY(int)
SIMD_CONV_FAMILY(uint)
SIMD_CONV_FAMILY(long)
SIMD_CONV_FAMILY(ulong)
SIMD_CONV_FAMILY(float)
SIMD_CONV_FAMILY(double)

#undef SIMD_CONV_FAMILY
#undef SIMD_CONV_ALL_SRC
#undef SIMD_CONV

#endif
