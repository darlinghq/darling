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

// OpenCL types: sizes and alignments of the scalar, vector and handle types, the limits, the error
// codes, and the argument setting functions with their argument-less outcome (no kernel exists).
// Builds against the real SDK too (clang -framework OpenCL). Prints PASS when everything holds;
// the exit code is the number of failed checks.

#include <OpenCL/opencl.h>
#include <stdarg.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)
#define LAYOUT(T, size, align) CHECK(sizeof(T) == (size) && _Alignof(T) == (align))
// 64-bit scalars are only 8-aligned on 64-bit targets; the i386 ABI aligns them to 4.
#define LAYOUT64(T) CHECK(sizeof(T) == 8 && _Alignof(T) == (sizeof(void *) == 8 ? 8 : _Alignof(T)))

static void test_scalar_layout(void)
{
	LAYOUT(cl_char, 1, 1); LAYOUT(cl_uchar, 1, 1);
	LAYOUT(cl_short, 2, 2); LAYOUT(cl_ushort, 2, 2); LAYOUT(cl_half, 2, 2);
	LAYOUT(cl_int, 4, 4); LAYOUT(cl_uint, 4, 4); LAYOUT(cl_float, 4, 4); LAYOUT(cl_bool, 4, 4);
	LAYOUT64(cl_long); LAYOUT64(cl_ulong); LAYOUT64(cl_double); LAYOUT64(cl_bitfield);
	CHECK((cl_char) -1 < 0 && (cl_uchar) -1 > 0);
	CHECK((cl_int) -1 < 0 && (cl_uint) -1 > 0);
	CHECK((cl_long) -1 < 0 && (cl_ulong) -1 > 0);
	CHECK((cl_half) 0xffff > 0);
}

static void test_vector_layout(void)
{
	LAYOUT(cl_char2, 2, 2); LAYOUT(cl_char3, 4, 4); LAYOUT(cl_char4, 4, 4); LAYOUT(cl_char8, 8, 8); LAYOUT(cl_char16, 16, 16);
	LAYOUT(cl_uchar4, 4, 4);
	LAYOUT(cl_short2, 4, 4); LAYOUT(cl_short3, 8, 8); LAYOUT(cl_short4, 8, 8);
	LAYOUT(cl_int2, 8, 8); LAYOUT(cl_int3, 16, 16); LAYOUT(cl_int4, 16, 16); LAYOUT(cl_int8, 32, 32); LAYOUT(cl_int16, 64, 64);
	LAYOUT(cl_uint4, 16, 16);
	LAYOUT(cl_long2, 16, 16); LAYOUT(cl_long3, 32, 32); LAYOUT(cl_long4, 32, 32); LAYOUT(cl_ulong8, 64, 64);
	LAYOUT(cl_float2, 8, 8); LAYOUT(cl_float3, 16, 16); LAYOUT(cl_float4, 16, 16); LAYOUT(cl_float8, 32, 32); LAYOUT(cl_float16, 64, 64);
	LAYOUT(cl_double2, 16, 16); LAYOUT(cl_double3, 32, 32); LAYOUT(cl_double4, 32, 32); LAYOUT(cl_double8, 64, 64); LAYOUT(cl_double16, 128, 128);

	// Components by name, by index, by array element and as halves share the storage.
	cl_float4 f = { { 1, 2, 3, 4 } };
	CHECK(f.x == 1 && f.y == 2 && f.z == 3 && f.w == 4);
	CHECK(f.s0 == 1 && f.s3 == 4 && f.s[1] == 2 && f.s[2] == 3);
	CHECK(f.lo.x == 1 && f.lo.y == 2 && f.hi.x == 3 && f.hi.y == 4);
	f.hi.y = 9;
	CHECK(f.w == 9 && f.s[3] == 9);
	cl_float3 g = { { 1, 2, 3 } };
	CHECK(g.x == 1 && g.z == 3 && g.s[2] == 3 && g.hi.x == 3);
	cl_int2 a = { { 1, 2 } };
	CHECK(a.x == 1 && a.s1 == 2 && a.s[1] == 2);
	cl_uchar16 u = { { 0 } };
	u.sf = 7;
	CHECK(u.s[15] == 7 && u.hi.hi.hi.y == 7 && u.hi.s7 == 7);
	u.s[8] = 3;
	CHECK(u.hi.lo.lo.x == 3);
	cl_double8 d = { { 0 } };
	d.s7 = 2.5;
	CHECK(d.hi.hi.y == 2.5 && d.s[7] == 2.5);
	cl_long4 l = { { 1, 2, 3, 4 } };
	CHECK(l.hi.x == 3 && l.lo.y == 2);
}

