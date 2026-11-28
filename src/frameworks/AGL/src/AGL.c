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
// The other drawables are CGL's: pixel buffers (CGLSetPBuffer), the off-screen memory buffer (CGLSetOffScreen) and
// the full-screen display (CGLSetFullScreenOnDisplay, after AGL captured the display); AGL and CGL give the same
// error codes. An HIView drawable is a surface of the view's window that covers the bounds of the view, and follows
// the view when it moves, is resized or hidden. Context copying is CGLCopyContext.
// Not implemented: surface textures (aglSurfaceTexture), which fails with an error code.

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
// The AGL context of the CGL context current on the thread
static __thread AGLContext t_current;

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

#define kMaxWatchedViews 16

// the indexes of the options that aglEnable keeps in the context (see enableIndex)
enum { kEnableBufferRect = 1, kEnableFSCaptureSingle = 6 };

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
	GLint surfaceOrder;
	// HIView drawable: the view, and the views (its own and its superviews) whose changes of bounds or visibility
	// move the surface
	HIViewRef view;
	struct {
		HIViewRef view;
		EventHandlerRef handler;
	} watched[kMaxWatchedViews];
	int watchedCount;
	CGRect surfaceRect;          // the bounds last given to the surface, in window coordinates
	int surfaceShown;
	// full-screen drawable: the display that was captured, whether all the displays were, and the display mode to
	// return to
	CGDirectDisplayID fullScreenDisplay;
	int fullScreenCapturedAll;
	CGDisplayModeRef fullScreenSavedMode;
};

struct __AGLRendererInfoRec {
	int unused;
};

static void sizeSurface(AGLContext ctx);
static void dropSurface(AGLContext ctx);
static void releaseDrawables(AGLContext ctx);

// AGL's error codes are CGL's
static GLboolean cglSucceeded(CGLError error)
{
	if (error == kCGLNoError)
		return GL_TRUE;
	failed((GLenum) error);
	return GL_FALSE;
}

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
		case AGL_FULLSCREEN: case AGL_OFFSCREEN: *value = 1; break;
		case AGL_ROBUST: case AGL_MP_SAFE: *value = 0; break;
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
	ctx->surfaceRect = CGRectNull;
	ctx->surfaceOrder = 1;
	return ctx;
}

AGL_EXPORT GLboolean aglDestroyContext(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (t_current == ctx)
		t_current = NULL;
	// the full-screen window goes before the display is released
	CGLClearDrawable(ctx->cgl);
	releaseDrawables(ctx);
	dropSurface(ctx);
	CGLDestroyContext(ctx->cgl);
	CGLReleasePixelFormat(ctx->format->cgl);
	free(ctx);
	return GL_TRUE;
}

