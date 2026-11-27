/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// AGL: the Carbon binding of OpenGL, built on CGL (OpenGL.framework) and the window server's windows. A pixel format
// keeps the AGL attribute list and a CGL pixel format; a context is a CGL context plus the drawable it was given.
// A window drawable is attached through the window's window server surface, as the contexts of NSOpenGLView are.
// Errors are kept per thread, as aglGetError reports the last error of the calling thread.
// Not implemented: pixel buffers, off-screen and full-screen drawables, HIView drawables, context copying and
// surface textures. They fail with an error code and leave the context unchanged.

#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLInternal.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define AGL_EXPORT __attribute__((visibility("default")))

#include <CoreText/CoreText.h>
#include <math.h>

#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <pthread.h>

// The window server window behind a Carbon window (HIToolbox)
CGSWindowID _HIWindowGetCGSWindowID(WindowRef window);

static __thread GLenum t_error = AGL_NO_ERROR;

// EGL_PLATFORM_WAYLAND_KHR
enum { kEGLPlatformWayland = 0x31D8 };

// The window system of the process connection: CGSNativeConnectionKind of CoreGraphics (its header is Objective-C)
enum { kConnectionKindX11 = 1 };
extern int _CGSNativeConnectionKind(void);

// The GL layer has to be told which window system the connection of the process is to, before the first context
// is made: AppKit tells it when it is loaded, and a Carbon process has nobody else to do it.
static pthread_once_t g_windowSystemRegistered = PTHREAD_ONCE_INIT;

static void registerWindowSystem(void)
{
	CGSConnectionID connection = CGSMainConnectionID();
	void *display = connection ? _CGSNativeDisplay(connection) : NULL;

	if (!display)
		return;
	if (_CGSNativeConnectionKind() == kConnectionKindX11)
		CGLRegisterNativeDisplay(display);
	else
		CGLRegisterNativeDisplayForPlatform(display, kEGLPlatformWayland);
}

static void failed(GLenum code) { t_error = code; }

#define kMaxFormatAttributes 64

struct __AGLPixelFormatRec {
	CGLPixelFormatObj cgl;
	// the requested attributes, as name and value pairs; booleans have the value 1
	GLint count;
	GLint name[kMaxFormatAttributes];
	GLint value[kMaxFormatAttributes];
};

struct __AGLContextRec {
	CGLContextObj cgl;
	AGLPixelFormat format;
	WindowRef window;
	AGLDrawable drawable;
	CGSWindowID surfaceWindow;   // the window server window and surface the context draws to, 0 for none
	CGSSurfaceID surface;
	GLboolean enabled[16];
	GLint bufferRect[4];
	GLint swapRect[4];
};

struct __AGLRendererInfoRec {
	int unused;
};

static void sizeSurface(AGLContext ctx);
static void dropSurface(AGLContext ctx);

// Attributes of the pixel format list: those that take a value
static int formatAttributeTakesValue(GLint attribute)
{
	switch (attribute) {
		case AGL_BUFFER_SIZE: case AGL_LEVEL: case AGL_AUX_BUFFERS:
		case AGL_RED_SIZE: case AGL_GREEN_SIZE: case AGL_BLUE_SIZE: case AGL_ALPHA_SIZE:
		case AGL_DEPTH_SIZE: case AGL_STENCIL_SIZE:
		case AGL_ACCUM_RED_SIZE: case AGL_ACCUM_GREEN_SIZE: case AGL_ACCUM_BLUE_SIZE: case AGL_ACCUM_ALPHA_SIZE:
		case AGL_PIXEL_SIZE: case AGL_SAMPLE_BUFFERS_ARB: case AGL_SAMPLES_ARB: case AGL_RENDERER_ID:
			return 1;
		default:
			return 0;
	}
}

static int formatAttributeIsBoolean(GLint attribute)
{
	switch (attribute) {
		case AGL_ALL_RENDERERS: case AGL_RGBA: case AGL_DOUBLEBUFFER: case AGL_STEREO:
		case AGL_MINIMUM_POLICY: case AGL_MAXIMUM_POLICY: case AGL_CLOSEST_POLICY:
		case AGL_OFFSCREEN: case AGL_FULLSCREEN: case AGL_AUX_DEPTH_STENCIL: case AGL_COLOR_FLOAT:
		case AGL_MULTISAMPLE: case AGL_SUPERSAMPLE: case AGL_SAMPLE_ALPHA:
		case AGL_SINGLE_RENDERER: case AGL_NO_RECOVERY: case AGL_ACCELERATED: case AGL_ROBUST:
		case AGL_BACKING_STORE: case AGL_MP_SAFE: case AGL_WINDOW: case AGL_MULTISCREEN: case AGL_COMPLIANT:
		case AGL_PBUFFER:
			return 1;
		default:
			return 0;
	}
}

