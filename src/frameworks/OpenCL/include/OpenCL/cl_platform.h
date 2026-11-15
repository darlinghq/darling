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

#ifndef _CL_PLATFORM_H_
#define _CL_PLATFORM_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Scalar types. The sizes are fixed: they do not follow the host's long.
typedef int8_t cl_char;
typedef uint8_t cl_uchar;
typedef int16_t cl_short;
typedef uint16_t cl_ushort;
typedef int32_t cl_int;
typedef uint32_t cl_uint;
typedef int64_t cl_long;
typedef uint64_t cl_ulong;
typedef uint16_t cl_half;
typedef float cl_float;
typedef double cl_double;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#endif

// Vector types. As on the macOS SDK they are unions: the components are reachable as an
// array (s), by name (x, y, z, w for up to four elements), by index (s0 ... sf), and
// as two halves (lo, hi). A three element vector takes the space of a four element one
// (its w component is padding). Each union is aligned to its size.

typedef union {
	cl_char s[2] __attribute__((aligned(2)));
	struct { cl_char x, y; };
	struct { cl_char s0, s1; };
} cl_char2;

typedef union {
	cl_char s[4] __attribute__((aligned(4)));
	struct { cl_char x, y, z, w; };
	struct { cl_char s0, s1, s2, s3; };
	struct { cl_char2 lo, hi; };
} cl_char4;

typedef union {
	cl_char s[4] __attribute__((aligned(4)));
	struct { cl_char x, y, z, w; };
	struct { cl_char s0, s1, s2, s3; };
	struct { cl_char2 lo, hi; };
} cl_char3;

typedef union {
	cl_char s[8] __attribute__((aligned(8)));
	struct { cl_char s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_char4 lo, hi; };
} cl_char8;

typedef union {
	cl_char s[16] __attribute__((aligned(16)));
	struct { cl_char s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_char8 lo, hi; };
} cl_char16;

typedef union {
	cl_uchar s[2] __attribute__((aligned(2)));
	struct { cl_uchar x, y; };
	struct { cl_uchar s0, s1; };
} cl_uchar2;

typedef union {
	cl_uchar s[4] __attribute__((aligned(4)));
	struct { cl_uchar x, y, z, w; };
	struct { cl_uchar s0, s1, s2, s3; };
	struct { cl_uchar2 lo, hi; };
} cl_uchar4;

typedef union {
	cl_uchar s[4] __attribute__((aligned(4)));
	struct { cl_uchar x, y, z, w; };
	struct { cl_uchar s0, s1, s2, s3; };
	struct { cl_uchar2 lo, hi; };
} cl_uchar3;

typedef union {
	cl_uchar s[8] __attribute__((aligned(8)));
	struct { cl_uchar s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_uchar4 lo, hi; };
} cl_uchar8;

typedef union {
	cl_uchar s[16] __attribute__((aligned(16)));
	struct { cl_uchar s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_uchar8 lo, hi; };
} cl_uchar16;

typedef union {
	cl_short s[2] __attribute__((aligned(4)));
	struct { cl_short x, y; };
	struct { cl_short s0, s1; };
} cl_short2;

typedef union {
	cl_short s[4] __attribute__((aligned(8)));
	struct { cl_short x, y, z, w; };
	struct { cl_short s0, s1, s2, s3; };
	struct { cl_short2 lo, hi; };
} cl_short4;

typedef union {
	cl_short s[4] __attribute__((aligned(8)));
	struct { cl_short x, y, z, w; };
	struct { cl_short s0, s1, s2, s3; };
	struct { cl_short2 lo, hi; };
} cl_short3;

typedef union {
	cl_short s[8] __attribute__((aligned(16)));
	struct { cl_short s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_short4 lo, hi; };
} cl_short8;

typedef union {
	cl_short s[16] __attribute__((aligned(32)));
	struct { cl_short s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_short8 lo, hi; };
} cl_short16;

typedef union {
	cl_ushort s[2] __attribute__((aligned(4)));
	struct { cl_ushort x, y; };
	struct { cl_ushort s0, s1; };
} cl_ushort2;

typedef union {
	cl_ushort s[4] __attribute__((aligned(8)));
	struct { cl_ushort x, y, z, w; };
	struct { cl_ushort s0, s1, s2, s3; };
	struct { cl_ushort2 lo, hi; };
} cl_ushort4;

