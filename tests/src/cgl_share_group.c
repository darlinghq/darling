// Copyright (C) 2026 Lubos Dolezel
//
// CGLGetShareGroup(): contexts that share objects have the same share group,
// a context created without a share context has a group of its own, the group
// is non-NULL for a live context and NULL for a NULL context, it stays valid
// while any context of the group lives, and it agrees with what the contexts
// really share (a texture made in one context is visible in another of the
// group and not in a context outside it).
//
// Needs an offscreen OpenGL context (a host EGL driver).
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLDevice.h>
#include <OpenGL/gl.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CGLPixelFormatAttribute attrs[] = { 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CGLContextObj a = NULL, b = NULL, c = NULL, d = NULL;

	CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
	CHECK(CGLCreateContext(format, NULL, &a) == kCGLNoError);
	CHECK(CGLCreateContext(format, a, &b) == kCGLNoError);
	CHECK(CGLCreateContext(format, NULL, &c) == kCGLNoError);
	CHECK(CGLCreateContext(format, b, &d) == kCGLNoError);
	if (!a || !b || !c || !d) {
		printf("FAIL: contexts not created\n");
		return 1;
	}

	CGLShareGroupObj ga = CGLGetShareGroup(a);
	CHECK(ga != NULL);
	CHECK(CGLGetShareGroup(b) == ga);
	CHECK(CGLGetShareGroup(d) == ga);
	CHECK(CGLGetShareGroup(c) != NULL);
	CHECK(CGLGetShareGroup(c) != ga);
	CHECK(CGLGetShareGroup(a) == ga);
	CHECK(CGLGetShareGroup(NULL) == NULL);

	// The group agrees with what is really shared.
	GLuint texture = 0;
	CGLSetCurrentContext(a);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char texel[4] = { 0, 0, 0, 0 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);
	glFinish();
	CGLSetCurrentContext(d);
	CHECK(glIsTexture(texture));
	CGLSetCurrentContext(c);
	CHECK(!glIsTexture(texture));
	CGLSetCurrentContext(a);
	glDeleteTextures(1, &texture);
	CGLSetCurrentContext(NULL);

	// The group outlives the context that created it, and the contexts that
	// joined it keep it.
	CGLDestroyContext(a);
	CHECK(CGLGetShareGroup(b) == ga);
	CHECK(CGLGetShareGroup(d) == ga);
	CGLDestroyContext(b);
	CHECK(CGLGetShareGroup(d) == ga);

	CGLDestroyContext(c);
	CGLDestroyContext(d);
	CGLDestroyPixelFormat(format);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
