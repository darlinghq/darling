/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// CoreMedia time and time range: layout, construction, arithmetic, comparison, scale conversion
// and the range operations. The expected values follow the documented definitions; the cases
// the documentation does not cover (marked QUIRK in CMTime.c / CMTimeRange.c) were observed on
// macOS 26.7.3 and are in test_quirk_* below. The file also builds against the real SDK
// (clang -framework CoreMedia -framework CoreFoundation). Prints PASS when everything holds;
// the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static int is_time(CMTime t, int64_t value, int32_t scale, uint32_t flags, int64_t epoch)
{
	return t.value == value && t.timescale == scale && t.flags == flags && t.epoch == epoch;
}

#define V kCMTimeFlags_Valid
#define R kCMTimeFlags_HasBeenRounded
#define PINF (kCMTimeFlags_Valid | kCMTimeFlags_PositiveInfinity)
#define NINF (kCMTimeFlags_Valid | kCMTimeFlags_NegativeInfinity)
#define INDEF (kCMTimeFlags_Valid | kCMTimeFlags_Indefinite)

static int is_invalid(CMTime t) { return is_time(t, 0, 0, 0, 0); }
static int is_pinf(CMTime t) { return is_time(t, 0, 0, PINF, 0); }
static int is_ninf(CMTime t) { return is_time(t, 0, 0, NINF, 0); }
static int is_indef(CMTime t) { return is_time(t, 0, 0, INDEF, 0); }

static int is_range(CMTimeRange r, int64_t sv, int32_t ss, int64_t dv, int32_t ds)
{
	return is_time(r.start, sv, ss, V, 0) && is_time(r.duration, dv, ds, V, 0);
}

static void test_layout(void)
{
	CHECK(sizeof(CMTime) == 24 && _Alignof(CMTime) == 4);
	CHECK(offsetof(CMTime, timescale) == 8 && offsetof(CMTime, flags) == 12 && offsetof(CMTime, epoch) == 16);
	CHECK(sizeof(CMTimeRange) == 48 && _Alignof(CMTimeRange) == 4);
	CHECK(sizeof(CMSampleTimingInfo) == 72 && offsetof(CMSampleTimingInfo, decodeTimeStamp) == 48);
	CHECK(sizeof(CMTimeValue) == 8 && sizeof(CMTimeScale) == 4 && sizeof(CMTimeEpoch) == 8);
	CHECK(kCMTimeFlags_Valid == 1 && kCMTimeFlags_HasBeenRounded == 2 && kCMTimeFlags_PositiveInfinity == 4);
	CHECK(kCMTimeFlags_NegativeInfinity == 8 && kCMTimeFlags_Indefinite == 16 && kCMTimeFlags_ImpliedValueFlagsMask == 28);
	CHECK(kCMTimeRoundingMethod_Default == 1 && kCMTimeRoundingMethod_RoundHalfAwayFromZero == 1);
	CHECK(kCMTimeRoundingMethod_RoundTowardZero == 2 && kCMTimeRoundingMethod_RoundAwayFromZero == 3);
	CHECK(kCMTimeRoundingMethod_QuickTime == 4 && kCMTimeRoundingMethod_RoundTowardPositiveInfinity == 5);
	CHECK(kCMTimeRoundingMethod_RoundTowardNegativeInfinity == 6);
	CHECK(kCMTimeMaxTimescale == 2147483647);
}

static void test_constants(void)
{
	CHECK(is_invalid(kCMTimeInvalid));
	CHECK(is_indef(kCMTimeIndefinite));
	CHECK(is_pinf(kCMTimePositiveInfinity));
	CHECK(is_ninf(kCMTimeNegativeInfinity));
	CHECK(is_time(kCMTimeZero, 0, 1, V, 0));
	CHECK(is_range(kCMTimeRangeZero, 0, 1, 0, 1));
	CHECK(is_invalid(kCMTimeRangeInvalid.start) && is_invalid(kCMTimeRangeInvalid.duration));
	CHECK(is_invalid(kCMTimingInfoInvalid.duration) && is_invalid(kCMTimingInfoInvalid.presentationTimeStamp)
		&& is_invalid(kCMTimingInfoInvalid.decodeTimeStamp));
}