AGL_EXPORT void aglGetVersion(GLint *major, GLint *minor)
{
	if (major)
		*major = 3;
	if (minor)
		*minor = 0;
}

// The CGL attribute list for the AGL attributes that CGL has a counterpart of: the AGL and CGL numbers agree for
// these, apart from the color sizes, which CGL takes as one total.
static GLint translateFormat(const struct __AGLPixelFormatRec *format, CGLPixelFormatAttribute *out)
{
	GLint n = 0;
	GLint colorBits = 0;
	for (GLint i = 0; i < format->count; i++) {
		GLint name = format->name[i], value = format->value[i];
		switch (name) {
			case AGL_DOUBLEBUFFER: case AGL_STEREO:
			case AGL_OFFSCREEN: case AGL_FULLSCREEN: case AGL_AUX_DEPTH_STENCIL: case AGL_COLOR_FLOAT:
			case AGL_MULTISAMPLE: case AGL_SUPERSAMPLE: case AGL_SAMPLE_ALPHA: case AGL_ACCELERATED:
			case AGL_NO_RECOVERY: case AGL_SINGLE_RENDERER: case AGL_ROBUST: case AGL_BACKING_STORE:
			case AGL_MP_SAFE: case AGL_WINDOW: case AGL_MULTISCREEN: case AGL_COMPLIANT: case AGL_PBUFFER:
			case AGL_MINIMUM_POLICY: case AGL_MAXIMUM_POLICY: case AGL_CLOSEST_POLICY:
				out[n++] = (CGLPixelFormatAttribute) name;
				break;
			case AGL_RED_SIZE: case AGL_GREEN_SIZE: case AGL_BLUE_SIZE:
				colorBits += value;
				break;
			case AGL_BUFFER_SIZE: case AGL_PIXEL_SIZE:
				if (value > colorBits)
					colorBits = value;
				break;
			case AGL_ALPHA_SIZE: case AGL_DEPTH_SIZE: case AGL_STENCIL_SIZE: case AGL_AUX_BUFFERS:
			case AGL_SAMPLE_BUFFERS_ARB: case AGL_SAMPLES_ARB: case AGL_RENDERER_ID:
				out[n++] = (CGLPixelFormatAttribute) name;
				out[n++] = (CGLPixelFormatAttribute) value;
				break;
			case AGL_ACCUM_RED_SIZE: case AGL_ACCUM_GREEN_SIZE: case AGL_ACCUM_BLUE_SIZE: case AGL_ACCUM_ALPHA_SIZE:
				out[n++] = kCGLPFAAccumSize;
				out[n++] = (CGLPixelFormatAttribute) value;
				break;
			default:
				break;
		}
	}
	if (colorBits > 0) {
		out[n++] = kCGLPFAColorSize;
		out[n++] = (CGLPixelFormatAttribute) colorBits;
	}
	out[n] = 0;
	return n;
}

AGL_EXPORT AGLPixelFormat aglChoosePixelFormat(const AGLDevice *gdevs, GLint ndev, const GLint *attribs)
{
	(void) gdevs;
	(void) ndev;
	pthread_once(&g_windowSystemRegistered, registerWindowSystem);
	AGLPixelFormat format = calloc(1, sizeof(*format));
	if (!format) {
		failed(AGL_BAD_ALLOC);
		return NULL;
	}
	// no attributes: the default pixel format of an RGBA, single buffered window
	for (const GLint *a = attribs; a && *a != AGL_NONE; a++) {
		GLint name = *a;
		if (format->count >= kMaxFormatAttributes) {
			free(format);
			failed(AGL_BAD_ATTRIBUTE);
			return NULL;
		}
		if (formatAttributeTakesValue(name)) {
			format->name[format->count] = name;
			format->value[format->count] = *++a;
			format->count++;
		} else if (formatAttributeIsBoolean(name)) {
			format->name[format->count] = name;
			format->value[format->count] = 1;
			format->count++;
		} else {
			free(format);
			failed(AGL_BAD_ATTRIBUTE);
			return NULL;
		}
	}
	CGLPixelFormatAttribute cglAttribs[2 * kMaxFormatAttributes + 4];
	translateFormat(format, cglAttribs);
	GLint screens = 0;
	if (CGLChoosePixelFormat(cglAttribs, &format->cgl, &screens) != kCGLNoError || !format->cgl) {
		free(format);
		failed(AGL_BAD_PIXELFMT);
		return NULL;
	}
	return format;
}

