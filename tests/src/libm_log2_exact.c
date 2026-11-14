// Copyright (C) 2026 Lubos Dolezel
// log2, log2f and log2l of an exact power of two return the exponent exactly
// (C99 7.12.6.10; several programs compare the result of log2 against an
// integer). Also checks the special values.
#include <math.h>
#include <stdio.h>
#include <float.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

int main(void)
{
	for (int k = DBL_MIN_EXP - 53; k < DBL_MAX_EXP; k++) {
		volatile double x = ldexp(1.0, k);
		double r = log2(x);
		if (r != (double) k) { printf("FAIL: log2(2^%d) = %.17g\n", k, r); failures++; }
	}
	for (int k = FLT_MIN_EXP - 24; k < FLT_MAX_EXP; k++) {
		volatile float x = ldexpf(1.0f, k);
		float r = log2f(x);
		if (r != (float) k) { printf("FAIL: log2f(2^%d) = %.9g\n", k, (double) r); failures++; }
	}
	for (int k = -16000; k < 16000; k += 7) {
		volatile long double x = ldexpl(1.0L, k);
		long double r = log2l(x);
		if (r != (long double) k) { printf("FAIL: log2l(2^%d) = %.21Lg\n", k, r); failures++; }
	}

	CHECK(log2(1.0) == 0.0);
	CHECK(isinf(log2(0.0)) && log2(0.0) < 0);
	CHECK(isnan(log2(-1.0)));
	CHECK(isinf(log2(INFINITY)) && log2(INFINITY) > 0);
	CHECK(isnan(log2(NAN)));
	CHECK(fabs(log2(3.0) - 1.5849625007211562) < 1e-15);

	if (failures) return 1;
	printf("PASS\n");
	return 0;
}
