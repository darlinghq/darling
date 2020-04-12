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
