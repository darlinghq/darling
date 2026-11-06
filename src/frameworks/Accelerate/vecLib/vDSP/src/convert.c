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

// The stride of a DSPComplex vector counts floats, so it's twice the number of elements stepped.

void vDSP_ctoz(const DSPComplex* __C, vDSP_Stride __IC, const DSPSplitComplex* __Z, vDSP_Stride __IZ, vDSP_Length __N)
{
	for (vDSP_Length n = 0; n < __N; n++)
	{
		const DSPComplex value = __C[(long) n * __IC / 2];
		__Z->realp[(long) n * __IZ] = value.real;
		__Z->imagp[(long) n * __IZ] = value.imag;
	}
}

void vDSP_ztoc(const DSPSplitComplex* __Z, vDSP_Stride __IZ, DSPComplex* __C, vDSP_Stride __IC, vDSP_Length __N)
{
	for (vDSP_Length n = 0; n < __N; n++)
	{
		DSPComplex* value = &__C[(long) n * __IC / 2];
		value->real = __Z->realp[(long) n * __IZ];
		value->imag = __Z->imagp[(long) n * __IZ];
	}
}

// __Conjugate is 1 for A * B, and -1 for conj(A) * B.
void vDSP_zvmul(const DSPSplitComplex* __A, vDSP_Stride __IA, const DSPSplitComplex* __B, vDSP_Stride __IB, const DSPSplitComplex* __C, vDSP_Stride __IC, vDSP_Length __N, int __Conjugate)
{
	const float sign = __Conjugate < 0 ? -1.0f : 1.0f;

	for (vDSP_Length n = 0; n < __N; n++)
	{
		const float ar = __A->realp[(long) n * __IA], ai = sign * __A->imagp[(long) n * __IA];
		const float br = __B->realp[(long) n * __IB], bi = __B->imagp[(long) n * __IB];

		__C->realp[(long) n * __IC] = ar * br - ai * bi;
		__C->imagp[(long) n * __IC] = ar * bi + ai * br;
	}
}
