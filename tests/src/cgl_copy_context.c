// Copyright (C) 2026 Lubos Dolezel
//
// CGLCopyContext(): copies the state of the attribute groups that the mask names (the glPushAttrib bits) from one
// context to another, and leaves the state of the other groups alone. Observed on Mac OS X 10.14.4 (x86_64): a NULL
// context gives kCGLBadContext (either one), a mask of GL_COLOR_BUFFER_BIT copies the clear color but not the depth
// function, the line width, the stencil test or the viewport, a mask of all bits copies those too, and a context
// can be copied to itself. Needs an offscreen OpenGL context (a host EGL driver).
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static int same(float a, float b) { return fabsf(a - b) < 0.01f; }

static GLint geti(GLenum name) { GLint v = -1; glGetIntegerv(name, &v); return v; }
static GLfloat getf(GLenum name) { GLfloat v = -1; glGetFloatv(name, &v); return v; }

// A distinctive value for the state of each group in the source context.
static void setUpSource(void)
{
	glClearColor(0.25f, 0.5f, 0.75f, 1.0f);                         // color buffer
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
	glDepthFunc(GL_GEQUAL);                                           // depth buffer
	glDepthMask(GL_FALSE);
	glClearDepth(0.5);
	glEnable(GL_STENCIL_TEST);                                        // stencil buffer
	glStencilFunc(GL_LESS, 3, 0x0F);
	glStencilOp(GL_INCR, GL_DECR, GL_INVERT);
	glViewport(1, 2, 3, 4);                                           // viewport
	glEnable(GL_SCISSOR_TEST);                                        // scissor
	glScissor(2, 3, 5, 6);
	glLineWidth(3);                                                   // line
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0x0F0F);
	glPointSize(5);                                                   // point
	glCullFace(GL_FRONT);                                             // polygon
	glFrontFace(GL_CW);
	glPolygonOffset(2, 4);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);                                            // lighting
	glEnable(GL_LIGHTING);
	GLfloat diffuse[4] = { 0.1f, 0.2f, 0.3f, 1 };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glFogi(GL_FOG_MODE, GL_EXP2);                                     // fog
	glFogf(GL_FOG_DENSITY, 0.25f);
	glColor4f(0.9f, 0.8f, 0.7f, 0.6f);                                // current
	glHint(GL_FOG_HINT, GL_NICEST);                                   // hint
	glListBase(77);                                                   // list
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);           // texture
	glEnable(GL_DITHER);
	glDisable(GL_DITHER);
}

static void checkColorBuffer(int copied)
{
	GLfloat clear[4], mask[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clear);
	GLboolean writeMask[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, writeMask);
	(void) mask;
	CHECK((same(clear[0], 0.25f) && same(clear[1], 0.5f) && same(clear[2], 0.75f)) == copied);
	CHECK((geti(GL_BLEND_SRC_RGB) == GL_SRC_ALPHA && geti(GL_BLEND_DST_RGB) == GL_ONE_MINUS_SRC_ALPHA) == copied);
	CHECK(((glIsEnabled(GL_BLEND) == GL_TRUE) == copied));
	CHECK(((writeMask[0] && !writeMask[1] && writeMask[2] && !writeMask[3]) == copied));
}

static void checkDepth(int copied)
{
	GLboolean depthMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
	CHECK((geti(GL_DEPTH_FUNC) == GL_GEQUAL) == copied);
	CHECK((depthMask == GL_FALSE) == copied);
	CHECK(same(getf(GL_DEPTH_CLEAR_VALUE), 0.5f) == copied);
}

static void checkStencil(int copied)
{
	CHECK((glIsEnabled(GL_STENCIL_TEST) == GL_TRUE) == copied);
	CHECK((geti(GL_STENCIL_FUNC) == GL_LESS && geti(GL_STENCIL_REF) == 3 && geti(GL_STENCIL_VALUE_MASK) == 0x0F) == copied);
	CHECK((geti(GL_STENCIL_FAIL) == GL_INCR && geti(GL_STENCIL_PASS_DEPTH_FAIL) == GL_DECR
		&& geti(GL_STENCIL_PASS_DEPTH_PASS) == GL_INVERT) == copied);
}

