// Copyright (C) 2026 Lubos Dolezel
//
// The documented simd logic, selection and conversion functions and the packed vector types.
// Public API only; the same source builds and passes against the macOS SDK.
#include <simd/simd.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

int main(void) {
    // simd_bitselect(x, y, mask): bit from y where the mask bit is 1, else from x.
    simd_int4 x = {0x00000000, 0x11111111, 0x22222222, 0x33333333};
    simd_int4 y = {-1, -1, -1, -1};
    simd_int4 m = {0, -1, 0x0000FFFF, 0x0F0F0F0F};
    simd_int4 r = simd_bitselect(x, y, m);
    CHECK(r[0] == 0x00000000);
    CHECK(r[1] == -1);
    CHECK(r[2] == (0x22222222 | 0x0000FFFF));
    CHECK(r[3] == (0x33333333 | 0x0F0F0F0F));
    // simd_select(x, y, mask): y where the mask lane's high bit is set, else x (float/double vectors, integer mask).
    simd_float4 fx = {1, 2, 3, 4}, fy = {10, 20, 30, 40};
    simd_float4 fs = simd_select(fx, fy, (simd_int4){0, -1, 0x7fffffff, -2});
    CHECK(fs[0] == 1 && fs[1] == 20 && fs[2] == 3 && fs[3] == 40);
    simd_int4 neg = {-1, 0, -5, 0};
    CHECK(simd_all(neg) == 0);
    CHECK(simd_any(neg) != 0);
    simd_int4 allneg = {-1, -2, -3, -4};
    CHECK(simd_all(allneg) != 0);
    simd_int4 none = {0, 1, 2, 3};
    CHECK(simd_any(none) == 0);
    simd_float4 f = {1.9f, -1.9f, 2.5f, 3.0f};
    simd_int4 i = simd_int(f);
    CHECK(i[0] == 1 && i[1] == -1 && i[2] == 2 && i[3] == 3);
    simd_float2 f2 = {0.5f, 2.0f};
    simd_double2 d = simd_double(f2);
    CHECK(d[0] == 0.5 && d[1] == 2.0);
    CHECK(sizeof(simd_packed_float4) == 16);
    CHECK(_Alignof(simd_packed_float4) == 4);
    CHECK(_Alignof(simd_packed_int2) == 4);
    CHECK(_Alignof(simd_packed_double4) == 8);
    CHECK(sizeof(simd_packed_char8) == 8);
    printf(failures ? "FAILED\n" : "PASSED\n");
    return failures != 0;
}
