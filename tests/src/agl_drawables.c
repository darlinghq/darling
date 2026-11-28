// Copyright (C) 2026 Lubos Dolezel
//
// AGL: the drawables other than a window, from the AGL reference, on top of CGL. A pixel buffer
// is rendered to and its contents become the texture of a context that draws to a window (aglCreatePBuffer,
// aglSetPBuffer, aglTexImagePBuffer), an off-screen context renders into memory of the client's (aglSetOffScreen),
// a full-screen context captures the display (aglSetFullScreen), an HIView drawable covers the bounds of the view
// and follows it (aglSetHIViewRef). The errors are the documented AGL error codes. Each drawable is also seen
// through CGL, which AGL is built on.
// Needs a window system (X11 or Wayland) and a GL driver. Exits non-zero on failure.
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLInternal.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __LP64__
#define COMP kWindowCompositingAttribute
#else
#define COMP 0
#endif

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_ERROR(expected) do { GLenum _e = aglGetError(); if (_e != (expected)) { printf("FAIL line %d: error %u, expected %u\n", __LINE__, (unsigned) _e, (unsigned) (expected)); g_failures++; } } while (0)

static void quitTimer(EventLoopTimerRef t, void *u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;
	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static int closeTo(int value, int expected) { return value >= expected - 3 && value <= expected + 3; }

static int pixelIs(int x, int y, int r, int g, int b)
{
	unsigned char p[4] = { 0, 0, 0, 0 };
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, p);
	if (closeTo(p[0], r) && closeTo(p[1], g) && closeTo(p[2], b))
		return 1;
	printf("   pixel (%d,%d) is %d,%d,%d, expected %d,%d,%d\n", x, y, p[0], p[1], p[2], r, g, b);
	return 0;
}

// the lower half (GL's first rows) of the drawable in one color, the upper half in another
static void fillHalves(int width, int height, float lr, float lg, float lb, float ur, float ug, float ub)
{
	glViewport(0, 0, width, height);
	glDisable(GL_DITHER);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, width, height / 2);
	glClearColor(lr, lg, lb, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glScissor(0, height / 2, width, height - height / 2);
	glClearColor(ur, ug, ub, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

static int viewportIs(int x, int y, int w, int h)
{
	GLint v[4] = { -1, -1, -1, -1 };
	glGetIntegerv(GL_VIEWPORT, v);
	if (v[0] == x && v[1] == y && v[2] == w && v[3] == h)
		return 1;
	printf("   viewport is %d %d %d %d, expected %d %d %d %d\n", v[0], v[1], v[2], v[3], x, y, w, h);
	return 0;
}

static AGLPixelFormat choose(GLint a, GLint b, GLint c)
{
	GLint attributes[] = { AGL_RGBA, AGL_ACCELERATED, AGL_DEPTH_SIZE, 16, a, b, c, AGL_NONE };
	return aglChoosePixelFormat(NULL, 0, attributes);
}

static WindowRef makeWindow(int left, int top, int width, int height)
{
	Rect bounds = { top, left, top + height, left + width };
	WindowRef window = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &bounds, &window) == noErr);
#ifndef __LP64__
	// a window without compositing has no views until it has a root control
	ControlRef root = NULL;
	CHECK(CreateRootControl(window, &root) == noErr);
#endif
	ShowWindow(window);
	spin(400);
	return window;
}

