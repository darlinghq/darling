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
