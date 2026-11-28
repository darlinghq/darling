// Copyright (C) 2026 Lubos Dolezel
//
// AGL: aglSurfaceTexture makes the window or HIView of another context the image of the texture bound in a context,
// as glTexImage2D does (AGL Reference, 2007-10-31): the image is a copy made by the call, the target is GL_TEXTURE_2D
// (the window is of power-of-two size), GL_TEXTURE_RECTANGLE_EXT or a face of a cube map (the window is square), the
// internal format GL_RGB or GL_RGBA. A pattern is drawn into a window context and into an HIView context, taken into
// the texture of a context that renders to a pixel buffer (one that shares objects with the source and one that does
// not, of a pixel format of another configuration), and a textured quad is drawn and read back. A source that is not
// a window or view (a pixel buffer, an off-screen area, no drawable) is AGL_BAD_DRAWABLE; the arguments that glTexImage2D
// rejects are its GL errors, nothing is textured. The same call through CGL's private entry point, on which AGL is
// built, is checked too.
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

// a pixel format of another configuration, for the contexts that texture from each other
static AGLPixelFormat chooseOther(void)
{
	GLint attributes[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 24, AGL_STENCIL_SIZE, 8, AGL_NONE };
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

// the GL errors raised so far, which a call is to leave alone (the first of them)
static GLenum takeGLError(void)
{
	GLenum first = glGetError();
	while (glGetError() != GL_NO_ERROR)
		;
	return first;
}

static void windowSource(WindowRef window, int width, int height, AGLPixelFormat pf, AGLPixelFormat otherPf)
{
	int powerOfTwo = (width & (width - 1)) == 0 && (height & (height - 1)) == 0;
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
	if (powerOfTwo) {
		CHECK(aglSetCurrentContext(source));
		fillHalves(width, height, 1, 0, 0, 0, 0, 1);
		takeAndCheck(plain, source, GL_TEXTURE_2D, GL_RGBA, width, height, red, blue);
		// a context that shares its objects with the source
		CHECK(aglSetCurrentContext(source));
		fillHalves(width, height, 0, 1, 0, 1, 1, 0);
		takeAndCheck(shared, source, GL_TEXTURE_2D, GL_RGBA, width, height, green, yellow);
		// the sized forms of the formats, as OpenGL Programming Guide for Mac OS X has them
		CHECK(aglSetCurrentContext(source));
		fillHalves(width, height, 1, 0, 0, 0, 0, 1);
		takeAndCheck(plain, source, GL_TEXTURE_2D, GL_RGBA8, width, height, red, blue);
	}
	// rectangle textures and RGB, of any size
	CHECK(aglSetCurrentContext(source));
	fillHalves(width, height, 1, 0, 0, 0, 0, 1);
	takeAndCheck(plain, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGB, width, height, red, blue);
	CHECK(aglSetCurrentContext(source));
	fillHalves(width, height, 0, 1, 0, 1, 1, 0);
	takeAndCheck(shared, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, width, height, green, yellow);
	// the contexts of two pixel formats are of the one renderer and virtual screen, which the reference needs
	AGLContext other = aglCreateContext(otherPf, NULL);
	CHECK(other != NULL);
	if (other) {
		CHECK(aglSetWindowRef(other, window));
		CHECK(aglGetVirtualScreen(other) == 0 && aglGetVirtualScreen(source) == 0 && aglGetVirtualScreen(plain) == 0);
		CHECK(aglSetCurrentContext(other));
		fillHalves(width, height, 0, 1, 0, 1, 1, 0);
		takeAndCheck(plain, other, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, width, height, green, yellow);
		CHECK(aglSetCurrentContext(NULL));
		CHECK(aglDestroyContext(other));
	}
	// the surface context may be the context itself
	CHECK(aglSetCurrentContext(source));
	fillHalves(width, height, 1, 0, 0, 0, 0, 1);
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texture);
	CHECK(aglSurfaceTexture(source, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_RECTANGLE_EXT, width, height));
	glDeleteTextures(1, &texture);

	// the same through CGL
	void *cglSource = NULL, *cglReader = NULL;
	CHECK(aglGetCGLContext(source, &cglSource) && aglGetCGLContext(plain, &cglReader));
	CHECK(aglSetCurrentContext(plain));
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texture);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, (CGLContextObj) cglSource) == kCGLNoError);
	CHECK(textureSizeIs(GL_TEXTURE_RECTANGLE_EXT, width, height));
	glDeleteTextures(1, &texture);
	CHECK(_CGLTexImageDrawable(NULL, GL_TEXTURE_2D, GL_RGBA, (CGLContextObj) cglSource) == kCGLBadContext);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_2D, GL_RGBA, NULL) == kCGLBadContext);
	// the target glTexImage2D does not take: it is its error, not the call's
	CHECK(takeGLError() == GL_NO_ERROR);
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglReader, GL_TEXTURE_3D, GL_RGBA, (CGLContextObj) cglSource) == kCGLNoError);
	CHECK(takeGLError() == GL_INVALID_ENUM);
	// the context of the pixel buffer is not a window one
	CHECK(_CGLTexImageDrawable((CGLContextObj) cglSource, GL_TEXTURE_2D, GL_RGBA, (CGLContextObj) cglReader) == kCGLBadDrawable);

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
	// a view is a windowed drawable too: the guide names views among the hidden drawables that textures are taken from
	HIViewRef view = makePane(window, 40, 20, 128, 64);
	CHECK(source != NULL && view != NULL);
	AGLPbuffer buffer = NULL;
	AGLContext reader = makeReader(pf, NULL, &buffer);
	if (!source || !view || !reader)
		return;
	CHECK(aglSetHIViewRef(source, view));
	CHECK(aglSetCurrentContext(source));
	fillHalves(128, 64, 0, 1, 0, 0, 0, 1);
	const int green[3] = { 0, 255, 0 }, blue[3] = { 0, 0, 255 };
	// the drawable of a view is the view's, not the window's
	takeAndCheck(reader, source, GL_TEXTURE_2D, GL_RGBA, 128, 64, green, blue);
	CHECK(aglSetCurrentContext(source));
	fillHalves(128, 64, 0, 1, 0, 0, 0, 1);
	takeAndCheck(reader, source, GL_TEXTURE_RECTANGLE_EXT, GL_RGB, 128, 64, green, blue);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(reader));
	CHECK(aglDestroyContext(source));
	CHECK(aglDestroyPBuffer(buffer));
}

