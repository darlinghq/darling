#ifndef _CGLIOSURFACE_H
#define _CGLIOSURFACE_H

#include <AvailabilityMacros.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/gltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

OPENGL_ASSUME_NONNULL_BEGIN

typedef struct  OPENGL_BRIDGED_TYPE(id) __IOSurface *IOSurfaceRef OPENGL_SWIFT_NAME(IOSurfaceRef);

extern CGLError CGLTexImageIOSurface2D(CGLContextObj ctx, GLenum target, GLenum internal_format,
						GLsizei width, GLsizei height, GLenum format, GLenum type, IOSurfaceRef ioSurface, GLuint plane) OPENGL_AVAILABLE(10_6);

OPENGL_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif