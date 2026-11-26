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

#ifndef SIMD_LOGIC_H
#define SIMD_LOGIC_H

#include <simd/base.h>
#include <simd/vector_types.h>

/*
 * Logical and bitwise functions on vectors (the documented simd contract):
 *   simd_all(x)                 true if the high bit of every lane of an integer vector is set
 *   simd_any(x)                 true if the high bit of any lane is set
 *   simd_bitselect(x, y, mask)  for each bit, the bit of y where the mask bit is 1, else the bit of x
 *   simd_select(x, y, mask)     for each lane of a float or double vector, the lane of y where the high
 *                               bit of the integer mask lane is set, else the lane of x
 */

#define SIMD_LOGIC_ALL_ANY(V, S, N)                                           \
SIMD_FN simd_bool simd_all(V x) {                                             \
    S s = (S)x;                                                               \
    return __builtin_reduce_and(s < 0) != 0;                                  \
}                                                                             \
SIMD_FN simd_bool simd_any(V x) {                                             \
    S s = (S)x;                                                               \
    return __builtin_reduce_or(s < 0) != 0;                                   \
}

#define SIMD_LOGIC_SIGNED(NAME, N) SIMD_LOGIC_ALL_ANY(simd_##NAME##N, simd_##NAME##N, N)
#define SIMD_LOGIC_UNSIGNED(UNAME, NAME, N) SIMD_LOGIC_ALL_ANY(simd_##UNAME##N, simd_##NAME##N, N)


SIMD_LOGIC_SIGNED(char, 2) SIMD_LOGIC_SIGNED(char, 3) SIMD_LOGIC_SIGNED(char, 4) SIMD_LOGIC_SIGNED(char, 8)
SIMD_LOGIC_SIGNED(char, 16) SIMD_LOGIC_SIGNED(char, 32) SIMD_LOGIC_SIGNED(char, 64)
SIMD_LOGIC_UNSIGNED(uchar, char, 2) SIMD_LOGIC_UNSIGNED(uchar, char, 3) SIMD_LOGIC_UNSIGNED(uchar, char, 4)
SIMD_LOGIC_UNSIGNED(uchar, char, 8) SIMD_LOGIC_UNSIGNED(uchar, char, 16) SIMD_LOGIC_UNSIGNED(uchar, char, 32)
SIMD_LOGIC_UNSIGNED(uchar, char, 64)
SIMD_LOGIC_SIGNED(short, 2) SIMD_LOGIC_SIGNED(short, 3) SIMD_LOGIC_SIGNED(short, 4) SIMD_LOGIC_SIGNED(short, 8)
SIMD_LOGIC_SIGNED(short, 16) SIMD_LOGIC_SIGNED(short, 32)
SIMD_LOGIC_UNSIGNED(ushort, short, 2) SIMD_LOGIC_UNSIGNED(ushort, short, 3) SIMD_LOGIC_UNSIGNED(ushort, short, 4)
SIMD_LOGIC_UNSIGNED(ushort, short, 8) SIMD_LOGIC_UNSIGNED(ushort, short, 16) SIMD_LOGIC_UNSIGNED(ushort, short, 32)
SIMD_LOGIC_SIGNED(int, 2) SIMD_LOGIC_SIGNED(int, 3) SIMD_LOGIC_SIGNED(int, 4) SIMD_LOGIC_SIGNED(int, 8)
SIMD_LOGIC_SIGNED(int, 16)
SIMD_LOGIC_UNSIGNED(uint, int, 2) SIMD_LOGIC_UNSIGNED(uint, int, 3) SIMD_LOGIC_UNSIGNED(uint, int, 4)
SIMD_LOGIC_UNSIGNED(uint, int, 8) SIMD_LOGIC_UNSIGNED(uint, int, 16)
SIMD_LOGIC_SIGNED(long, 2) SIMD_LOGIC_SIGNED(long, 3) SIMD_LOGIC_SIGNED(long, 4) SIMD_LOGIC_SIGNED(long, 8)
SIMD_LOGIC_UNSIGNED(ulong, long, 2) SIMD_LOGIC_UNSIGNED(ulong, long, 3) SIMD_LOGIC_UNSIGNED(ulong, long, 4)
SIMD_LOGIC_UNSIGNED(ulong, long, 8)

#undef SIMD_LOGIC_UNSIGNED
#undef SIMD_LOGIC_SIGNED
#undef SIMD_LOGIC_ALL_ANY

/* Integer vectors: x and y are the operands, mask picks bits of y. */
#define SIMD_BITSELECT(V) \
SIMD_FN V simd_bitselect(V x, V y, V mask) { return (x & ~mask) | (y & mask); }

#define SIMD_BITSELECT_SIZES(NAME) \
    SIMD_BITSELECT(simd_##NAME##2) SIMD_BITSELECT(simd_##NAME##3) SIMD_BITSELECT(simd_##NAME##4) \
    SIMD_BITSELECT(simd_##NAME##8)

SIMD_BITSELECT_SIZES(char) SIMD_BITSELECT(simd_char16) SIMD_BITSELECT(simd_char32) SIMD_BITSELECT(simd_char64)
SIMD_BITSELECT_SIZES(uchar) SIMD_BITSELECT(simd_uchar16) SIMD_BITSELECT(simd_uchar32) SIMD_BITSELECT(simd_uchar64)
SIMD_BITSELECT_SIZES(short) SIMD_BITSELECT(simd_short16) SIMD_BITSELECT(simd_short32)
SIMD_BITSELECT_SIZES(ushort) SIMD_BITSELECT(simd_ushort16) SIMD_BITSELECT(simd_ushort32)
SIMD_BITSELECT_SIZES(int) SIMD_BITSELECT(simd_int16)
SIMD_BITSELECT_SIZES(uint) SIMD_BITSELECT(simd_uint16)
SIMD_BITSELECT_SIZES(long)
SIMD_BITSELECT_SIZES(ulong)

#undef SIMD_BITSELECT_SIZES
#undef SIMD_BITSELECT

/* Float and double vectors: the mask is an integer vector of the same lane width. */
#define SIMD_SELECT(V, M, I) \
SIMD_FN V simd_select(V x, V y, M mask) { \
    return __builtin_bit_cast(V, simd_bitselect(__builtin_bit_cast(I, x), __builtin_bit_cast(I, y), (I)(mask < 0))); \
}

SIMD_SELECT(simd_float2, simd_int2, simd_int2)
SIMD_SELECT(simd_float3, simd_int3, simd_int3)
SIMD_SELECT(simd_float4, simd_int4, simd_int4)
SIMD_SELECT(simd_float8, simd_int8, simd_int8)
SIMD_SELECT(simd_float16, simd_int16, simd_int16)
/* simd_long is a 64-bit lane type only where long is 64 bits wide. */
#ifdef __LP64__
SIMD_SELECT(simd_double2, simd_long2, simd_long2)
SIMD_SELECT(simd_double3, simd_long3, simd_long3)
SIMD_SELECT(simd_double4, simd_long4, simd_long4)
SIMD_SELECT(simd_double8, simd_long8, simd_long8)
#endif

#undef SIMD_SELECT

#endif