static const GLenum kFaces[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

// A cube map takes its face targets, as glTexImage2D does, from a square window
static void cubeMap(WindowRef squareWindow, WindowRef wideWindow, AGLPixelFormat pf)
{
	AGLContext square = aglCreateContext(pf, NULL), wide = aglCreateContext(pf, NULL);
	AGLPbuffer buffer = NULL;
	AGLContext reader = makeReader(pf, NULL, &buffer);
	CHECK(square && wide && reader);
	if (!square || !wide || !reader)
		return;
	CHECK(aglSetWindowRef(square, squareWindow));
	CHECK(aglSetWindowRef(wide, wideWindow));
	CHECK(aglSetCurrentContext(square));
	fillHalves(128, 128, 1, 0, 0, 0, 0, 1);
	CHECK(aglSetCurrentContext(reader));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	CHECK(takeGLError() == GL_NO_ERROR);
	for (int i = 0; i < 6; i++) {
		CHECK(aglSurfaceTexture(reader, kFaces[i], GL_RGBA, square));
		CHECK_ERROR(AGL_NO_ERROR);
		CHECK(glGetError() == GL_NO_ERROR);
		CHECK(textureSizeIs(kFaces[i], 128, 128));
	}
	// the colors of a face are the drawable's
	unsigned char *texels = malloc(128 * 128 * 4);
	CHECK(texels != NULL);
	if (texels) {
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);
		CHECK(closeTo(texels[(10 * 128 + 10) * 4], 255) && closeTo(texels[(10 * 128 + 10) * 4 + 2], 0));
		CHECK(closeTo(texels[(120 * 128 + 10) * 4], 0) && closeTo(texels[(120 * 128 + 10) * 4 + 2], 255));
		free(texels);
	}
	// the cube map target itself is not one of glTexImage2D's, and a window that is not square does not suit a face
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_CUBE_MAP, GL_RGBA, square));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(takeGLError() == GL_INVALID_ENUM);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &texture);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGBA, wide));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(takeGLError() == GL_INVALID_VALUE);
	CHECK(textureSizeIs(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0));
	glDeleteTextures(1, &texture);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(reader));
	CHECK(aglDestroyContext(wide));
	CHECK(aglDestroyContext(square));
	CHECK(aglDestroyPBuffer(buffer));
}