typedef union {
	cl_ushort s[4] __attribute__((aligned(8)));
	struct { cl_ushort x, y, z, w; };
	struct { cl_ushort s0, s1, s2, s3; };
	struct { cl_ushort2 lo, hi; };
} cl_ushort3;

typedef union {
	cl_ushort s[8] __attribute__((aligned(16)));
	struct { cl_ushort s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_ushort4 lo, hi; };
} cl_ushort8;

typedef union {
	cl_ushort s[16] __attribute__((aligned(32)));
	struct { cl_ushort s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_ushort8 lo, hi; };
} cl_ushort16;

typedef union {
	cl_int s[2] __attribute__((aligned(8)));
	struct { cl_int x, y; };
	struct { cl_int s0, s1; };
} cl_int2;

typedef union {
	cl_int s[4] __attribute__((aligned(16)));
	struct { cl_int x, y, z, w; };
	struct { cl_int s0, s1, s2, s3; };
	struct { cl_int2 lo, hi; };
} cl_int4;

typedef union {
	cl_int s[4] __attribute__((aligned(16)));
	struct { cl_int x, y, z, w; };
	struct { cl_int s0, s1, s2, s3; };
	struct { cl_int2 lo, hi; };
} cl_int3;

typedef union {
	cl_int s[8] __attribute__((aligned(32)));
	struct { cl_int s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_int4 lo, hi; };
} cl_int8;

typedef union {
	cl_int s[16] __attribute__((aligned(64)));
	struct { cl_int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_int8 lo, hi; };
} cl_int16;

typedef union {
	cl_uint s[2] __attribute__((aligned(8)));
	struct { cl_uint x, y; };
	struct { cl_uint s0, s1; };
} cl_uint2;

typedef union {
	cl_uint s[4] __attribute__((aligned(16)));
	struct { cl_uint x, y, z, w; };
	struct { cl_uint s0, s1, s2, s3; };
	struct { cl_uint2 lo, hi; };
} cl_uint4;

typedef union {
	cl_uint s[4] __attribute__((aligned(16)));
	struct { cl_uint x, y, z, w; };
	struct { cl_uint s0, s1, s2, s3; };
	struct { cl_uint2 lo, hi; };
} cl_uint3;

typedef union {
	cl_uint s[8] __attribute__((aligned(32)));
	struct { cl_uint s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_uint4 lo, hi; };
} cl_uint8;

typedef union {
	cl_uint s[16] __attribute__((aligned(64)));
	struct { cl_uint s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_uint8 lo, hi; };
} cl_uint16;

typedef union {
	cl_long s[2] __attribute__((aligned(16)));
	struct { cl_long x, y; };
	struct { cl_long s0, s1; };
} cl_long2;

typedef union {
	cl_long s[4] __attribute__((aligned(32)));
	struct { cl_long x, y, z, w; };
	struct { cl_long s0, s1, s2, s3; };
	struct { cl_long2 lo, hi; };
} cl_long4;

typedef union {
	cl_long s[4] __attribute__((aligned(32)));
	struct { cl_long x, y, z, w; };
	struct { cl_long s0, s1, s2, s3; };
	struct { cl_long2 lo, hi; };
} cl_long3;

typedef union {
	cl_long s[8] __attribute__((aligned(64)));
	struct { cl_long s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_long4 lo, hi; };
} cl_long8;

typedef union {
	cl_long s[16] __attribute__((aligned(128)));
	struct { cl_long s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_long8 lo, hi; };
} cl_long16;

typedef union {
	cl_ulong s[2] __attribute__((aligned(16)));
	struct { cl_ulong x, y; };
	struct { cl_ulong s0, s1; };
} cl_ulong2;

typedef union {
	cl_ulong s[4] __attribute__((aligned(32)));
	struct { cl_ulong x, y, z, w; };
	struct { cl_ulong s0, s1, s2, s3; };
	struct { cl_ulong2 lo, hi; };
} cl_ulong4;

typedef union {
	cl_ulong s[4] __attribute__((aligned(32)));
	struct { cl_ulong x, y, z, w; };
	struct { cl_ulong s0, s1, s2, s3; };
	struct { cl_ulong2 lo, hi; };
} cl_ulong3;

