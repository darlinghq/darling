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

#include <CarbonCore/FixMath.h>
#include <cmath>
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


float FixedToFloat(Fixed f)
{
	return float(f) / Darling::FixedRatio;
}

Fixed FloatToFixed(float f)
{
	return Darling::checkNumberBounds(f * Darling::FixedRatio);
}

Fract FloatToFract(float f)
{
	return Darling::checkNumberBounds(f * Darling::FractRatio);
}

float FractToFloat(Fract f)
{
	return float(f) / Darling::FractRatio;
}

Fixed FixRatio(short n, short denominator)
{
	float f = float(n) / float(denominator);
	return FloatToFixed(f);
}

Fixed FixMul(Fixed a, Fixed b)
{
	float f = FixedToFloat(a) * FixedToFloat(b);
	return FloatToFixed(f);
}

short FixRound(Fixed f)
{
	return f >> 16;
}

Fract Fix2Frac(Fixed f)
{
	return f << 14;
}

int32_t Fix2Long(Fixed f)
{
	return f >> 16;
}

Fixed Long2Fix(int32_t v)
{ 
	return Darling::checkNumberBounds(int64_t(v) * Darling::FixedRatio);
}

Fixed Frac2Fix(Fract f)
{
	return Darling::checkNumberBounds(f * 0x4000);
}

Fract FracMul(Fract x, Fract y)
{
	return FloatToFract(FractToFloat(x) * FractToFloat(y));
}

Fixed FixDiv(Fixed x, Fixed y)
{
	return FloatToFixed(FixedToFloat(x) / FixedToFloat(y));
}

Fract FracDiv(Fract x, Fract y)
{
	return FloatToFract(FractToFloat(x) / FractToFloat(y));
}

Fract FracSqrt(Fract x)
{
	return FloatToFract(sqrtf(FractToFloat(x)));
}

Fract FracSin(Fixed x)
{
	return FloatToFract(sinf(FractToFloat(x)));
}

Fract FracCos(Fixed x)
{
	return FloatToFract(cosf(FractToFloat(x)));
}

Fixed FixATan2(int32_t x, int32_t y)
{
	float f = atan2f(x, y);
	return FloatToFixed(f);
}

double Frac2X(Fract f)
{
	return double(f) / Darling::FractRatio;
}

double Fix2X(Fract f)
{
	return double(f) / Darling::FixedRatio;
}

Fixed X2Fix(double d)
{
	return Darling::checkNumberBounds(d * Darling::FixedRatio);
}

Fract X2Frac(double d)
{
	return Darling::checkNumberBounds(d * Darling::FractRatio);
}

short WideCompare(const wide* a, const wide* b)
{
	if (*reinterpret_cast<const int64_t*>(a) > *reinterpret_cast<const int64_t*>(b))
		return 1;
	else if (*reinterpret_cast<const int64_t*>(a) < *reinterpret_cast<const int64_t*>(b))
		return -1;
	else
		return 0;
}

wide* WideAdd(wide* dst, const wide* val)
{
	*reinterpret_cast<int64_t*>(dst) += *reinterpret_cast<const int64_t*>(val);
	return dst;
}

wide* WideSubtract(wide* dst, const wide* val)
{
	*reinterpret_cast<int64_t*>(dst) -= *reinterpret_cast<const int64_t*>(val);
	return dst;
}

wide* WideNegate(wide* val)
{
	*reinterpret_cast<int64_t*>(val) = -*reinterpret_cast<int64_t*>(val);
	return val;
}

wide* WideShift(wide* dst, int32_t shift) // rounds upwards
{
	int64_t result, mask;
	bool round;

	if (shift >= 0)
	{
		result = *(reinterpret_cast<int64_t*>(dst)) >> shift;
		mask = result << shift;
	}
	else
	{
		result = *(reinterpret_cast<int64_t*>(dst)) << (-shift);
		mask = result >> (-shift);
	}
	
	round = ((*reinterpret_cast<int64_t*>(dst)) & ~mask) != 0;

	if (round)
		result++;
	*reinterpret_cast<int64_t*>(dst) = result;

	return dst;
}

uint32_t WideSquareRoot(const wide* val)
{
	return (uint32_t) sqrt(*reinterpret_cast<const int64_t*>(val));
}

wide* WideMultiply(int32_t a, int32_t b, wide* dst)
{
	*reinterpret_cast<int64_t*>(dst) = int64_t(a) * int64_t(b);
	return dst;
}

int32_t WideDivide(const wide* divd, int32_t divs, int32_t* remainder)
{
	if (remainder)
		*remainder = *reinterpret_cast<const int64_t*>(divd) % divs;
	return int32_t(*reinterpret_cast<const int64_t*>(divd) / divs);
}

wide* WideWideDivide(wide* divd, int32_t divs, int32_t* remainder)
{
	if (remainder)
		*remainder = *reinterpret_cast<int64_t*>(divd) % divs;
	*reinterpret_cast<int64_t*>(divd) /= divs;
	return divd;
}

wide* WideBitShift(wide* dst, int32_t shift)
{
	// positive -> right
	// negative -> left
	
	if (shift >= 0)
		*reinterpret_cast<int64_t*>(dst) >>= shift;
	else
		*reinterpret_cast<int64_t*>(dst) <<= (-shift);

	return dst;
}

UnsignedFixed UnsignedFixedMulDiv(UnsignedFixed a, UnsignedFixed mul, UnsignedFixed div)
{
	// Is this correct?
	return uint64_t(a) * uint64_t(mul) / uint64_t(div);
}

