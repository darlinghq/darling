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

#ifndef SIMD_MATH_H
#define SIMD_MATH_H

#include <simd/base.h>
#include <simd/vector_types.h>

#define SIMD_MATH_ELEM_FN(V, FN, BUILTIN) \
SIMD_FN V simd_##FN(V x) { return BUILTIN(x); }

#define SIMD_MATH_FAMILY(V) \
    SIMD_MATH_ELEM_FN(V, ceil, __builtin_elementwise_ceil) \
    SIMD_MATH_ELEM_FN(V, floor, __builtin_elementwise_floor) \
    SIMD_MATH_ELEM_FN(V, round, __builtin_elementwise_round) \
    SIMD_MATH_ELEM_FN(V, trunc, __builtin_elementwise_trunc)

SIMD_MATH_FAMILY(simd_float2)
SIMD_MATH_FAMILY(simd_float3)
SIMD_MATH_FAMILY(simd_float4)
SIMD_MATH_FAMILY(simd_double2)
SIMD_MATH_FAMILY(simd_double3)
SIMD_MATH_FAMILY(simd_double4)

#undef SIMD_MATH_FAMILY
#undef SIMD_MATH_ELEM_FN

#endif
