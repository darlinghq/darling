/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#ifndef _OpenGL_H_
#define _OpenGL_H_

#include <OpenGL/OpenGLAvailability.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLDevice.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/gltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

OPENGL_ASSUME_NONNULL_BEGIN

#define CGL_VERSION_1_0  1
#define CGL_VERSION_1_1  1
#define CGL_VERSION_1_2  1
#define CGL_VERSION_1_3  1

extern CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute *attribs, CGLPixelFormatObj OPENGL_NULLABLE * OPENGL_NONNULL pix, GLint *npix);
extern CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix);
extern CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint *value);
extern void CGLReleasePixelFormat(CGLPixelFormatObj pix) OPENGL_AVAILABLE(10_5);
extern CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pix) OPENGL_AVAILABLE(10_5);
extern GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pix) OPENGL_AVAILABLE(10_5);

extern CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj OPENGL_NULLABLE * OPENGL_NONNULL rend, GLint *nrend);
extern CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend);
extern CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint * OPENGL_NULLABLE value);

extern CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj OPENGL_NULLABLE share, CGLContextObj OPENGL_NULLABLE * OPENGL_NONNULL ctx);
extern CGLError CGLDestroyContext(CGLContextObj ctx);
extern CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask) OPENGL_DEPRECATED(10_0, 10_8);
extern CGLContextObj CGLRetainContext(CGLContextObj ctx) OPENGL_AVAILABLE(10_5);
extern void CGLReleaseContext(CGLContextObj ctx) OPENGL_AVAILABLE(10_5);
extern GLuint CGLGetContextRetainCount(CGLContextObj ctx) OPENGL_AVAILABLE(10_5);
extern CGLPixelFormatObj OPENGL_NULLABLE CGLGetPixelFormat(CGLContextObj ctx) OPENGL_AVAILABLE(10_5);

extern CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj OPENGL_NULLABLE * OPENGL_NONNULL pbuffer) OPENGL_DEPRECATED(10_3, 10_7);
extern CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer) OPENGL_DEPRECATED(10_3, 10_7);
extern CGLError CGLDescribePBuffer(CGLPBufferObj obj, GLsizei *width, GLsizei *height, GLenum *target, GLenum *internalFormat, GLint *mipmap) OPENGL_DEPRECATED(10_3, 10_7);
extern CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source) OPENGL_DEPRECATED(10_3, 10_7);
extern CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer) OPENGL_DEPRECATED(10_5, 10_7);
extern void CGLReleasePBuffer(CGLPBufferObj pbuffer) OPENGL_DEPRECATED(10_5, 10_7);
extern GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer) OPENGL_DEPRECATED(10_5, 10_7);

extern CGLError CGLSetOffScreen(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void *baseaddr) OPENGL_DEPRECATED(10_0, 10_7);
extern CGLError CGLGetOffScreen(CGLContextObj ctx, GLsizei *width, GLsizei *height, GLint *rowbytes, void * OPENGL_NULLABLE * OPENGL_NONNULL baseaddr) OPENGL_DEPRECATED(10_0, 10_7);
extern CGLError CGLSetFullScreen(CGLContextObj ctx) OPENGL_DEPRECATED(10_0, 10_6);
extern CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask) OPENGL_DEPRECATED(10_5, 10_7);

extern CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen) OPENGL_DEPRECATED(10_3, 10_7);
extern CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj OPENGL_NULLABLE * OPENGL_NONNULL pbuffer, GLenum *face, GLint *level, GLint *screen) OPENGL_DEPRECATED(10_3, 10_7);

extern CGLError CGLClearDrawable(CGLContextObj ctx);
extern CGLError CGLFlushDrawable(CGLContextObj ctx);

extern CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname);
extern CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname);
extern CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint *enable);
extern CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const GLint *params);
extern CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, GLint *params);

extern CGLError CGLSetVirtualScreen(CGLContextObj ctx, GLint screen);
extern CGLError CGLGetVirtualScreen(CGLContextObj ctx, GLint *screen);

extern CGLError CGLUpdateContext(CGLContextObj ctx) OPENGL_AVAILABLE(10_7);

extern CGLError CGLSetGlobalOption(CGLGlobalOption pname, const GLint * OPENGL_NULLABLE params) OPENGL_AVAILABLE(10_6);
extern CGLError CGLGetGlobalOption(CGLGlobalOption pname, GLint *params) OPENGL_AVAILABLE(10_6);
extern CGLError CGLSetOption(CGLGlobalOption pname, GLint param);
extern CGLError CGLGetOption(CGLGlobalOption pname, GLint *param);

extern CGLError CGLLockContext(CGLContextObj ctx) OPENGL_AVAILABLE(10_4);

extern CGLError CGLUnlockContext(CGLContextObj ctx) OPENGL_AVAILABLE(10_4);

extern void CGLGetVersion(GLint * OPENGL_NULLABLE majorvers, GLint * OPENGL_NULLABLE minorvers);

const char *CGLErrorString(CGLError error);

OPENGL_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif
