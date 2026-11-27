// Copyright (C) 2026 Lubos Dolezel
//
// kCGLCPSurfaceOpacity: CGLSetParameter accepts it (0 and 1) and returns
// kCGLNoError. CGLGetParameter reads back the opacity (default 1, then the last value set) where it is
// available; macOS reports kCGLBadDrawable for a context that has no drawable (observed on macOS 26.7.3), which
// the test accepts.
#include <OpenGL/OpenGL.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	CGLPixelFormatAttribute attrs[] = { kCGLPFAColorSize, (CGLPixelFormatAttribute) 24, (CGLPixelFormatAttribute) 0 };
	CGLPixelFormatObj pf = NULL;
	GLint n = 0;
	CGLContextObj ctx = NULL;
	CHECK(CGLChoosePixelFormat(attrs, &pf, &n) == kCGLNoError && pf);
	CHECK(CGLCreateContext(pf, NULL, &ctx) == kCGLNoError && ctx);
	if (!ctx) {
		printf("FAIL: no context\n");
		return 1;
	}

	GLint v = -7;
	CGLError rc = CGLGetParameter(ctx, kCGLCPSurfaceOpacity, &v);
	CHECK(rc == kCGLBadDrawable || (rc == kCGLNoError && v == 1));

	GLint values[] = { 0, 1 };
	GLint expected[] = { 0, 1 };
	for (int i = 0; i < 2; i++) {
		CHECK(CGLSetParameter(ctx, kCGLCPSurfaceOpacity, &values[i]) == kCGLNoError);
		v = -7;
		rc = CGLGetParameter(ctx, kCGLCPSurfaceOpacity, &v);
		CHECK(rc == kCGLBadDrawable || (rc == kCGLNoError && v == expected[i]));
	}

	// Other parameters are unaffected: the swap interval still reads back what was set.
	GLint interval = 0;
	CHECK(CGLSetParameter(ctx, kCGLCPSwapInterval, &interval) == kCGLNoError);
	v = -7;
	CHECK(CGLGetParameter(ctx, kCGLCPSwapInterval, &v) == kCGLNoError && v == 0);

	CGLDestroyContext(ctx);
	CGLDestroyPixelFormat(pf);
	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
