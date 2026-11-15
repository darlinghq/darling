/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <CoreMedia/CMTime.h>
#include <math.h>
#include <stdbool.h>

// Exact arithmetic on 96 bit intermediates is done with this small unsigned
// type: the i386 slice of the framework has no 128-bit integers.
typedef struct {
	uint64_t hi;
	uint64_t lo;
} wide_t;

static wide_t wide_mul(uint64_t a, uint32_t b)
{
	uint64_t p0 = (a & 0xffffffffu) * b;
	uint64_t p1 = (a >> 32) * b;
	uint64_t mid = (p0 >> 32) + (p1 & 0xffffffffu);
	wide_t r;
	r.lo = (p0 & 0xffffffffu) | (mid << 32);
	r.hi = (p1 >> 32) + (mid >> 32);
	return r;
}

// Divides n by d (d != 0), returning the quotient and storing the remainder.
static wide_t wide_div(wide_t n, uint32_t d, uint32_t* rem)
{
	uint32_t limbs[4] = { (uint32_t) (n.hi >> 32), (uint32_t) n.hi, (uint32_t) (n.lo >> 32), (uint32_t) n.lo };
	uint64_t r = 0;
	for (int i = 0; i < 4; i++) {
		uint64_t cur = (r << 32) | limbs[i];
		limbs[i] = (uint32_t) (cur / d);
		r = cur % d;
	}
	*rem = (uint32_t) r;
	wide_t q;
	q.hi = ((uint64_t) limbs[0] << 32) | limbs[1];
	q.lo = ((uint64_t) limbs[2] << 32) | limbs[3];
	return q;
}

static int wide_cmp(wide_t a, wide_t b)
{
	if (a.hi != b.hi)
		return a.hi < b.hi ? -1 : 1;
	if (a.lo != b.lo)
		return a.lo < b.lo ? -1 : 1;
	return 0;
}

static uint64_t magnitude(int64_t v)
{
	return v < 0 ? 0 - (uint64_t) v : (uint64_t) v;
}

static uint64_t gcd64(uint64_t a, uint64_t b)
{
	while (b != 0) {
		uint64_t t = a % b;
		a = b;
		b = t;
	}
	return a;
}

typedef enum {
	kClassNegInf,
	kClassNumeric,
	kClassIndefinite,
	kClassPosInf,
	kClassInvalid,
} time_class;

static time_class classify(CMTime t)
{
	if (!(t.flags & kCMTimeFlags_Valid))
		return kClassInvalid;
	if (t.flags & kCMTimeFlags_PositiveInfinity)
		return kClassPosInf;
	if (t.flags & kCMTimeFlags_NegativeInfinity)
		return kClassNegInf;
	if (t.flags & kCMTimeFlags_Indefinite)
		return kClassIndefinite;
	return kClassNumeric;
}

const CMTime kCMTimeInvalid = { .value = 0, .timescale = 0, .flags = 0, .epoch = 0 };
const CMTime kCMTimeIndefinite = { .value = 0, .timescale = 0, .flags = kCMTimeFlags_Valid | kCMTimeFlags_Indefinite, .epoch = 0 };
const CMTime kCMTimePositiveInfinity = { .value = 0, .timescale = 0, .flags = kCMTimeFlags_Valid | kCMTimeFlags_PositiveInfinity, .epoch = 0 };
const CMTime kCMTimeNegativeInfinity = { .value = 0, .timescale = 0, .flags = kCMTimeFlags_Valid | kCMTimeFlags_NegativeInfinity, .epoch = 0 };
const CMTime kCMTimeZero = { .value = 0, .timescale = 1, .flags = kCMTimeFlags_Valid, .epoch = 0 };

static CMTime infinity(bool negative)
{
	return negative ? kCMTimeNegativeInfinity : kCMTimePositiveInfinity;
}

CMTime CMTimeMakeWithEpoch(int64_t value, int32_t timescale, int64_t epoch)
{
	if (timescale <= 0)
		return kCMTimeInvalid;
	CMTime t = { .value = value, .timescale = timescale, .flags = kCMTimeFlags_Valid, .epoch = epoch };
	return t;
}