AGL_EXPORT void aglDestroyPixelFormat(AGLPixelFormat pix)
{
	if (!pix) {
		failed(AGL_BAD_PIXELFMT);
		return;
	}
	CGLReleasePixelFormat(pix->cgl);
	free(pix);
}

// Every pixel format list has one entry
AGL_EXPORT AGLPixelFormat aglNextPixelFormat(AGLPixelFormat pix)
{
	(void) pix;
	return NULL;
}

static GLint requestedValue(const struct __AGLPixelFormatRec *format, GLint name, GLint fallback)
{
	for (GLint i = 0; i < format->count; i++)
		if (format->name[i] == name)
			return format->value[i];
	return fallback;
}

static int requestedFlag(const struct __AGLPixelFormatRec *format, GLint name)
{
	for (GLint i = 0; i < format->count; i++)
		if (format->name[i] == name)
			return 1;
	return 0;
}

static GLint atLeast(GLint requested, GLint available)
{
	return requested > available ? requested : available;
}

// What the pixel format provides: RGBA with 8 bits per channel, a 24 bit depth and an 8 bit stencil buffer, on one
// accelerated renderer, so a request is reported as at least that.
AGL_EXPORT GLboolean aglDescribePixelFormat(AGLPixelFormat pix, GLint attrib, GLint *value)
{
	if (!pix) {
		failed(AGL_BAD_PIXELFMT);
		return GL_FALSE;
	}
	if (!value) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	switch (attrib) {
		case AGL_RGBA: *value = 1; break;
		case AGL_BUFFER_SIZE: case AGL_PIXEL_SIZE: *value = 32; break;
		case AGL_LEVEL: *value = requestedValue(pix, AGL_LEVEL, 0); break;
		case AGL_DOUBLEBUFFER: *value = requestedFlag(pix, AGL_DOUBLEBUFFER); break;
		case AGL_STEREO: *value = requestedFlag(pix, AGL_STEREO); break;
		case AGL_AUX_BUFFERS: *value = requestedValue(pix, AGL_AUX_BUFFERS, 0); break;
		case AGL_RED_SIZE: case AGL_GREEN_SIZE: case AGL_BLUE_SIZE: case AGL_ALPHA_SIZE:
			*value = atLeast(requestedValue(pix, attrib, 0), 8); break;
		case AGL_DEPTH_SIZE: *value = atLeast(requestedValue(pix, attrib, 0), 24); break;
		case AGL_STENCIL_SIZE: *value = atLeast(requestedValue(pix, attrib, 0), 8); break;
		case AGL_ACCUM_RED_SIZE: case AGL_ACCUM_GREEN_SIZE: case AGL_ACCUM_BLUE_SIZE: case AGL_ACCUM_ALPHA_SIZE:
			*value = requestedValue(pix, attrib, 0); break;
		case AGL_SAMPLE_BUFFERS_ARB: case AGL_SAMPLES_ARB: *value = requestedValue(pix, attrib, 0); break;
		case AGL_OFFSCREEN: case AGL_FULLSCREEN: case AGL_PBUFFER: case AGL_AUX_DEPTH_STENCIL: case AGL_COLOR_FLOAT:
		case AGL_MULTISAMPLE: case AGL_SUPERSAMPLE: case AGL_SAMPLE_ALPHA: case AGL_ROBUST:
			*value = requestedFlag(pix, attrib); break;
		case AGL_ACCELERATED: case AGL_BACKING_STORE: case AGL_WINDOW: case AGL_COMPLIANT: case AGL_MP_SAFE:
			*value = 1; break;
		case AGL_NO_RECOVERY: case AGL_SINGLE_RENDERER: case AGL_MULTISCREEN:
			*value = requestedFlag(pix, attrib); break;
		case AGL_RENDERER_ID: *value = requestedValue(pix, AGL_RENDERER_ID, 0); break;
		case AGL_VIRTUAL_SCREEN: *value = 0; break;
		default:
			failed(AGL_BAD_ATTRIBUTE);
			return GL_FALSE;
	}
	return GL_TRUE;
}

AGL_EXPORT AGLDevice *aglDevicesOfPixelFormat(AGLPixelFormat pix, GLint *ndevs)
{
	if (ndevs)
		*ndevs = 0;
	if (!pix)
		failed(AGL_BAD_PIXELFMT);
	return NULL;
}

AGL_EXPORT GLboolean aglGetCGLPixelFormat(AGLPixelFormat pix, void **cgl_pix)
{
	if (!pix) {
		failed(AGL_BAD_PIXELFMT);
		return GL_FALSE;
	}
	if (!cgl_pix) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	*cgl_pix = pix->cgl;
	return GL_TRUE;
}

