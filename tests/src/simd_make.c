// Copyright (C) 2026 Lubos Dolezel
//
// <simd/simd.h>: the simd_make_<type>N constructors, from scalars and smaller
// vectors that add up to N, and from one scalar or vector, which is truncated
// or extended with zeros. Values as observed on macOS 26.7.3; builds and runs
// there too.
// Prints PASS, or one FAIL line per failed check; the exit code is the
// number of failures.
#include <simd/simd.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define EQ2(v, a, b) ((v).x == (a) && (v).y == (b))
#define EQ3(v, a, b, c) (EQ2(v, a, b) && (v).z == (c))
#define EQ4(v, a, b, c, d) (EQ3(v, a, b, c) && (v).w == (d))

static void test_zero_extension(void)
{
	CHECK(EQ2(simd_make_float2(9.f), 9, 0));
	CHECK(EQ3(simd_make_float3(7.f), 7, 0, 0));
	CHECK(EQ3(simd_make_float3(simd_make_float2(1, 2)), 1, 2, 0));
	CHECK(EQ4(simd_make_float4(5.f), 5, 0, 0, 0));
	CHECK(EQ4(simd_make_float4(simd_make_float2(1, 2)), 1, 2, 0, 0));
	CHECK(EQ4(simd_make_float4(simd_make_float3(1, 2, 3)), 1, 2, 3, 0));
	CHECK(EQ4(simd_make_double4(2.0), 2, 0, 0, 0));
	CHECK(EQ4(simd_make_int4(4), 4, 0, 0, 0));
	CHECK(EQ3(simd_make_uint3(simd_make_uint2(1, 2)), 1, 2, 0));
}

static void test_truncation_and_identity(void)
{
	simd_float4 f = {1, 2, 3, 4};
	CHECK(EQ2(simd_make_float2(f), 1, 2));
	CHECK(EQ3(simd_make_float3(f), 1, 2, 3));
	CHECK(EQ4(simd_make_float4(f), 1, 2, 3, 4));
	CHECK(EQ2(simd_make_int2(simd_make_int2(5, 6)), 5, 6));
	CHECK(EQ3(simd_make_double3(simd_make_double3(1, 2, 3)), 1, 2, 3));
}

static void test_combinations(void)
{
	CHECK(EQ3(simd_make_double3(1, simd_make_double2(2, 3)), 1, 2, 3));
	CHECK(EQ3(simd_make_float3(simd_make_float2(1, 2), 3), 1, 2, 3));
	CHECK(EQ4(simd_make_int4(1, simd_make_int3(2, 3, 4)), 1, 2, 3, 4));
	CHECK(EQ4(simd_make_int4(1, simd_make_int2(2, 3), 4), 1, 2, 3, 4));
	CHECK(EQ4(simd_make_int4(1, 2, simd_make_int2(3, 4)), 1, 2, 3, 4));
	CHECK(EQ4(simd_make_int4(simd_make_int2(1, 2), simd_make_int2(3, 4)), 1, 2, 3, 4));
	CHECK(EQ4(simd_make_uint4(simd_make_uint2(1, 2), 3, 4), 1, 2, 3, 4));
	CHECK(EQ4(simd_make_float4(simd_make_float3(1, 2, 3), 4), 1, 2, 3, 4));
}

int main(void)
{
	test_zero_extension();
	test_truncation_and_identity();
	test_combinations();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures;
}
