/*
 * Matrix inverse entry points of the simd library: __invert_f2/f3/f4,
 * __invert_d2/d3/d4 and __invert_h2/h3/h4.
 *
 * The SDK declares them in <simd/extern.h>, and the inline simd_inverse()
 * functions of <simd/matrix.h> call them, so every binary that inverts a
 * simd matrix imports them from libSystem (which re-exports libsystem_m).
 *
 * ABI: each takes one simd_<type>NxN aggregate by value and returns one. The
 * aggregate is a struct of N column vectors declared with ext_vector_type,
 * so the types below reproduce the SDK layout and the compiler applies the
 * same calling convention as the caller.
 *
 * The inverse is computed in double precision from cofactors. Reading the
 * columns as rows is harmless: transpose(inverse(transpose(M))) == inverse(M).
 * The padding lane of 3-element columns is written as zero. A singular
 * matrix gives non-finite elements.
 */

typedef float invert_float2 __attribute__((ext_vector_type(2)));
typedef float invert_float3 __attribute__((ext_vector_type(3)));
typedef float invert_float4 __attribute__((ext_vector_type(4)));
typedef double invert_double2 __attribute__((ext_vector_type(2)));
typedef double invert_double3 __attribute__((ext_vector_type(3)));
typedef double invert_double4 __attribute__((ext_vector_type(4)));

typedef struct { invert_float2 columns[2]; } invert_float2x2;
typedef struct { invert_float3 columns[3]; } invert_float3x3;
typedef struct { invert_float4 columns[4]; } invert_float4x4;
typedef struct { invert_double2 columns[2]; } invert_double2x2;
typedef struct { invert_double3 columns[3]; } invert_double3x3;
typedef struct { invert_double4 columns[4]; } invert_double4x4;

/* Each helper inverts an n x n matrix stored as n * n consecutive values. */

static void invert2(const double *in, double *out)
{
	const double a = in[0], b = in[1];
	const double c = in[2], d = in[3];
	const double det = 1.0 / (a * d - b * c);

	out[0] = d * det;
	out[1] = -b * det;
	out[2] = -c * det;
	out[3] = a * det;
}

static void invert3(const double *in, double *out)
{
	const double a = in[0], b = in[1], c = in[2];
	const double d = in[3], e = in[4], f = in[5];
	const double g = in[6], h = in[7], i = in[8];

	const double A = e * i - f * h;
	const double B = f * g - d * i;
	const double C = d * h - e * g;
	const double det = 1.0 / (a * A + b * B + c * C);

	out[0] = A * det;
	out[1] = (c * h - b * i) * det;
	out[2] = (b * f - c * e) * det;
	out[3] = B * det;
	out[4] = (a * i - c * g) * det;
	out[5] = (c * d - a * f) * det;
	out[6] = C * det;
	out[7] = (b * g - a * h) * det;
	out[8] = (a * e - b * d) * det;
}

static void invert4(const double *in, double *out)
{
	const double a00 = in[0], a01 = in[1], a02 = in[2], a03 = in[3];
	const double a10 = in[4], a11 = in[5], a12 = in[6], a13 = in[7];
	const double a20 = in[8], a21 = in[9], a22 = in[10], a23 = in[11];
	const double a30 = in[12], a31 = in[13], a32 = in[14], a33 = in[15];

	const double b00 = a00 * a11 - a01 * a10;
	const double b01 = a00 * a12 - a02 * a10;
	const double b02 = a00 * a13 - a03 * a10;
	const double b03 = a01 * a12 - a02 * a11;
	const double b04 = a01 * a13 - a03 * a11;
	const double b05 = a02 * a13 - a03 * a12;
	const double b06 = a20 * a31 - a21 * a30;
	const double b07 = a20 * a32 - a22 * a30;
	const double b08 = a20 * a33 - a23 * a30;
	const double b09 = a21 * a32 - a22 * a31;
	const double b10 = a21 * a33 - a23 * a31;
	const double b11 = a22 * a33 - a23 * a32;

	const double det = 1.0 / (b00 * b11 - b01 * b10 + b02 * b09
		+ b03 * b08 - b04 * b07 + b05 * b06);

	out[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
	out[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
	out[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
	out[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
	out[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
	out[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
	out[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
	out[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
	out[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
	out[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
	out[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
	out[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
	out[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
	out[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
	out[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
	out[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;
}

/*
 * Defines one entry point: copy the N columns into a double array, invert,
 * and copy back. Assigning the whole column vector clears the padding lane.
 */
#define DEFINE_INVERT(sym, matrix_type, scalar, vec_type, n)		\
	matrix_type sym##_impl(matrix_type m) __asm("_" #sym);		\
	matrix_type sym##_impl(matrix_type m)				\
	{								\
		double in[n * n], out[n * n];				\
		matrix_type r;						\
		int c, k;						\
		for (c = 0; c < n; c++)					\
			for (k = 0; k < n; k++)				\
				in[c * n + k] = m.columns[c][k];	\
		invert##n(in, out);					\
		for (c = 0; c < n; c++) {				\
			vec_type v = 0;					\
			for (k = 0; k < n; k++)				\
				v[k] = (scalar) out[c * n + k];		\
			r.columns[c] = v;				\
		}							\
		return r;						\
	}

DEFINE_INVERT(__invert_f2, invert_float2x2, float, invert_float2, 2)
DEFINE_INVERT(__invert_f3, invert_float3x3, float, invert_float3, 3)
DEFINE_INVERT(__invert_f4, invert_float4x4, float, invert_float4, 4)
DEFINE_INVERT(__invert_d2, invert_double2x2, double, invert_double2, 2)
DEFINE_INVERT(__invert_d3, invert_double3x3, double, invert_double3, 3)
DEFINE_INVERT(__invert_d4, invert_double4x4, double, invert_double4, 4)

/*
 * The half-precision variants exist only for the 64-bit architectures; the
 * SDK declares them for simd library version 6 (macOS 13 and later).
 */
#if defined(__x86_64__) || defined(__aarch64__) || defined(__arm64__)
typedef _Float16 invert_half2 __attribute__((ext_vector_type(2)));
typedef _Float16 invert_half3 __attribute__((ext_vector_type(3)));
typedef _Float16 invert_half4 __attribute__((ext_vector_type(4)));

typedef struct { invert_half2 columns[2]; } invert_half2x2;
typedef struct { invert_half3 columns[3]; } invert_half3x3;
typedef struct { invert_half4 columns[4]; } invert_half4x4;

DEFINE_INVERT(__invert_h2, invert_half2x2, _Float16, invert_half2, 2)
DEFINE_INVERT(__invert_h3, invert_half3x3, _Float16, invert_half3, 3)
DEFINE_INVERT(__invert_h4, invert_half4x4, _Float16, invert_half4, 4)
#endif
