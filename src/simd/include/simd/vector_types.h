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
    NAME_TYPE##B columns[A]; \
} NAME_TYPE##A##x##B

#define simd_type(NAME_TYPE, SIZE) \
typedef NAME_TYPE##1 __attribute__((ext_vector_type(SIZE))) NAME_TYPE##SIZE

typedef bool simd_bool;


typedef char simd_char1;
simd_type(simd_char, 2);   // simd_char2
simd_type(simd_char, 3);   // simd_char3
simd_type(simd_char, 4);   // simd_char4
simd_type(simd_char, 8);   // simd_char8
simd_type(simd_char, 16);  // simd_char16
simd_type(simd_char, 32);  // simd_char32
simd_type(simd_char, 64);  // simd_char64

typedef simd_char2    vector_char2;
typedef simd_char3    vector_char3;
typedef simd_char4    vector_char4;
typedef simd_char8    vector_char8;
typedef simd_char16   vector_char16;
typedef simd_char32   vector_char32;


typedef unsigned char simd_uchar1;
simd_type(simd_uchar, 2);   // simd_uchar2
simd_type(simd_uchar, 3);   // simd_uchar3
simd_type(simd_uchar, 4);   // simd_uchar4
simd_type(simd_uchar, 8);   // simd_uchar8
simd_type(simd_uchar, 16);  // simd_uchar16
simd_type(simd_uchar, 32);  // simd_uchar32
simd_type(simd_uchar, 64);  // simd_uchar64

typedef simd_uchar2   vector_uchar2;
typedef simd_uchar3   vector_uchar3;
typedef simd_uchar4   vector_uchar4;
typedef simd_uchar8   vector_uchar8;
typedef simd_uchar16  vector_uchar16;
typedef simd_uchar32  vector_uchar32;


typedef short simd_short1;
simd_type(simd_short, 2);   // simd_short2
simd_type(simd_short, 3);   // simd_short3
simd_type(simd_short, 4);   // simd_short4
simd_type(simd_short, 8);   // simd_short8
simd_type(simd_short, 16);  // simd_short16
simd_type(simd_short, 32);  // simd_short32

typedef simd_short2      vector_short2;
typedef simd_short3      vector_short3;
typedef simd_short4      vector_short4;
typedef simd_short8      vector_short8;
typedef simd_short16     vector_short16;
typedef simd_short32     vector_short32;


typedef unsigned short simd_ushort1;
simd_type(simd_ushort, 2);   // simd_ushort2
simd_type(simd_ushort, 3);   // simd_ushort3
simd_type(simd_ushort, 4);   // simd_ushort4
simd_type(simd_ushort, 8);   // simd_ushort8
simd_type(simd_ushort, 16);  // simd_ushort16
simd_type(simd_ushort, 32);  // simd_ushort32

typedef simd_ushort2      vector_ushort2;
typedef simd_ushort3      vector_ushort3;
typedef simd_ushort4      vector_ushort4;
typedef simd_ushort8      vector_ushort8;
typedef simd_ushort16     vector_ushort16;
typedef simd_ushort32     vector_ushort32;


typedef int simd_int1;
simd_type(simd_int, 2);     // simd_int2
simd_type(simd_int, 3);     // simd_int3
simd_type(simd_int, 4);     // simd_int4
simd_type(simd_int, 8);     // simd_int8
simd_type(simd_int, 16);    // simd_int16

typedef simd_int2     vector_int2;
typedef simd_int3     vector_int3;
typedef simd_int4     vector_int4;
typedef simd_int8     vector_int8;
typedef simd_int16    vector_int16;


typedef unsigned int simd_uint1;
simd_type(simd_uint, 2);    // simd_uint2
simd_type(simd_uint, 3);    // simd_uint3
simd_type(simd_uint, 4);    // simd_uint4
simd_type(simd_uint, 8);    // simd_uint8
simd_type(simd_uint, 16);   // simd_uint16

typedef simd_uint2     vector_uint2;
typedef simd_uint3     vector_uint3;
typedef simd_uint4     vector_uint4;
typedef simd_uint8     vector_uint8;
typedef simd_uint16    vector_uint16;


typedef long simd_long1;
simd_type(simd_long, 2);    // simd_long2
simd_type(simd_long, 3);    // simd_long3
simd_type(simd_long, 4);    // simd_long4
simd_type(simd_long, 8);    // simd_long8

typedef simd_long1    vector_long1;
typedef simd_long2    vector_long2;
typedef simd_long3    vector_long3;
typedef simd_long4    vector_long4;
typedef simd_long8    vector_long8;


typedef unsigned long simd_ulong1;
simd_type(simd_ulong, 2);   // simd_ulong2
simd_type(simd_ulong, 3);   // simd_ulong3
simd_type(simd_ulong, 4);   // simd_ulong4
simd_type(simd_ulong, 8);   // simd_ulong8

typedef simd_ulong1    vector_ulong1;
typedef simd_ulong2    vector_ulong2;
typedef simd_ulong3    vector_ulong3;
typedef simd_ulong4    vector_ulong4;
typedef simd_ulong8    vector_ulong8;


typedef float simd_float1;
simd_type(simd_float, 2);     // simd_float2
simd_type(simd_float, 3);     // simd_float3
simd_type(simd_float, 4);     // simd_float4
simd_type(simd_float, 8);     // simd_float8
simd_type(simd_float, 16);    // simd_float16

typedef simd_float2     vector_float2;
typedef simd_float4     vector_float4;
typedef simd_float8     vector_float8;
typedef simd_float16    vector_float16;


typedef double simd_double1;
simd_type(simd_double, 2); // simd_double2
simd_type(simd_double, 3); // simd_double3
simd_type(simd_double, 4); // simd_double4
simd_type(simd_double, 8); // simd_double8

typedef simd_double2  vector_double2;
typedef simd_double3  vector_double3;
typedef simd_double4  vector_double4;
typedef simd_double8  vector_double8;


simd_struct(simd_float, 2, 2); // simd_float2x2
simd_struct(simd_float, 3, 2); // simd_float3x2
simd_struct(simd_float, 4, 2); // simd_float4x2
simd_struct(simd_float, 2, 3); // simd_float2x3
simd_struct(simd_float, 3, 3); // simd_float3x3
simd_struct(simd_float, 4, 3); // simd_float4x3
simd_struct(simd_float, 2, 4); // simd_float2x4
simd_struct(simd_float, 3, 4); // simd_float3x4
simd_struct(simd_float, 4, 4); // simd_float4x4

simd_struct(simd_double, 2, 2); // simd_double2x2
simd_struct(simd_double, 3, 2); // simd_double3x2
simd_struct(simd_double, 4, 2); // simd_double4x2
simd_struct(simd_double, 2, 3); // simd_double2x3
simd_struct(simd_double, 3, 3); // simd_double3x3
simd_struct(simd_double, 4, 3); // simd_double4x3
simd_struct(simd_double, 2, 4); // simd_double2x4
simd_struct(simd_double, 3, 4); // simd_double3x4
simd_struct(simd_double, 4, 4); // simd_double4x4


typedef struct {
    simd_double4 vector;
} simd_quatd;

typedef struct {
    simd_float4 vector;
} simd_quatf;

#undef simd_struct
#undef simd_type
#endif