static void test_make(void)
{
	CHECK(is_time(CMTimeMake(-5, 600), -5, 600, V, 0));
	CHECK(is_time(CMTimeMake(5, INT_MAX), 5, INT_MAX, V, 0));
	CHECK(is_invalid(CMTimeMake(5, 0)));
	CHECK(is_invalid(CMTimeMake(5, -1)));
	CHECK(is_time(CMTimeMakeWithEpoch(7, 30, 4), 7, 30, V, 4));
	CHECK(is_invalid(CMTimeMakeWithEpoch(7, 0, 4)));

	CHECK(is_time(CMTimeMakeWithSeconds(1.5, 600), 900, 600, V, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(0, 5), 0, 5, V, 0));
	CHECK(is_invalid(CMTimeMakeWithSeconds(1.5, 0)));
	CHECK(is_invalid(CMTimeMakeWithSeconds(1.5, -3)));
	CHECK(is_pinf(CMTimeMakeWithSeconds(INFINITY, 600)));
	CHECK(is_ninf(CMTimeMakeWithSeconds(-INFINITY, 600)));
	// Values that need rounding set the rounded flag; the fraction is cut off, toward zero.
	CHECK(is_time(CMTimeMakeWithSeconds(2.5 / 600, 600), 2, 600, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(3.5, 1), 3, 1, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(-0.5, 1), 0, 1, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(-2.5 / 600, 600), -2, 600, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(0.0001, 600), 0, 600, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(1.23456789, 100000), 123456, 100000, V | R, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(1e10, 600), 6000000000000LL, 600, V, 0));
	// Overflow beyond any timescale is infinity.
	CHECK(is_pinf(CMTimeMakeWithSeconds(1e300, 600)));
	CHECK(is_ninf(CMTimeMakeWithSeconds(-1e300, 600)));
	CHECK(is_pinf(CMTimeMakeWithSeconds(9.3e18, 1)));
}

// A value that does not fit with the asked timescale is stored with a reduced one (observed to
// be reached by halving).
static void test_make_with_seconds_reduced_timescale(void)
{
	CHECK(is_time(CMTimeMakeWithSeconds(1e10, 1000000000), 5000000000000000000LL, 500000000, V, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(1e16, 4096), 5120000000000000000LL, 512, V, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(1e16, 1000), 5000000000000000000LL, 500, V, 0));
	CHECK(is_time(CMTimeMakeWithSeconds(5e18, 3), 5000000000000000000LL, 1, V, 0));
	CHECK(is_pinf(CMTimeMakeWithSeconds(1e19, 3)));
}

static void test_quirk_make_with_seconds_nan(void)
{
	CHECK(is_time(CMTimeMakeWithSeconds(NAN, 600), INT64_MIN, 600, V | R, 0));
}

static void test_seconds(void)
{
	CHECK(fabs(CMTimeGetSeconds(CMTimeMake(1, 3)) - 1.0 / 3.0) < 1e-15);
	CHECK(CMTimeGetSeconds(CMTimeMake(-3, 2)) == -1.5);
	CHECK(isnan(CMTimeGetSeconds(kCMTimeInvalid)));
	CHECK(isnan(CMTimeGetSeconds(kCMTimeIndefinite)));
	CHECK(isinf(CMTimeGetSeconds(kCMTimePositiveInfinity)) && CMTimeGetSeconds(kCMTimePositiveInfinity) > 0);
	CHECK(isinf(CMTimeGetSeconds(kCMTimeNegativeInfinity)) && CMTimeGetSeconds(kCMTimeNegativeInfinity) < 0);
	CMTime zero_over_zero = { 0, 0, kCMTimeFlags_Valid, 0 };
	CHECK(isnan(CMTimeGetSeconds(zero_over_zero)));
	CMTime with_epoch = CMTimeMakeWithEpoch(3, 1, 5);
	CHECK(CMTimeGetSeconds(with_epoch) == 3);
}

