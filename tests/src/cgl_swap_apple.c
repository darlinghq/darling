// Copyright (C) 2026 Lubos Dolezel
//
// glSwapAPPLE (GL_APPLE_flush_render) swaps the current context's drawable.
// With no current context (built with -DTEST_NO_CURRENT_CONTEXT, as the call
// is not safe on macOS) it returns without effect; with an offscreen
// context current (no drawable to swap) the call returns and the
// current context is unchanged.
#define GL_GLEXT_PROTOTYPES 1
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CHECK(CGLGetCurrentContext() == NULL);
#ifdef TEST_NO_CURRENT_CONTEXT
	// Not part of the Mac run: a call with no current context faults in some macOS processes.
	glSwapAPPLE();
	CHECK(CGLGetCurrentContext() == NULL);
#endif

	CGLPixelFormatAttribute attrs[] = { 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &format, &screens) != kCGLNoError ||
	    CGLCreateContext(format, NULL, &context) != kCGLNoError || context == NULL) {
		printf("FAIL: no offscreen context\n");
		return 1;
	}
	CGLDestroyPixelFormat(format);
	CHECK(CGLSetCurrentContext(context) == kCGLNoError);

	glClearColor(0.f, 1.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glSwapAPPLE();
	glGetError(); // with no drawable attached, macOS may flag the swap as an error: not checked
	CHECK(CGLGetCurrentContext() == context);

	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