// Renderer information: a single renderer describes this system's OpenGL implementation
static struct __AGLRendererInfoRec gRenderer;

AGL_EXPORT AGLRendererInfo aglQueryRendererInfo(const AGLDevice *gdevs, GLint ndev)
{
	(void) gdevs;
	(void) ndev;
	return &gRenderer;
}

AGL_EXPORT void aglDestroyRendererInfo(AGLRendererInfo rend)
{
	if (rend != &gRenderer)
		failed(AGL_BAD_RENDINFO);
}

AGL_EXPORT AGLRendererInfo aglNextRendererInfo(AGLRendererInfo rend)
{
	if (rend != &gRenderer)
		failed(AGL_BAD_RENDINFO);
	return NULL;
}

AGL_EXPORT GLboolean aglDescribeRenderer(AGLRendererInfo rend, GLint prop, GLint *value)
{
	if (rend != &gRenderer) {
		failed(AGL_BAD_RENDINFO);
		return GL_FALSE;
	}
	if (!value) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	switch (prop) {
		case AGL_RENDERER_ID: *value = 0; break;
		case AGL_ACCELERATED: case AGL_WINDOW: case AGL_BACKING_STORE: case AGL_COMPLIANT: *value = 1; break;
		case AGL_FULLSCREEN: case AGL_OFFSCREEN: case AGL_ROBUST: case AGL_MP_SAFE: *value = 0; break;
		case AGL_BUFFER_MODES: *value = AGL_MONOSCOPIC_BIT | AGL_DOUBLEBUFFER_BIT | AGL_SINGLEBUFFER_BIT; break;
		case AGL_MIN_LEVEL: case AGL_MAX_LEVEL: *value = 0; break;
		case AGL_COLOR_MODES: *value = AGL_ARGB8888_BIT | AGL_RGB888_BIT; break;
		case AGL_ACCUM_MODES: *value = AGL_0_BIT; break;
		case AGL_DEPTH_MODES: *value = AGL_0_BIT | AGL_16_BIT | AGL_24_BIT; break;
		case AGL_STENCIL_MODES: *value = AGL_0_BIT | AGL_8_BIT; break;
		case AGL_MAX_AUX_BUFFERS: *value = 0; break;
		case AGL_VIDEO_MEMORY: case AGL_TEXTURE_MEMORY: *value = 0; break;
		case AGL_RENDERER_COUNT: *value = 1; break;
		default:
			failed(AGL_BAD_PROPERTY);
			return GL_FALSE;
	}
	return GL_TRUE;
}

AGL_EXPORT AGLContext aglCreateContext(AGLPixelFormat pix, AGLContext share)
{
	if (!pix) {
		failed(AGL_BAD_PIXELFMT);
		return NULL;
	}
	AGLContext ctx = calloc(1, sizeof(*ctx));
	if (!ctx) {
		failed(AGL_BAD_ALLOC);
		return NULL;
	}
	if (CGLCreateContext(pix->cgl, share ? share->cgl : NULL, &ctx->cgl) != kCGLNoError || !ctx->cgl) {
		free(ctx);
		failed(share ? AGL_BAD_CONTEXT : AGL_BAD_MATCH);
		return NULL;
	}
	CGLRetainPixelFormat(pix->cgl);
	ctx->format = pix;
	return ctx;
}

AGL_EXPORT GLboolean aglDestroyContext(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	dropSurface(ctx);
	CGLDestroyContext(ctx->cgl);
	CGLReleasePixelFormat(ctx->format->cgl);
	free(ctx);
	return GL_TRUE;
}

