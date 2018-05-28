#ifndef _CGL_INTERNAL_H_
#define _CGL_INTERNAL_H_

#include <OpenGL/OpenGL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CGLWindowRef;

CGL_EXPORT CGLError CGLRegisterNativeDisplay(void *native_display);

CGL_EXPORT CGLWindowRef CGLGetWindow(void *native_window);
CGL_EXPORT void CGLDestroyWindow(CGLWindowRef window);

CGL_EXPORT CGLError CGLContextMakeCurrentAndAttachToWindow(CGLContextObj context, CGLWindowRef window);
CGL_EXPORT CGLError CGLSwapBuffers(CGLWindowRef window);

#ifdef __cplusplus
}
#endif

#endif /* _CGL_INTERNAL_H_ */