// Pixel buffer: render into it, take it as a texture in a context that draws to a window
static void pixelBuffers(WindowRef window)
{
	AGLPbuffer pbuffer = NULL;
	CHECK(!aglCreatePBuffer(16, 16, GL_TEXTURE_2D, GL_RGBA, 0, NULL));
	CHECK_ERROR(AGL_BAD_POINTER);
	CHECK(!aglCreatePBuffer(16, 16, GL_TEXTURE_1D, GL_RGBA, 0, &pbuffer));
	CHECK_ERROR(AGL_BAD_ENUM);
	CHECK(!aglCreatePBuffer(0, 16, GL_TEXTURE_2D, GL_RGBA, 0, &pbuffer));
	CHECK_ERROR(AGL_BAD_VALUE);
	CHECK(!aglDescribePBuffer(NULL, NULL, NULL, NULL, NULL, NULL));
	CHECK_ERROR(AGL_BAD_POINTER);
	CHECK(!aglDestroyPBuffer(NULL));
	CHECK_ERROR(AGL_BAD_POINTER);

	enum { W = 64, H = 32 };
	CHECK(aglCreatePBuffer(W, H, GL_TEXTURE_2D, GL_RGBA, 1, &pbuffer) && pbuffer != NULL);
	GLint width = 0, height = 0, maxLevel = -1;
	GLenum target = 0, format = 0;
	CHECK(aglDescribePBuffer(pbuffer, &width, &height, &target, &format, &maxLevel));
	CHECK(width == W && height == H && target == GL_TEXTURE_2D && format == GL_RGBA && maxLevel == 1);

	AGLPixelFormat pf = choose(AGL_NONE, AGL_NONE, AGL_NONE);
	AGLContext drawing = aglCreateContext(pf, NULL), windowed = aglCreateContext(pf, NULL);
	CHECK(drawing && windowed);
	if (!drawing || !windowed)
		return;
	CHECK(!aglSetPBuffer(NULL, pbuffer, 0, 0, 0));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSetPBuffer(drawing, pbuffer, 0, 2, 0));
	CHECK_ERROR(AGL_BAD_VALUE);
	CHECK(!aglSetPBuffer(drawing, NULL, 0, 0, 0));
	CHECK_ERROR(AGL_BAD_POINTER);
	CHECK(!aglTexImagePBuffer(NULL, pbuffer, GL_BACK));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglTexImagePBuffer(windowed, pbuffer, GL_RGBA));
	CHECK_ERROR(AGL_BAD_ENUM);

	// a context with a window gives it up for the buffer
	CHECK(aglSetWindowRef(drawing, window));
	CHECK(aglSetPBuffer(drawing, pbuffer, 0, 0, 0));
	CHECK(aglGetWindowRef(drawing) == NULL);
	AGLPbuffer attached = NULL;
	GLint face = -1, level = -1, screen = -1;
	CHECK(aglGetPBuffer(drawing, &attached, &face, &level, &screen));
	CHECK(attached == pbuffer && face == 0 && level == 0 && screen == 0);
	// the same buffer through CGL, which AGL is built on
	void *cgl = NULL;
	CHECK(aglGetCGLContext(drawing, &cgl) && cgl != NULL);
	CGLPBufferObj cglBuffer = NULL;
	CGLGetPBuffer((CGLContextObj) cgl, &cglBuffer, NULL, NULL, NULL);
	CHECK(cglBuffer == (CGLPBufferObj) pbuffer);

	CHECK(aglSetCurrentContext(drawing));
	fillHalves(W, H, 0, 1, 0, 0, 0, 1);
	CHECK(pixelIs(3, 3, 0, 255, 0));
	CHECK(pixelIs(W - 3, H - 3, 0, 0, 255));
	aglSwapBuffers(drawing);
	CHECK_ERROR(AGL_NO_ERROR);

	// the window context textures a quad with the buffer
	CHECK(aglSetWindowRef(windowed, window));
	CHECK(aglSetCurrentContext(windowed));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(aglTexImagePBuffer(windowed, pbuffer, GL_BACK));
	CHECK_ERROR(AGL_NO_ERROR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glViewport(0, 0, 200, 120);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(1, 0); glVertex2f(1, 0);
	glTexCoord2f(1, 1); glVertex2f(1, 1);
	glTexCoord2f(0, 1); glVertex2f(0, 1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glFinish();
	CHECK(pixelIs(20, 20, 0, 255, 0));      // the lower half of the buffer, at the bottom of the window
	CHECK(pixelIs(180, 100, 0, 0, 255));    // the upper half
	CHECK(glGetError() == GL_NO_ERROR);
	aglSwapBuffers(windowed);

	// the buffer drawable is left like any other: with the window again, the context draws there
	CHECK(aglSetWindowRef(drawing, window));
	CHECK(aglSetCurrentContext(drawing));
	glViewport(0, 0, 200, 120);
	glClearColor(1, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK(pixelIs(100, 60, 255, 255, 0));
	CHECK(aglGetPBuffer(drawing, &attached, &face, &level, &screen) && attached == NULL);

	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(drawing));
	CHECK(aglDestroyContext(windowed));
	CHECK(aglDestroyPBuffer(pbuffer));
	aglDestroyPixelFormat(pf);
}

// Off-screen: the rendering is in the client's memory, as 32 bit pixels with the alpha first, rows rowbytes apart
static void offScreen(WindowRef window)
{
	enum { W = 24, H = 10, ROWBYTES = W * 4 + 8 };
	static unsigned char memory[ROWBYTES * H + 8];
	AGLPixelFormat plain = choose(AGL_NONE, AGL_NONE, AGL_NONE);
	AGLPixelFormat offscreen = choose(AGL_OFFSCREEN, AGL_ALPHA_SIZE, 8);
	CHECK(plain && offscreen);
	if (!plain || !offscreen)
		return;
	GLint value = 0;
	CHECK(aglDescribePixelFormat(offscreen, AGL_OFFSCREEN, &value) && value == 1);
	AGLContext plainContext = aglCreateContext(plain, NULL), ctx = aglCreateContext(offscreen, NULL);
	CHECK(plainContext && ctx);
	if (!plainContext || !ctx)
		return;

	CHECK(!aglSetOffScreen(NULL, W, H, ROWBYTES, memory));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSetOffScreen(plainContext, W, H, ROWBYTES, memory));
	CHECK_ERROR(AGL_BAD_OFFSCREEN);
	CHECK(!aglSetOffScreen(ctx, W, H, ROWBYTES, NULL));
	CHECK_ERROR(AGL_BAD_POINTER);
	CHECK(!aglSetOffScreen(ctx, 0, H, ROWBYTES, memory));
	CHECK_ERROR(AGL_BAD_VALUE);
	CHECK(!aglSetOffScreen(ctx, W, H, W * 4 - 4, memory));
	CHECK_ERROR(AGL_BAD_VALUE);

	// the context that had a window gives it up for the memory
	CHECK(aglSetWindowRef(ctx, window));
	memset(memory, 0xEE, sizeof(memory));
	CHECK(aglSetOffScreen(ctx, W, H, ROWBYTES, memory));
	CHECK(aglGetWindowRef(ctx) == NULL);
	void *cgl = NULL;
	CHECK(aglGetCGLContext(ctx, &cgl));
	GLsizei width = 0, height = 0;
	GLint rowbytes = 0;
	void *base = NULL;
	CHECK(CGLGetOffScreen((CGLContextObj) cgl, &width, &height, &rowbytes, &base) == kCGLNoError);
	CHECK(width == W && height == H && rowbytes == ROWBYTES && base == memory);

	CHECK(aglSetCurrentContext(ctx));
	glViewport(0, 0, W, H);
	fillHalves(W, H, 0.2f, 0.4f, 0.6f, 1, 0, 0);
	aglSwapBuffers(ctx);
	CHECK_ERROR(AGL_NO_ERROR);
	// the lower half of the GL image is at the end of the memory: alpha, red, green, blue
	const unsigned char *top = memory, *bottom = memory + (H - 1) * ROWBYTES;
	CHECK(closeTo(top[0], 255) && closeTo(top[1], 255) && closeTo(top[2], 0) && closeTo(top[3], 0));
	CHECK(closeTo(bottom[0], 255) && closeTo(bottom[1], 51) && closeTo(bottom[2], 102) && closeTo(bottom[3], 153));
	CHECK(memory[W * 4] == 0xEE && memory[ROWBYTES * H] == 0xEE);
	// glFinish brings the memory up to date as well
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
	CHECK(closeTo(memory[3 * ROWBYTES + 4 * 5 + 3], 255) && closeTo(memory[3 * ROWBYTES + 4 * 5 + 1], 0));

	CHECK(aglSetWindowRef(ctx, NULL));
	CHECK(CGLGetOffScreen((CGLContextObj) cgl, &width, &height, &rowbytes, &base) == kCGLNoError && base == NULL);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(ctx));
	CHECK(aglDestroyContext(plainContext));
	aglDestroyPixelFormat(offscreen);
	aglDestroyPixelFormat(plain);
}

