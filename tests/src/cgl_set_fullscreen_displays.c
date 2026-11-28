// Copyright (C) 2026 Lubos Dolezel
//
// CGLSetFullScreenOnDisplay() on every display of a multi-display layout: the
// context renders into a drawable that covers the bounds of the display its
// mask names. With a single display this repeats the single-display case and
// prints SKIP for the multi-display part. Uses CGL alone, with no AppKit, so
// nothing but CGL registers the window system. Needs a window system and a
// host EGL driver.
#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// Declared by the macOS SDK's OpenGL.h; spelled out for SDKs that lack it.
extern CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask);
extern CGOpenGLDisplayMask CGDisplayIDToOpenGLDisplayMask(CGDirectDisplayID display);

static int g_failures = 0;

#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(60);

	CGDirectDisplayID displays[8];
	uint32_t count = 0;
	CHECK_EQ(CGGetActiveDisplayList(8, displays, &count), kCGErrorSuccess);
	CHECK_EQ(count >= 1, 1);

	for (uint32_t i = 0; i < count; i++) {
		CGRect bounds = CGDisplayBounds(displays[i]);
		CGOpenGLDisplayMask mask = CGDisplayIDToOpenGLDisplayMask(displays[i]);

		CGLPixelFormatAttribute attrs[] = {
			kCGLPFAFullScreen, kCGLPFADisplayMask, (CGLPixelFormatAttribute) mask,
			kCGLPFAColorSize, 24, kCGLPFADoubleBuffer, 0
		};
		CGLPixelFormatObj format = NULL;
		GLint npix = 0;
		if (CGLChoosePixelFormat(attrs, &format, &npix) != kCGLNoError) {
			printf("SKIP: no fullscreen pixel format\n");
			printf(g_failures ? "FAILED\n" : "OK\n");
			return g_failures ? 1 : 0;
		}
		CGLContextObj context = NULL;
		CHECK_EQ(CGLCreateContext(format, NULL, &context), kCGLNoError);
		CHECK_EQ(CGLSetFullScreenOnDisplay(context, mask), kCGLNoError);
		CHECK_EQ(CGLSetCurrentContext(context), kCGLNoError);

		GLint viewport[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_VIEWPORT, viewport);
		CHECK_EQ(viewport[2], (long) bounds.size.width);
		CHECK_EQ(viewport[3], (long) bounds.size.height);

		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		GLubyte pixel[4] = { 0, 0, 0, 0 };
		glReadPixels(viewport[2] / 2, viewport[3] / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		CHECK_EQ(pixel[1] >= 250 && pixel[0] <= 5 && pixel[2] <= 5, 1);
		CHECK_EQ(CGLFlushDrawable(context), kCGLNoError);

		CHECK_EQ(CGLSetCurrentContext(NULL), kCGLNoError);
		CGLDestroyContext(context);
		CGLDestroyPixelFormat(format);
	}
	if (count < 2)
		printf("SKIP: one display only, the other displays were not tested\n");

	printf(g_failures ? "FAILED\n" : "OK\n");
	return g_failures ? 1 : 0;
}