static void test_add_subtract(void)
{
	CMTime a = CMTimeMake(1, 3), b = CMTimeMake(1, 4);
	CHECK(is_time(CMTimeAdd(a, b), 7, 12, V, 0));
	CHECK(is_time(CMTimeSubtract(a, b), 1, 12, V, 0));
	CHECK(is_time(CMTimeAdd(a, a), 2, 3, V, 0));
	CHECK(is_time(CMTimeSubtract(a, a), 0, 3, V, 0));
	CHECK(is_time(CMTimeAdd(kCMTimeZero, kCMTimeZero), 0, 1, V, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 1000), CMTimeMake(1, 600)), 8, 3000, V, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 65536), CMTimeMake(1, 32768)), 3, 65536, V, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, INT_MAX), CMTimeMake(1, 1)), 2147483648LL, INT_MAX, V, 0));
	// The rounded flag of an operand is kept.
	CMTime rounded = { 1, 3, V | R, 0 };
	CHECK(is_time(CMTimeAdd(rounded, CMTimeMake(1, 3)), 2, 3, V | R, 0));

	// Special values.
	CHECK(is_invalid(CMTimeAdd(a, kCMTimeInvalid)));
	CHECK(is_invalid(CMTimeAdd(kCMTimeInvalid, a)));
	CHECK(is_pinf(CMTimeAdd(a, kCMTimePositiveInfinity)));
	CHECK(is_ninf(CMTimeAdd(a, kCMTimeNegativeInfinity)));
	CHECK(is_invalid(CMTimeAdd(kCMTimePositiveInfinity, kCMTimeNegativeInfinity)));
	CHECK(is_pinf(CMTimeAdd(kCMTimePositiveInfinity, kCMTimePositiveInfinity)));
	CHECK(is_indef(CMTimeAdd(a, kCMTimeIndefinite)));
	CHECK(is_pinf(CMTimeAdd(kCMTimeIndefinite, kCMTimePositiveInfinity)));
	CHECK(is_ninf(CMTimeAdd(kCMTimeIndefinite, kCMTimeNegativeInfinity)));
	CHECK(is_invalid(CMTimeSubtract(kCMTimePositiveInfinity, kCMTimePositiveInfinity)));
	CHECK(is_pinf(CMTimeSubtract(kCMTimePositiveInfinity, kCMTimeNegativeInfinity)));
	CHECK(is_ninf(CMTimeSubtract(a, kCMTimePositiveInfinity)));
	CHECK(is_pinf(CMTimeSubtract(a, kCMTimeNegativeInfinity)));
	CHECK(is_indef(CMTimeSubtract(a, kCMTimeIndefinite)));
	CHECK(is_pinf(CMTimeSubtract(kCMTimeIndefinite, kCMTimeNegativeInfinity)));

	// Epochs: equal epochs cancel, one zero epoch is taken over, two different non-zero ones are invalid.
	CMTime e1 = CMTimeMakeWithEpoch(1, 3, 1), e2 = CMTimeMakeWithEpoch(1, 3, 2);
	CHECK(is_time(CMTimeAdd(e1, b), 7, 12, V, 1));
	CHECK(is_time(CMTimeAdd(b, e1), 7, 12, V, 1));
	CHECK(is_time(CMTimeAdd(e1, e1), 2, 3, V, 0));
	CHECK(is_time(CMTimeSubtract(e1, e1), 0, 3, V, 0));
	CHECK(is_invalid(CMTimeAdd(e2, e1)));
	CHECK(is_invalid(CMTimeSubtract(e2, e1)));
}

// Sums that do not fit, or whose common timescale is too large, are rounded onto a smaller one.
static void test_add_overflow_and_rounding(void)
{
	CHECK(is_pinf(CMTimeAdd(CMTimeMake(LLONG_MAX, 1), CMTimeMake(LLONG_MAX, 1))));
	CHECK(is_ninf(CMTimeSubtract(CMTimeMake(LLONG_MIN + 1, 1), CMTimeMake(LLONG_MAX, 1))));
	CHECK(is_pinf(CMTimeAdd(CMTimeMake(LLONG_MAX, 2), CMTimeMake(LLONG_MAX, 2))));
	CHECK(is_time(CMTimeAdd(CMTimeMake(LLONG_MAX, 1000), CMTimeMake(5, 1000)), 4611686018427387907LL, 500, V | R, 0));
	CHECK(is_time(CMTimeSubtract(CMTimeMake(LLONG_MAX, 1000), CMTimeMake(-5, 1000)), 4611686018427387907LL, 500, V | R, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(LLONG_MAX, 7), CMTimeMake(5, 7)), 3952873730080618205LL, 3, V | R, 0));
	// Timescales whose least common multiple exceeds the maximum timescale use 1e9.
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 1000000007), CMTimeMake(1, 1000000009)), 2, 1000000000, V | R, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 3), CMTimeMake(1, 2000000000)), 333333334, 1000000000, V | R, 0));
	CHECK(is_time(CMTimeSubtract(CMTimeMake(1, 3000000), CMTimeMake(1, 1000003)), -667, 1000000000, V | R, 0));
	// A least common multiple of exactly the maximum timescale or below is exact.
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 46341), CMTimeMake(1, 46340)), 92681, 2147441940, V, 0));
	CHECK(is_time(CMTimeAdd(CMTimeMake(1, 46341), CMTimeMake(1, 46349)), 43154, 1000000000, V | R, 0));
}

