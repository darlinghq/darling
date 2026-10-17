#ifndef _CGL_INTERNAL_H_
#define _CGL_INTERNAL_H_

#include <stdint.h>
#include <OpenGL/OpenGL.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <CoreFoundation/CFDictionary.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CGLWindowRef;

CGL_EXPORT CGLError CGLRegisterNativeDisplay(void *native_display);
// platform is an EGL platform enum; native_display must match that platform.
CGL_EXPORT CGLError CGLRegisterNativeDisplayForPlatform(void *native_display, unsigned int platform);

CGL_EXPORT CGLWindowRef CGLGetWindow(void *native_window);
CGL_EXPORT CGLWindowRef CGLGetWindowForContext(CGLContextObj context, void *native_window);
CGL_EXPORT void CGLDestroyWindow(CGLWindowRef window);

// Refresh rate of the output the context presents on, in millihertz (the unit
// wl_output reports). A Wayland context keeps its swap interval in software
// and paces swaps to this rate; 0 selects the 60 Hz default. No effect on
// contexts of other platforms.
CGL_EXPORT CGLError CGLSetDrawableRefreshRate(CGLContextObj context, uint32_t millihertz);

CGL_EXPORT CGLError CGLContextMakeCurrentAndAttachToWindow(CGLContextObj context, CGLWindowRef window);
CGLError CGLSetSurface(CGLContextObj gl, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid);
CGLContextObj CGWindowContextCreate(CGSConnectionID cid, CGSWindowID wid, CFDictionaryRef options);

#ifdef __cplusplus
}
#endif

#endif /* _CGL_INTERNAL_H_ */