static void fullScreen(WindowRef window)
{
	AGLPixelFormat plain = choose(AGL_NONE, AGL_NONE, AGL_NONE);
	AGLPixelFormat full = choose(AGL_FULLSCREEN, AGL_DOUBLEBUFFER, AGL_NONE);
	CHECK(plain && full);
	if (!plain || !full)
		return;
	AGLContext plainContext = aglCreateContext(plain, NULL), ctx = aglCreateContext(full, NULL);
	CHECK(plainContext && ctx);
	if (!plainContext || !ctx)
		return;
	CGDirectDisplayID display = CGMainDisplayID();
	CGRect bounds = CGDisplayBounds(display);

	CHECK(!aglSetFullScreen(NULL, 0, 0, 0, 0));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSetFullScreen(plainContext, 0, 0, 0, 0));
	CHECK_ERROR(AGL_BAD_FULLSCREEN);
	CHECK(!CGDisplayIsCaptured(display));
	CHECK(!aglSetFullScreen(ctx, 0, 0, 0, 0x7777));
	CHECK_ERROR(AGL_BAD_GDEV);
	CHECK(!aglSetFullScreen(ctx, 12345, 6789, 0, 0));
	CHECK_ERROR(AGL_BAD_VALUE);
	CHECK(!aglSetFullScreen(ctx, 800, 0, 0, 0));
	CHECK_ERROR(AGL_BAD_VALUE);
	CHECK(!CGDisplayIsCaptured(display));

	CHECK(aglSetWindowRef(ctx, window));
	CHECK(aglSetFullScreen(ctx, 0, 0, 0, 0));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(CGDisplayIsCaptured(display));
	CHECK(aglGetWindowRef(ctx) == NULL);
	CHECK(aglSetCurrentContext(ctx));
	// the viewport is the whole device when the context is first attached to it, and is left alone when it is attached
	// to the same one again
	CHECK(viewportIs(0, 0, (int) CGDisplayPixelsWide(display), (int) CGDisplayPixelsHigh(display)));
	glViewport(1, 2, 3, 4);
	CHECK(aglSetFullScreen(ctx, 0, 0, 0, 0));
	CHECK(viewportIs(1, 2, 3, 4));
	glViewport(0, 0, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK(pixelIs((int) bounds.size.width / 2, (int) bounds.size.height / 2, 255, 0, 0));
	CHECK(pixelIs(2, 2, 255, 0, 0));
	CHECK(pixelIs((int) bounds.size.width - 3, (int) bounds.size.height - 3, 255, 0, 0));
	aglSwapBuffers(ctx);
	CHECK_ERROR(AGL_NO_ERROR);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK(pixelIs(10, 10, 0, 255, 0));

	// asking again for the size the display has (and for its refresh rate, which is any if it is 0) is no change
	CHECK(aglSetFullScreen(ctx, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height, 0, 0));
	CHECK(CGDisplayIsCaptured(display));
	glViewport(0, 0, (GLsizei) bounds.size.width, (GLsizei) bounds.size.height);
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK(pixelIs(10, 10, 0, 0, 255));

	// another display mode, if there is one: it is shown while the context is full screen, and the old one comes back
	CGDisplayModeRef current = CGDisplayCopyDisplayMode(display), other = NULL;
	CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, NULL);
	for (CFIndex i = 0; modes && i < CFArrayGetCount(modes) && !other; i++) {
		CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
		if (CGDisplayModeIsUsableForDesktopGUI(mode)
		    && (CGDisplayModeGetWidth(mode) != CGDisplayModeGetWidth(current) || CGDisplayModeGetHeight(mode) != CGDisplayModeGetHeight(current)))
			other = mode;
	}
	if (other) {
		size_t w = CGDisplayModeGetWidth(other), h = CGDisplayModeGetHeight(other);
		CHECK(aglSetFullScreen(ctx, (GLsizei) w, (GLsizei) h, 0, 0));
		CGRect now = CGDisplayBounds(display);
		CHECK((size_t) now.size.width == w && (size_t) now.size.height == h);
		CHECK(aglSetWindowRef(ctx, NULL));
		now = CGDisplayBounds(display);
		CHECK((size_t) now.size.width == CGDisplayModeGetWidth(current));
	} else {
		printf("SKIP: the display has one mode, no switch of the mode was checked\n");
	}
	if (modes)
		CFRelease(modes);
	if (current)
		CGDisplayModeRelease(current);

	// leaving full screen: the display is released, and the context draws to the window again
	CHECK(aglSetFullScreen(ctx, 0, 0, 0, 0));
	CHECK(CGDisplayIsCaptured(display));
	CHECK(aglSetWindowRef(ctx, window));
	CHECK(!CGDisplayIsCaptured(display));
	CHECK(aglSetCurrentContext(ctx));
	glViewport(0, 0, 200, 120);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK(pixelIs(100, 60, 255, 0, 255));

	// AGL_FS_CAPTURE_SINGLE captures the one display
	CHECK(aglEnable(ctx, AGL_FS_CAPTURE_SINGLE));
	CHECK(aglSetFullScreen(ctx, 0, 0, 0, 0));
	CHECK(CGDisplayIsCaptured(display));
	CHECK(aglSetDrawable(ctx, NULL));
	CHECK(!CGDisplayIsCaptured(display));

	// destroying a full-screen context lets go of the display
	CHECK(aglSetFullScreen(ctx, 0, 0, 0, 0));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(CGDisplayIsCaptured(display));
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(ctx));
	CHECK(!CGDisplayIsCaptured(display));
	CHECK(aglDestroyContext(plainContext));
	aglDestroyPixelFormat(full);
	aglDestroyPixelFormat(plain);
}