// the surface context is not attached to a windowed drawable: a pixel buffer, an off-screen area, the full screen
static void notWindowed(WindowRef window, AGLPixelFormat pf)
{
	enum { W = 32, H = 32 };
	static unsigned char memory[W * H * 4];
	AGLContext target = aglCreateContext(pf, NULL), noDrawable = aglCreateContext(pf, NULL);
	AGLContext viaWindow = aglCreateContext(pf, NULL);
	AGLPbuffer buffer = NULL;
	AGLContext inBuffer = makeReader(pf, NULL, &buffer);
	GLint attributes[] = { AGL_RGBA, AGL_OFFSCREEN, AGL_PIXEL_SIZE, 32, AGL_NONE };
	AGLPixelFormat offscreenFormat = aglChoosePixelFormat(NULL, 0, attributes);
	GLint fullAttributes[] = { AGL_RGBA, AGL_FULLSCREEN, AGL_DOUBLEBUFFER, AGL_NONE };
	AGLPixelFormat fullFormat = aglChoosePixelFormat(NULL, 0, fullAttributes);
	CHECK(target && noDrawable && viaWindow && inBuffer && offscreenFormat && fullFormat);
	if (!target || !noDrawable || !viaWindow || !inBuffer || !offscreenFormat || !fullFormat)
		return;
	AGLContext offscreen = aglCreateContext(offscreenFormat, NULL), fullScreen = aglCreateContext(fullFormat, NULL);
	CHECK(offscreen && fullScreen);
	if (!offscreen || !fullScreen)
		return;
	CHECK(aglSetOffScreen(offscreen, W, H, W * 4, memory));
	CHECK(aglSetWindowRef(viaWindow, window));
	CHECK(aglSetCurrentContext(target));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	(void) takeGLError();

	CHECK(!aglSurfaceTexture(target, GL_TEXTURE_2D, GL_RGBA, noDrawable));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	CHECK(!aglSurfaceTexture(target, GL_TEXTURE_2D, GL_RGBA, inBuffer));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	CHECK(!aglSurfaceTexture(target, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, offscreen));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	if (aglSetFullScreen(fullScreen, 0, 0, 0, 0)) {
		CHECK(!aglSurfaceTexture(target, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, fullScreen));
		CHECK_ERROR(AGL_BAD_DRAWABLE);
		CHECK(aglSetDrawable(fullScreen, NULL));
	} else {
		printf("   (no full-screen drawable here, not checked)\n");
		(void) aglGetError();
	}
	// nothing was textured and no GL error was raised by the calls that failed
	CHECK(glGetError() == GL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 0, 0));
	// a window context is fine, and a context that moves from a window to a buffer stops being one
	CHECK(aglSurfaceTexture(target, GL_TEXTURE_2D, GL_RGBA, viaWindow));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 256, 128));
	CHECK(aglSetPBuffer(viaWindow, buffer, 0, 0, 0));
	CHECK(!aglSurfaceTexture(target, GL_TEXTURE_2D, GL_RGBA, viaWindow));
	CHECK_ERROR(AGL_BAD_DRAWABLE);

	glDeleteTextures(1, &texture);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(fullScreen));
	CHECK(aglDestroyContext(offscreen));
	CHECK(aglDestroyContext(inBuffer));
	CHECK(aglDestroyContext(viaWindow));
	CHECK(aglDestroyContext(noDrawable));
	CHECK(aglDestroyContext(target));
	CHECK(aglDestroyPBuffer(buffer));
	aglDestroyPixelFormat(offscreenFormat);
	aglDestroyPixelFormat(fullFormat);
}