static void test_multiply(void)
{
	CMTime a = CMTimeMake(1, 3);
	CHECK(is_time(CMTimeMultiply(a, 3), 3, 3, V, 0));
	CHECK(is_time(CMTimeMultiply(a, -2), -2, 3, V, 0));
	CHECK(is_time(CMTimeMultiply(a, 0), 0, 3, V, 0));
	CHECK(is_time(CMTimeMultiply(CMTimeMakeWithEpoch(1, 3, 5), 2), 2, 3, V, 5));
	CMTime rounded = { 1, 3, V | R, 0 };
	CHECK(is_time(CMTimeMultiply(rounded, 2), 2, 3, V | R, 0));
	CHECK(is_invalid(CMTimeMultiply(kCMTimeInvalid, 2)));
	CHECK(is_pinf(CMTimeMultiply(kCMTimePositiveInfinity, 2)));
	CHECK(is_ninf(CMTimeMultiply(kCMTimePositiveInfinity, -2)));
	CHECK(is_ninf(CMTimeMultiply(kCMTimeNegativeInfinity, 2)));
	CHECK(is_pinf(CMTimeMultiply(kCMTimeNegativeInfinity, -2)));
	CHECK(is_indef(CMTimeMultiply(kCMTimeIndefinite, 2)));
	CHECK(is_pinf(CMTimeMultiply(CMTimeMake(LLONG_MAX / 2 + 1, 1), 2)));
	CHECK(is_ninf(CMTimeMultiply(CMTimeMake(LLONG_MAX / 2 + 1, 1), -4)));
	CHECK(is_time(CMTimeMultiply(CMTimeMake(LLONG_MAX / 2 + 1, 5), 2), 3689348814741910324LL, 2, V | R, 0));
	// The infinities by zero keep their sign (observed).
	CHECK(is_pinf(CMTimeMultiply(kCMTimePositiveInfinity, 0)));
	CHECK(is_ninf(CMTimeMultiply(kCMTimeNegativeInfinity, 0)));
}

static void test_compare(void)
{
	CMTime a = CMTimeMake(1, 3), b = CMTimeMake(1, 4);
	CHECK(CMTimeCompare(a, b) == 1 && CMTimeCompare(b, a) == -1);
	CHECK(CMTimeCompare(a, CMTimeMake(2, 6)) == 0);
	CHECK(CMTimeCompare(CMTimeMake(-1, 3), CMTimeMake(-1, 4)) == -1);
	CHECK(CMTimeCompare(CMTimeMake(-1, 3), CMTimeMake(1, 4)) == -1);
	CHECK(CMTimeCompare(CMTimeMake(1, 1000000007), CMTimeMake(1, 1000000009)) == 1);
	CHECK(CMTimeCompare(CMTimeMake(LLONG_MAX, 1000000007), CMTimeMake(LLONG_MAX, 1000000009)) == 1);
	CHECK(CMTimeCompare(CMTimeMake(LLONG_MIN, 1000000007), CMTimeMake(LLONG_MIN, 1000000009)) == -1);
	// Order: negative infinity < numeric < indefinite < positive infinity < invalid.
	CHECK(CMTimeCompare(kCMTimeNegativeInfinity, a) == -1 && CMTimeCompare(a, kCMTimeNegativeInfinity) == 1);
	CHECK(CMTimeCompare(a, kCMTimeIndefinite) == -1 && CMTimeCompare(kCMTimeIndefinite, a) == 1);
	CHECK(CMTimeCompare(kCMTimeIndefinite, kCMTimePositiveInfinity) == -1);
	CHECK(CMTimeCompare(kCMTimePositiveInfinity, a) == 1);
	CHECK(CMTimeCompare(kCMTimeInvalid, a) == 1 && CMTimeCompare(a, kCMTimeInvalid) == -1);
	CHECK(CMTimeCompare(kCMTimeInvalid, kCMTimeInvalid) == 0);
	CHECK(CMTimeCompare(kCMTimePositiveInfinity, kCMTimePositiveInfinity) == 0);
	CHECK(CMTimeCompare(kCMTimeNegativeInfinity, kCMTimePositiveInfinity) == -1);
	CHECK(CMTimeCompare(kCMTimeIndefinite, kCMTimeIndefinite) == 0);
	// A greater epoch is greater.
	CMTime e2 = CMTimeMakeWithEpoch(1, 3, 2);
	CHECK(CMTimeCompare(e2, a) == 1 && CMTimeCompare(a, e2) == -1);

	CHECK(is_time(CMTimeMaximum(a, b), 1, 3, V, 0) && is_time(CMTimeMinimum(a, b), 1, 4, V, 0));
	CHECK(is_invalid(CMTimeMaximum(a, kCMTimeInvalid)) && is_invalid(CMTimeMaximum(kCMTimeInvalid, a)));
	CHECK(is_time(CMTimeMinimum(a, kCMTimeInvalid), 1, 3, V, 0) && is_time(CMTimeMinimum(kCMTimeInvalid, a), 1, 3, V, 0));
	// On equal times the maximum is the first argument and the minimum the second (observed).
	CHECK(is_time(CMTimeMaximum(CMTimeMake(1, 3), CMTimeMake(2, 6)), 1, 3, V, 0));
	CHECK(is_time(CMTimeMinimum(CMTimeMake(1, 3), CMTimeMake(2, 6)), 2, 6, V, 0));
}

static void test_absolute_value(void)
{
	CHECK(is_time(CMTimeAbsoluteValue(CMTimeMake(-1, 3)), 1, 3, V, 0));
	CHECK(is_time(CMTimeAbsoluteValue(CMTimeMake(1, 3)), 1, 3, V, 0));
	CHECK(is_pinf(CMTimeAbsoluteValue(kCMTimeNegativeInfinity)));
	CHECK(is_pinf(CMTimeAbsoluteValue(kCMTimePositiveInfinity)));
	CHECK(is_indef(CMTimeAbsoluteValue(kCMTimeIndefinite)));
	CHECK(is_invalid(CMTimeAbsoluteValue(kCMTimeInvalid)));
	CMTime rounded = { -5, 3, V | R, 5 };
	CHECK(is_time(CMTimeAbsoluteValue(rounded), 5, 3, V | R, 5));
}

static void test_convert_scale(void)
{
	CMTime a = CMTimeMake(1, 3);
	CHECK(is_time(CMTimeConvertScale(a, 6, kCMTimeRoundingMethod_Default), 2, 6, V, 0));
	CHECK(is_time(CMTimeConvertScale(a, 3, kCMTimeRoundingMethod_Default), 1, 3, V, 0));
	CHECK(is_invalid(CMTimeConvertScale(a, 0, kCMTimeRoundingMethod_Default)));
	CHECK(is_invalid(CMTimeConvertScale(a, -5, kCMTimeRoundingMethod_Default)));
	CHECK(is_invalid(CMTimeConvertScale(kCMTimeInvalid, 5, kCMTimeRoundingMethod_Default)));
	CHECK(is_pinf(CMTimeConvertScale(kCMTimePositiveInfinity, 5, kCMTimeRoundingMethod_Default)));
	CHECK(is_indef(CMTimeConvertScale(kCMTimeIndefinite, 5, kCMTimeRoundingMethod_Default)));
	CHECK(is_pinf(CMTimeConvertScale(CMTimeMake(LLONG_MAX / 2, 1), 10, kCMTimeRoundingMethod_Default)));
	CHECK(is_ninf(CMTimeConvertScale(CMTimeMake(LLONG_MIN / 2, 1), 10, kCMTimeRoundingMethod_Default)));
	// Exact conversions do not set the rounded flag, inexact ones do; an existing flag stays.
	CHECK(is_time(CMTimeConvertScale(CMTimeMake(7, 10), 10, kCMTimeRoundingMethod_Default), 7, 10, V, 0));
	CHECK(is_time(CMTimeConvertScale(CMTimeMake(7, 10), 5, kCMTimeRoundingMethod_Default), 4, 5, V | R, 0));
	CMTime rounded = { 7, 3, V | R, 0 };
	CHECK(is_time(CMTimeConvertScale(rounded, 6, kCMTimeRoundingMethod_Default), 14, 6, V | R, 0));
	CHECK(is_time(CMTimeConvertScale(CMTimeMakeWithEpoch(1, 3, 5), 2, kCMTimeRoundingMethod_Default), 1, 2, V | R, 5));

	// Each rounding method on v / 10 -> timescale 1 for v = 5, -5, 6, -6, 4, -4, 0.
	static const int64_t in[] = { 5, -5, 6, -6, 4, -4, 0 };
	static const int64_t half_away[] = { 1, -1, 1, -1, 0, 0, 0 };
	static const int64_t toward_zero[] = { 0, 0, 0, 0, 0, 0, 0 };
	static const int64_t away[] = { 1, -1, 1, -1, 1, -1, 0 };
	static const int64_t to_pos[] = { 1, 0, 1, 0, 1, 0, 0 };
	static const int64_t to_neg[] = { 0, -1, 0, -1, 0, -1, 0 };
	for (int i = 0; i < 7; i++) {
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_RoundHalfAwayFromZero).value == half_away[i]);
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_RoundTowardZero).value == toward_zero[i]);
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_RoundAwayFromZero).value == away[i]);
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_RoundTowardPositiveInfinity).value == to_pos[i]);
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_RoundTowardNegativeInfinity).value == to_neg[i]);
	}
	// 3/10 and -3/10 at timescale 4 are +-1.2.
	CHECK(CMTimeConvertScale(CMTimeMake(3, 10), 4, kCMTimeRoundingMethod_RoundAwayFromZero).value == 2);
	CHECK(CMTimeConvertScale(CMTimeMake(-3, 10), 4, kCMTimeRoundingMethod_RoundAwayFromZero).value == -2);
	CHECK(CMTimeConvertScale(CMTimeMake(3, 10), 4, kCMTimeRoundingMethod_RoundTowardPositiveInfinity).value == 2);
	CHECK(CMTimeConvertScale(CMTimeMake(-3, 10), 4, kCMTimeRoundingMethod_RoundTowardPositiveInfinity).value == -1);
	CHECK(CMTimeConvertScale(CMTimeMake(-3, 10), 4, kCMTimeRoundingMethod_RoundTowardNegativeInfinity).value == -2);
}

