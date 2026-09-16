#ifndef _CGL_INTERNAL_H_
#define _CGL_INTERNAL_H_

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
CGL_EXPORT void CGLDestroyWindow(CGLWindowRef window);

CGL_EXPORT CGLError CGLContextMakeCurrentAndAttachToWindow(CGLContextObj context, CGLWindowRef window);
CGLError CGLSetSurface(CGLContextObj gl, CGSConnectionID cid, CGSWindowID wid, CGSSurfaceID sid);
CGLContextObj CGWindowContextCreate(CGSConnectionID cid, CGSWindowID wid, CFDictionaryRef options);

#ifdef __cplusplus
}
#endif

#endif /* _CGL_INTERNAL_H_ */