static void errors(WindowRef window, WindowRef oddWindow, AGLPixelFormat pf)
{
	AGLContext source = aglCreateContext(pf, NULL), odd = aglCreateContext(pf, NULL);
	AGLContext noDrawable = aglCreateContext(pf, NULL);
	AGLPbuffer buffer = NULL;
	AGLContext reader = makeReader(pf, NULL, &buffer);
	CHECK(source && odd && noDrawable && reader);
	if (!source || !odd || !noDrawable || !reader)
		return;
	CHECK(aglSetWindowRef(source, window));
	CHECK(aglSetWindowRef(odd, oddWindow));
	CHECK(aglSetCurrentContext(reader));
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	(void) takeGLError();

	CHECK(!aglSurfaceTexture(NULL, GL_TEXTURE_2D, GL_RGBA, source));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, NULL));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSurfaceTexture(NULL, GL_TEXTURE_2D, GL_RGBA, NULL));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, noDrawable));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	// a source without a drawable is not checked against the target: the drawable comes first
	CHECK(!aglSurfaceTexture(reader, GL_TEXTURE_1D, GL_RGBA, noDrawable));
	CHECK_ERROR(AGL_BAD_DRAWABLE);
	CHECK(glGetError() == GL_NO_ERROR);

	// the targets glTexImage2D does not take: GL_INVALID_ENUM, with nothing textured and no error of AGL's
	const GLenum badTargets[] = { GL_TEXTURE_1D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_RGBA };
	for (unsigned i = 0; i < sizeof(badTargets) / sizeof(badTargets[0]); i++) {
		CHECK(aglSurfaceTexture(reader, badTargets[i], GL_RGBA, source));
		CHECK_ERROR(AGL_NO_ERROR);
		GLenum glError = takeGLError();
		if (glError != GL_INVALID_ENUM)
			printf("   target 0x%x: GL error 0x%x\n", (unsigned) badTargets[i], (unsigned) glError);
		CHECK(glError == GL_INVALID_ENUM);
	}
	// an internal format that is not GL_RGB or GL_RGBA: GL_INVALID_VALUE
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_LUMINANCE, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(takeGLError() == GL_INVALID_VALUE);
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA16F_ARB, source));
	CHECK(takeGLError() == GL_INVALID_VALUE);
	(void) aglGetError();
	// a window of a size that is not a power of two does not suit GL_TEXTURE_2D: GL_INVALID_VALUE, nothing textured;
	// a rectangle texture takes it
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, odd));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(takeGLError() == GL_INVALID_VALUE);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 0, 0));
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
	GLuint rectangle = 0;
	glGenTextures(1, &rectangle);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, rectangle);
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, odd));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(glGetError() == GL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_RECTANGLE_EXT, 200, 120));
	glDeleteTextures(1, &rectangle);
	// a failed call leaves the texture as it was, and the next call is not affected
	CHECK(textureSizeIs(GL_TEXTURE_2D, 0, 0));
	CHECK(aglSurfaceTexture(reader, GL_TEXTURE_2D, GL_RGBA, source));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(glGetError() == GL_NO_ERROR);
	CHECK(textureSizeIs(GL_TEXTURE_2D, 256, 128));
	// the error is per call: aglGetError has cleared it
	CHECK(aglGetError() == AGL_NO_ERROR);

	glDeleteTextures(1, &texture);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(reader));
	CHECK(aglDestroyContext(noDrawable));
	CHECK(aglDestroyContext(odd));
	CHECK(aglDestroyContext(source));
	CHECK(aglDestroyPBuffer(buffer));
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	WindowRef window = makeWindow(100, 100, 256, 128);
	WindowRef oddWindow = makeWindow(400, 100, 200, 120);
	WindowRef squareWindow = makeWindow(100, 300, 128, 128);
	AGLPixelFormat pf = choose(), otherPf = chooseOther();
	CHECK(pf != NULL && otherPf != NULL);
	if (pf && otherPf) {
		windowSource(window, 256, 128, pf, otherPf);
		windowSource(oddWindow, 200, 120, pf, otherPf);
		viewSource(window, pf);
		cubeMap(squareWindow, window, pf);
		notWindowed(window, pf);
		errors(window, oddWindow, pf);
		aglDestroyPixelFormat(pf);
		aglDestroyPixelFormat(otherPf);
	}
	DisposeWindow(squareWindow);
	DisposeWindow(oddWindow);
	DisposeWindow(window);
	printf(g_failures ? "FAIL agl_surface_texture\n" : "PASS agl_surface_texture\n");
	return g_failures != 0;
}