static void test_quirk_convert_scale_quicktime(void)
{
	// Downscaling: toward zero, but a negative value never becomes zero.
	static const int64_t in[] = { 8, 5, 1, 0, -1, -4, -5, -8 };
	static const int64_t down1[] = { 0, 0, 0, 0, -1, -1, -1, -1 };
	for (int i = 0; i < 8; i++)
		CHECK(CMTimeConvertScale(CMTimeMake(in[i], 10), 1, kCMTimeRoundingMethod_QuickTime).value == down1[i]);
	// 10 -> 4: values -8..8 give -3 -2 -2 -2 -1 -1 -1 -1 0 0 0 1 1 2 2 2 3 on the observed Mac (written as the 17 expected values).
	static const int64_t down4[] = { -3, -2, -2, -2, -1, -1, -1, -1, 0, 0, 0, 1, 1, 2, 2, 2, 3 };
	for (int v = -8; v <= 8; v++)
		CHECK(CMTimeConvertScale(CMTimeMake(v, 10), 4, kCMTimeRoundingMethod_QuickTime).value == down4[v + 8]);
	// Upscaling (3 -> 10): away from zero.
	static const int64_t up10[] = { -27, -24, -20, -17, -14, -10, -7, -4, 0, 4, 7, 10, 14, 17, 20, 24, 27 };
	for (int v = -8; v <= 8; v++)
		CHECK(CMTimeConvertScale(CMTimeMake(v, 3), 10, kCMTimeRoundingMethod_QuickTime).value == up10[v + 8]);
	CHECK(CMTimeConvertScale(CMTimeMake(1, 10), 1, kCMTimeRoundingMethod_QuickTime).flags == (V | R));
	CHECK(CMTimeConvertScale(CMTimeMake(0, 10), 1, kCMTimeRoundingMethod_QuickTime).flags == V);
}

static void test_quirk_convert_scale_unknown_method(void)
{
	// A method outside the constants truncates toward zero.
	static const int64_t expect10to5[] = { -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4 };
	for (int v = -8; v <= 8; v++) {
		CHECK(CMTimeConvertScale(CMTimeMake(v, 10), 5, (CMTimeRoundingMethod) 0).value == expect10to5[v + 8]);
		CHECK(CMTimeConvertScale(CMTimeMake(v, 10), 5, (CMTimeRoundingMethod) 99).value == expect10to5[v + 8]);
	}
	CHECK(CMTimeConvertScale(CMTimeMake(-8, 3), 10, (CMTimeRoundingMethod) 0).value == -26);
}

