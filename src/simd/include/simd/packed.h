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

#ifndef SIMD_PACKED_H
#define SIMD_PACKED_H

#include <simd/vector_types.h>

/*
 * Packed vector types: the same lanes as the corresponding simd vector, but aligned only to
 * the element type, so they can be used for unaligned data. There are no packed three-lane
 * types.
 */
#define SIMD_PACKED_TYPE(NAME, S, N) \
    typedef __attribute__((__ext_vector_type__(N), __aligned__(sizeof(S)))) S simd_packed_##NAME##N

#define SIMD_PACKED_UP_TO_8(NAME, S) \
    SIMD_PACKED_TYPE(NAME, S, 2); SIMD_PACKED_TYPE(NAME, S, 4); SIMD_PACKED_TYPE(NAME, S, 8)

SIMD_PACKED_UP_TO_8(char, char);   SIMD_PACKED_TYPE(char, char, 16);
SIMD_PACKED_TYPE(char, char, 32);  SIMD_PACKED_TYPE(char, char, 64);
SIMD_PACKED_UP_TO_8(uchar, unsigned char);   SIMD_PACKED_TYPE(uchar, unsigned char, 16);
SIMD_PACKED_TYPE(uchar, unsigned char, 32);  SIMD_PACKED_TYPE(uchar, unsigned char, 64);
SIMD_PACKED_UP_TO_8(short, short);   SIMD_PACKED_TYPE(short, short, 16);   SIMD_PACKED_TYPE(short, short, 32);
SIMD_PACKED_UP_TO_8(ushort, unsigned short);   SIMD_PACKED_TYPE(ushort, unsigned short, 16);
SIMD_PACKED_TYPE(ushort, unsigned short, 32);
SIMD_PACKED_UP_TO_8(int, int);   SIMD_PACKED_TYPE(int, int, 16);
SIMD_PACKED_UP_TO_8(uint, unsigned int);   SIMD_PACKED_TYPE(uint, unsigned int, 16);
SIMD_PACKED_UP_TO_8(long, long);
SIMD_PACKED_UP_TO_8(ulong, unsigned long);
SIMD_PACKED_UP_TO_8(float, float);   SIMD_PACKED_TYPE(float, float, 16);
SIMD_PACKED_UP_TO_8(double, double);

#undef SIMD_PACKED_UP_TO_8
#undef SIMD_PACKED_TYPE

#endif