// The mask is the one of glPushAttrib, as for CGLCopyContext
AGL_EXPORT GLboolean aglCopyContext(AGLContext src, AGLContext dst, GLuint mask)
{
	if (!src || !dst) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	return cglSucceeded(CGLCopyContext(src->cgl, dst->cgl, mask));
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

// The context draws to a surface of the window: a drawable of its own that covers the content of the window, or the
// frame of the view for an HIView drawable, or the rectangle that AGL_BUFFER_RECT gives.
//
// The area of the surface in the coordinates of the window, whose origin is at the top left of its content.
static CGRect surfaceArea(AGLContext ctx)
{
	Rect content;
	if (GetWindowBounds(ctx->window, kWindowContentRgn, &content) != noErr)
		return CGRectNull;
	CGRect area = CGRectMake(0, 0, content.right - content.left, content.bottom - content.top);
	if (ctx->view) {
		Rect frame;
		GetControlBounds(ctx->view, &frame);
		area = CGRectMake(frame.left, frame.top, frame.right - frame.left, frame.bottom - frame.top);
	}
	if (ctx->enabled[kEnableBufferRect]) {
		// x and y of the rectangle count from the bottom left of the area
		CGRect buffer = CGRectMake(area.origin.x + ctx->bufferRect[0],
			area.origin.y + area.size.height - ctx->bufferRect[1] - ctx->bufferRect[3],
			ctx->bufferRect[2], ctx->bufferRect[3]);
		area = CGRectIntersection(buffer, area);
	}
	return area;
}

// Puts the surface where the drawable is now: its bounds are those of the area, and it is shown unless the area is
// empty or the view is hidden (the view and its superviews and the window must be shown).
static void sizeSurface(AGLContext ctx)
{
	if (!ctx->surface)
		return;
	CGRect area = surfaceArea(ctx);
	int shown = !CGRectIsNull(area) && area.size.width >= 1 && area.size.height >= 1
		&& (!ctx->view || HIViewIsVisible(ctx->view));
	CGSConnectionID connection = CGSMainConnectionID();
	if (shown && !CGRectEqualToRect(area, ctx->surfaceRect)) {
		CGSSetSurfaceBounds(connection, ctx->surfaceWindow, ctx->surface, area);
		ctx->surfaceRect = area;
	}
	if (shown != ctx->surfaceShown) {
		CGSOrderSurface(connection, ctx->surfaceWindow, ctx->surface, shown ? (ctx->surfaceOrder < 0 ? -1 : 1) : 0, 0);
		ctx->surfaceShown = shown;
	}
}

// The surface of the window stays until the context is given another or is destroyed
static void dropSurface(AGLContext ctx)
{
	if (ctx->surface) {
		CGLClearDrawable(ctx->cgl);
		CGSRemoveSurface(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface);
	}
	ctx->surface = 0;
	ctx->surfaceWindow = 0;
	ctx->surfaceShown = 0;
	ctx->surfaceRect = CGRectNull;
}

// The views that an HIView drawable follows tell when their bounds or visibility change
static OSStatus viewChanged(EventHandlerCallRef next, EventRef event, void *context)
{
	(void) next;
	(void) event;
	sizeSurface((AGLContext) context);
	return eventNotHandledErr;
}

static void unwatchViews(AGLContext ctx)
{
	for (int i = 0; i < ctx->watchedCount; i++)
		RemoveEventHandler(ctx->watched[i].handler);
	ctx->watchedCount = 0;
	ctx->view = NULL;
}

static void watchViews(AGLContext ctx, HIViewRef view)
{
	static const EventTypeSpec types[] = {
		{ kEventClassControl, kEventControlBoundsChanged },
		{ kEventClassControl, kEventControlVisibilityChanged },
	};
	static EventHandlerUPP handler;
	if (!handler)
		handler = NewEventHandlerUPP(viewChanged);
	ctx->view = view;
	// a superview that moves or hides takes the view along without telling it
	for (HIViewRef v = view; v && ctx->watchedCount < kMaxWatchedViews; v = HIViewGetSuperview(v)) {
		if (InstallControlEventHandler(v, handler, 2, types, ctx, &ctx->watched[ctx->watchedCount].handler) == noErr)
			ctx->watched[ctx->watchedCount++].view = v;
	}
}

// Leaves the full-screen display: it is released, and shows the mode it had before.
static void releaseFullScreen(AGLContext ctx)
{
	if (ctx->fullScreenDisplay == kCGNullDirectDisplay)
		return;
	if (ctx->fullScreenSavedMode) {
		CGDisplaySetDisplayMode(ctx->fullScreenDisplay, ctx->fullScreenSavedMode, NULL);
		CGDisplayModeRelease(ctx->fullScreenSavedMode);
		ctx->fullScreenSavedMode = NULL;
	}
	if (ctx->fullScreenCapturedAll)
		CGReleaseAllDisplays();
	else
		CGDisplayRelease(ctx->fullScreenDisplay);
	ctx->fullScreenDisplay = kCGNullDirectDisplay;
	ctx->fullScreenCapturedAll = 0;
}

// What the context was given to draw to other than a window surface: the HIView it follows, the display it holds
// full screen. The caller changes the CGL drawable.
static void releaseDrawables(AGLContext ctx)
{
	unwatchViews(ctx);
	releaseFullScreen(ctx);
}

// The drawable is replaced by one that is not a window: the window surface goes (CGL has the new drawable already)
static void leaveWindow(AGLContext ctx)
{
	if (ctx->surface)
		CGSRemoveSurface(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface);
	ctx->surface = 0;
	ctx->surfaceWindow = 0;
	ctx->surfaceShown = 0;
	ctx->surfaceRect = CGRectNull;
	ctx->window = NULL;
	ctx->drawable = NULL;
	unwatchViews(ctx);
}

// The surface of the window, which the context draws to, in place of its drawable. view is the HIView that the
// surface covers, if there is one.
static GLboolean attachWindow(AGLContext ctx, WindowRef window, HIViewRef view)
{
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
	releaseDrawables(ctx);
	ctx->window = window;
	ctx->drawable = NULL;
	ctx->surfaceWindow = wid;
	ctx->surface = surface;
	ctx->surfaceShown = 0;
	ctx->surfaceRect = CGRectNull;
	if (view)
		watchViews(ctx, view);
	sizeSurface(ctx);
	return GL_TRUE;
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
		releaseDrawables(ctx);
		ctx->window = NULL;
		ctx->drawable = NULL;
		return GL_TRUE;
	}
	return attachWindow(ctx, window, NULL);
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

// The view's window, which the surface is of, and the bounds of the view, which the surface covers
AGL_EXPORT GLboolean aglSetHIViewRef(AGLContext ctx, HIViewRef hiview)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!hiview)
		return aglSetWindowRef(ctx, NULL);
	WindowRef window = HIViewGetWindow(hiview);
	if (!window) {
		failed(AGL_BAD_DRAWABLE);
		return GL_FALSE;
	}
	return attachWindow(ctx, window, hiview);
}

