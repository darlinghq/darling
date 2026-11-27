// Copyright (C) 2026 Lubos Dolezel
//
// AGL: pixel formats, contexts and window drawables of the Carbon binding of OpenGL, from the AGL
// reference. A context draws into a Carbon window (aglSetWindowRef everywhere, aglSetDrawable with the window's
// port on 32-bit), what it draws reads back with glReadPixels, the errors of the calls are the AGL error codes
// that aglGetError returns once, and on 32-bit aglUseFont makes display lists of bitmap glyphs.
// Needs a window system (X11 or Wayland) and a GL driver. Exits non-zero on failure.
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __LP64__
#define COMP kWindowCompositingAttribute
#else
#define COMP 0
#endif

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void quitTimer(EventLoopTimerRef t, void *u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;
	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

static void formats(void)
{
	GLint major = 0, minor = 0;
	aglGetVersion(&major, &minor);
	CHECK(major >= 2);

	GLint good[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_ACCELERATED, AGL_DEPTH_SIZE, 16, AGL_NONE };
	AGLPixelFormat format = aglChoosePixelFormat(NULL, 0, good);
	CHECK(format != NULL);
	CHECK(aglGetError() == AGL_NO_ERROR);
	GLint value = -1;
	CHECK(aglDescribePixelFormat(format, AGL_DOUBLEBUFFER, &value) && value == 1);
	CHECK(aglDescribePixelFormat(format, AGL_RGBA, &value) && value == 1);
	CHECK(aglDescribePixelFormat(format, AGL_ACCELERATED, &value) && value == 1);
	// a request is a minimum
	CHECK(aglDescribePixelFormat(format, AGL_DEPTH_SIZE, &value) && value >= 16);
	CHECK(aglDescribePixelFormat(format, AGL_STEREO, &value) && value == 0);
	CHECK(aglNextPixelFormat(format) == NULL);
	CHECK(!aglDescribePixelFormat(format, 9999, &value));
	CHECK(aglGetError() == AGL_BAD_ATTRIBUTE);
	CHECK(aglGetError() == AGL_NO_ERROR); // read once
	CHECK(!aglDescribePixelFormat(NULL, AGL_RGBA, &value));
	CHECK(aglGetError() == AGL_BAD_PIXELFMT);
	CHECK(!aglDescribePixelFormat(format, AGL_RGBA, NULL));
	CHECK(aglGetError() == AGL_BAD_POINTER);
	aglDestroyPixelFormat(format);

	// the default format, and an attribute list with an attribute that does not exist
	format = aglChoosePixelFormat(NULL, 0, NULL);
	CHECK(format != NULL);
	aglDestroyPixelFormat(format);
	GLint bad[] = { AGL_RGBA, 12345, AGL_NONE };
	CHECK(aglChoosePixelFormat(NULL, 0, bad) == NULL);
	CHECK(aglGetError() == AGL_BAD_ATTRIBUTE);

	AGLRendererInfo info = aglQueryRendererInfo(NULL, 0);
	CHECK(info != NULL);
	CHECK(aglDescribeRenderer(info, AGL_RENDERER_ID, &value));
	CHECK(!aglDescribeRenderer(info, 31337, &value));
	CHECK(aglGetError() == AGL_BAD_PROPERTY);
	aglDestroyRendererInfo(info);

	CHECK(aglCreateContext(NULL, NULL) == NULL);
	CHECK(aglGetError() == AGL_BAD_PIXELFMT);
	CHECK(!aglDestroyContext(NULL));
	CHECK(aglGetError() == AGL_BAD_CONTEXT);
	CHECK(strlen((const char *) aglErrorString(AGL_BAD_CONTEXT)) > 0);
	CHECK(strcmp((const char *) aglErrorString(AGL_NO_ERROR), (const char *) aglErrorString(AGL_BAD_CONTEXT)) != 0);
}

static void options(AGLContext ctx)
{
	GLint interval = 1, value = -1;
	CHECK(aglSetInteger(ctx, AGL_SWAP_INTERVAL, &interval));
	CHECK(aglGetInteger(ctx, AGL_SWAP_INTERVAL, &value) && value == 1);
	interval = 0;
	CHECK(aglSetInteger(ctx, AGL_SWAP_INTERVAL, &interval));
	CHECK(aglGetInteger(ctx, AGL_SWAP_INTERVAL, &value) && value == 0);
	CHECK(!aglSetInteger(ctx, 777, &interval));
	CHECK(aglGetError() == AGL_BAD_ENUM);
	CHECK(!aglSetInteger(ctx, AGL_SWAP_INTERVAL, NULL));
	CHECK(aglGetError() == AGL_BAD_POINTER);
	CHECK(!aglSetInteger(NULL, AGL_SWAP_INTERVAL, &interval));
	CHECK(aglGetError() == AGL_BAD_CONTEXT);

	GLint rect[4] = { 1, 2, 30, 40 }, back[4] = { 0, 0, 0, 0 };
	CHECK(aglSetInteger(ctx, AGL_BUFFER_RECT, rect));
	CHECK(aglGetInteger(ctx, AGL_BUFFER_RECT, back) && memcmp(rect, back, sizeof(rect)) == 0);
	CHECK(!aglIsEnabled(ctx, AGL_BUFFER_RECT));
	CHECK(aglEnable(ctx, AGL_BUFFER_RECT) && aglIsEnabled(ctx, AGL_BUFFER_RECT));
	CHECK(aglDisable(ctx, AGL_BUFFER_RECT) && !aglIsEnabled(ctx, AGL_BUFFER_RECT));
	CHECK(!aglEnable(ctx, 777));
	CHECK(aglGetError() == AGL_BAD_ENUM);
	CHECK(aglGetVirtualScreen(ctx) == 0);
	CHECK(aglSetVirtualScreen(ctx, 0));
	CHECK(!aglSetVirtualScreen(ctx, 5));
	CHECK(aglGetError() == AGL_BAD_VALUE);
}

// The pixel at the center of the window, as read back from the context's drawable
static void centerPixel(int width, int height, unsigned char *rgba)
{
	glFinish();
	glReadPixels(width / 2, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

static void draw(AGLContext ctx, WindowRef window, int width, int height)
{
	unsigned char px[4];
	CHECK(aglSetCurrentContext(ctx));
	CHECK(aglGetCurrentContext() == ctx);
	glViewport(0, 0, width, height);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	centerPixel(width, height, px);
	CHECK(px[0] > 240 && px[1] < 16 && px[2] < 16);
	aglSwapBuffers(ctx);
	CHECK(aglGetError() == AGL_NO_ERROR);
	CHECK(aglUpdateContext(ctx));

	// the same context draws on after the swap, and a second color shows
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	centerPixel(width, height, px);
	CHECK(px[0] < 16 && px[1] < 16 && px[2] > 240);
	aglSwapBuffers(ctx);
	CHECK(glGetError() == GL_NO_ERROR);
}

#ifndef __LP64__
// Bitmap glyphs: A at 24 points makes some set pixels where the raster position is, and none far from it
static void fontLists(AGLContext ctx, int width, int height)
{
	short family;
	GetFNum("\pHelvetica", &family);
	CHECK(aglSetCurrentContext(ctx));
	GLuint base = glGenLists(256);
	CHECK(base != 0);
	CHECK(aglUseFont(ctx, family, 0, 24, 0, 256, (GLint) base));
	CHECK(aglGetError() == AGL_NO_ERROR);
	CHECK(glIsList(base + 'A'));
	CHECK(glIsList(base + 'z'));

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1, 1, 1);
	glRasterPos2i(20, 20);
	glListBase(base);
	glCallLists(1, GL_UNSIGNED_BYTE, "A");
	glFinish();
	enum { w = 64, h = 64 };
	unsigned char pixels[w * h * 4];
	glReadPixels(20, 10, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	int lit = 0;
	for (int i = 0; i < w * h; i++)
		if (pixels[i * 4] > 128)
			lit++;
	CHECK(lit > 20 && lit < w * h / 2);
	glReadPixels(width - 80, height - 80, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	lit = 0;
	for (int i = 0; i < w * h; i++)
		if (pixels[i * 4] > 128)
			lit++;
	CHECK(lit == 0);

	// the raster position moves by the width of the glyph
	GLfloat position[4];
	glRasterPos2i(20, 20);
	glCallLists(1, GL_UNSIGNED_BYTE, "A");
	glGetFloatv(GL_CURRENT_RASTER_POSITION, position);
	// A control list that moves the position by 10: a driver whose glBitmap does not move the raster position of a
	// list (software rasterizer of Mesa on a Wayland display) is not asked for it of the glyph lists either
	GLuint control = glGenLists(1);
	GLfloat controlPosition[4];
	glNewList(control, GL_COMPILE);
	glBitmap(0, 0, 0, 0, 10, 0, NULL);
	glEndList();
	glRasterPos2i(20, 20);
	glCallList(control);
	glGetFloatv(GL_CURRENT_RASTER_POSITION, controlPosition);
	if (controlPosition[0] > 29.5f)
		CHECK(position[0] > 20.5f);
	else
		printf("SKIP: the driver does not move the raster position by glBitmap in a list\n");
	glDeleteLists(control, 1);
	glDeleteLists(base, 256);

	CHECK(!aglUseFont(ctx, family, 0, 24, 0, 300, 1));
	CHECK(aglGetError() == AGL_BAD_VALUE);
	CHECK(!aglUseFont(NULL, family, 0, 24, 0, 1, 1));
	CHECK(aglGetError() == AGL_BAD_CONTEXT);
}
#endif

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	formats();

	enum { width = 200, height = 120 };
	Rect bounds = { 100, 100, 100 + height, 100 + width };
	WindowRef window = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &bounds, &window) == noErr);
	ShowWindow(window);
	spin(500);

	GLint attributes[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_ACCELERATED, AGL_DEPTH_SIZE, 16, AGL_NONE };
	AGLPixelFormat format = aglChoosePixelFormat(NULL, 0, attributes);
	AGLContext ctx = aglCreateContext(format, NULL);
	CHECK(ctx != NULL);
	void *cgl = NULL;
	CHECK(aglGetCGLContext(ctx, &cgl) && cgl != NULL);
	CHECK(aglGetWindowRef(ctx) == NULL);
	CHECK(aglSetWindowRef(ctx, window));
	CHECK(aglGetWindowRef(ctx) == window);
	// a context sharing the first one's objects
	AGLContext shared = aglCreateContext(format, ctx);
	CHECK(shared != NULL);
	if (shared)
		CHECK(aglDestroyContext(shared));

	// On Wayland a format with no alpha size gets a drawable with no alpha channel (shown opaque by every compositor)
	if (getenv("WAYLAND_DISPLAY") != NULL && getenv("DISPLAY") == NULL) {
		GLint alphaBits = -1;
		CHECK(aglSetCurrentContext(ctx));
		glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
		CHECK(alphaBits == 0);
	}
	draw(ctx, window, width, height);
	options(ctx);
#ifndef __LP64__
	// the window's port is the same drawable
	CHECK(aglSetWindowRef(ctx, NULL));
	CHECK(aglGetWindowRef(ctx) == NULL);
	CHECK(aglSetDrawable(ctx, GetWindowPort(window)));
	CHECK(aglGetDrawable(ctx) == GetWindowPort(window));
	CHECK(aglGetWindowRef(ctx) == window);
	draw(ctx, window, width, height);
	// an off-screen graphics world has no window
	GWorldPtr world = NULL;
	Rect wr = { 0, 0, 16, 16 };
	CHECK(NewGWorld(&world, 32, &wr, NULL, NULL, 0) == noErr);
	CHECK(!aglSetDrawable(ctx, world));
	CHECK(aglGetError() == AGL_BAD_DRAWABLE);
	DisposeGWorld(world);
	fontLists(ctx, width, height);
	CHECK(aglSetDrawable(ctx, NULL));
	CHECK(aglGetDrawable(ctx) == NULL);
#endif
	CHECK(!aglSetWindowRef(NULL, window));
	CHECK(aglGetError() == AGL_BAD_CONTEXT);

	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglGetCurrentContext() == NULL);
	CHECK(aglDestroyContext(ctx));
	aglDestroyPixelFormat(format);
	DisposeWindow(window);

	printf(g_failures ? "FAIL agl_contexts\n" : "PASS agl_contexts\n");
	return g_failures != 0;
}