CMTime CMTimeMake(int64_t value, int32_t timescale)
{
	return CMTimeMakeWithEpoch(value, timescale, 0);
}

CMTime CMTimeMakeWithSeconds(Float64 seconds, int32_t preferredTimescale)
{
	if (preferredTimescale <= 0)
		return kCMTimeInvalid;

	if (isnan(seconds)) {
		// QUIRK: a NaN number of seconds gives a valid, rounded time whose
		// value is the smallest int64 and whose timescale is the one asked
		// for, rather than an invalid time. Not documented. Evidence:
		// black-box observation on macOS 26.7.3 (tier 2).
		// Test: test_quirk_make_with_seconds_nan
		CMTime t = { .value = INT64_MIN, .timescale = preferredTimescale, .flags = kCMTimeFlags_Valid | kCMTimeFlags_HasBeenRounded, .epoch = 0 };
		return t;
	}
	if (isinf(seconds))
		return infinity(seconds < 0);

	// A value that does not fit in 64 bits is stored with a reduced timescale:
	// the timescale is halved until the value fits, and a result that cannot
	// fit even with a timescale of 1 is infinite. That the reduction goes in
	// halving steps is observed behaviour, not documented (see
	// test_make_with_seconds_reduced_timescale).
	int32_t scale = preferredTimescale;
	double x = seconds * (double) scale;
	while (fabs(x) >= 9223372036854775808.0 && scale > 1) {
		scale /= 2;
		x = seconds * (double) scale;
	}
	if (fabs(x) >= 9223372036854775808.0)
		return infinity(seconds < 0);

	// The value is truncated toward zero; whatever is cut off marks the time rounded.
	double r = trunc(x);
	CMTime t = { .value = (int64_t) r, .timescale = scale, .flags = kCMTimeFlags_Valid, .epoch = 0 };
	if (r != x)
		t.flags |= kCMTimeFlags_HasBeenRounded;
	return t;
}

Float64 CMTimeGetSeconds(CMTime time)
{
	switch (classify(time)) {
		case kClassPosInf:
			return INFINITY;
		case kClassNegInf:
			return -INFINITY;
		case kClassNumeric:
			// A zero timescale divides by zero: NaN for a zero value.
			return (double) time.value / (double) time.timescale;
		default:
			return NAN;
	}
}

// Computes value * mul / div (negated if asked), rounded as the method says,
// into *out. Returns false if the result does not fit in an int64. *inexact
// tells whether anything was rounded away.
static bool scale_value(int64_t value, bool negate, uint32_t mul, uint32_t div,
	CMTimeRoundingMethod method, bool upscaling, int64_t* out, bool* inexact)
{
	bool negative = (value < 0) != negate && value != 0;
	uint32_t rem;
	wide_t q = wide_div(wide_mul(magnitude(value), mul), div, &rem);

	*inexact = rem != 0;
	if (q.hi != 0)
		return false;

	uint64_t m = q.lo;
	if (rem != 0) {
		bool bump = false;
		switch ((uint32_t) method) {
			case kCMTimeRoundingMethod_RoundHalfAwayFromZero:
				bump = (uint64_t) rem * 2 >= div;
				break;
			case kCMTimeRoundingMethod_RoundTowardZero:
				break;
			case kCMTimeRoundingMethod_RoundAwayFromZero:
				bump = true;
				break;
			case kCMTimeRoundingMethod_RoundTowardPositiveInfinity:
				bump = !negative;
				break;
			case kCMTimeRoundingMethod_RoundTowardNegativeInfinity:
				bump = negative;
				break;
			case kCMTimeRoundingMethod_QuickTime:
				// QUIRK: the QuickTime method rounds away from zero when the
				// timescale grows and toward zero when it shrinks, except that
				// a negative value never rounds to zero. Not documented.
				// Evidence: black-box observation on macOS 26.7.3 (tier 2).
				// Test: test_quirk_convert_scale_quicktime
				bump = upscaling || (negative && m == 0);
				break;
			default:
				// QUIRK: a method that is none of the listed constants rounds
				// toward zero. Not documented. Evidence: black-box
				// observation on macOS 26.7.3 with the values 0 and 99
				// (tier 2).
				// Test: test_quirk_convert_scale_unknown_method
				break;
		}
		if (bump)
			m++;
	}

	if (negative) {
		if (m > 0x8000000000000000ull)
			return false;
		*out = (int64_t) (0 - m);
	} else {
		if (m > 0x7fffffffffffffffull)
			return false;
		*out = (int64_t) m;
	}
	return true;
}

