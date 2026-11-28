// Copyright (C) 2026 Lubos Dolezel
//
// aglCopyContext(): copies the state of the attribute groups that the mask names (the glPushAttrib bits) from one
// context to another, and leaves the other groups alone; a NULL context gives AGL_BAD_CONTEXT. It is CGLCopyContext
// on the CGL contexts underneath, which is checked too. Needs a window system (X11 or Wayland) and a GL driver.
// Exits non-zero on failure.
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <stdio.h>
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

static AGLPixelFormat choose(GLint a, GLint b, GLint c)
{
	GLint attributes[] = { AGL_RGBA, AGL_ACCELERATED, AGL_DEPTH_SIZE, 16, a, b, c, AGL_NONE };
	return aglChoosePixelFormat(NULL, 0, attributes);
}

static void copying(WindowRef window)
{
	AGLPixelFormat pf = choose(AGL_NONE, AGL_NONE, AGL_NONE);
	AGLContext src = aglCreateContext(pf, NULL), dst = aglCreateContext(pf, NULL);
	CHECK(src && dst);
	if (!src || !dst)
		return;
	CHECK(!aglCopyContext(NULL, dst, GL_ALL_ATTRIB_BITS));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(!aglCopyContext(src, NULL, GL_ALL_ATTRIB_BITS));
	CHECK_ERROR(AGL_BAD_CONTEXT);
	CHECK(aglSetWindowRef(src, window));
	CHECK(aglSetCurrentContext(src));
	glClearColor(0.25f, 0.5f, 0.75f, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_GREATER);
	glLineWidth(4);
	glEnable(GL_SCISSOR_TEST);
	glScissor(1, 2, 3, 4);

	// the color buffer group only: the clear color and the blend function are copied, the rest is not
	CHECK(aglCopyContext(src, dst, GL_COLOR_BUFFER_BIT));
	CHECK_ERROR(AGL_NO_ERROR);
	CHECK(aglSetCurrentContext(dst));
	GLfloat clear[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clear);
	GLint blend = 0, depthFunc = 0;
	glGetIntegerv(GL_BLEND_SRC_RGB, &blend);
	glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
	CHECK(fabsf(clear[0] - 0.25f) < 0.01f && fabsf(clear[1] - 0.5f) < 0.01f && fabsf(clear[2] - 0.75f) < 0.01f);
	CHECK(blend == GL_SRC_ALPHA);
	CHECK(depthFunc == GL_LESS);
	CHECK(!glIsEnabled(GL_SCISSOR_TEST));
	// more groups
	CHECK(aglCopyContext(src, dst, GL_DEPTH_BUFFER_BIT | GL_LINE_BIT | GL_SCISSOR_BIT));
	GLfloat width = 0;
	GLint box[4];
	glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
	glGetFloatv(GL_LINE_WIDTH, &width);
	glGetIntegerv(GL_SCISSOR_BOX, box);
	CHECK(depthFunc == GL_GREATER && fabsf(width - 4) < 0.01f);
	CHECK(glIsEnabled(GL_SCISSOR_TEST) && box[0] == 1 && box[1] == 2 && box[2] == 3 && box[3] == 4);
	// the same through CGL
	void *cglSrc = NULL, *cglDst = NULL;
	CHECK(aglGetCGLContext(src, &cglSrc) && aglGetCGLContext(dst, &cglDst));
	CHECK(CGLCopyContext((CGLContextObj) cglSrc, (CGLContextObj) cglDst, GL_ALL_ATTRIB_BITS) == kCGLNoError);
	CHECK(aglSetCurrentContext(NULL));
	CHECK(aglDestroyContext(src));
	CHECK(aglDestroyContext(dst));
	aglDestroyPixelFormat(pf);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	Rect bounds = { 100, 100, 220, 300 };
	WindowRef window = NULL;
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | COMP, &bounds, &window) == noErr);
	ShowWindow(window);
	spin(400);
	copying(window);
	DisposeWindow(window);
	printf(g_failures ? "FAIL agl_copy_context\n" : "PASS agl_copy_context\n");
	return g_failures != 0;
}