static void checkViewport(int copied)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	CHECK((viewport[0] == 1 && viewport[1] == 2 && viewport[2] == 3 && viewport[3] == 4) == copied);
}

static void checkScissor(int copied)
{
	GLint box[4];
	glGetIntegerv(GL_SCISSOR_BOX, box);
	CHECK((glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE) == copied);
	CHECK((box[0] == 2 && box[1] == 3 && box[2] == 5 && box[3] == 6) == copied);
}

static void checkLineAndPoint(int lineCopied, int pointCopied)
{
	CHECK(same(getf(GL_LINE_WIDTH), 3) == lineCopied);
	CHECK((glIsEnabled(GL_LINE_STIPPLE) == GL_TRUE) == lineCopied);
	CHECK((geti(GL_LINE_STIPPLE_REPEAT) == 2 && geti(GL_LINE_STIPPLE_PATTERN) == 0x0F0F) == lineCopied);
	CHECK(same(getf(GL_POINT_SIZE), 5) == pointCopied);
}

static void checkPolygon(int copied)
{
	CHECK((geti(GL_CULL_FACE_MODE) == GL_FRONT && geti(GL_FRONT_FACE) == GL_CW) == copied);
	CHECK((same(getf(GL_POLYGON_OFFSET_FACTOR), 2) && same(getf(GL_POLYGON_OFFSET_UNITS), 4)) == copied);
	CHECK((glIsEnabled(GL_CULL_FACE) == GL_TRUE) == copied);
}

static void checkLighting(int copied)
{
	GLfloat diffuse[4];
	glGetLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	CHECK((geti(GL_SHADE_MODEL) == GL_FLAT) == copied);
	CHECK((glIsEnabled(GL_LIGHTING) == GL_TRUE) == copied);
	CHECK((same(diffuse[0], 0.1f) && same(diffuse[1], 0.2f) && same(diffuse[2], 0.3f)) == copied);
}

static void checkFog(int copied)
{
	CHECK((geti(GL_FOG_MODE) == GL_EXP2 && same(getf(GL_FOG_DENSITY), 0.25f)) == copied);
}

static void checkCurrent(int copied)
{
	GLfloat color[4];
	glGetFloatv(GL_CURRENT_COLOR, color);
	CHECK((same(color[0], 0.9f) && same(color[1], 0.8f) && same(color[2], 0.7f) && same(color[3], 0.6f)) == copied);
}

static void checkMisc(int hintCopied, int listCopied, int textureCopied)
{
	CHECK((geti(GL_FOG_HINT) == GL_NICEST) == hintCopied);
	CHECK((geti(GL_LIST_BASE) == 77) == listCopied);
	GLint mode = 0;
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &mode);
	CHECK((mode == GL_ADD) == textureCopied);
}