static CGRect surfaceBounds(AGLContext ctx)
{
	void *cgl = NULL;
	CGSConnectionID cid = 0;
	CGSWindowID wid = 0;
	CGSSurfaceID sid = 0;
	CGRect bounds = CGRectNull;
	aglGetCGLContext(ctx, &cgl);
	if (CGLGetSurface((CGLContextObj) cgl, &cid, &wid, &sid) == kCGLNoError && sid != 0)
		CGSGetSurfaceBounds(cid, wid, sid, &bounds);
	return bounds;
}

static int boundsAre(AGLContext ctx, int x, int y, int width, int height)
{
	CGRect b = surfaceBounds(ctx);
	if ((int) b.origin.x == x && (int) b.origin.y == y && (int) b.size.width == width && (int) b.size.height == height)
		return 1;
	printf("   surface bounds %g,%g %gx%g, expected %d,%d %dx%d\n", b.origin.x, b.origin.y, b.size.width, b.size.height, x, y, width, height);
	return 0;
}

static HIViewRef makePane(WindowRef window, int left, int top, int width, int height)
{
	Rect r = { top, left, top + height, left + width };
	HIViewRef pane = NULL;
	// any view will do: a control without contents
	CHECK(CreateStaticTextControl(window, &r, CFSTR(""), NULL, &pane) == noErr);
	return pane;
}

