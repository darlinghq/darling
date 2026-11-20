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

#define SIMD_PACKED_TYPE(NAME, S, N) \
    typedef __attribute__((__ext_vector_type__(N), __aligned__(sizeof(S)))) S simd_packed_##NAME##N;

#define SIMD_PACKED_FAMILY(NAME, S) \
    SIMD_PACKED_TYPE(NAME, S, 2) \
    SIMD_PACKED_TYPE(NAME, S, 3) \
    SIMD_PACKED_TYPE(NAME, S, 4) \
    SIMD_PACKED_TYPE(NAME, S, 8) \
    SIMD_PACKED_TYPE(NAME, S, 16)

SIMD_PACKED_FAMILY(char, char)
SIMD_PACKED_FAMILY(uchar, unsigned char)
SIMD_PACKED_FAMILY(short, short)
SIMD_PACKED_FAMILY(ushort, unsigned short)
SIMD_PACKED_FAMILY(int, int)
SIMD_PACKED_FAMILY(uint, unsigned int)
SIMD_PACKED_FAMILY(long, long)
SIMD_PACKED_FAMILY(ulong, unsigned long)
SIMD_PACKED_FAMILY(float, float)
SIMD_PACKED_FAMILY(double, double)

#undef SIMD_PACKED_FAMILY
#undef SIMD_PACKED_TYPE

#endif
