/*
 * SIMD matrix inverse entry points that stock arm64e macOS binaries call as
 * plain C symbols: __invert_f3 and __invert_f4.
 *
 * They belong to the vector half of libm (the libmaltvec-style helpers that
 * libSystem re-exports), so a guest that looks for them in libSystem.B.dylib
 * only finds them if libsystem_m provides them. Stock apps such as Chess call
 * them from their own transform math, so dyld aborts the process at load time
 * without them.
 *
 * ABI, verified against Chess's call sites in both slices:
 *   - the matrix is one homogeneous float aggregate passed by value (three
 *     float32x4_t rows, or four) and the inverse comes back the same way;
 *   - aarch64 therefore passes and returns it in v0-v2 / v0-v3, and x86_64
 *     passes it in memory with the hidden return pointer in rdi.
 *
 * The order of the rows inside the aggregate does not have to match Apple's:
 * for a non-singular M, transpose(invert(transpose(M))) == invert(M), so
 * reading the same floats as a transposed matrix and writing the result back
 * in the same order produces byte-identical output. Only the 3x3 padding lanes
 * are unspecified here; they are written as zeros.
 */

typedef float __invert_v4 __attribute__((vector_size(16)));
typedef struct { __invert_v4 r[3]; } __invert_m3;
typedef struct { __invert_v4 r[4]; } __invert_m4;

static void invert3(const float *in, float *out)
{
	const float a = in[0], b = in[1], c = in[2];
	const float d = in[4], e = in[5], f = in[6];
	const float g = in[8], h = in[9], i = in[10];

	const float A = e * i - f * h;
	const float B = f * g - d * i;
	const float C = d * h - e * g;
	const float det = 1.0f / (a * A + b * B + c * C);

	out[0] = A * det;
	out[1] = (c * h - b * i) * det;
	out[2] = (b * f - c * e) * det;
	out[4] = B * det;
	out[5] = (a * i - c * g) * det;
	out[6] = (c * d - a * f) * det;
	out[8] = C * det;
	out[9] = (b * g - a * h) * det;
	out[10] = (a * e - b * d) * det;
}

static void invert4(const float *in, float *out)
{
	const float a00 = in[0], a01 = in[1], a02 = in[2], a03 = in[3];
	const float a10 = in[4], a11 = in[5], a12 = in[6], a13 = in[7];
	const float a20 = in[8], a21 = in[9], a22 = in[10], a23 = in[11];
	const float a30 = in[12], a31 = in[13], a32 = in[14], a33 = in[15];

	const float b00 = a00 * a11 - a01 * a10;
	const float b01 = a00 * a12 - a02 * a10;
	const float b02 = a00 * a13 - a03 * a10;
	const float b03 = a01 * a12 - a02 * a11;
	const float b04 = a01 * a13 - a03 * a11;
	const float b05 = a02 * a13 - a03 * a12;
	const float b06 = a20 * a31 - a21 * a30;
	const float b07 = a20 * a32 - a22 * a30;
	const float b08 = a20 * a33 - a23 * a30;
	const float b09 = a21 * a32 - a22 * a31;
	const float b10 = a21 * a33 - a23 * a31;
	const float b11 = a22 * a33 - a23 * a32;

	const float det = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09
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

__invert_m3 invert_f3(__invert_m3 m) __asm("___invert_f3");
__invert_m3 invert_f3(__invert_m3 m)
{
	__invert_m3 r;
	float in[12], out[12];
	int i;

	for (i = 0; i < 3; i++) {
		in[4 * i + 0] = m.r[i][0];
		in[4 * i + 1] = m.r[i][1];
		in[4 * i + 2] = m.r[i][2];
	}
	invert3(in, out);
	for (i = 0; i < 3; i++)
		r.r[i] = (__invert_v4){ out[4 * i + 0], out[4 * i + 1], out[4 * i + 2], 0.0f };
	return r;
}

__invert_m4 invert_f4(__invert_m4 m) __asm("___invert_f4");
__invert_m4 invert_f4(__invert_m4 m)
{
	__invert_m4 r;
	float in[16], out[16];
	int i;

	for (i = 0; i < 4; i++) {
		in[4 * i + 0] = m.r[i][0];
		in[4 * i + 1] = m.r[i][1];
		in[4 * i + 2] = m.r[i][2];
		in[4 * i + 3] = m.r[i][3];
	}
	invert4(in, out);
	for (i = 0; i < 4; i++)
		r.r[i] = (__invert_v4){ out[4 * i + 0], out[4 * i + 1],
			out[4 * i + 2], out[4 * i + 3] };
	return r;
}