static void hiView(WindowRef window)
{
	AGLPixelFormat pf = choose(AGL_NONE, AGL_NONE, AGL_NONE);
	AGLContext ctx = aglCreateContext(pf, NULL);
	CHECK(ctx != NULL);
	if (!ctx)
		return;
	HIViewRef view = makePane(window, 40, 50, 160, 100);
	CHECK(view != NULL);
	if (!view)
		return;

	CHECK(!aglSetHIViewRef(NULL, view));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(aglGetHIViewRef(ctx) == NULL);
	CHECK(aglSetHIViewRef(ctx, view));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(aglGetHIViewRef(ctx) == view);
	// the drawable is a surface of the window of the view that covers the frame of the view
	CHECK(boundsAre(ctx, 40, 50, 160, 100));
	CHECK(aglSetCurrentContext(ctx));
	fillHalves(160, 100, 1, 0, 0, 0, 0, 1);
	CHECK(pixelIs(0, 0, 255, 0, 0));
	CHECK(pixelIs(159, 99, 0, 0, 255));
	CHECK(pixelIs(159, 0, 255, 0, 0));
	aglSwapBuffers(ctx);
	CHECK_ERROR(AGL_NO_ERROR);

	// the surface follows the view when it moves and when it is resized
	CHECK(HIViewMoveBy(view, 10, 20) == noErr);
	CHECK(boundsAre(ctx, 50, 70, 160, 100));
	HIRect frame = CGRectMake(30, 40, 100, 60);
	CHECK(HIViewSetFrame(view, &frame) == noErr);
	CHECK(boundsAre(ctx, 30, 40, 100, 60));
	CHECK(aglUpdateContext(ctx));
	fillHalves(100, 60, 0, 1, 0, 1, 1, 0);
	CHECK(pixelIs(99, 59, 255, 255, 0));
	CHECK(pixelIs(99, 0, 0, 255, 0));
	// hiding the view takes the drawable away from the screen, showing it brings it back at the same place
	CHECK(HIViewSetVisible(view, false) == noErr);
	CHECK(HIViewSetVisible(view, true) == noErr);
	CHECK(boundsAre(ctx, 30, 40, 100, 60));

	// a view in a view: the window's coordinates are those of the superview's plus its own
	HIViewRef outer = makePane(window, 20, 10, 170, 110);
	HIViewRef inner = makePane(window, 0, 0, 40, 30);
	CHECK(outer && inner);
	if (outer && inner) {
		HIViewAddSubview(outer, inner);
		HIRect innerFrame = CGRectMake(5, 8, 40, 30);
		HIViewSetFrame(inner, &innerFrame);
		CHECK(aglSetHIViewRef(ctx, inner));
		CHECK(aglGetHIViewRef(ctx) == inner);
		CHECK(boundsAre(ctx, 25, 18, 40, 30));
		// the superview moves and the inner view's frame in it stays the same
		CHECK(HIViewMoveBy(outer, 30, 25) == noErr);
		aglSwapBuffers(ctx);
		CHECK(boundsAre(ctx, 55, 43, 40, 30));
	}

	// no view: the context has no drawable
	CHECK(aglSetHIViewRef(ctx, NULL));
	CHECK(aglGetHIViewRef(ctx) == NULL);
	// a window ref replaces the view
	CHECK(aglSetHIViewRef(ctx, view));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(aglSetWindowRef(ctx, window));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(aglGetHIViewRef(ctx) == NULL);
	CHECK(boundsAre(ctx, 0, 0, 200, 120));
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(ctx));
	aglDestroyPixelFormat(pf);
}

