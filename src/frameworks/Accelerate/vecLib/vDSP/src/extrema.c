/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

void vDSP_maxmgv(const float *__A, vDSP_Stride __IA, float *__C, vDSP_Length __N)
{
	*__C = -INFINITY;

	if (__N != 1)
	{
		#pragma clang loop vectorize(enable)
		for (vDSP_Length i = 0; i < __N; i++)
		{
			if (*__C < fabsf(__A[i * __IA]))
				*__C = fabsf(__A[i * __IA]);
		}
	}
	else
	{
		#pragma clang loop vectorize(enable)
		for (vDSP_Length i = 0; i < __N; i++)
		{
			if (*__C < fabsf(__A[i]))
				*__C = fabsf(__A[i]);
		}
	}
}

// With N equal to zero the result is -infinity (maxv) or infinity (minv), and A is not read.
// GUESS: a NaN element never compares greater (or less) than the best so far, so it is skipped and a vector
// of only NaNs gives -infinity (or infinity). A negative stride walks backwards by the same formula. The
// pages cover neither.
#define VDSP_MAXV(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T best = -INFINITY; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T value = __A[(long) n * __IA]; \
		if (value > best) \
			best = value; \
	} \
	*__C = best; \
}

#define VDSP_MINV(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length __N) \
{ \
	T best = INFINITY; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T value = __A[(long) n * __IA]; \
		if (value < best) \
			best = value; \
	} \
	*__C = best; \
}

VDSP_MAXV(vDSP_maxv, float)
VDSP_MAXV(vDSP_maxvD, double)
VDSP_MINV(vDSP_minv, float)
VDSP_MINV(vDSP_minvD, double)

// The maximum or minimum and the index at which it occurs. With N equal to zero C is -infinity (maxvi) or infinity
// (minvi), I is 0, and A is not read; the vDSP_maxvi and vDSP_minvi pages state this.
// I is the offset into A, n * IA for element n, not the position n within the strided vector. The public vDSP.h
// of the macOS 10.15 SDK gives, for vDSP_maxvi and vDSP_minvi, I as the smallest offset i*IA of an element equal
// to C (https://github.com/phracker/MacOSX-SDKs, MacOSX10.15.sdk, Accelerate.framework, vecLib Headers/vDSP.h).
// That also gives the tie rule: the first occurrence, so a later equal value does not replace it (the strict
// comparison vDSP_maxv uses above).
// GUESS: a NaN element is skipped as in vDSP_maxv, so a vector of only NaNs gives -infinity (or infinity) and an
// I of 0. Neither the pages nor the header cover it.
// GUESS: a negative stride walks backwards by the same formula, and I is (vDSP_Length) (n * IA), which wraps
// because a vDSP_Length cannot hold the negative offset the header's wording gives; for ties the first element
// read wins. The pages and the header cover neither.
// Test: vDSPTest.c, testExtremaIndex.
#define VDSP_MAXVI(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length* __I, vDSP_Length __N) \
{ \
	T best = -INFINITY; \
	vDSP_Length bestIndex = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T value = __A[(long) n * __IA]; \
		if (value > best) \
		{ \
			best = value; \
			bestIndex = (vDSP_Length) ((long) n * __IA); \
		} \
	} \
	*__C = best; \
	*__I = bestIndex; \
}

#define VDSP_MINVI(NAME, T) \
void NAME(const T* __A, vDSP_Stride __IA, T* __C, vDSP_Length* __I, vDSP_Length __N) \
{ \
	T best = INFINITY; \
	vDSP_Length bestIndex = 0; \
	for (vDSP_Length n = 0; n < __N; n++) \
	{ \
		const T value = __A[(long) n * __IA]; \
		if (value < best) \
		{ \
			best = value; \
			bestIndex = (vDSP_Length) ((long) n * __IA); \
		} \
	} \
	*__C = best; \
	*__I = bestIndex; \
}

VDSP_MAXVI(vDSP_maxvi, float)
VDSP_MAXVI(vDSP_maxviD, double)
VDSP_MINVI(vDSP_minvi, float)
VDSP_MINVI(vDSP_minviD, double)
