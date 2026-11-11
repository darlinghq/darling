// Exercise libsystem_m's matrix inverse entry points through by-value vectors.
#include <math.h>
#include <stdio.h>

typedef float simd_float3 __attribute__((ext_vector_type(3)));
typedef float simd_float4 __attribute__((ext_vector_type(4)));
typedef struct { simd_float3 columns[3]; } simd_float3x3;
typedef struct { simd_float4 columns[4]; } simd_float4x4;

extern simd_float3x3 __invert_f3(simd_float3x3) __asm("___invert_f3");
extern simd_float4x4 __invert_f4(simd_float4x4) __asm("___invert_f4");

typedef float simd_float2 __attribute__((ext_vector_type(2)));
typedef double simd_double2 __attribute__((ext_vector_type(2)));
typedef double simd_double3 __attribute__((ext_vector_type(3)));
typedef double simd_double4 __attribute__((ext_vector_type(4)));
typedef struct { simd_float2 columns[2]; } simd_float2x2;
typedef struct { simd_double2 columns[2]; } simd_double2x2;
typedef struct { simd_double3 columns[3]; } simd_double3x3;
typedef struct { simd_double4 columns[4]; } simd_double4x4;

extern simd_float2x2 __invert_f2(simd_float2x2) __asm("___invert_f2");
extern simd_double2x2 __invert_d2(simd_double2x2) __asm("___invert_d2");
extern simd_double3x3 __invert_d3(simd_double3x3) __asm("___invert_d3");
extern simd_double4x4 __invert_d4(simd_double4x4) __asm("___invert_d4");

#if defined(__x86_64__) || defined(__arm64__)
#define HAVE_HALF 1
typedef _Float16 simd_half2 __attribute__((ext_vector_type(2)));
typedef _Float16 simd_half3 __attribute__((ext_vector_type(3)));
typedef _Float16 simd_half4 __attribute__((ext_vector_type(4)));
typedef struct { simd_half2 columns[2]; } simd_half2x2;
typedef struct { simd_half3 columns[3]; } simd_half3x3;
typedef struct { simd_half4 columns[4]; } simd_half4x4;

extern simd_half2x2 __invert_h2(simd_half2x2) __asm("___invert_h2");
extern simd_half3x3 __invert_h3(simd_half3x3) __asm("___invert_h3");
extern simd_half4x4 __invert_h4(simd_half4x4) __asm("___invert_h4");
#endif

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

// Same as identity_error, in double precision.
static double identity_error_d(const double *a, const double *b, int n)
{
	double worst = 0;
	for (int c = 0; c < n; c++)
		for (int r = 0; r < n; r++) {
			double sum = 0;
			for (int k = 0; k < n; k++)
				sum += a[k * n + r] * b[c * n + k];
			double error = fabs(sum - (r == c));
			if (!isfinite(error))
				return INFINITY;
			if (error > worst)
				worst = error;
		}
	return worst;
}

// Copies the n columns of a simd matrix (any element type) into doubles.
#define FLATTEN(m, n, out) \
	for (int c_ = 0; c_ < (n); c_++) \
		for (int r_ = 0; r_ < (n); r_++) \
			(out)[c_ * (n) + r_] = (double)(m).columns[c_][r_]

// The same nonsingular matrices as above, for the other variants.
static const double M2[4] = { 3, 1, 4, 2 };
static const double M3[9] = { 2, 0, 1, 1, 3, 0, 0, 1, 4 };
static const double M4[16] = { 4, 0, 0, 0, 1, 3, 0, 2, 0, 1, 2, 0, 5, -2, 1, 1 };

#define LOAD(m, n, src) \
	for (int c_ = 0; c_ < (n); c_++) \
		for (int r_ = 0; r_ < (n); r_++) \
			(m).columns[c_][r_] = (src)[c_ * (n) + r_]

#define CHECK_VARIANT(name, type, fn, n, src, tolerance) do { \
		type m_, i_; \
		double a_[(n) * (n)], b_[(n) * (n)]; \
		LOAD(m_, n, src); \
		i_ = fn(m_); \
		FLATTEN(m_, n, a_); \
		FLATTEN(i_, n, b_); \
		check(name, identity_error_d(a_, b_, n) < (tolerance) \
			&& identity_error_d(b_, a_, n) < (tolerance)); \
	} while (0)

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
	      fabsf(ti.columns[3][0] + 3) < 1e-5f && fabsf(ti.columns[3][1] - 5) < 1e-5f &&
	      fabsf(ti.columns[3][2] + 7) < 1e-5f && fabsf(ti.columns[3][3] - 1) < 1e-5f);

	check("inverse(M) * M == I for a 3x3 matrix", identity_error(b3, a3, 3) < 1e-5f);

	CHECK_VARIANT("__invert_f2 gives the inverse of a 2x2 matrix", simd_float2x2, __invert_f2, 2, M2, 1e-5);
	CHECK_VARIANT("__invert_d2 gives the inverse of a 2x2 matrix", simd_double2x2, __invert_d2, 2, M2, 1e-12);
	CHECK_VARIANT("__invert_d3 gives the inverse of a 3x3 matrix", simd_double3x3, __invert_d3, 3, M3, 1e-12);
	CHECK_VARIANT("__invert_d4 gives the inverse of a 4x4 matrix", simd_double4x4, __invert_d4, 4, M4, 1e-12);
#ifdef HAVE_HALF
	CHECK_VARIANT("__invert_h2 gives the inverse of a 2x2 matrix", simd_half2x2, __invert_h2, 2, M2, 1e-2);
	CHECK_VARIANT("__invert_h3 gives the inverse of a 3x3 matrix", simd_half3x3, __invert_h3, 3, M3, 1e-2);
	CHECK_VARIANT("__invert_h4 gives the inverse of a 4x4 matrix", simd_half4x4, __invert_h4, 4, M4, 2e-2);
#endif

	printf("failures=%d\n", failures);
	return failures != 0;
}