static void test_range_basics(void)
{
	CMTimeRange r1 = CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(3, 1));
	CHECK(is_range(r1, 2, 1, 3, 1));
	CHECK(is_time(CMTimeRangeGetEnd(r1), 5, 1, V, 0));
	CHECK(is_time(CMTimeRangeGetEnd(CMTimeRangeMake(CMTimeMake(1, 3), CMTimeMake(1, 4))), 7, 12, V, 0));
	CHECK(is_invalid(CMTimeRangeGetEnd(kCMTimeRangeInvalid)));
	CHECK(is_indef(CMTimeRangeGetEnd(CMTimeRangeMake(kCMTimeIndefinite, CMTimeMake(1, 1)))));
	CHECK(is_pinf(CMTimeRangeGetEnd(CMTimeRangeMake(CMTimeMake(1, 1), kCMTimePositiveInfinity))));

	CHECK(is_range(CMTimeRangeFromTimeToTime(CMTimeMake(2, 1), CMTimeMake(5, 1)), 2, 1, 3, 1));
	CHECK(is_range(CMTimeRangeFromTimeToTime(CMTimeMake(1, 3), CMTimeMake(1, 2)), 1, 3, 1, 6));
	CMTimeRange backwards = CMTimeRangeFromTimeToTime(CMTimeMake(5, 1), CMTimeMake(2, 1));
	CHECK(is_invalid(backwards.start) && is_invalid(backwards.duration));
	CMTimeRange bad = CMTimeRangeFromTimeToTime(kCMTimeInvalid, CMTimeMake(1, 2));
	CHECK(is_invalid(bad.start) && is_invalid(bad.duration));

	CHECK(CMTimeRangeEqual(r1, r1));
	CHECK(!CMTimeRangeEqual(r1, CMTimeRangeMake(CMTimeMake(4, 1), CMTimeMake(3, 1))));
	CHECK(CMTimeRangeEqual(CMTimeRangeMake(CMTimeMake(4, 2), CMTimeMake(6, 2)), r1));
	CHECK(CMTimeRangeEqual(kCMTimeRangeInvalid, kCMTimeRangeInvalid));
	CHECK(!CMTimeRangeEqual(kCMTimeRangeZero, kCMTimeRangeInvalid));
}

static void test_range_contains(void)
{
	CMTimeRange r1 = CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(3, 1));
	CHECK(CMTimeRangeContainsTime(r1, CMTimeMake(2, 1)));
	CHECK(!CMTimeRangeContainsTime(r1, CMTimeMake(5, 1)));
	CHECK(CMTimeRangeContainsTime(r1, CMTimeMake(49, 10)));
	CHECK(!CMTimeRangeContainsTime(r1, CMTimeMake(19, 10)));
	CHECK(!CMTimeRangeContainsTime(r1, kCMTimeInvalid));
	CHECK(!CMTimeRangeContainsTime(kCMTimeRangeZero, kCMTimeZero));
	CMTimeRange half = CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 2));
	CHECK(CMTimeRangeContainsTime(half, CMTimeMake(1, 3)));
	CHECK(!CMTimeRangeContainsTime(half, CMTimeMake(2, 4)));
	CHECK(CMTimeRangeContainsTime(half, CMTimeMake(1, 4)));
	CHECK(CMTimeRangeContainsTime(CMTimeRangeMake(kCMTimeZero, kCMTimePositiveInfinity), CMTimeMake(5, 1)));
	CHECK(CMTimeRangeContainsTime(CMTimeRangeMake(kCMTimeZero, kCMTimeIndefinite), CMTimeMake(5, 1)));

	CMTimeRange r2 = CMTimeRangeMake(CMTimeMake(4, 1), CMTimeMake(3, 1));
	CHECK(CMTimeRangeContainsTimeRange(r1, CMTimeRangeMake(CMTimeMake(3, 1), CMTimeMake(1, 1))));
	CHECK(CMTimeRangeContainsTimeRange(r1, r1));
	CHECK(!CMTimeRangeContainsTimeRange(r1, r2));
	CHECK(!CMTimeRangeContainsTimeRange(r1, kCMTimeRangeZero));
	CHECK(!CMTimeRangeContainsTimeRange(kCMTimeRangeInvalid, kCMTimeRangeZero));
}

static void test_quirk_contains_time_range_empty(void)
{
	// The zero range does not contain itself.
	CHECK(!CMTimeRangeContainsTimeRange(kCMTimeRangeZero, kCMTimeRangeZero));
}

