// Copyright (C) 2026 Lubos Dolezel
//
// AGL: aglSurfaceTexture makes the drawable of another context (a window, an HIView) the image of the texture
// bound in a context, 2D or rectangle, RGB or RGBA. A pattern is drawn into a window context and into an HIView
// context, taken into texture of a context that renders to a pixel buffer (one that shares objects with the source and
// one that does not), and a textured quad is drawn and read back. The errors are the documented AGL ones: bad
// context, bad drawable for a surface context without a drawable, bad enum for another target or format. The same
// call through CGL's private entry point, on which AGL is built, is checked too.
// Needs a window system (X11 or Wayland) and a GL driver. Exits non-zero on failure.
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLInternal.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <stdio.h>
#include <stdlib.h>

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

static AGLPixelFormat choose(void)
{
	GLint attributes[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_ACCELERATED, AGL_DEPTH_SIZE, 16, AGL_NONE };
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

static HIViewRef makePane(WindowRef window, int left, int top, int width, int height)
{
	Rect r = { top, left, top + height, left + width };
	HIViewRef pane = NULL;
	CHECK(CreateStaticTextControl(window, &r, CFSTR(""), NULL, &pane) == noErr);
	return pane;
}

enum { QW = 64, QH = 32 };

// Draws the texture bound to target over the whole pixel buffer of the current context. The rectangle texture is
// addressed in texels (texW x texH), the 2D texture from 0 to 1.
static void drawQuad(GLenum target, float texW, float texH)
{
	glViewport(0, 0, QW, QH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(target);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(texW, 0); glVertex2f(1, 0);
	glTexCoord2f(texW, texH); glVertex2f(1, 1);
	glTexCoord2f(0, texH); glVertex2f(0, 1);
	glEnd();
	glDisable(target);
	glFinish();
}

static int textureSizeIs(GLenum target, int width, int height)
{
	GLint w = -1, h = -1;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &h);
	if (w == width && h == height)
		return 1;
	printf("   texture is %dx%d, expected %dx%d\n", w, h, width, height);
	return 0;
}

// A context that renders to a pixel buffer, to read the quad from
static AGLContext makeReader(AGLPixelFormat pf, AGLContext share, AGLPbuffer *pbuffer)
{
	AGLContext reader = aglCreateContext(pf, share);
	CHECK(reader != NULL);
	CHECK(aglCreatePBuffer(QW, QH, GL_TEXTURE_2D, GL_RGBA, 0, pbuffer));
	if (reader)
		CHECK(aglSetPBuffer(reader, *pbuffer, 0, 0, 0));
	return reader;
}

// Takes the drawable of source into the texture of reader and checks the quad. The source holds
// lower/upper half colors; the quad shows them at the bottom and the top.
static void takeAndCheck(AGLContext reader, AGLContext source, GLenum target, GLenum format, int width, int height,
                         const int low[3], const int up[3])
{
	CHECK(aglSetCurrentContext(reader));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	CHECK(aglSurfaceTexture(reader, target, format, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(glGetError() == GL_NO_ERROR);
	CHECK(textureSizeIs(target, width, height));
	if (target == GL_TEXTURE_2D)
		drawQuad(target, 1, 1);
	else
		drawQuad(target, (float) width, (float) height);
	CHECK(pixelIs(10, 4, low[0], low[1], low[2]));
	CHECK(pixelIs(QW - 10, QH - 4, up[0], up[1], up[2]));
	CHECK(glGetError() == GL_NO_ERROR);
	// the texture is the image of the call: it does not follow what the source draws afterwards
	CHECK(aglSetCurrentContext(source));
	fillHalves(width, height, 1, 1, 1, 1, 1, 1);
	CHECK(aglSetCurrentContext(reader));
	if (target == GL_TEXTURE_2D)
		drawQuad(target, 1, 1);
	else
		drawQuad(target, (float) width, (float) height);
	CHECK(pixelIs(10, 4, low[0], low[1], low[2]));
	glDeleteTextures(1, &texture);
	CHECK(aglSetCurrentContext(NULL));
}

static void windowSource(WindowRef window, AGLPixelFormat pf)
{
	AGLContext source = aglCreateContext(pf, NULL);
	CHECK(source != NULL);
	CHECK(aglSetWindowRef(source, window));
	AGLPbuffer plainBuffer = NULL, sharedBuffer = NULL;
	AGLContext plain = makeReader(pf, NULL, &plainBuffer);
	AGLContext shared = makeReader(pf, source, &sharedBuffer);
	if (!source || !plain || !shared)
		return;

	const int red[3] = { 255, 0, 0 }, blue[3] = { 0, 0, 255 }, green[3] = { 0, 255, 0 }, yellow[3] = { 255, 255, 0 };
	// not swapped: what the context drew to its back buffer
	CHECK(aglSetCurrentContext(source));
	fillHalves(200, 120, 1, 0, 0, 0, 0, 1);
	takeAndCheck(plain, source, GL_TEXTURE_2D, GL_RGBA, 200, 120, red, blue);
	// a context that shares its objects with the source
	CHECK(aglSetCurrentContext(source));
	fillHalves(200, 120, 0, 1, 0, 1, 1, 0);
	takeAndCheck(shared, source, GL_TEXTURE_2D, GL_RGBA, 200, 120, green, yellow);
	// rectangle textures and RGB
	CHECK(aglSetCurrentContext(source));
	fillHalves(200, 120, 1, 0, 0, 0, 0, 1);
	takeAndCheck(plain, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGB, 200, 120, red, blue);
	CHECK(aglSetCurrentContext(source));
	fillHalves(200, 120, 0, 1, 0, 1, 1, 0);
	takeAndCheck(shared, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, 200, 120, green, yellow);
	// the surface context may be the context itself
	CHECK(aglSetCurrentContext(source));
	fillHalves(200, 120, 1, 0, 0, 0, 0, 1);
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(aglSurfaceTexture(source, GL_TEXTURE_2D, GL_RGBA, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 200, 120));
	glDeleteTextures(1, &texture);

	// the same through CGL
	void *cglSource = NULL, *cglReader = NULL;
	CHECK(aglGetCGLContext(source, &cglSource) && aglGetCGLContext(plain, &cglReader));
	CHECK(aglSetCurrentContext(plain));
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_2D, GL_RGBA, (CGLContextObj) cglSource) == kCGLNoError);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 200, 120));
	glDeleteTextures(1, &texture);
	CHECK(_CGLTexImageDrawable(NULL, GL_TEXTURE_2D, GL_RGBA, (CGLContextObj) cglSource) == kCGLBadContext);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_2D, GL_RGBA, NULL) == kCGLBadContext);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_3D, GL_RGBA, (CGLContextObj) cglSource) == kCGLBadEnumeration);

	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(shared));
	CHECK(aglDestroyContext(plain));
	CHECK(aglDestroyContext(source));
	CHECK(aglDestroyPBuffer(plainBuffer));
	CHECK(aglDestroyPBuffer(sharedBuffer));
}

