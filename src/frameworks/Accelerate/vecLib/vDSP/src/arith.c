/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <vDSP/vDSP.h>
#include <math.h>

// Vectors may overlap when both strides match, so these run strictly front to back.

void vDSP_vadd(const float* __A, vDSP_Stride __IA, const float* __B, vDSP_Stride __IB, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	for (vDSP_Length n = 0; n < __N; n++)
		__C[(long) n * __IC] = __A[(long) n * __IA] + __B[(long) n * __IB];
}

// The arguments are in the documented order: the result is A - B, with B first.
void vDSP_vsub(const float* __B, vDSP_Stride __IB, const float* __A, vDSP_Stride __IA, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	for (vDSP_Length n = 0; n < __N; n++)
		__C[(long) n * __IC] = __A[(long) n * __IA] - __B[(long) n * __IB];
}

void vDSP_vmul(const float* __A, vDSP_Stride __IA, const float* __B, vDSP_Stride __IB, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	for (vDSP_Length n = 0; n < __N; n++)
		__C[(long) n * __IC] = __A[(long) n * __IA] * __B[(long) n * __IB];
}

void vDSP_vsmul(const float* __A, vDSP_Stride __IA, const float* __B, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	const float scalar = *__B;
	for (vDSP_Length n = 0; n < __N; n++)
		__C[(long) n * __IC] = __A[(long) n * __IA] * scalar;
}

void vDSP_vsadd(const float* __A, vDSP_Stride __IA, const float* __B, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	const float scalar = *__B;
	for (vDSP_Length n = 0; n < __N; n++)
		__C[(long) n * __IC] = __A[(long) n * __IA] + scalar;
}

void vDSP_vsma(const float* __A, vDSP_Stride __IA, const float* __B, const float* __C, vDSP_Stride __IC, float* __D, vDSP_Stride __ID, vDSP_Length __N)
{
	const float scalar = *__B;
	for (vDSP_Length n = 0; n < __N; n++)
		__D[(long) n * __ID] = __A[(long) n * __IA] * scalar + __C[(long) n * __IC];
}

void vDSP_svesq(const float* __A, vDSP_Stride __IA, float* __C, vDSP_Length __N)
{
	float sum = 0;
	for (vDSP_Length n = 0; n < __N; n++)
		sum += __A[(long) n * __IA] * __A[(long) n * __IA];
	*__C = sum;
}

void vDSP_vclip(const float* __A, vDSP_Stride __IA, const float* __B, const float* __C, float* __D, vDSP_Stride __ID, vDSP_Length __N)
{
	const float low = *__B, high = *__C;
	for (vDSP_Length n = 0; n < __N; n++)
	{
		const float value = __A[(long) n * __IA];
		__D[(long) n * __ID] = value < low ? low : (value > high ? high : value);
	}
}

// Correlates with a positive __IF. A negative one walks the filter backwards from __F,
// which points to its last element, and makes this a convolution. __A needs __N + __P - 1 elements.
void vDSP_conv(const float* __A, vDSP_Stride __IA, const float* __F, vDSP_Stride __IF, float* __C, vDSP_Stride __IC, vDSP_Length __N, vDSP_Length __P)
{
	for (vDSP_Length n = 0; n < __N; n++)
	{
		float sum = 0;
		for (vDSP_Length p = 0; p < __P; p++)
			sum += __A[(long) (n + p) * __IA] * __F[(long) p * __IF];
		__C[(long) n * __IC] = sum;
	}
}

// Two-pole two-zero filter. __A and __C hold __N + 2 elements, and the caller
// provides the first two elements of __C as the filter's history.
#define DEQ22(A, IA, B, C, IC, N) \
	for (vDSP_Length n = 2; n < (N) + 2; n++) \
	{ \
		const long i = (long) n; \
		(C)[i * (IC)] = (A)[i * (IA)] * (B)[0] + (A)[(i - 1) * (IA)] * (B)[1] + (A)[(i - 2) * (IA)] * (B)[2] \
			- (C)[(i - 1) * (IC)] * (B)[3] - (C)[(i - 2) * (IC)] * (B)[4]; \
	}

void vDSP_deq22(const float* __A, vDSP_Stride __IA, const float* __B, float* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	DEQ22(__A, __IA, __B, __C, __IC, __N)
}

void vDSP_deq22D(const double* __A, vDSP_Stride __IA, const double* __B, double* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	DEQ22(__A, __IA, __B, __C, __IC, __N)
}

// The single and double precision variants below share one body each. Element n of a strided vector is
// read or written at index n * stride, as for the functions above.
// GUESS: a negative stride walks backwards by the same formula (the pages define the stride only as the distance
// between elements), and scalar arguments are read once before the loop, which matters only if one aliases the output.

// The documented operation is C = A / B, but B is passed first.
#define VDSP_VDIV(NAME, T) \
void NAME(const T* __B, vDSP_Stride __IB, const T* __A, vDSP_Stride __IA, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
		__C[(long) n * __IC] = __A[(long) n * __IA] / __B[(long) n * __IB]; \
}

VDSP_VDIV(vDSP_vdiv, float)
VDSP_VDIV(vDSP_vdivD, double)

#define VDSP_VSDIV(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	const T scalar = *__B; \
	for (vDSP_Length n = 0; n < __N; n++) \
		__C[(long) n * __IC] = __A[(long) n * __IA] / scalar; \
}

VDSP_VSDIV(vDSP_vsdiv, float)
VDSP_VSDIV(vDSP_vsdivD, double)

#define VDSP_UNARY(NAME, T, EXPR) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA]; \
		__C[(long) n * __IC] = (EXPR); \
	} \
}

VDSP_UNARY(vDSP_vneg, float, -a)
VDSP_UNARY(vDSP_vnegD, double, -a)
VDSP_UNARY(vDSP_vabs, float, fabsf(a))
VDSP_UNARY(vDSP_vabsD, double, fabs(a))
VDSP_UNARY(vDSP_vnabs, float, -fabsf(a))
VDSP_UNARY(vDSP_vnabsD, double, -fabs(a))
VDSP_UNARY(vDSP_vsq, float, a * a)
VDSP_UNARY(vDSP_vsqD, double, a * a)

// D = (A * B) + C
#define VDSP_VMA(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, vDSP_Stride __IB, const T* __C, vDSP_Stride __IC, T* __D, vDSP_Stride __ID, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
		__D[(long) n * __ID] = __A[(long) n * __IA] * __B[(long) n * __IB] + __C[(long) n * __IC]; \
}

VDSP_VMA(vDSP_vma, float)
VDSP_VMA(vDSP_vmaD, double)

// D = (A * B) + C, with C a scalar.
#define VDSP_VMSA(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, vDSP_Stride __IB, const T* __C, T* __D, vDSP_Stride __ID, vDSP_Length __N) \
{ \
	const T scalar = *__C; \
	for (vDSP_Length n = 0; n < __N; n++) \
		__D[(long) n * __ID] = __A[(long) n * __IA] * __B[(long) n * __IB] + scalar; \
}

VDSP_VMSA(vDSP_vmsa, float)
VDSP_VMSA(vDSP_vmsaD, double)

// The comparisons are written as documented, which decides what a NaN input produces.
#define VDSP_VMAX(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, vDSP_Stride __IB, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA], b = __B[(long) n * __IB]; \
		__C[(long) n * __IC] = b <= a ? a : b; \
	} \
}

#define VDSP_VMIN(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, vDSP_Stride __IB, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA], b = __B[(long) n * __IB]; \
		__C[(long) n * __IC] = a <= b ? a : b; \
	} \
}

VDSP_VMAX(vDSP_vmax, float)
VDSP_VMAX(vDSP_vmaxD, double)
VDSP_VMIN(vDSP_vmin, float)
VDSP_VMIN(vDSP_vminD, double)

// Raises every element below the threshold *B to it.
// QUIRK: a NaN element is passed through unchanged instead of being replaced by the threshold, which is what
// the page loop "A >= B ? A : B" would give. Not documented. Evidence: black-box observation of
// vDSP_vthr on macOS 26.7.3 (x86_64) and macOS 27.0.1 (arm64), tier 2, both returning NaN for a NaN element.
// Test: testRealVectorOps, "vthr quirk NaN passes through".
#define VDSP_VTHR(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	const T threshold = *__B; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA]; \
		__C[(long) n * __IC] = a < threshold ? threshold : a; \
	} \
}

VDSP_VTHR(vDSP_vthr, float)
VDSP_VTHR(vDSP_vthrD, double)

#define VDSP_VFILL(NAME, T) \
void NAME(const T* __A, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	const T scalar = *__A; \
	for (vDSP_Length n = 0; n < __N; n++) \
		__C[(long) n * __IC] = scalar; \
}

VDSP_VFILL(vDSP_vfill, float)
VDSP_VFILL(vDSP_vfillD, double)

#define VDSP_VCLR(NAME, T) \
void NAME(T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N; n++) \
		__C[(long) n * __IC] = 0; \
}

VDSP_VCLR(vDSP_vclr, float)
VDSP_VCLR(vDSP_vclrD, double)

// The page gives the initial value, the increment and an example, not a formula. C[n] = A + n * B
// is the reading that fits them (GUESS: whether the original accumulates instead is not documented).
#define VDSP_VRAMP(NAME, T) \
void NAME(const T* __A, const T* __B, T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	const T start = *__A, step = *__B; \
	for (vDSP_Length n = 0; n < __N; n++) \
		__C[(long) n * __IC] = start + (T) n * step; \
}

VDSP_VRAMP(vDSP_vramp, float)
VDSP_VRAMP(vDSP_vrampD, double)

// In place: element n and element N - 1 - n swap places.
#define VDSP_VRVRS(NAME, T) \
void NAME(T* __C, vDSP_Stride __IC, vDSP_Length __N) \
{ \
	for (vDSP_Length n = 0; n < __N / 2; n++) \
	{ \
		T* const front = &__C[(long) n * __IC]; \
		T* const back = &__C[(long) (__N - 1 - n) * __IC]; \
		const T held = *front; \
		*front = *back; \
		*back = held; \
	} \
}

VDSP_VRVRS(vDSP_vrvrs, float)
VDSP_VRVRS(vDSP_vrvrsD, double)