AGL_EXPORT HIViewRef aglGetHIViewRef(AGLContext ctx)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return NULL;
	}
	return ctx->view;
}

// The rendering goes to memory of the client's, which holds 32 bit pixels
AGL_EXPORT GLboolean aglSetOffScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei rowbytes, GLvoid *baseaddr)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!cglSucceeded(CGLSetOffScreen(ctx->cgl, width, height, rowbytes, baseaddr)))
		return GL_FALSE;
	releaseFullScreen(ctx);
	leaveWindow(ctx);
	return GL_TRUE;
}

// The display of aglSetFullScreen's device: 0 is the display the pixel format was made for, or the main display
static CGDirectDisplayID fullScreenDisplayOf(AGLContext ctx, GLint device)
{
	if (device == 0) {
		GLint mask = 0;
		if (CGLDescribePixelFormat(ctx->format->cgl, 0, kCGLPFADisplayMask, &mask) == kCGLNoError && mask != 0) {
			CGDirectDisplayID display = CGOpenGLDisplayMaskToDisplayID((CGOpenGLDisplayMask) mask);
			if (display != kCGNullDirectDisplay)
				return display;
		}
		return CGMainDisplayID();
	}
	CGDirectDisplayID displays[32];
	uint32_t count = 0;
	if (CGGetActiveDisplayList(32, displays, &count) == kCGErrorSuccess)
		for (uint32_t i = 0; i < count; i++)
			if (displays[i] == (CGDirectDisplayID) device)
				return displays[i];
	return kCGNullDirectDisplay;
}

// A display mode of the display with the size and, unless it is 0, the refresh rate, which the display can show
static CGDisplayModeRef copyFullScreenMode(CGDirectDisplayID display, GLsizei width, GLsizei height, GLsizei freq)
{
	CGDisplayModeRef found = NULL;
	CFArrayRef modes = CGDisplayCopyAllDisplayModes(display, NULL);
	for (CFIndex i = 0; modes && i < CFArrayGetCount(modes) && !found; i++) {
		CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
		if (CGDisplayModeGetWidth(mode) != (size_t) width || CGDisplayModeGetHeight(mode) != (size_t) height)
			continue;
		if (!CGDisplayModeIsUsableForDesktopGUI(mode))
			continue;
		if (freq != 0 && fabs(CGDisplayModeGetRefreshRate(mode) - freq) > 0.5 && CGDisplayModeGetRefreshRate(mode) != 0)
			continue;
		found = CGDisplayModeRetain(mode);
	}
	if (modes)
		CFRelease(modes);
	return found;
}

