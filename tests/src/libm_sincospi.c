// Copyright (C) 2026 Lubos Dolezel
//
// __sincospi_stret and __sincospif_stret: exact results at integers and
// half-integers (C23 F.10.1.13 sinpi, F.10.1.14 cospi), NaN for NaN, NaN
// and FE_INVALID for infinities, and accuracy where v * pi would lose it: next to the zeros
// of sinpi/cospi and for large non-integer arguments.
// Exits by itself (alarm).

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int failures;

static void check(int ok, const char* what, double v, double got, double want)
{
	printf("%s: %s(%.17g) = %.17g, expected %.17g\n", ok ? "PASS" : "FAIL", what, v, got, want);
	if (!ok)
		failures++;
}

// Same value, and the same sign for a zero
static int same(double got, double want)
{
	return got == want && signbit(got) == signbit(want);
}

static int close_ulps(double got, double want, double ulps)
{
	return fabs(got - want) <= ulps * fabs(want) * DBL_EPSILON;
}

static int close_ulpsf(float got, float want, float ulps)
{
	return fabsf(got - want) <= ulps * fabsf(want) * FLT_EPSILON;
}

static void exact(double v, double s, double c)
{
	struct __double2 r = __sincospi_stret(v);
	check(same(r.__sinval, s), "sinpi", v, r.__sinval, s);
	check(same(r.__cosval, c), "cospi", v, r.__cosval, c);
}

static void exactf(float v, float s, float c)
{
	struct __float2 r = __sincospif_stret(v);
	check(same(r.__sinval, s), "sinpif", v, r.__sinval, s);
	check(same(r.__cosval, c), "cospif", v, r.__cosval, c);
}

static void approx(double v, double s, double c)
{
	struct __double2 r = __sincospi_stret(v);
	check(close_ulps(r.__sinval, s, 4), "sinpi", v, r.__sinval, s);
	check(close_ulps(r.__cosval, c, 4), "cospi", v, r.__cosval, c);
}

static void approxf(float v, float s, float c)
{
	struct __float2 r = __sincospif_stret(v);
	check(close_ulpsf(r.__sinval, s, 2), "sinpif", v, r.__sinval, s);
	check(close_ulpsf(r.__cosval, c, 2), "cospif", v, r.__cosval, c);
}

static void quiet_nan(void)
{
	struct __double2 r = __sincospi_stret(NAN);
	check(isnan(r.__sinval) && isnan(r.__cosval), "sincospi NaN", NAN, r.__sinval, NAN);
	struct __float2 rf = __sincospif_stret(NAN);
	check(isnan(rf.__sinval) && isnan(rf.__cosval), "sincospif NaN", NAN, rf.__sinval, NAN);
}

static void invalid(double v)
{
	feclearexcept(FE_ALL_EXCEPT);
	struct __double2 r = __sincospi_stret(v);
	int raised = fetestexcept(FE_INVALID) != 0;
	check(isnan(r.__sinval) && isnan(r.__cosval) && raised, "sincospi NaN+FE_INVALID", v,
		r.__sinval, NAN);

	feclearexcept(FE_ALL_EXCEPT);
	struct __float2 rf = __sincospif_stret((float)v);
	raised = fetestexcept(FE_INVALID) != 0;
	check(isnan(rf.__sinval) && isnan(rf.__cosval) && raised, "sincospif NaN+FE_INVALID", v,
		rf.__sinval, NAN);
}

static void on_alarm(int sig)
{
	(void)sig;
	printf("FAIL: timed out\n");
	_exit(1);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(10);

	// Integers: sinpi(n) is +0 for positive n and -0 for negative n,
	// cospi(n) is +1 for even n and -1 for odd n.
	exact(0.0, 0.0, 1.0);
	exact(-0.0, -0.0, 1.0);
	exact(1.0, 0.0, -1.0);
	exact(-1.0, -0.0, -1.0);
	exact(2.0, 0.0, 1.0);
	exact(-3.0, -0.0, -1.0);
	exact(4503599627370497.0, 0.0, -1.0);   // 2^52 + 1, odd
	exact(9007199254740992.0, 0.0, 1.0);    // 2^53
	exact(-1e300, -0.0, 1.0);

	// Half-integers: cospi is +0, sinpi alternates between 1 and -1
	exact(0.5, 1.0, 0.0);
	exact(1.5, -1.0, 0.0);
	exact(-0.5, -1.0, 0.0);
	exact(-1.5, 1.0, 0.0);
	exact(2.5, 1.0, 0.0);
	exact(1048576.5, 1.0, 0.0);
	exact(4503599627370495.5, -1.0, 0.0);  // 2^52 - 0.5

	exactf(0.0f, 0.0f, 1.0f);
	exactf(-0.0f, -0.0f, 1.0f);
	exactf(1.0f, 0.0f, -1.0f);
	exactf(-1.0f, -0.0f, -1.0f);
	exactf(16777215.0f, 0.0f, -1.0f);       // 2^24 - 1, odd
	exactf(16777216.0f, 0.0f, 1.0f);        // 2^24
	exactf(-3e38f, -0.0f, 1.0f);
	exactf(0.5f, 1.0f, 0.0f);
	exactf(-0.5f, -1.0f, 0.0f);
	exactf(1.5f, -1.0f, 0.0f);
	exactf(4194303.5f, -1.0f, 0.0f);        // 2^22 - 0.5

	invalid(INFINITY);
	invalid(-INFINITY);
	quiet_nan();

	// Ordinary arguments
	approx(0.25, M_SQRT1_2, M_SQRT1_2);
	approx(0.75, M_SQRT1_2, -M_SQRT1_2);
	approx(-0.25, -M_SQRT1_2, M_SQRT1_2);
	approx(1.0 / 6.0, 0.5, 0.86602540378443865);
	approx(1e-300, 3.1415926535897931e-300, 1.0);
	approxf(0.25f, (float)M_SQRT1_2, (float)M_SQRT1_2);
	approxf(1.75f, -(float)M_SQRT1_2, (float)M_SQRT1_2);

	// Next to a zero of sinpi or cospi the result is tiny, and the rounding
	// error of v * pi would dominate it: sinpi(1 - e) = sin(pi * e) and
	// cospi(1/2 - e) = sin(pi * e), both pi * e to well within an ulp here.
	approx(1.0 - 0x1p-30, M_PI * 0x1p-30, -1.0);
	approx(0.5 - 0x1p-40, 1.0, M_PI * 0x1p-40);
	approx(-2.0 + 0x1p-33, M_PI * 0x1p-33, 1.0);
	approxf(1.0f - 0x1p-20f, (float)(M_PI * 0x1p-20), -1.0f);

	// Large non-integer arguments: only the fraction matters
	approx(1e15 + 0.25, M_SQRT1_2, M_SQRT1_2);
	approx(123456789.75, -M_SQRT1_2, M_SQRT1_2);
	approxf(1048576.25f, (float)M_SQRT1_2, (float)M_SQRT1_2);
	approxf(-4194303.75f, (float)M_SQRT1_2, (float)M_SQRT1_2);

	if (failures) {
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
