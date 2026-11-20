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
#include <stdbool.h>

#ifdef __cplusplus
typedef bool simd_bool;
#else
typedef _Bool simd_bool;
#endif

/*
 * Logical and bitwise functions on vectors:
 * simd_all, simd_any, simd_select, simd_bitselect.
 */

#define SIMD_LOGIC_ALL_ANY(V, N)                                               \
SIMD_FN simd_bool simd_all(V x) {                                              \
    return __builtin_reduce_and(__builtin_convertvector(x < 0, V)) != 0;      \
}                                                                              \
SIMD_FN simd_bool simd_any(V x) {                                              \
    return __builtin_reduce_or(__builtin_convertvector(x < 0, V)) != 0;       \
}

#define SIMD_LOGIC_FAMILY(NAME) \
    SIMD_LOGIC_ALL_ANY(simd_##NAME##2, 2) \
    SIMD_LOGIC_ALL_ANY(simd_##NAME##3, 3) \
    SIMD_LOGIC_ALL_ANY(simd_##NAME##4, 4)

SIMD_LOGIC_FAMILY(char)
SIMD_LOGIC_FAMILY(short)
SIMD_LOGIC_FAMILY(int)
SIMD_LOGIC_FAMILY(long)

#undef SIMD_LOGIC_FAMILY
#undef SIMD_LOGIC_ALL_ANY

#define SIMD_BITSELECT(V) \
SIMD_FN V simd_bitselect(V mask, V a, V b) { return (a & mask) | (b & ~mask); }

SIMD_BITSELECT(simd_char2)
SIMD_BITSELECT(simd_char3)
SIMD_BITSELECT(simd_char4)
SIMD_BITSELECT(simd_uchar2)
SIMD_BITSELECT(simd_uchar3)
SIMD_BITSELECT(simd_uchar4)
SIMD_BITSELECT(simd_short2)
SIMD_BITSELECT(simd_short3)
SIMD_BITSELECT(simd_short4)
SIMD_BITSELECT(simd_ushort2)
SIMD_BITSELECT(simd_ushort3)
SIMD_BITSELECT(simd_ushort4)
SIMD_BITSELECT(simd_int2)
SIMD_BITSELECT(simd_int3)
SIMD_BITSELECT(simd_int4)
SIMD_BITSELECT(simd_uint2)
SIMD_BITSELECT(simd_uint3)
SIMD_BITSELECT(simd_uint4)
SIMD_BITSELECT(simd_long2)
SIMD_BITSELECT(simd_long3)
SIMD_BITSELECT(simd_long4)
SIMD_BITSELECT(simd_ulong2)
SIMD_BITSELECT(simd_ulong3)
SIMD_BITSELECT(simd_ulong4)

#undef SIMD_BITSELECT

#endif