static void checkAll(CGLContextObj ctx, int on)
{
	CGLSetCurrentContext(ctx);
	checkColorBuffer(on);
	checkDepth(on);
	checkStencil(on);
	checkViewport(on);
	checkScissor(on);
	checkLineAndPoint(on, on);
	checkPolygon(on);
	checkLighting(on);
	checkFog(on);
	checkCurrent(on);
	checkMisc(on, on, on);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	CGLPixelFormatAttribute attributes[] = { kCGLPFAColorSize, 24, kCGLPFADepthSize, 16, kCGLPFAStencilSize, 8, 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK_EQ(CGLChoosePixelFormat(attributes, &format, &screens), kCGLNoError);
	CGLContextObj src = NULL, dst = NULL;
	CHECK_EQ(CGLCreateContext(format, NULL, &src), kCGLNoError);
	CHECK_EQ(CGLCreateContext(format, NULL, &dst), kCGLNoError);
	if (!src || !dst) {
		printf("FAIL: contexts not created\n");
		return 1;
	}

	CHECK_EQ(CGLCopyContext(NULL, dst, GL_ALL_ATTRIB_BITS), kCGLBadContext);
	CHECK_EQ(CGLCopyContext(src, NULL, GL_ALL_ATTRIB_BITS), kCGLBadContext);

	CGLSetCurrentContext(src);
	setUpSource();
	// the destination starts with the defaults
	checkAll(dst, 0);

	// no bits: nothing is copied; copying a context to itself is no error
	CHECK_EQ(CGLCopyContext(src, dst, 0), kCGLNoError);
	checkAll(dst, 0);
	CHECK_EQ(CGLCopyContext(src, src, GL_ALL_ATTRIB_BITS), kCGLNoError);

	// the bits copy their own groups only
	CHECK_EQ(CGLCopyContext(src, dst, GL_COLOR_BUFFER_BIT), kCGLNoError);
	CGLSetCurrentContext(dst);
	checkColorBuffer(1);
	checkDepth(0);
	checkStencil(0);
	checkViewport(0);
	checkLineAndPoint(0, 0);
	checkPolygon(0);
	checkLighting(0);
	CHECK(CGLGetCurrentContext() == dst);

	CGLSetCurrentContext(src);
	CHECK_EQ(CGLCopyContext(src, dst, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_VIEWPORT_BIT), kCGLNoError);
	CGLSetCurrentContext(dst);
	checkDepth(1);
	checkStencil(1);
	checkViewport(1);
	checkScissor(0);
	checkLineAndPoint(0, 0);

	CGLSetCurrentContext(src);
	CHECK_EQ(CGLCopyContext(src, dst, GL_LINE_BIT | GL_POLYGON_BIT | GL_SCISSOR_BIT), kCGLNoError);
	CGLSetCurrentContext(dst);
	checkScissor(1);
	checkLineAndPoint(1, 0);
	checkPolygon(1);
	checkLighting(0);
	checkFog(0);
	checkCurrent(0);
	checkMisc(0, 0, 0);

	CGLSetCurrentContext(src);
	CHECK_EQ(CGLCopyContext(src, dst, GL_POINT_BIT | GL_LIGHTING_BIT | GL_FOG_BIT | GL_CURRENT_BIT), kCGLNoError);
	CGLSetCurrentContext(dst);
	checkLineAndPoint(1, 1);
	checkLighting(1);
	checkFog(1);
	checkCurrent(1);
	checkMisc(0, 0, 0);

	CGLSetCurrentContext(src);
	CHECK_EQ(CGLCopyContext(src, dst, GL_HINT_BIT | GL_LIST_BIT | GL_TEXTURE_BIT), kCGLNoError);
	checkAll(dst, 1);

	// the enable bit alone copies the enables of every group, and not their other state
	CGLContextObj third = NULL;
	CHECK_EQ(CGLCreateContext(format, NULL, &third), kCGLNoError);
	CGLSetCurrentContext(src);
	CHECK_EQ(CGLCopyContext(src, third, GL_ENABLE_BIT), kCGLNoError);
	CGLSetCurrentContext(third);
	CHECK(glIsEnabled(GL_BLEND) && glIsEnabled(GL_STENCIL_TEST) && glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_LIGHTING)
		&& glIsEnabled(GL_CULL_FACE) && glIsEnabled(GL_LINE_STIPPLE) && !glIsEnabled(GL_DEPTH_TEST));
	checkDepth(0);
	CHECK(!same(getf(GL_LINE_WIDTH), 3));
	CHECK(geti(GL_LINE_STIPPLE_PATTERN) != 0x0F0F);

	// all the bits of the first context were copied to the second, and copying leaves no error behind
	CGLSetCurrentContext(dst);
	CHECK_EQ(glGetError(), GL_NO_ERROR);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(src);
	CGLDestroyContext(dst);
	CGLDestroyContext(third);
	CGLReleasePixelFormat(format);

	printf(g_failures ? "FAIL cgl_copy_context\n" : "PASS cgl_copy_context\n");
	return g_failures != 0;
}
