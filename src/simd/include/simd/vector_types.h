/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

#ifndef VECTOR_TYPE_H
#define VECTOR_TYPE_H

// typedef  __attribute__((ext_vector_type())) ;
#define simd_struct(NAME_TYPE, A, B) \
typedef struct { \
    simd_float##B columns[A]; \
} simd_##NAME_TYPE##A##x##B

#define simd_type(TYPE, NAME_TYPE, SIZE) \
typedef TYPE __attribute__((ext_vector_type(SIZE))) simd_##NAME_TYPE##SIZE

simd_type(int, int, 2); // simd_int2
simd_type(int, int, 3); // simd_int3
simd_type(int, int, 4); // simd_int4

simd_type(unsigned int, uint, 2); // simd_uint2
simd_type(unsigned int, uint, 3); // simd_uint3
simd_type(unsigned int, uint, 4); // simd_uint4

simd_type(float, float, 2); // simd_float2
simd_type(float, float, 3); // simd_float3
simd_type(float, float, 4); // simd_float4

simd_type(double, double, 2); // simd_double2
simd_type(double, double, 3); // simd_double3
simd_type(double, double, 4); // simd_double4

simd_struct(float, 2, 2); // simd_float2x2
simd_struct(float, 3, 2); // simd_float3x2
simd_struct(float, 4, 2); // simd_float4x2
simd_struct(float, 2, 3); // simd_float2x3
simd_struct(float, 3, 3); // simd_float3x3
simd_struct(float, 4, 3); // simd_float4x3
simd_struct(float, 2, 4); // simd_float2x4
simd_struct(float, 3, 4); // simd_float3x4
simd_struct(float, 4, 4); // simd_float4x4

simd_struct(double, 2, 2); // simd_double2x2
simd_struct(double, 3, 2); // simd_double3x2
simd_struct(double, 4, 2); // simd_double4x2
simd_struct(double, 2, 3); // simd_double2x3
simd_struct(double, 3, 3); // simd_double3x3
simd_struct(double, 4, 3); // simd_double4x3
simd_struct(double, 2, 4); // simd_double2x4
simd_struct(double, 3, 4); // simd_double3x4
simd_struct(double, 4, 4); // simd_double4x4

typedef struct {
    simd_double4 vector;
} simd_quatd;

typedef struct {
    simd_float4 vector;
} simd_quatf;

#undef simd_struct
#undef simd_type
#endif
