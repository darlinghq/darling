// Copyright (C) 2026 Lubos Dolezel
//
// roundeven() and roundevenf() (C23, 7.12.9.8): halfway cases go to the even
// integer, other values to the nearest one, in every rounding direction;
// zeros keep their sign and infinities and NaNs come back unchanged.
//   clang -o /tmp/libm-roundeven tests/libm-roundeven.c   (run inside Darling; any arch)
// Exits non-zero on failure.
#include <fenv.h>
#include <math.h>
#include <stdio.h>

static int fail;

// Called through pointers, so that the compiler can't replace the calls with
// a rounding instruction and the test reaches libm.
static double (*volatile roundeven_d)(double) = roundeven;
static float (*volatile roundeven_f)(float) = roundevenf;

static void check_d(double x, double expected)
{
	double r = roundeven_d(x);
	int ok = (isnan(expected) ? isnan(r) : r == expected && signbit(r) == signbit(expected));
	printf("%s: roundeven(%g) = %g\n", ok ? "PASS" : "FAIL", x, r);
	fail |= !ok;
}

static void check_f(float x, float expected)
{
	float r = roundeven_f(x);
	int ok = (isnan(expected) ? isnan(r) : r == expected && signbit(r) == signbit(expected));
	printf("%s: roundevenf(%g) = %g\n", ok ? "PASS" : "FAIL", x, r);
	fail |= !ok;
}

static void run(void)
{
	check_d(0.5, 0.0);
	check_d(-0.5, -0.0);
	check_d(1.5, 2.0);
	check_d(2.5, 2.0);
	check_d(-2.5, -2.0);
	check_d(3.5, 4.0);
	check_d(2.4, 2.0);
	check_d(2.6, 3.0);
	check_d(-0.0, -0.0);
	check_d(4503599627370497.0, 4503599627370497.0); // 2^52 + 1, already integral
	check_d(INFINITY, INFINITY);
	check_d(-INFINITY, -INFINITY);
	check_d(NAN, NAN);
	check_f(0.5f, 0.0f);
	check_f(2.5f, 2.0f);
	check_f(-3.5f, -4.0f);
	check_f(2.6f, 3.0f);
	check_f(8388609.0f, 8388609.0f); // 2^23 + 1
	check_f(INFINITY, INFINITY);
	check_f(NAN, NAN);
}

int main(void)
{
	static const int modes[] = { FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO };
	for (unsigned i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
		fesetround(modes[i]);
		run();
	}
	fesetround(FE_TONEAREST);
	return fail;
}