AGL_EXPORT GLboolean aglCopyContext(AGLContext src, AGLContext dst, GLuint mask)
{
	(void) mask;
	failed(src && dst ? AGL_BAD_ENUM : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglUpdateContext(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	sizeSurface(ctx);
	return CGLUpdateContext(ctx->cgl) == kCGLNoError;
}

// The AGL context of the CGL context current on the thread
static __thread AGLContext t_current;

AGL_EXPORT GLboolean aglSetCurrentContext(AGLContext ctx)
{
	if (CGLSetCurrentContext(ctx ? ctx->cgl : NULL) != kCGLNoError) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	t_current = ctx;
	return GL_TRUE;
}

AGL_EXPORT AGLContext aglGetCurrentContext(void)
{
	return t_current && CGLGetCurrentContext() == t_current->cgl ? t_current : NULL;
}

AGL_EXPORT GLboolean aglGetCGLContext(AGLContext ctx, void **cgl_ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!cgl_ctx) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	*cgl_ctx = ctx->cgl;
	return GL_TRUE;
}

// The context draws to a surface of the window: a drawable of its own that covers the content of the window
static void sizeSurface(AGLContext ctx)
{
	Rect content;

	if (!ctx->surface || GetWindowBounds(ctx->window, kWindowContentRgn, &content) != noErr)
		return;
	CGSSetSurfaceBounds(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface,
		CGRectMake(0, 0, content.right - content.left, content.bottom - content.top));
}

static void dropSurface(AGLContext ctx)
{
	if (ctx->surface) {
		CGLClearDrawable(ctx->cgl);
		CGSRemoveSurface(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface);
	}
	ctx->surface = 0;
	ctx->surfaceWindow = 0;
}

AGL_EXPORT GLboolean aglSetWindowRef(AGLContext ctx, WindowRef window)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!window) {
		// The surface stays until the context is given another or is destroyed: a drawable taken away from a
		// context and a new one made after that fails on Wayland with the EGL of Mesa
		CGLClearDrawable(ctx->cgl);
		ctx->window = NULL;
		ctx->drawable = NULL;
		return GL_TRUE;
	}
	CGSWindowID wid = _HIWindowGetCGSWindowID(window);
	CGSSurfaceID surface = 0;
	if (!wid) {
		failed(AGL_BAD_WINDOW);
		return GL_FALSE;
	}
	if (CGSAddSurface(CGSMainConnectionID(), wid, &surface) != kCGErrorSuccess || !surface) {
		failed(AGL_BAD_WINDOW);
		return GL_FALSE;
	}
	if (CGLSetSurface(ctx->cgl, CGSMainConnectionID(), wid, surface) != kCGLNoError) {
		CGSRemoveSurface(CGSMainConnectionID(), wid, surface);
		failed(AGL_BAD_WINDOW);
		return GL_FALSE;
	}
	// the surface of the window the context drew to before is not needed any more
	if (ctx->surface)
		CGSRemoveSurface(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface);
	ctx->window = window;
	ctx->drawable = NULL;
	ctx->surfaceWindow = wid;
	ctx->surface = surface;
	sizeSurface(ctx);
	CGSOrderSurface(CGSMainConnectionID(), wid, surface, 1, 0);
	return GL_TRUE;
}

AGL_EXPORT WindowRef aglGetWindowRef(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return NULL;
	}
	return ctx->window;
}

AGL_EXPORT GLboolean aglSetDrawable(AGLContext ctx, AGLDrawable draw)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
#ifdef __LP64__
	failed(draw ? AGL_BAD_DRAWABLE : AGL_NO_ERROR);
	if (draw)
		return GL_FALSE;
	return aglSetWindowRef(ctx, NULL);
#else
	if (!draw)
		return aglSetWindowRef(ctx, NULL);
	// the port of a window; the port of an off-screen graphics world has no window
	WindowRef window = GetWindowFromPort(draw);
	if (!window) {
		failed(AGL_BAD_DRAWABLE);
		return GL_FALSE;
	}
	if (!aglSetWindowRef(ctx, window)) {
		failed(AGL_BAD_DRAWABLE);
		return GL_FALSE;
	}
	ctx->drawable = draw;
	return GL_TRUE;
#endif
}

AGL_EXPORT AGLDrawable aglGetDrawable(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return NULL;
	}
	return ctx->drawable;
}

AGL_EXPORT GLboolean aglSetHIViewRef(AGLContext ctx, HIViewRef hiview)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!hiview) {
		return aglSetWindowRef(ctx, NULL);
	}
	failed(AGL_BAD_DRAWABLE);
	return GL_FALSE;
}

AGL_EXPORT HIViewRef aglGetHIViewRef(AGLContext ctx)
{
	if (!ctx)
		failed(AGL_BAD_CONTEXT);
	return NULL;
}

