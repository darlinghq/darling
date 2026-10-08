// Exercise libsystem_m's matrix inverse entry points through by-value vectors.
#include <math.h>
#include <stdio.h>

typedef float simd_float3 __attribute__((ext_vector_type(3)));
typedef float simd_float4 __attribute__((ext_vector_type(4)));
typedef struct { simd_float3 columns[3]; } simd_float3x3;
typedef struct { simd_float4 columns[4]; } simd_float4x4;

extern simd_float3x3 __invert_f3(simd_float3x3) __asm("___invert_f3");
extern simd_float4x4 __invert_f4(simd_float4x4) __asm("___invert_f4");

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

// Largest |(A*B)[r][c] - I[r][c]| for n x n column-major matrices.
static float identity_error(const float *a, const float *b, int n)
{
	float worst = 0;
	for (int c = 0; c < n; c++)
		for (int r = 0; r < n; r++) {
			float sum = 0;
			for (int k = 0; k < n; k++)
				sum += a[k * n + r] * b[c * n + k];
			float error = fabsf(sum - (r == c));
			if (!isfinite(error))
				return INFINITY;
			worst = fmaxf(worst, error);
		}
	return worst;
}

int main(void)
{
	simd_float3x3 m3 = {{ { 2, 0, 1 }, { 1, 3, 0 }, { 0, 1, 4 } }};
	simd_float3x3 i3 = __invert_f3(m3);
	float a3[9], b3[9];
	for (int c = 0; c < 3; c++)
		for (int r = 0; r < 3; r++) {
			a3[c * 3 + r] = m3.columns[c][r];
			b3[c * 3 + r] = i3.columns[c][r];
		}
	check("M * inverse(M) == I for a 3x3 matrix", identity_error(a3, b3, 3) < 1e-5f);
	// det = 25; element (row 0, col 1) of the inverse is -4/25.
	check("3x3 inverse element (0,1) is -4/25", fabsf(i3.columns[1][0] + 4.0f / 25) < 1e-6f);

	simd_float4x4 m4 = {{ { 4, 0, 0, 0 }, { 1, 3, 0, 2 }, { 0, 1, 2, 0 }, { 5, -2, 1, 1 } }};
	simd_float4x4 i4 = __invert_f4(m4);
	float a4[16], b4[16];
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++) {
			a4[c * 4 + r] = m4.columns[c][r];
			b4[c * 4 + r] = i4.columns[c][r];
		}
	check("M * inverse(M) == I for a 4x4 matrix", identity_error(a4, b4, 4) < 1e-5f);
	check("inverse(M) * M == I for a 4x4 matrix", identity_error(b4, a4, 4) < 1e-5f);

	// A translation matrix inverts to the opposite translation.
	simd_float4x4 t = {{ { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 3, -5, 7, 1 } }};
	simd_float4x4 ti = __invert_f4(t);
	check("translation inverts to the opposite translation",
	      ti.columns[3][0] == -3 && ti.columns[3][1] == 5 && ti.columns[3][2] == -7 && ti.columns[3][3] == 1);

	check("inverse(M) * M == I for a 3x3 matrix", identity_error(b3, a3, 3) < 1e-5f);

	printf("failures=%d\n", failures);
	return failures != 0;
}