// The context draws to the whole display. The display is captured first (all displays, unless AGL_FS_CAPTURE_SINGLE
// is enabled), and shows the size and refresh rate asked for unless they are 0; the display is released, with its old
// mode back, when the context is given another drawable or is destroyed.
AGL_EXPORT GLboolean aglSetFullScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei freq, GLint device)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!requestedFlag(ctx->format, AGL_FULLSCREEN)) {
		failed(AGL_BAD_FULLSCREEN);
		return GL_FALSE;
	}
	if (width < 0 || height < 0 || freq < 0 || (width == 0) != (height == 0)) {
		failed(AGL_BAD_VALUE);
		return GL_FALSE;
	}
	CGDirectDisplayID display = fullScreenDisplayOf(ctx, device);
	if (display == kCGNullDirectDisplay) {
		failed(AGL_BAD_GDEV);
		return GL_FALSE;
	}
	CGDisplayModeRef wanted = NULL;
	if (width != 0) {
		CGDisplayModeRef current = CGDisplayCopyDisplayMode(display);
		int already = current && CGDisplayModeGetWidth(current) == (size_t) width
			&& CGDisplayModeGetHeight(current) == (size_t) height
			&& (freq == 0 || CGDisplayModeGetRefreshRate(current) == 0 || fabs(CGDisplayModeGetRefreshRate(current) - freq) <= 0.5);
		if (current)
			CGDisplayModeRelease(current);
		if (!already) {
			wanted = copyFullScreenMode(display, width, height, freq);
			if (!wanted) {
				failed(AGL_BAD_VALUE);
				return GL_FALSE;
			}
		}
	}

	// the display of an earlier full-screen drawable is let go of first
	releaseFullScreen(ctx);
	int captureAll = !ctx->enabled[kEnableFSCaptureSingle];
	if ((captureAll ? CGCaptureAllDisplays() : CGDisplayCapture(display)) != kCGErrorSuccess) {
		if (wanted)
			CGDisplayModeRelease(wanted);
		failed(AGL_BAD_FULLSCREEN);
		return GL_FALSE;
	}
	ctx->fullScreenDisplay = display;
	ctx->fullScreenCapturedAll = captureAll;
	if (wanted) {
		CGDisplayModeRef previous = CGDisplayCopyDisplayMode(display);
		if (CGDisplaySetDisplayMode(display, wanted, NULL) == kCGErrorSuccess)
			ctx->fullScreenSavedMode = previous;
		else if (previous)
			CGDisplayModeRelease(previous);
		CGDisplayModeRelease(wanted);
	}
	CGLError error = CGLSetFullScreenOnDisplay(ctx->cgl, CGDisplayIDToOpenGLDisplayMask(display));
	if (error != kCGLNoError) {
		releaseFullScreen(ctx);
		failed((GLenum) error);
		return GL_FALSE;
	}
	leaveWindow(ctx);
	return GL_TRUE;
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
	// an HIView drawable may have been moved by a change that sent no event (its window's layout)
	if (ctx->view)
		sizeSurface(ctx);
	// a hidden drawable shows nothing, where a swap on some window systems would show it again
	if (ctx->surface && !ctx->surfaceShown) {
		glFlush();
		return;
	}
	CGLFlushDrawable(ctx->cgl);
}

// The options of aglEnable that CGL has an enable for are CGL's (their numbers differ from AGL's); the others are
// remembered by the context
static int cglEnableOf(GLenum pname)
{
	switch (pname) {
		case AGL_SWAP_RECT: return kCGLCESwapRectangle;
		case AGL_SWAP_LIMIT: return kCGLCESwapLimit;
		case AGL_RASTERIZATION: return kCGLCERasterization;
		case AGL_STATE_VALIDATION: return kCGLCEStateValidation;
		case AGL_ENABLE_SURFACE_BACKING_SIZE: return kCGLCESurfaceBackingSize;
		default: return 0;
	}
}

static int enableIndex(GLenum pname)
{
	switch (pname) {
		case AGL_BUFFER_RECT: return kEnableBufferRect;
		case AGL_COLORMAP_TRACKING: return 3;
		case AGL_CLIP_REGION: return 5;
		case AGL_FS_CAPTURE_SINGLE: return kEnableFSCaptureSingle;
		case AGL_SURFACE_VOLATILE: return 8;
		default: return -1;
	}
}

static GLboolean setEnabled(AGLContext ctx, GLenum pname, GLboolean on)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	int cgl = cglEnableOf(pname);
	if (cgl)
		return cglSucceeded(on ? CGLEnable(ctx->cgl, (CGLContextEnable) cgl) : CGLDisable(ctx->cgl, (CGLContextEnable) cgl));
	int index = enableIndex(pname);
	if (index < 0) {
		failed(AGL_BAD_ENUM);
		return GL_FALSE;
	}
	ctx->enabled[index] = on;
	if (index == kEnableBufferRect)
		sizeSurface(ctx);
	return GL_TRUE;
}