static void test_types_and_structs(void)
{
	LAYOUT(cl_platform_id, sizeof(void *), sizeof(void *));
	LAYOUT(cl_device_id, sizeof(void *), sizeof(void *));
	LAYOUT(cl_context, sizeof(void *), sizeof(void *));
	LAYOUT(cl_command_queue, sizeof(void *), sizeof(void *));
	LAYOUT(cl_mem, sizeof(void *), sizeof(void *));
	LAYOUT(cl_program, sizeof(void *), sizeof(void *));
	LAYOUT(cl_kernel, sizeof(void *), sizeof(void *));
	LAYOUT(cl_event, sizeof(void *), sizeof(void *));
	LAYOUT(cl_sampler, sizeof(void *), sizeof(void *));
	LAYOUT64(cl_device_type); LAYOUT64(cl_mem_flags); LAYOUT64(cl_map_flags);
	LAYOUT64(cl_command_queue_properties);
	LAYOUT(cl_context_properties, sizeof(void *), sizeof(void *));
	LAYOUT(cl_platform_info, 4, 4); LAYOUT(cl_addressing_mode, 4, 4); LAYOUT(cl_filter_mode, 4, 4);
	LAYOUT(cl_mem_object_type, 4, 4); LAYOUT(cl_channel_order, 4, 4); LAYOUT(cl_channel_type, 4, 4);
	LAYOUT(cl_profiling_info, 4, 4); LAYOUT(cl_event_info, 4, 4); LAYOUT(cl_command_type, 4, 4);
	LAYOUT(cl_image_format, 8, 4);
	if (sizeof(void *) == 8) {
		LAYOUT(cl_buffer_region, 16, 8);
		LAYOUT(cl_image_desc, 72, 8);
	}
	cl_image_format fmt = { 1, 2 };
	CHECK(fmt.image_channel_order == 1 && fmt.image_channel_data_type == 2);
	cl_buffer_region region = { 4, 8 };
	CHECK(region.origin == 4 && region.size == 8);
}

static void test_constants(void)
{
	CHECK(CL_FALSE == 0 && CL_TRUE == 1);
	CHECK(CL_SUCCESS == 0 && CL_DEVICE_NOT_FOUND == -1 && CL_OUT_OF_HOST_MEMORY == -6);
	CHECK(CL_BUILD_PROGRAM_FAILURE == -11 && CL_INVALID_VALUE == -30 && CL_INVALID_CONTEXT == -34);
	CHECK(CL_INVALID_KERNEL == -48 && CL_INVALID_ARG_INDEX == -49 && CL_INVALID_ARG_VALUE == -50);
	CHECK(CL_INVALID_ARG_SIZE == -51 && CL_INVALID_DEVICE_PARTITION_COUNT == -68);
	CHECK(CL_CHAR_MAX == 127 && CL_CHAR_MIN == -128 && CL_UCHAR_MAX == 255);
	CHECK(CL_SHRT_MAX == 32767 && CL_SHRT_MIN == -32768 && CL_USHRT_MAX == 65535);
	CHECK(CL_INT_MAX == 2147483647 && CL_INT_MIN == -2147483647 - 1 && CL_UINT_MAX == 4294967295u);
	CHECK(CL_LONG_MAX == 9223372036854775807LL && CL_LONG_MIN == -9223372036854775807LL - 1);
	CHECK(CL_ULONG_MAX == 18446744073709551615ULL);
	CHECK(CL_FLT_EPSILON == 1.1920928955078125e-07f && CL_FLT_MAX > 3.4e38f && CL_FLT_MIN < 1.2e-38f);
	CHECK(CL_DBL_EPSILON == 2.220446049250313e-16 && CL_DBL_MAX > 1.79e308);
	CHECK(CL_FLT_DIG == 6 && CL_FLT_MANT_DIG == 24 && CL_DBL_DIG == 15 && CL_DBL_MANT_DIG == 53);
}

static cl_int set_args_va(cl_kernel kernel, cl_uint count, ...)
{
	va_list args;
	va_start(args, count);
	cl_int err = clSetKernelArgsVaListAPPLE(kernel, count, args);
	va_end(args);
	return err;
}

static void test_set_kernel_args(void)
{
	int value = 5;
	// No kernel can exist in this implementation, and NULL is invalid anywhere.
	CHECK(clSetKernelArg(NULL, 0, sizeof value, &value) == CL_INVALID_KERNEL);
	// Zero arguments: nothing to set, success.
	CHECK(clSetKernelArgsListAPPLE(NULL, 0) == CL_SUCCESS);
	CHECK(set_args_va(NULL, 0) == CL_SUCCESS);
	// The first error of an argument is returned.
	CHECK(clSetKernelArgsListAPPLE(NULL, 1, (cl_uint) 0, (size_t) sizeof value, &value) == CL_INVALID_KERNEL);
	CHECK(clSetKernelArgsListAPPLE(NULL, 2, (cl_uint) 0, (size_t) sizeof value, &value, (cl_uint) 1, (size_t) sizeof value, &value)
		== CL_INVALID_KERNEL);
	CHECK(set_args_va(NULL, 1, (cl_uint) 0, (size_t) sizeof value, &value) == CL_INVALID_KERNEL);
}

int main(void)
{
	test_scalar_layout();
	test_vector_layout();
	test_types_and_structs();
	test_constants();
	test_set_kernel_args();
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
