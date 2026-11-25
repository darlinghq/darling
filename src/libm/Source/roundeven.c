/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// roundeven() and roundevenf() of C23 (ISO/IEC 9899:2024, 7.12.9.8): the
// integer nearest to the argument, with halfway cases going to the even one,
// whatever the current rounding direction is.

#include <math.h>

double roundeven(double x)
{
	// 2^52 and above (and infinities and NaNs) are already integral
	if (!(fabs(x) < 0x1p52))
		return x;
	double nearest = round(x); // halfway cases away from zero
	if (fabs(x - trunc(x)) == 0.5) {
		// x / 2 is exact here: a halfway case is at least 0.5 in magnitude
		nearest = 2.0 * round(x / 2.0);
	}
	return nearest;
}

float roundevenf(float x)
{
	if (!(fabsf(x) < 0x1p23f))
		return x;
	float nearest = roundf(x);
	if (fabsf(x - truncf(x)) == 0.5f)
		nearest = 2.0f * roundf(x / 2.0f);
	return nearest;
}