static void test_range_union_intersection(void)
{
	CMTimeRange r1 = CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(3, 1));
	CMTimeRange r2 = CMTimeRangeMake(CMTimeMake(4, 1), CMTimeMake(3, 1));
	CMTimeRange r3 = CMTimeRangeMake(CMTimeMake(10, 1), CMTimeMake(1, 1));
	CHECK(is_range(CMTimeRangeGetUnion(r1, r2), 2, 1, 5, 1));
	CHECK(is_range(CMTimeRangeGetUnion(r1, r3), 2, 1, 9, 1));
	CHECK(is_range(CMTimeRangeGetUnion(r1, kCMTimeRangeZero), 0, 1, 5, 1));
	CHECK(is_range(CMTimeRangeGetUnion(r1, CMTimeRangeMake(CMTimeMake(1, 1), CMTimeMake(10, 1))), 1, 1, 10, 1));
	CHECK(is_range(CMTimeRangeGetUnion(CMTimeRangeMake(CMTimeMake(1, 3), CMTimeMake(1, 1)), CMTimeRangeMake(CMTimeMake(1, 2), CMTimeMake(1, 1))), 1, 3, 7, 6));
	CMTimeRange with_invalid = CMTimeRangeGetUnion(r1, kCMTimeRangeInvalid);
	CHECK(is_time(with_invalid.start, 2, 1, V, 0) && is_invalid(with_invalid.duration));
	CMTimeRange infinite = CMTimeRangeGetUnion(CMTimeRangeMake(kCMTimeZero, kCMTimePositiveInfinity), r1);
	CHECK(is_time(infinite.start, 0, 1, V, 0) && is_pinf(infinite.duration));

	CHECK(is_range(CMTimeRangeGetIntersection(r1, r2), 4, 1, 1, 1));
	CHECK(is_range(CMTimeRangeGetIntersection(r1, CMTimeRangeMake(CMTimeMake(1, 1), CMTimeMake(10, 1))), 2, 1, 3, 1));
	CHECK(is_range(CMTimeRangeGetIntersection(CMTimeRangeMake(CMTimeMake(1, 3), CMTimeMake(1, 1)), CMTimeRangeMake(CMTimeMake(1, 2), CMTimeMake(1, 1))), 1, 2, 5, 6));
	CHECK(is_range(CMTimeRangeGetIntersection(CMTimeRangeMake(kCMTimeZero, kCMTimePositiveInfinity), CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(1, 1))), 2, 1, 1, 1));
	// Disjoint ranges, ranges that only touch, and an invalid range give the zero range.
	CHECK(is_range(CMTimeRangeGetIntersection(r1, r3), 0, 1, 0, 1));
	CHECK(is_range(CMTimeRangeGetIntersection(r1, CMTimeRangeMake(CMTimeMake(5, 1), CMTimeMake(1, 1))), 0, 1, 0, 1));
	CHECK(is_range(CMTimeRangeGetIntersection(r1, kCMTimeRangeInvalid), 0, 1, 0, 1));
}

static void test_clamp(void)
{
	CMTimeRange r1 = CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(3, 1));
	CHECK(is_time(CMTimeClampToRange(CMTimeMake(9, 1), r1), 5, 1, V, 0));
	CHECK(is_time(CMTimeClampToRange(CMTimeMake(1, 1), r1), 2, 1, V, 0));
	CHECK(is_time(CMTimeClampToRange(CMTimeMake(3, 1), r1), 3, 1, V, 0));
	// The end of the range is returned for a time past it, and a time at it is kept.
	CHECK(is_time(CMTimeClampToRange(CMTimeMake(5, 1), r1), 5, 1, V, 0));
	CHECK(is_time(CMTimeClampToRange(CMTimeMake(20, 3), CMTimeRangeMake(CMTimeMake(1, 2), CMTimeMake(1, 3))), 5, 6, V, 0));
	CHECK(is_invalid(CMTimeClampToRange(CMTimeMake(1, 1), kCMTimeRangeInvalid)));
	CHECK(is_invalid(CMTimeClampToRange(kCMTimeInvalid, r1)));
}

int main(void)
{
	test_layout();
	test_constants();
	test_make();
	test_make_with_seconds_reduced_timescale();
	test_quirk_make_with_seconds_nan();
	test_seconds();
	test_add_subtract();
	test_add_overflow_and_rounding();
	test_multiply();
	test_compare();
	test_absolute_value();
	test_convert_scale();
	test_quirk_convert_scale_quicktime();
	test_quirk_convert_scale_unknown_method();
	test_range_basics();
	test_range_contains();
	test_quirk_contains_time_range_empty();
	test_range_union_intersection();
	test_clamp();
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
