// Copyright (C) 2026 Lubos Dolezel
//
// Calls into a host ELF library wrapped by wrapgen must pass floating-point
// arguments through intact, including the first call, which resolves the symbol.
// The functions are called through pointers bound at load time: those point to
// the library's resolver stubs, so the first call runs the symbol resolver
// between the caller and the function. Each one is first called with a double.
// Build: clang -o wrapgen_float_args wrapgen_float_args.c -L/usr/lib/native -lavutil
#include <stdint.h>
#include <stdio.h>
#include <math.h>

// From libavutil (rational.h, display.h)
typedef struct AVRational { int num, den; } AVRational;
AVRational av_d2q(double d, int max);
void av_display_rotation_set(int32_t matrix[9], double angle);
double av_display_rotation_get(const int32_t matrix[9]);

static AVRational (*volatile d2q)(double, int) = av_d2q;
static void (*volatile rotation_set)(int32_t[9], double) = av_display_rotation_set;
static double (*volatile rotation_get)(const int32_t[9]) = av_display_rotation_get;

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	AVRational q = d2q(0.75, 100);
	int32_t matrix[9] = { 0 };
	double angle;

	printf("av_d2q(0.75) = %d/%d\n", q.num, q.den);
	CHECK(q.num == 3 && q.den == 4);

	// A clockwise rotation, read back as the counterclockwise angle
	rotation_set(matrix, 30.0);
	angle = rotation_get(matrix);
	printf("rotation = %.3f\n", angle);
	CHECK(fabs(fabs(angle) - 30.0) < 0.01);

	if (g_failures)
		return 1;
	printf("OK\n");
	return 0;
}