static void viewSource(WindowRef window, AGLPixelFormat pf)
{
	AGLContext source = aglCreateContext(pf, NULL);
	HIViewRef view = makePane(window, 40, 50, 160, 100);
	CHECK(source != NULL && view != NULL);
	AGLPbuffer buffer = NULL;
	AGLContext reader = makeReader(pf, NULL, &buffer);
	if (!source || !view || !reader)
		return;
	CHECK(aglSetHIViewRef(source, view));
	CHECK(aglSetCurrentContext(source));
	fillHalves(160, 100, 0, 1, 0, 0, 0, 1);
	const int green[3] = { 0, 255, 0 }, blue[3] = { 0, 0, 255 };
	// the drawable of a view is the view's, not the window's
	takeAndCheck(reader, source, GL_TEXTURE_2D, GL_RGBA, 160, 100, green, blue);
	CHECK(aglSetCurrentContext(source));
	fillHalves(160, 100, 0, 1, 0, 0, 0, 1);
	takeAndCheck(reader, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGB, 160, 100, green, blue);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(reader));
	CHECK(aglDestroyContext(source));
	CHECK(aglDestroyPBuffer(buffer));
}

static void errors(WindowRef window, AGLPixelFormat pf)
{
	AGLContext source = aglCreateContext(pf, NULL), noDrawable = aglCreateContext(pf, NULL);
	AGLPbuffer buffer = NULL;
	AGLContext reader = makeReader(pf, NULL, &buffer);
	CHECK(source && noDrawable && reader);
	if (!source || !noDrawable || !reader)
		return;
	CHECK(aglSetWindowRef(source, window));
	CHECK(aglSetCurrentContext(reader));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	CHECK(!aglSurfaceTexture(NULL, GL_TEXTURE_2D, GL_RGBA, source));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, NULL));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSurfaceTexture(NULL, GL_TEXTURE_2D, GL_RGBA, NULL));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	// the surface context has no drawable
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, noDrawable));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	// targets and formats other than the two of each
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_1D, GL_RGBA, source));
	CHECK_ERROR(AGL_BAD_ENUM);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_CUBE_MAP, GL_RGBA, source));
	CHECK_ERROR(AGL_BAD_ENUM);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_LUMINANCE, source));
	CHECK_ERROR(AGL_BAD_ENUM);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA8, source));
	CHECK_ERROR(AGL_BAD_ENUM);
	// the arguments are checked before the drawable
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_1D, GL_RGBA, noDrawable));
	CHECK_ERROR(AGL_BAD_ENUM);
	// a failed call leaves the texture as it was, and the next call is not affected
	CHECK(textureSizeIs(GL_TEXTURE_2D, 0, 0));
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 200, 120));
	// the error is per call: aglGetError has cleared it
	CHECK(aglGetError() == AGL_NO_ERROR);

	glDeleteTextures(1, &texture);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(reader));
	CHECK(aglDestroyContext(noDrawable));
	CHECK(aglDestroyContext(source));
	CHECK(aglDestroyPBuffer(buffer));
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	WindowRef window = makeWindow(100, 100, 200, 120);
	AGLPixelFormat pf = choose();
	CHECK(pf != NULL);
	if (pf) {
		windowSource(window, pf);
		viewSource(window, pf);
		errors(window, pf);
		aglDestroyPixelFormat(pf);
	}
	DisposeWindow(window);
	printf(g_failures ? "FAIL agl_surface_texture\n" : "PASS agl_surface_texture\n");
	return g_failures != 0;
}