AGL_EXPORT GLboolean aglEnable(AGLContext ctx, GLenum pname)
{
	return setEnabled(ctx, pname, GL_TRUE);
}

AGL_EXPORT GLboolean aglDisable(AGLContext ctx, GLenum pname)
{
	return setEnabled(ctx, pname, GL_FALSE);
}

AGL_EXPORT GLboolean aglIsEnabled(AGLContext ctx, GLenum pname)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	int cgl = cglEnableOf(pname);
	if (cgl) {
		GLint enabled = 0;
		return cglSucceeded(CGLIsEnabled(ctx->cgl, (CGLContextEnable) cgl, &enabled)) && enabled;
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
			sizeSurface(ctx);
			return GL_TRUE;
		case AGL_SWAP_RECT:
			memcpy(ctx->swapRect, params, sizeof(ctx->swapRect));
			return GL_TRUE;
		case AGL_SURFACE_ORDER:
			// above (1) or below (-1) the contents of the window
			ctx->surfaceOrder = *params;
			if (ctx->surface && ctx->surfaceShown)
				CGSOrderSurface(CGSMainConnectionID(), ctx->surfaceWindow, ctx->surface, *params < 0 ? -1 : 1, 0);
			return GL_TRUE;
		case AGL_SWAP_LIMIT: case AGL_COLORMAP_TRACKING: case AGL_STATE_VALIDATION: case AGL_ORDER_CONTEXT_TO_FRONT:
		case AGL_RASTERIZATION: case AGL_SURFACE_VOLATILE:
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
		case AGL_SURFACE_ORDER:
			*params = ctx->surfaceOrder;
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

// A pixel buffer is a CGL one
AGL_EXPORT GLboolean aglCreatePBuffer(GLint width, GLint height, GLenum target, GLenum internalFormat, long max_level, AGLPbuffer *pbuffer)
{
	return cglSucceeded(CGLCreatePBuffer(width, height, target, internalFormat, (GLint) max_level, (CGLPBufferObj *) pbuffer));
}

AGL_EXPORT GLboolean aglDestroyPBuffer(AGLPbuffer pbuffer)
{
	return cglSucceeded(CGLDestroyPBuffer((CGLPBufferObj) pbuffer));
}

AGL_EXPORT GLboolean aglDescribePBuffer(AGLPbuffer pbuffer, GLint *width, GLint *height, GLenum *target, GLenum *internalFormat, GLint *max_level)
{
	GLsizei w, h;
	if (!cglSucceeded(CGLDescribePBuffer((CGLPBufferObj) pbuffer, &w, &h, target, internalFormat, max_level)))
		return GL_FALSE;
	if (width)
		*width = w;
	if (height)
		*height = h;
	return GL_TRUE;
}

// Makes the contents of the pixel buffer the image of the texture bound in the context
AGL_EXPORT GLboolean aglTexImagePBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint source)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	return cglSucceeded(CGLTexImagePBuffer(ctx->cgl, (CGLPBufferObj) pbuffer, (GLenum) source));
}

AGL_EXPORT GLboolean aglSetPBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint face, GLint level, GLint screen)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	if (!cglSucceeded(CGLSetPBuffer(ctx->cgl, (CGLPBufferObj) pbuffer, (GLenum) face, level, screen)))
		return GL_FALSE;
	releaseFullScreen(ctx);
	leaveWindow(ctx);
	return GL_TRUE;
}

AGL_EXPORT GLboolean aglGetPBuffer(AGLContext ctx, AGLPbuffer *pbuffer, GLint *face, GLint *level, GLint *screen)
{
	if (!ctx) {
		failed(AGL_BAD_CONTEXT);
		return GL_FALSE;
	}
	GLenum cglFace = 0;
	if (!cglSucceeded(CGLGetPBuffer(ctx->cgl, (CGLPBufferObj *) pbuffer, &cglFace, level, screen)))
		return GL_FALSE;
	if (face)
		*face = (GLint) cglFace;
	return GL_TRUE;
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
