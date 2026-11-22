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

// Element n of a strided vector is read at index n * stride.
// GUESS: a negative stride walks the vector backwards by the same formula; the pages define the stride only
// as the distance between elements.
// GUESS: the sums run front to back in the precision of the data (single precision for the float variants,
// as vDSP_svesq does). The pages do not give an accumulation order or precision, so results can differ from
// the original in the last bits.

// C[0] = sum(A[n])
#define VDSP_SVE(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += __A[(long) n * __IA]; \
	*__C = sum; \
}

// C[0] = sum(|A[n]|)
#define VDSP_SVEMG(NAME, T, ABS) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += ABS(__A[(long) n * __IA]); \
	*__C = sum; \
}

// C[0] = sum(A[n] * |A[n]|), the sum of signed squares.
#define VDSP_SVS(NAME, T, ABS) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA]; \
		sum += a * ABS(a); \
	} \
	*__C = sum; \
}

// The pages describe the mean, the mean of squares and the root mean square in words and say nothing
// about N equal to zero. These divide the sum by N, so an empty vector gives NaN (GUESS).
#define VDSP_MEANV(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += __A[(long) n * __IA]; \
	*__C = sum / (T) __N; \
}

#define VDSP_MEASQV(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += __A[(long) n * __IA] * __A[(long) n * __IA]; \
	*__C = sum / (T) __N; \
}

#define VDSP_RMSQV(NAME, T, SQRT) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += __A[(long) n * __IA] * __A[(long) n * __IA]; \
	*__C = SQRT(sum / (T) __N); \
}

// The mean of signed squares, sum(A[n] * |A[n]|) / N. The vDSP_mvessq abstract and output-parameter text call
// the result the mean of "signed squares", and the vDSP_svs page defines the signed square sum as
// C[0] = sum(A[n] * |A[n]|) (see VDSP_SVS above); that is the meaning used. The mvessq page's example prints 0
// for -8, -4, -2, 0, 2, 4, 8, which rules out a mean of plain squares (24) but does not by itself pick the
// signed reading; only the discussion sentence says "squares".
// The vDSP_mvessqD page, and only that page, says N equal to zero gives -infinity. This applies it to both
// precisions.
// GUESS: low confidence. The -infinity rests on the __N text of the D page, whose parameter block looks like
// boilerplate from another function; the single-precision page is silent about N equal to zero.
#define VDSP_MVESSQ(NAME, T, ABS) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	if (__N == 0) \
	{ \
		*__C = -INFINITY; \
		return; \
	} \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T a = __A[(long) n * __IA]; \
		sum += a * ABS(a); \
	} \
	*__C = sum / (T) __N; \
}

// C[0] = sum(A[n] * B[n])
#define VDSP_DOTPR(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, const T* __B, vDSP_Stride __IB, T* __C, vDSP_Length __N) \
{ \
	T sum = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
		sum += __A[(long) n * __IA] * __B[(long) n * __IB]; \
	*__C = sum; \
}

VDSP_SVE(vDSP_sve, float)
VDSP_SVE(vDSP_sveD, double)
VDSP_SVEMG(vDSP_svemg, float, fabsf)
VDSP_SVEMG(vDSP_svemgD, double, fabs)
VDSP_SVS(vDSP_svs, float, fabsf)
VDSP_SVS(vDSP_svsD, double, fabs)
VDSP_MVESSQ(vDSP_mvessq, float, fabsf)
VDSP_MVESSQ(vDSP_mvessqD, double, fabs)
VDSP_MEANV(vDSP_meanv, float)
VDSP_MEANV(vDSP_meanvD, double)
VDSP_MEASQV(vDSP_measqv, float)
VDSP_MEASQV(vDSP_measqvD, double)
VDSP_RMSQV(vDSP_rmsqv, float, sqrtf)
VDSP_RMSQV(vDSP_rmsqvD, double, sqrt)
VDSP_DOTPR(vDSP_dotpr, float)
VDSP_DOTPR(vDSP_dotprD, double)