typedef union {
	cl_ulong s[8] __attribute__((aligned(64)));
	struct { cl_ulong s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_ulong4 lo, hi; };
} cl_ulong8;

typedef union {
	cl_ulong s[16] __attribute__((aligned(128)));
	struct { cl_ulong s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_ulong8 lo, hi; };
} cl_ulong16;

typedef union {
	cl_float s[2] __attribute__((aligned(8)));
	struct { cl_float x, y; };
	struct { cl_float s0, s1; };
} cl_float2;

typedef union {
	cl_float s[4] __attribute__((aligned(16)));
	struct { cl_float x, y, z, w; };
	struct { cl_float s0, s1, s2, s3; };
	struct { cl_float2 lo, hi; };
} cl_float4;

typedef union {
	cl_float s[4] __attribute__((aligned(16)));
	struct { cl_float x, y, z, w; };
	struct { cl_float s0, s1, s2, s3; };
	struct { cl_float2 lo, hi; };
} cl_float3;

typedef union {
	cl_float s[8] __attribute__((aligned(32)));
	struct { cl_float s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_float4 lo, hi; };
} cl_float8;

typedef union {
	cl_float s[16] __attribute__((aligned(64)));
	struct { cl_float s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_float8 lo, hi; };
} cl_float16;

typedef union {
	cl_double s[2] __attribute__((aligned(16)));
	struct { cl_double x, y; };
	struct { cl_double s0, s1; };
} cl_double2;

typedef union {
	cl_double s[4] __attribute__((aligned(32)));
	struct { cl_double x, y, z, w; };
	struct { cl_double s0, s1, s2, s3; };
	struct { cl_double2 lo, hi; };
} cl_double4;

typedef union {
	cl_double s[4] __attribute__((aligned(32)));
	struct { cl_double x, y, z, w; };
	struct { cl_double s0, s1, s2, s3; };
	struct { cl_double2 lo, hi; };
} cl_double3;

typedef union {
	cl_double s[8] __attribute__((aligned(64)));
	struct { cl_double s0, s1, s2, s3, s4, s5, s6, s7; };
	struct { cl_double4 lo, hi; };
} cl_double8;

typedef union {
	cl_double s[16] __attribute__((aligned(128)));
	struct { cl_double s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, sa, sb, sc, sd, se, sf; };
	struct { cl_double8 lo, hi; };
} cl_double16;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

// Limits of the scalar types.
#define CL_CHAR_BIT 8
#define CL_SCHAR_MAX 127
#define CL_SCHAR_MIN (-127 - 1)
#define CL_CHAR_MAX CL_SCHAR_MAX
#define CL_CHAR_MIN CL_SCHAR_MIN
#define CL_UCHAR_MAX 255
#define CL_SHRT_MAX 32767
#define CL_SHRT_MIN (-32767 - 1)
#define CL_USHRT_MAX 65535
#define CL_INT_MAX 2147483647
#define CL_INT_MIN (-2147483647 - 1)
#define CL_UINT_MAX 0xffffffffU
#define CL_LONG_MAX ((cl_long) 0x7fffffffffffffffLL)
#define CL_LONG_MIN ((cl_long) -0x7fffffffffffffffLL - 1LL)
#define CL_ULONG_MAX ((cl_ulong) 0xffffffffffffffffULL)

#define CL_FLT_DIG 6
#define CL_FLT_MANT_DIG 24
#define CL_FLT_MAX_EXP 128
#define CL_FLT_MIN_EXP (-125)
#define CL_FLT_RADIX 2
#define CL_FLT_MAX 0x1.fffffep127f
#define CL_FLT_MIN 0x1.0p-126f
#define CL_FLT_EPSILON 0x1.0p-23f

#define CL_DBL_DIG 15
#define CL_DBL_MANT_DIG 53
#define CL_DBL_MAX_EXP 1024
#define CL_DBL_MIN_EXP (-1021)
#define CL_DBL_RADIX 2
#define CL_DBL_MAX 0x1.fffffffffffffp1023
#define CL_DBL_MIN 0x1.0p-1022
#define CL_DBL_EPSILON 0x1.0p-52

#ifdef __cplusplus
}
#endif

#endif