CMTime CMTimeConvertScale(CMTime time, int32_t newTimescale, CMTimeRoundingMethod method)
{
	if (newTimescale <= 0)
		return kCMTimeInvalid;

	time_class c = classify(time);
	if (c == kClassInvalid)
		return kCMTimeInvalid;
	if (c != kClassNumeric)
		return time;
	if (time.timescale == newTimescale)
		return time;
	if (time.timescale <= 0)
		return kCMTimeInvalid;

	int64_t v;
	bool inexact;
	if (!scale_value(time.value, false, (uint32_t) newTimescale, (uint32_t) time.timescale, method,
			newTimescale > time.timescale, &v, &inexact))
		return infinity(time.value < 0);

	time.value = v;
	time.timescale = newTimescale;
	if (inexact)
		time.flags |= kCMTimeFlags_HasBeenRounded;
	return time;
}

// Sums a and b (or subtracts b from a) after bringing both to a common
// timescale; when the sum does not fit the timescale is halved until it does.
// The epoch of the result is the shared epoch's absence (0) when both epochs
// are equal, the nonzero one when only one is nonzero, and invalid otherwise.
static CMTime add_numeric(CMTime a, CMTime b, bool subtract)
{
	int64_t epoch;
	if (a.epoch == b.epoch)
		epoch = 0;
	else if (a.epoch == 0)
		epoch = b.epoch;
	else if (b.epoch == 0)
		epoch = a.epoch;
	else
		return kCMTimeInvalid;

	CMTimeFlags carried = (a.flags | b.flags) & kCMTimeFlags_HasBeenRounded;

	int64_t scale;
	if (a.timescale == b.timescale) {
		scale = a.timescale;
	} else if (a.timescale <= 0 || b.timescale <= 0) {
		return kCMTimeInvalid;
	} else {
		uint64_t g = gcd64((uint64_t) a.timescale, (uint64_t) b.timescale);
		uint64_t lcm = (uint64_t) a.timescale / g * (uint64_t) b.timescale;
		// A common timescale too large for a CMTime is replaced by a fixed
		// one, with rounding.
		scale = lcm <= (uint64_t) kCMTimeMaxTimescale ? (int64_t) lcm : 1000000000;
	}

	for (; scale >= 1; scale /= 2) {
		int64_t va, vb, sum;
		bool ia, ib;
		if (!scale_value(a.value, false, (uint32_t) scale, (uint32_t) a.timescale, kCMTimeRoundingMethod_RoundHalfAwayFromZero, false, &va, &ia))
			continue;
		if (!scale_value(b.value, subtract, (uint32_t) scale, (uint32_t) b.timescale, kCMTimeRoundingMethod_RoundHalfAwayFromZero, false, &vb, &ib))
			continue;
		if (__builtin_add_overflow(va, vb, &sum))
			continue;
		CMTime r = { .value = sum, .timescale = (int32_t) scale, .flags = kCMTimeFlags_Valid | carried, .epoch = epoch };
		if (ia || ib)
			r.flags |= kCMTimeFlags_HasBeenRounded;
		return r;
	}

	// Nothing fits: the result is infinite, with the sign of the true sum.
	bool b_negative = (b.value < 0) != subtract && b.value != 0;
	return infinity(a.value < 0 || (a.value == 0 && b_negative));
}

static CMTime add_or_subtract(CMTime a, CMTime b, bool subtract)
{
	time_class ca = classify(a);
	time_class cb = classify(b);
	if (ca == kClassInvalid || cb == kClassInvalid)
		return kCMTimeInvalid;

	if (subtract) {
		if (cb == kClassPosInf)
			cb = kClassNegInf;
		else if (cb == kClassNegInf)
			cb = kClassPosInf;
	}

	if ((ca == kClassPosInf && cb == kClassNegInf) || (ca == kClassNegInf && cb == kClassPosInf))
		return kCMTimeInvalid;
	if (ca == kClassPosInf || cb == kClassPosInf)
		return kCMTimePositiveInfinity;
	if (ca == kClassNegInf || cb == kClassNegInf)
		return kCMTimeNegativeInfinity;
	if (ca == kClassIndefinite || cb == kClassIndefinite)
		return kCMTimeIndefinite;

	return add_numeric(a, b, subtract);
}