// The viewport of a context that is attached to a drawable is the whole drawable (AGL Reference, aglSetDrawable,
// aglSetOffScreen, aglSetFullScreen): for a window when the context is first attached to it, not when it is attached
// to the same one again; for off-screen memory every time. The depth of an off-screen pixel format is that of the
// buffer: a request for another than 32 bit pixels has no pixel format.
static void viewports(WindowRef window)
{
	WindowRef other = makeWindow(400, 100, 80, 100);
	AGLPixelFormat pf = choose(AGL_DOUBLEBUFFER, AGL_NONE, AGL_NONE);
	AGLContext ctx = aglCreateContext(pf, NULL);
	CHECK(ctx != NULL);
	if (!ctx)
		return;
	// attached while current: the viewport is the window's
	CHECK(aglSetCurrentContext(ctx));
	CHECK(aglSetWindowRef(ctx, window));
	CHECK(viewportIs(0, 0, 200, 120));
	// attached again to the same window: left alone
	glViewport(1, 2, 3, 4);
	CHECK(aglSetWindowRef(ctx, window));
	CHECK(viewportIs(1, 2, 3, 4));
	CHECK(aglUpdateContext(ctx));
	CHECK(viewportIs(1, 2, 3, 4));
	// another window: the whole of it
	CHECK(aglSetWindowRef(ctx, other));
	CHECK(viewportIs(0, 0, 80, 100));
	// a view: the whole of the view
	Rect r = { 10, 20, 50, 70 };
	HIViewRef pane = NULL;
	CHECK(CreateStaticTextControl(window, &r, CFSTR(""), NULL, &pane) == noErr);
	CHECK(aglSetHIViewRef(ctx, pane));
	CHECK(viewportIs(0, 0, 50, 40));
	// not current when attached
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglSetWindowRef(ctx, window));
	CHECK(aglSetCurrentContext(ctx));
	CHECK(viewportIs(0, 0, 200, 120));

	// off-screen memory
	enum { W = 24, H = 10 };
	static unsigned char memory[W * H * 4];
	AGLPixelFormat offscreen = choose(AGL_OFFSCREEN, AGL_PIXEL_SIZE, 32);
	CHECK(offscreen != NULL);
	AGLContext off = offscreen ? aglCreateContext(offscreen, NULL) : NULL;
	CHECK(off != NULL);
	if (off) {
		CHECK(aglSetCurrentContext(off));
		CHECK(aglSetOffScreen(off, W, H, W * 4, memory));
		CHECK(viewportIs(0, 0, W, H));
		glViewport(1, 2, 3, 4);
		CHECK(aglSetOffScreen(off, W, H, W * 4, memory));
		CHECK(viewportIs(0, 0, W, H));
		CHECK(aglSetCurrentContext(NULL));
		CHECK(aglDestroyContext(off));
		CHECK(aglSetCurrentContext(ctx));
	}
	// the buffer depth: 16 bit and 15 bit pixels, and other sizes, are not what an off-screen area holds
	GLint sixteen[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_PIXEL_SIZE, 16, AGL_NONE };
	GLint buffer16[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_BUFFER_SIZE, 16, AGL_NONE };
	GLint rgb565[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_RED_SIZE, 5, AGL_GREEN_SIZE, 6, AGL_BLUE_SIZE, 5, AGL_NONE };
	GLint wide[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_PIXEL_SIZE, 64, AGL_NONE };
	GLint rgb888[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_RED_SIZE, 8, AGL_GREEN_SIZE, 8, AGL_BLUE_SIZE, 8, AGL_NONE };
	GLint unspecified[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_NONE };
	GLint *refused[] = { sixteen, buffer16, rgb565, wide };
	for (unsigned i = 0; i < sizeof(refused) / sizeof(refused[0]); i++) {
		AGLPixelFormat none = aglChoosePixelFormat(NULL, 0, refused[i]);
		CHECK(none == NULL);
		CHECK_ERROR(AGL_BAD_PIXELFMT);
		if (none)
			aglDestroyPixelFormat(none);
	}
	GLint *accepted[] = { rgb888, unspecified };
	for (unsigned i = 0; i < sizeof(accepted) / sizeof(accepted[0]); i++) {
		AGLPixelFormat pix = aglChoosePixelFormat(NULL, 0, accepted[i]);
		CHECK(pix != NULL);
		GLint size = 0;
		// the pixels are 32 bit ones
		if (pix) {
			CHECK(aglDescribePixelFormat(pix, AGL_PIXEL_SIZE, &size) && size == 32);
			aglDestroyPixelFormat(pix);
		}
	}
	// a window format is not held to it
	GLint window16[] = { AGL_RGBA, AGL_PIXEL_SIZE, 16, AGL_NONE };
	AGLPixelFormat windowed16 = aglChoosePixelFormat(NULL, 0, window16);
	CHECK(windowed16 != NULL);
	if (windowed16)
		aglDestroyPixelFormat(windowed16);

	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(ctx));
	aglDestroyPixelFormat(pf);
	if (offscreen)
		aglDestroyPixelFormat(offscreen);
	DisposeWindow(other);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	WindowRef window = makeWindow(100, 100, 200, 120);
	viewports(window);
	pixelBuffers(window);
	offScreen(window);
	hiView(window);
	fullScreen(window);
	DisposeWindow(window);
	printf(g_failures ? "FAIL agl_drawables\n" : "PASS agl_drawables\n");
	return g_failures != 0;
}