AGL_EXPORT GLboolean aglSetOffScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei rowbytes, GLvoid *baseaddr)
{
	(void) width; (void) height; (void) rowbytes; (void) baseaddr;
	failed(ctx ? AGL_BAD_OFFSCREEN : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglSetFullScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei freq, GLint device)
{
	(void) width; (void) height; (void) freq; (void) device;
	failed(ctx ? AGL_BAD_FULLSCREEN : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglSetVirtualScreen(AGLContext ctx, GLint screen)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	// the one virtual screen of a pixel format is number 0
	if (screen != 0) {
		failed(AGL_BAD_VALUE);
		return GL_FALSE;
	}
	return GL_TRUE;
}

AGL_EXPORT GLint aglGetVirtualScreen(AGLContext ctx)
{
	if (!ctx)
		failed(AGL_BAD_CONTEXT);
	return 0;
}

AGL_EXPORT void aglSwapBuffers(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return;
	}
	CGLFlushDrawable(ctx->cgl);
}

// aglEnable and aglIsEnabled keep the state of the options that only need to be remembered
static int enableIndex(GLenum pname)
{
	switch (pname) {
		case AGL_SWAP_RECT: return 0;
		case AGL_BUFFER_RECT: return 1;
		case AGL_SWAP_LIMIT: return 2;
		case AGL_COLORMAP_TRACKING: return 3;
		case AGL_STATE_VALIDATION: return 4;
		case AGL_CLIP_REGION: return 5;
		case AGL_FS_CAPTURE_SINGLE: return 6;
		case AGL_ENABLE_SURFACE_BACKING_SIZE: return 7;
		case AGL_SURFACE_VOLATILE: return 8;
		default: return -1;
	}
}

AGL_EXPORT GLboolean aglEnable(AGLContext ctx, GLenum pname)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	int index = enableIndex(pname);
	if (index < 0) {
		failed(AGL_BAD_ENUM);
		return GL_FALSE;
	}
	ctx->enabled[index] = GL_TRUE;
	return GL_TRUE;
}

AGL_EXPORT GLboolean aglDisable(AGLContext ctx, GLenum pname)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	int index = enableIndex(pname);
	if (index < 0) {
		failed(AGL_BAD_ENUM);
		return GL_FALSE;
	}
	ctx->enabled[index] = GL_FALSE;
	return GL_TRUE;
}

AGL_EXPORT GLboolean aglIsEnabled(AGLContext ctx, GLenum pname)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	int index = enableIndex(pname);
	if (index < 0) {
		failed(AGL_BAD_ENUM);
		return GL_FALSE;
	}
	return ctx->enabled[index];
}

AGL_EXPORT GLboolean aglSetInteger(AGLContext ctx, GLenum pname, const GLint *params)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!params) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	switch (pname) {
		case AGL_SWAP_INTERVAL:
			return CGLSetParameter(ctx->cgl, kCGLCPSwapInterval, params) == kCGLNoError;
		case AGL_SURFACE_OPACITY:
			return CGLSetParameter(ctx->cgl, kCGLCPSurfaceOpacity, params) == kCGLNoError;
		case AGL_BUFFER_RECT:
			memcpy(ctx->bufferRect, params, sizeof(ctx->bufferRect));
			return GL_TRUE;
		case AGL_SWAP_RECT:
			memcpy(ctx->swapRect, params, sizeof(ctx->swapRect));
			return GL_TRUE;
		case AGL_SWAP_LIMIT: case AGL_COLORMAP_TRACKING: case AGL_STATE_VALIDATION: case AGL_ORDER_CONTEXT_TO_FRONT:
		case AGL_SURFACE_ORDER: case AGL_RASTERIZATION: case AGL_SURFACE_VOLATILE:
			return GL_TRUE;
		default:
			failed(AGL_BAD_ENUM);
			return GL_FALSE;
	}
}

AGL_EXPORT GLboolean aglGetInteger(AGLContext ctx, GLenum pname, GLint *params)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!params) {
		failed(AGL_BAD_POINTER);
		return GL_FALSE;
	}
	switch (pname) {
		case AGL_SWAP_INTERVAL:
			return CGLGetParameter(ctx->cgl, kCGLCPSwapInterval, params) == kCGLNoError;
		case AGL_SURFACE_OPACITY:
			return CGLGetParameter(ctx->cgl, kCGLCPSurfaceOpacity, params) == kCGLNoError;
		case AGL_BUFFER_RECT:
			memcpy(params, ctx->bufferRect, sizeof(ctx->bufferRect));
			return GL_TRUE;
		case AGL_SWAP_RECT:
			memcpy(params, ctx->swapRect, sizeof(ctx->swapRect));
			return GL_TRUE;
		case AGL_VIRTUAL_SCREEN:
			*params = 0;
			return GL_TRUE;
		default:
			failed(AGL_BAD_ENUM);
			return GL_FALSE;
	}
}

static int g_formatCacheSize = 5;

AGL_EXPORT GLboolean aglConfigure(GLenum pname, GLuint param)
{
	switch (pname) {
		case AGL_FORMAT_CACHE_SIZE: g_formatCacheSize = (int) param; return GL_TRUE;
		case AGL_CLEAR_FORMAT_CACHE: case AGL_RETAIN_RENDERERS: return GL_TRUE;
		default:
			failed(AGL_BAD_ENUM);
			return GL_FALSE;
	}
}

