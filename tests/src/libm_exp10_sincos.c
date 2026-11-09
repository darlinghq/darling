// Copyright (C) 2026 Lubos Dolezel
//
// __exp10, __exp10f and __sincos_stret return the right values instead of calling
// themselves until the stack runs out. Exits by itself (alarm).

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double __exp10(double);
float __exp10f(float);

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
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

	// volatile, so the compiler cannot fold the calls at build time
	volatile double two = 2.0, half = 0.5;
	volatile float twof = 2.0f;

	check(__exp10(two) == 100.0, "__exp10(2) == 100");
	check(fabs(__exp10(half) - 3.1622776601683795) < 1e-15, "__exp10(0.5) == sqrt(10)");
	check(__exp10f(twof) == 100.0f, "__exp10f(2) == 100");

	struct __double2 sc = __sincos_stret(half);
	check(sc.__sinval == sin(half) && sc.__cosval == cos(half),
		"__sincos_stret(0.5) matches sin and cos");
	struct __float2 scf = __sincosf_stret((float)half);
	check(scf.__sinval == sinf((float)half) && scf.__cosval == cosf((float)half),
		"__sincosf_stret(0.5) matches sinf and cosf");

	if (failures) {
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
