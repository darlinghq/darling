/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#ifndef FIXMATH_H
#define FIXMATH_H
#include "MacTypes.h"
#include <stdint.h>

#ifdef __cplusplus
#include <limits>

namespace Darling
{
	static const int32_t InfPlus = std::numeric_limits<int32_t>::max();
	static const int32_t InfMinus = std::numeric_limits<int32_t>::min();
	static const int32_t FixedRatio = 0x10000;
	static const int32_t FractRatio = 0x40000000;

	template <typename T> int32_t checkNumberBounds(T t)
	{
		if (t < -0x1p31f)
			return InfMinus;
		else if (t > 0x1p31f)
			return InfPlus;
		else
			return int32_t(t);
	}


}

extern "C" {
#endif

float FixedToFloat(Fixed f);
Fixed FloatToFixed(float f);

Fract FloatToFract(float f);
float FractToFloat(Fract f);

Fixed FixRatio(short n, short denominator);
Fixed FixMul(Fixed a, Fixed b);
short FixRound(Fixed f);
Fract Fix2Frac(Fixed f);
int32_t Fix2Long(Fixed f);
Fixed Long2Fix(int32_t v);
Fixed Frac2Fix(Fract f);

Fract FracMul(Fract x, Fract y);
Fixed FixDiv(Fixed x, Fixed y);
Fract FracDiv(Fract x, Fract y);
Fract FracSqrt(Fract x);
Fract FracSin(Fixed x);
Fract FracCos(Fixed x);
Fixed FixATan2(int32_t x, int32_t y);

double Frac2X(Fract f);
double Fix2X(Fract f);
Fixed X2Fix(double d);
Fract X2Frac(double d);

short WideCompare(const wide* a, const wide* b);
wide* WideAdd(wide* dst, const wide* val);
wide* WideSubtract(wide* dst, const wide* val);
wide* WideNegate(wide* val);
wide* WideShift(wide* dst, int32_t shift); // rounds upwards
uint32_t WideSquareRoot(const wide* val);
wide* WideMultiply(int32_t a, int32_t b, wide* dst);
int32_t WideDivide(const wide* divd, int32_t divs, int32_t* remainder);
wide* WideWideDivide(wide* divd, int32_t divs, int32_t* remainder);
wide* WideBitShift(wide* dst, int32_t shift);

UnsignedFixed UnsignedFixedMulDiv(UnsignedFixed a, UnsignedFixed mul, UnsignedFixed div);

#ifdef __cplusplus
}
#endif

#endif

