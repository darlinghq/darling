// Copyright (C) 2026 Lubos Dolezel
//
// CGLSetFullScreen() and CGLSetFullScreenOnDisplay(): a context whose pixel
// format asked for kCGLPFAFullScreen renders into the whole display; other
// contexts and bad arguments give the documented errors; CGLClearDrawable()
// leaves fullscreen; destroying a fullscreen context cleans up.
//
// Error returns for the invalid-argument cases were observed on Mac OS X
// 10.14.4 (x86_64 and i386): NULL context kCGLBadContext (both calls), a
// context whose pixel format lacks kCGLPFAFullScreen kCGLBadFullScreen (both
// calls), CGLClearDrawable on a context with no drawable no error.
// On that machine kCGLPFAFullScreen pixel formats were refused
// (kCGLBadPixelFormat), so the fullscreen part is skipped when the pixel
// format cannot be chosen. Needs a window system (X11 or Wayland) and a host
// EGL driver; captures the main display while it runs.
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

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static void check_red_pixel(const char* what)
{
	GLubyte pixel[4] = { 0, 0, 0, 0 };
	GLint viewport[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(viewport[2] / 2, viewport[3] / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	if (pixel[0] < 250 || pixel[1] > 5 || pixel[2] > 5) {
		printf("FAIL: %s: centre pixel is %d,%d,%d\n", what, pixel[0], pixel[1], pixel[2]);
		g_failures++;
	}
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(60);

	// Invalid arguments.
	CHECK_EQ(CGLSetFullScreen(NULL), kCGLBadContext);
	CHECK_EQ(CGLSetFullScreenOnDisplay(NULL, 1), kCGLBadContext);

	CGDirectDisplayID main_display = CGMainDisplayID();
	CGOpenGLDisplayMask mask = CGDisplayIDToOpenGLDisplayMask(main_display);
	CHECK(mask != 0);
	CHECK_EQ(CGOpenGLDisplayMaskToDisplayID(mask), main_display);
	CHECK_EQ(CGOpenGLDisplayMaskToDisplayID(0), kCGNullDirectDisplay);

	// A context without a fullscreen pixel format cannot go fullscreen.
	CGLPixelFormatAttribute plain_attrs[] = { kCGLPFAColorSize, 24, kCGLPFADoubleBuffer, 0 };
	CGLPixelFormatObj plain = NULL;
	GLint count = 0;
	CHECK_EQ(CGLChoosePixelFormat(plain_attrs, &plain, &count), kCGLNoError);
	CGLContextObj plain_context = NULL;
	CHECK_EQ(CGLCreateContext(plain, NULL, &plain_context), kCGLNoError);
	CHECK_EQ(CGLSetFullScreen(plain_context), kCGLBadFullScreen);
	CHECK_EQ(CGLSetFullScreenOnDisplay(plain_context, mask), kCGLBadFullScreen);
	CHECK_EQ(CGLClearDrawable(plain_context), kCGLNoError);
	CGLDestroyContext(plain_context);
	CGLDestroyPixelFormat(plain);
	CHECK_EQ(CGLClearDrawable(NULL), kCGLBadContext);

	CGLPixelFormatAttribute full_attrs[] = {
		kCGLPFAFullScreen, kCGLPFADisplayMask, (CGLPixelFormatAttribute) mask,
		kCGLPFAColorSize, 24, kCGLPFADepthSize, 16, kCGLPFADoubleBuffer, 0
	};
	CGLPixelFormatObj format = NULL;
	CGLError error = CGLChoosePixelFormat(full_attrs, &format, &count);
	if (error != kCGLNoError) {
		printf("SKIP: no fullscreen pixel format (error %d)\n", (int) error);
		printf(g_failures ? "FAILED\n" : "OK\n");
		return g_failures ? 1 : 0;
	}
	CGLContextObj context = NULL;
	CHECK_EQ(CGLCreateContext(format, NULL, &context), kCGLNoError);
	if (!context)
		return 1;

	// An unknown display mask is refused.
	CHECK_EQ(CGLSetFullScreenOnDisplay(context, 0), kCGLBadDisplay);

	// The usual sequence: capture the display, then fullscreen the context.
	CHECK_EQ(CGDisplayCapture(main_display), kCGErrorSuccess);
	CHECK_EQ(CGLSetFullScreen(context), kCGLNoError);
	CHECK_EQ(CGLSetCurrentContext(context), kCGLNoError);

	GLint viewport[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_VIEWPORT, viewport);
	CGRect bounds = CGDisplayBounds(main_display);
	CHECK_EQ(viewport[2], (long) bounds.size.width);
	CHECK_EQ(viewport[3], (long) bounds.size.height);

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	check_red_pixel("fullscreen back buffer");
	CHECK_EQ(CGLFlushDrawable(context), kCGLNoError);

	// Going fullscreen again replaces the drawable; the current context keeps drawing.
	CHECK_EQ(CGLSetFullScreenOnDisplay(context, mask), kCGLNoError);
	CHECK_EQ(CGLGetCurrentContext() == context, 1);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	check_red_pixel("replaced fullscreen drawable");
	CHECK_EQ(CGLFlushDrawable(context), kCGLNoError);

	// Leaving fullscreen: the context stays usable without a drawable.
	CHECK_EQ(CGLClearDrawable(context), kCGLNoError);
	CHECK_EQ(CGLGetCurrentContext() == context, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	// Destroying a fullscreen context needs no CGLClearDrawable.
	CHECK_EQ(CGLSetFullScreen(context), kCGLNoError);
	CHECK_EQ(CGLSetCurrentContext(NULL), kCGLNoError);
	CGLDestroyContext(context);
	CGLDestroyPixelFormat(format);
	CHECK_EQ(CGDisplayRelease(main_display), kCGErrorSuccess);

	printf(g_failures ? "FAILED\n" : "OK\n");
	return g_failures ? 1 : 0;
}