CMTime CMTimeAdd(CMTime addend1, CMTime addend2)
{
	return add_or_subtract(addend1, addend2, false);
}

CMTime CMTimeSubtract(CMTime minuend, CMTime subtrahend)
{
	return add_or_subtract(minuend, subtrahend, true);
}

CMTime CMTimeMultiply(CMTime time, int32_t multiplier)
{
	switch (classify(time)) {
		case kClassInvalid:
			return kCMTimeInvalid;
		case kClassPosInf:
			return infinity(multiplier < 0);
		case kClassNegInf:
			return infinity(multiplier >= 0);
		case kClassIndefinite:
			return kCMTimeIndefinite;
		case kClassNumeric:
			break;
	}

	if (time.timescale <= 0)
		return kCMTimeInvalid;

	// When the product does not fit, the value is first brought to a smaller
	// timescale (halved until it fits) and then multiplied.
	for (int32_t scale = time.timescale; scale >= 1; scale /= 2) {
		int64_t v, product;
		bool inexact = false;
		if (scale == time.timescale) {
			v = time.value;
		} else if (!scale_value(time.value, false, (uint32_t) scale, (uint32_t) time.timescale,
				kCMTimeRoundingMethod_RoundHalfAwayFromZero, false, &v, &inexact)) {
			continue;
		}
		if (__builtin_mul_overflow(v, (int64_t) multiplier, &product))
			continue;
		time.value = product;
		time.timescale = scale;
		if (inexact)
			time.flags |= kCMTimeFlags_HasBeenRounded;
		return time;
	}
	return infinity((time.value < 0) != (multiplier < 0));
}

// Orders times: negative infinity, numeric times (by epoch, then by value),
// indefinite, positive infinity, invalid.
int32_t CMTimeCompare(CMTime time1, CMTime time2)
{
	time_class c1 = classify(time1);
	time_class c2 = classify(time2);
	if (c1 != c2)
		return c1 < c2 ? -1 : 1;
	if (c1 != kClassNumeric)
		return 0;

	if (time1.epoch != time2.epoch)
		return time1.epoch < time2.epoch ? -1 : 1;
	if (time1.timescale == time2.timescale) {
		if (time1.value == time2.value)
			return 0;
		return time1.value < time2.value ? -1 : 1;
	}

	// Compare value1 / timescale1 with value2 / timescale2 by cross multiplying.
	if (time1.timescale <= 0 || time2.timescale <= 0)
		return time1.timescale < time2.timescale ? -1 : 1;
	bool n1 = time1.value < 0;
	bool n2 = time2.value < 0;
	if (n1 != n2)
		return n1 ? -1 : 1;
	wide_t l = wide_mul(magnitude(time1.value), (uint32_t) time2.timescale);
	wide_t r = wide_mul(magnitude(time2.value), (uint32_t) time1.timescale);
	int c = wide_cmp(l, r);
	return n1 ? -c : c;
}

CMTime CMTimeMinimum(CMTime time1, CMTime time2)
{
	return CMTimeCompare(time1, time2) < 0 ? time1 : time2;
}

CMTime CMTimeMaximum(CMTime time1, CMTime time2)
{
	return CMTimeCompare(time1, time2) < 0 ? time2 : time1;
}

CMTime CMTimeAbsoluteValue(CMTime time)
{
	switch (classify(time)) {
		case kClassInvalid:
			return kCMTimeInvalid;
		case kClassNegInf:
		case kClassPosInf:
			return kCMTimePositiveInfinity;
		case kClassIndefinite:
			return kCMTimeIndefinite;
		case kClassNumeric:
			break;
	}
	if (time.value < 0) {
		if (time.value == INT64_MIN)
			return kCMTimePositiveInfinity;
		time.value = -time.value;
	}
	return time;
}