AGL_EXPORT void aglResetLibrary(void)
{
	g_formatCacheSize = 5;
}

AGL_EXPORT GLenum aglGetError(void)
{
	GLenum error = t_error;
	t_error = AGL_NO_ERROR;
	return error;
}

AGL_EXPORT const GLubyte *aglErrorString(GLenum code)
{
	switch (code) {
		case AGL_NO_ERROR: return (const GLubyte *) "no error";
		case AGL_BAD_ATTRIBUTE: return (const GLubyte *) "invalid pixel format attribute";
		case AGL_BAD_PROPERTY: return (const GLubyte *) "invalid renderer property";
		case AGL_BAD_PIXELFMT: return (const GLubyte *) "invalid pixel format";
		case AGL_BAD_RENDINFO: return (const GLubyte *) "invalid renderer information";
		case AGL_BAD_CONTEXT: return (const GLubyte *) "invalid context";
		case AGL_BAD_DRAWABLE: return (const GLubyte *) "invalid drawable";
		case AGL_BAD_GDEV: return (const GLubyte *) "invalid graphics device";
		case AGL_BAD_STATE: return (const GLubyte *) "invalid context state";
		case AGL_BAD_VALUE: return (const GLubyte *) "invalid numerical value";
		case AGL_BAD_MATCH: return (const GLubyte *) "invalid share context";
		case AGL_BAD_ENUM: return (const GLubyte *) "invalid enumerant";
		case AGL_BAD_OFFSCREEN: return (const GLubyte *) "invalid off-screen drawable";
		case AGL_BAD_FULLSCREEN: return (const GLubyte *) "invalid full-screen drawable";
		case AGL_BAD_WINDOW: return (const GLubyte *) "invalid window";
		case AGL_BAD_POINTER: return (const GLubyte *) "invalid pointer";
		case AGL_BAD_MODULE: return (const GLubyte *) "invalid code module";
		case AGL_BAD_ALLOC: return (const GLubyte *) "memory allocation failure";
		case AGL_BAD_CONNECTION: return (const GLubyte *) "invalid CoreGraphics connection";
		default: return (const GLubyte *) "unknown error";
	}
}

