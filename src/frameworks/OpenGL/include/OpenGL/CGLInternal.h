#ifndef _CGL_INTERNAL_H_
#define _CGL_INTERNAL_H_

#include <stdint.h>
#include <OpenGL/OpenGL.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNotificationCenter.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CGLWindowRef;

CGL_EXPORT CGLError CGLRegisterNativeDisplay(void *native_display);
// platform is an EGL platform enum; native_display must match that platform.
CGL_EXPORT CGLError CGLRegisterNativeDisplayForPlatform(void *native_display, unsigned int platform);

CGL_EXPORT CGLWindowRef CGLGetWindow(void *native_window);
CGL_EXPORT CGLWindowRef CGLGetWindowForContext(CGLContextObj context, void *native_window);
// A window surface with an alpha channel on the registered display, for a
// native window created with the given visual (an X11 visual ID). A visual_id
// of 0 accepts any alpha-capable configuration.
CGL_EXPORT CGLWindowRef CGLGetWindowWithAlpha(void *native_window, unsigned long visual_id);
CGL_EXPORT void CGLDestroyWindow(CGLWindowRef window);

// Posted to CoreFoundation's local notification center, on the swapping
// thread, after CGLFlushDrawable has swapped a surface made by CGLGetWindow*.
// The object is the native_window the surface was made for. A window-system
// backend that has to do work after a swap (a Wayland subsurface commit)
// observes it, so that a client that swaps through CGL directly, not through
// AppKit, is presented too.
#define kCGLWindowDidSwapNotification CFSTR("CGLWindowDidSwapNotification")

// Refresh rate of the output the context presents on, in millihertz (the unit
// wl_output reports). A Wayland context keeps its swap interval in software
// and paces swaps to this rate; 0 selects the 60 Hz default. No effect on
// contexts of other platforms.
CGL_EXPORT CGLError CGLSetDrawableRefreshRate(CGLContextObj context, uint32_t millihertz);

// A context that can be made current on the window: one created with the
// display and configuration of the window's surface.
CGL_EXPORT CGLError CGLCreateContextForWindow(CGLWindowRef window, CGLContextObj *result);
CGL_EXPORT CGLError CGLContextMakeCurrentAndAttachToWindow(CGLContextObj context, CGLWindowRef window);
CGLError CGLSetSurface(CGLContextObj gl, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid);
// The window server connection, window and surface ids of the context's drawable; zeros when
// it has none. NULL output pointers are skipped. Private in macOS, but real applications use it
// together with CGSGetSurfaceBounds() and CGSGetWindowBounds().
CGLError CGLGetSurface(CGLContextObj gl, CGSConnectionID *cid, CGSWindowID *wid, CGSSurfaceID *sid);

// Copies level 0 of the texture last specified from ioSurface with
// CGLTexImageIOSurface2D back into the surface plane, in that call's format
// and type. The surface's seed changes, as after any write.
CGL_EXPORT CGLError CGLReadbackIOSurface(IOSurfaceRef ioSurface);
CGLContextObj CGWindowContextCreate(CGSConnectionID cid, CGSWindowID wid, CFDictionaryRef options);

#ifdef __cplusplus
}
#endif

#endif /* _CGL_INTERNAL_H_ */