AGL_EXPORT GLboolean aglCreatePBuffer(GLint width, GLint height, GLenum target, GLenum internalFormat, long max_level, AGLPbuffer *pbuffer)
{
	(void) width; (void) height; (void) target; (void) internalFormat; (void) max_level;
	if (pbuffer)
		*pbuffer = NULL;
	failed(AGL_BAD_ALLOC);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglDestroyPBuffer(AGLPbuffer pbuffer)
{
	(void) pbuffer;
	failed(AGL_BAD_POINTER);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglDescribePBuffer(AGLPbuffer pbuffer, GLint *width, GLint *height, GLenum *target, GLenum *internalFormat, GLint *max_level)
{
	(void) pbuffer; (void) width; (void) height; (void) target; (void) internalFormat; (void) max_level;
	failed(AGL_BAD_POINTER);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglTexImagePBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint source)
{
	(void) pbuffer; (void) source;
	failed(ctx ? AGL_BAD_POINTER : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglSetPBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint face, GLint level, GLint screen)
{
	(void) pbuffer; (void) face; (void) level; (void) screen;
	failed(ctx ? AGL_BAD_POINTER : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglGetPBuffer(AGLContext ctx, AGLPbuffer *pbuffer, GLint *face, GLint *level, GLint *screen)
{
	if (pbuffer)
		*pbuffer = NULL;
	failed(ctx ? AGL_BAD_POINTER : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

AGL_EXPORT GLboolean aglSurfaceTexture(AGLContext context, GLenum target, GLenum internalformat, AGLContext surfacecontext)
{
	(void) target; (void) internalformat;
	failed(context && surfacecontext ? AGL_BAD_ENUM : AGL_BAD_CONTEXT);
	return GL_FALSE;
}

// Bitmap fonts: each character of the font family, face and size is drawn black on white by Core Text and kept as a
// display list of one glBitmap, whose pixel is set when it is at least half covered, and which moves the raster
// position by the advance of the character. The characters are the Mac OS Roman ones. A font family is a QuickDraw
// family number, which only 32-bit processes have: the families it has no name for, and every family of a 64-bit
// process other than 0, the system font, use the system font.
static CTFontRef copyBitmapFont(GLint fontID, Style face, GLint size)
{
	CTFontRef font = NULL;
#ifndef __LP64__
	Str255 pascalName;
	GetFontName((short) fontID, pascalName);
	if (pascalName[0]) {
		CFStringRef name = CFStringCreateWithPascalString(NULL, pascalName, kCFStringEncodingMacRoman);
		font = name ? CTFontCreateWithName(name, size, NULL) : NULL;
		if (name)
			CFRelease(name);
	}
#endif
	if (!font)
		font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
	if (!font)
		return NULL;
	CTFontSymbolicTraits traits = 0;
	if (face & 1)
		traits |= kCTFontBoldTrait;
	if (face & 2)
		traits |= kCTFontItalicTrait;
	if (traits) {
		CTFontRef styled = CTFontCreateCopyWithSymbolicTraits(font, 0, NULL, traits, traits);
		if (styled) {
			CFRelease(font);
			font = styled;
		}
	}
	return font;
}

AGL_EXPORT GLboolean aglUseFont(AGLContext ctx, GLint fontID, Style face, GLint size, GLint first, GLint count, GLint base)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (count < 0 || first < 0 || first + count > 256 || size <= 0 || base <= 0) {
		failed(AGL_BAD_VALUE);
		return GL_FALSE;
	}
#ifdef __LP64__
	if (fontID != 0) {
		failed(AGL_BAD_VALUE);
		return GL_FALSE;
	}
#endif
	CTFontRef font = copyBitmapFont(fontID, face, size);
	if (!font) {
		failed(AGL_BAD_ALLOC);
		return GL_FALSE;
	}
	size_t height = (size_t) ceil(CTFontGetAscent(font) + CTFontGetDescent(font)) + 1;
	GLint descent = (GLint) ceil(CTFontGetDescent(font));
	GLboolean ok = GL_TRUE;
	CGLContextObj previous = CGLGetCurrentContext();
	CGLSetCurrentContext(ctx->cgl);
	GLint unpackAlignment, unpackLsb;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpackAlignment);
	glGetIntegerv(GL_UNPACK_LSB_FIRST, &unpackLsb);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	CGColorSpaceRef gray = CGColorSpaceCreateDeviceGray();
	for (GLint c = first; c < first + count && ok; c++) {
		unsigned char byte = (unsigned char) c;
		CFStringRef character = CFStringCreateWithBytes(NULL, &byte, 1, kCFStringEncodingMacRoman, false);
		UniChar unicode = character && CFStringGetLength(character) > 0 ? CFStringGetCharacterAtIndex(character, 0) : 0;
		CGGlyph glyph = 0;
		if (character)
			CFRelease(character);
		if (unicode)
			CTFontGetGlyphsForCharacters(font, &unicode, &glyph, 1);
		CGSize advance = { 0, 0 };
		if (glyph)
			CTFontGetAdvancesForGlyphs(font, kCTFontOrientationHorizontal, &glyph, &advance, 1);
		size_t width = (size_t) ceil(advance.width);
		glNewList((GLuint) (base + c - first), GL_COMPILE);
		if (!glyph || width == 0) {
			glBitmap(0, 0, 0, 0, (GLfloat) advance.width, 0, NULL);
		} else {
			unsigned char *pixels = malloc(width * height);
			size_t stride = (width + 7) / 8;
			unsigned char *bitmap = calloc(stride * height, 1);
			CGContextRef context = pixels ? CGBitmapContextCreate(pixels, width, height, 8, width, gray, kCGImageAlphaNone) : NULL;
			if (!bitmap || !context) {
				ok = GL_FALSE;
			} else {
				CGPoint position = { 0, (CGFloat) descent };
				memset(pixels, 0xFF, width * height);
				CGContextSetGrayFillColor(context, 0, 1);
				CTFontDrawGlyphs(font, &glyph, &position, 1, context);
				// the first bitmap row of OpenGL is the bottom one, the last row of the pixels
				for (size_t y = 0; y < height; y++)
					for (size_t x = 0; x < width; x++)
						if (pixels[(height - 1 - y) * width + x] < 128)
							bitmap[stride * y + x / 8] |= (unsigned char) (0x80 >> (x % 8));
				glBitmap((GLsizei) width, (GLsizei) height, 0, (GLfloat) descent, (GLfloat) advance.width, 0, bitmap);
			}
			if (context)
				CGContextRelease(context);
			free(bitmap);
			free(pixels);
		}
		glEndList();
	}
	CGColorSpaceRelease(gray);
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlignment);
	glPixelStorei(GL_UNPACK_LSB_FIRST, (GLboolean) unpackLsb);
	CGLSetCurrentContext(previous);
	CFRelease(font);
	if (!ok)
		failed(AGL_BAD_ALLOC);
	return ok;
}
