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


#include <OpenGL/OpenGL.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void)  {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute *attribs, CGLPixelFormatObj OPENGL_NULLABLE * OPENGL_NONNULL pix, GLint *npix)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint *value)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
void CGLReleasePixelFormat(CGLPixelFormatObj pix)  {
    if (verbose) puts("STUB:  called");
}
CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pix)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pix)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj OPENGL_NULLABLE * OPENGL_NONNULL rend, GLint *nrend)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint * OPENGL_NULLABLE value)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj OPENGL_NULLABLE share, CGLContextObj OPENGL_NULLABLE * OPENGL_NONNULL ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDestroyContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLContextObj CGLRetainContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
void CGLReleaseContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
}
GLuint CGLGetContextRetainCount(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLPixelFormatObj OPENGL_NULLABLE CGLGetPixelFormat(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj OPENGL_NULLABLE * OPENGL_NONNULL pbuffer)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDescribePBuffer(CGLPBufferObj obj, GLsizei *width, GLsizei *height, GLenum *target, GLenum *internalFormat, GLint *mipmap)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
void CGLReleasePBuffer(CGLPBufferObj pbuffer)  {
    if (verbose) puts("STUB:  called");
}
GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLSetOffScreen(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void *baseaddr)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetOffScreen(CGLContextObj ctx, GLsizei *width, GLsizei *height, GLint *rowbytes, void * OPENGL_NULLABLE * OPENGL_NONNULL baseaddr)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLSetFullScreen(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLSetFullScreenOnDisplay(CGLContextObj ctx, GLuint display_mask)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj OPENGL_NULLABLE * OPENGL_NONNULL pbuffer, GLenum *face, GLint *level, GLint *screen)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLClearDrawable(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLFlushDrawable(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint *enable)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const GLint *params)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, GLint *params)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLSetVirtualScreen(CGLContextObj ctx, GLint screen)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetVirtualScreen(CGLContextObj ctx, GLint *screen)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLUpdateContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLSetGlobalOption(CGLGlobalOption pname, const GLint * OPENGL_NULLABLE params)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetGlobalOption(CGLGlobalOption pname, GLint *params)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLSetOption(CGLGlobalOption pname, GLint param)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}
CGLError CGLGetOption(CGLGlobalOption pname, GLint *param)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLLockContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

CGLError CGLUnlockContext(CGLContextObj ctx)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

void CGLGetVersion(GLint * OPENGL_NULLABLE majorvers, GLint * OPENGL_NULLABLE minorvers)  {
    if (verbose) puts("STUB:  called");
}

const char *CGLErrorString(CGLError error)  {
    if (verbose) puts("STUB:  called");
    return NULL;
}

void glAccum (GLenum op, GLfloat value) {
    if (verbose) puts("STUB: OpenGL called");
}

void glAlphaFunc (GLenum func, GLclampf ref) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences) {
    if (verbose) puts("STUB: OpenGL called");
}


void glArrayElement (GLint i) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBegin (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBindTexture (GLenum target, GLuint texture) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBlendEquation (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBlendFunc (GLenum sfactor, GLenum dfactor) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCallList (GLuint list) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCallLists (GLsizei n, GLenum type, const GLvoid *lists) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClear (GLbitfield mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClearDepth (GLclampd depth) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClearIndex (GLfloat c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClearStencil (GLint s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClipPlane (GLenum plane, const GLdouble *equation) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3b (GLbyte red, GLbyte green, GLbyte blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3bv (const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3d (GLdouble red, GLdouble green, GLdouble blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3f (GLfloat red, GLfloat green, GLfloat blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3i (GLint red, GLint green, GLint blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3s (GLshort red, GLshort green, GLshort blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3ub (GLubyte red, GLubyte green, GLubyte blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3ubv (const GLubyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3ui (GLuint red, GLuint green, GLuint blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3uiv (const GLuint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3us (GLushort red, GLushort green, GLushort blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor3usv (const GLushort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4bv (const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4i (GLint red, GLint green, GLint blue, GLint alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4ubv (const GLubyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4uiv (const GLuint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColor4usv (const GLushort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorMaterial (GLenum face, GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorSubTable (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorTable (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorTableParameterfv (GLenum target, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glColorTableParameteriv (GLenum target, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionFilter1D (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionParameterf (GLenum target, GLenum pname, GLfloat params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionParameterfv (GLenum target, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionParameteri (GLenum target, GLenum pname, GLint params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glConvolutionParameteriv (GLenum target, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyColorSubTable (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyColorTable (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyConvolutionFilter1D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyConvolutionFilter2D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyTexImage1D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCullFace (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteLists (GLuint list, GLsizei range) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteTextures (GLsizei n, const GLuint *textures) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDepthFunc (GLenum func) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDepthMask (GLboolean flag) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDepthRange (GLclampd zNear, GLclampd zFar) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDisable (GLenum cap) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDisableClientState (GLenum array) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDrawArrays (GLenum mode, GLint first, GLsizei count) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDrawBuffer (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEdgeFlag (GLboolean flag) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEdgeFlagv (const GLboolean *flag) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEnable (GLenum cap) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEnableClientState (GLenum array) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEnd (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEndList (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord1d (GLdouble u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord1dv (const GLdouble *u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord1f (GLfloat u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord1fv (const GLfloat *u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord2d (GLdouble u, GLdouble v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord2dv (const GLdouble *u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord2f (GLfloat u, GLfloat v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalCoord2fv (const GLfloat *u) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalMesh1 (GLenum mode, GLint i1, GLint i2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalPoint1 (GLint i) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEvalPoint2 (GLint i, GLint j) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFinish (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFlush (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogf (GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogfv (GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogi (GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogiv (GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFrontFace (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    if (verbose) puts("STUB: OpenGL called");
}


GLuint glGenLists (GLsizei range) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGenTextures (GLsizei n, GLuint *textures) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetBooleanv (GLenum pname, GLboolean *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetClipPlane (GLenum plane, GLdouble *equation) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetColorTable (GLenum target, GLenum format, GLenum type, GLvoid *table) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetColorTableParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetColorTableParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetConvolutionFilter (GLenum target, GLenum format, GLenum type, GLvoid *image) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetConvolutionParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetConvolutionParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetDoublev (GLenum pname, GLdouble *params) {
    if (verbose) puts("STUB: OpenGL called");
}


GLenum glGetError (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetFloatv (GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetHistogram (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetHistogramParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetHistogramParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetIntegerv (GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetLightfv (GLenum light, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetLightiv (GLenum light, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMapdv (GLenum target, GLenum query, GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMapfv (GLenum target, GLenum query, GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMapiv (GLenum target, GLenum query, GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMaterialiv (GLenum face, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMinmax (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMinmaxParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetMinmaxParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetPixelMapfv (GLenum map, GLfloat *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetPixelMapuiv (GLenum map, GLuint *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetPixelMapusv (GLenum map, GLushort *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetPointerv (GLenum pname, GLvoid **params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetPolygonStipple (GLubyte *mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetSeparableFilter (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span) {
    if (verbose) puts("STUB: OpenGL called");
}


const GLubyte *glGetString (GLenum name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexEnviv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexGeniv (GLenum coord, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glHint (GLenum target, GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glHistogram (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexMask (GLuint mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexd (GLdouble c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexdv (const GLdouble *c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexf (GLfloat c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexfv (const GLfloat *c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexi (GLint c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexiv (const GLint *c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexs (GLshort c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexsv (const GLshort *c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexub (GLubyte c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glIndexubv (const GLubyte *c) {
    if (verbose) puts("STUB: OpenGL called");
}


void glInitNames (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsEnabled (GLenum cap) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsList (GLuint list) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsTexture (GLuint texture) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightModelf (GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightModelfv (GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightModeli (GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightModeliv (GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightf (GLenum light, GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightfv (GLenum light, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLighti (GLenum light, GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLightiv (GLenum light, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLineStipple (GLint factor, GLushort pattern) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLineWidth (GLfloat width) {
    if (verbose) puts("STUB: OpenGL called");
}


void glListBase (GLuint base) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLoadIdentity (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLoadMatrixd (const GLdouble *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLoadMatrixf (const GLfloat *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLoadName (GLuint name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLogicOp (GLenum opcode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMapGrid1d (GLint un, GLdouble u1, GLdouble u2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMapGrid1f (GLint un, GLfloat u1, GLfloat u2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMaterialf (GLenum face, GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMaterialfv (GLenum face, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMateriali (GLenum face, GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMaterialiv (GLenum face, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMatrixMode (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMinmax (GLenum target, GLenum internalformat, GLboolean sink) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultMatrixd (const GLdouble *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultMatrixf (const GLfloat *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNewList (GLuint list, GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3bv (const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3i (GLint nx, GLint ny, GLint nz) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3s (GLshort nx, GLshort ny, GLshort nz) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormal3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPassThrough (GLfloat token) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelMapfv (GLenum map, GLint mapsize, const GLfloat *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelMapuiv (GLenum map, GLint mapsize, const GLuint *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelMapusv (GLenum map, GLint mapsize, const GLushort *values) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelStoref (GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelStorei (GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelTransferf (GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelTransferi (GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPixelZoom (GLfloat xfactor, GLfloat yfactor) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPointSize (GLfloat size) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPolygonMode (GLenum face, GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPolygonOffset (GLfloat factor, GLfloat units) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPolygonStipple (const GLubyte *mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPopAttrib (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPopClientAttrib (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPopMatrix (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPopName (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPushAttrib (GLbitfield mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPushClientAttrib (GLbitfield mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPushMatrix (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPushName (GLuint name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2d (GLdouble x, GLdouble y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2f (GLfloat x, GLfloat y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2i (GLint x, GLint y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2s (GLshort x, GLshort y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos2sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3d (GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3f (GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3i (GLint x, GLint y, GLint z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3s (GLshort x, GLshort y, GLshort z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4i (GLint x, GLint y, GLint z, GLint w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRasterPos4sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glReadBuffer (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectdv (const GLdouble *v1, const GLdouble *v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectfv (const GLfloat *v1, const GLfloat *v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRecti (GLint x1, GLint y1, GLint x2, GLint y2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectiv (const GLint *v1, const GLint *v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRectsv (const GLshort *v1, const GLshort *v2) {
    if (verbose) puts("STUB: OpenGL called");
}


GLint glRenderMode (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glResetHistogram (GLenum target) {
    if (verbose) puts("STUB: OpenGL called");
}


void glResetMinmax (GLenum target) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glScaled (GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glScalef (GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSelectBuffer (GLsizei size, GLuint *buffer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column) {
    if (verbose) puts("STUB: OpenGL called");
}


void glShadeModel (GLenum mode) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilFunc (GLenum func, GLint ref, GLuint mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilMask (GLuint mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1d (GLdouble s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1f (GLfloat s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1i (GLint s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1s (GLshort s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord1sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2d (GLdouble s, GLdouble t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2f (GLfloat s, GLfloat t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2i (GLint s, GLint t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2s (GLshort s, GLshort t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord2sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3d (GLdouble s, GLdouble t, GLdouble r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3f (GLfloat s, GLfloat t, GLfloat r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3i (GLint s, GLint t, GLint r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3s (GLshort s, GLshort t, GLshort r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4i (GLint s, GLint t, GLint r, GLint q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoord4sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexEnvf (GLenum target, GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexEnvi (GLenum target, GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexEnviv (GLenum target, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGend (GLenum coord, GLenum pname, GLdouble param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGendv (GLenum coord, GLenum pname, const GLdouble *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGenf (GLenum coord, GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGeni (GLenum coord, GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexGeniv (GLenum coord, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexParameteri (GLenum target, GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexParameteriv (GLenum target, GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTranslated (GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glTranslatef (GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2d (GLdouble x, GLdouble y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2f (GLfloat x, GLfloat y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2i (GLint x, GLint y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2s (GLshort x, GLshort y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex2sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3d (GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3f (GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3i (GLint x, GLint y, GLint z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3s (GLshort x, GLshort y, GLshort z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4i (GLint x, GLint y, GLint z, GLint w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertex4sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glViewport (GLint x, GLint y, GLsizei width, GLsizei height) {
    if (verbose) puts("STUB: OpenGL called");
}



void glSampleCoverage (GLclampf value, GLboolean invert) {
    if (verbose) puts("STUB: OpenGL called");
}



void glLoadTransposeMatrixf (const GLfloat *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLoadTransposeMatrixd (const GLdouble *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultTransposeMatrixf (const GLfloat *m) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultTransposeMatrixd (const GLdouble *m) {
    if (verbose) puts("STUB: OpenGL called");
}



void glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompressedTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompressedTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetCompressedTexImage (GLenum target, GLint lod, GLvoid *img) {
    if (verbose) puts("STUB: OpenGL called");
}



void glActiveTexture (GLenum texture) {
    if (verbose) puts("STUB: OpenGL called");
}


void glClientActiveTexture (GLenum texture) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1d (GLenum target, GLdouble s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1dv (GLenum target, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1f (GLenum target, GLfloat s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1fv (GLenum target, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1i (GLenum target, GLint s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1iv (GLenum target, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1s (GLenum target, GLshort s) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord1sv (GLenum target, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2d (GLenum target, GLdouble s, GLdouble t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2dv (GLenum target, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2fv (GLenum target, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2i (GLenum target, GLint s, GLint t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2iv (GLenum target, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2s (GLenum target, GLshort s, GLshort t) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord2sv (GLenum target, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3d (GLenum target, GLdouble s, GLdouble t, GLdouble r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3dv (GLenum target, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3f (GLenum target, GLfloat s, GLfloat t, GLfloat r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3fv (GLenum target, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3i (GLenum target, GLint s, GLint t, GLint r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3iv (GLenum target, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3s (GLenum target, GLshort s, GLshort t, GLshort r) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord3sv (GLenum target, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4d (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4dv (GLenum target, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4fv (GLenum target, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4i (GLenum target, GLint s, GLint t, GLint r, GLint q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4iv (GLenum target, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4s (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiTexCoord4sv (GLenum target, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}



void glFogCoordf (GLfloat coord) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogCoordfv (const GLfloat *coord) {
    if (verbose) puts("STUB: OpenGL called");
}

  
void glFogCoordd (GLdouble coord) {
    if (verbose) puts("STUB: OpenGL called");
}


void glFogCoorddv (const GLdouble *coord) {
    if (verbose) puts("STUB: OpenGL called");
}

   
void glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}



void glSecondaryColor3b (GLbyte red, GLbyte green, GLbyte blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3bv (const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3d (GLdouble red, GLdouble green, GLdouble blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3f (GLfloat red, GLfloat green, GLfloat blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3i (GLint red, GLint green, GLint blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3s (GLshort red, GLshort green, GLshort blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3ub (GLubyte red, GLubyte green, GLubyte blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3ubv (const GLubyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3ui (GLuint red, GLuint green, GLuint blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3uiv (const GLuint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3us (GLushort red, GLushort green, GLushort blue) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColor3usv (const GLushort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glSecondaryColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}



void glPointParameterf (GLenum pname, GLfloat param) {
    if (verbose) puts("STUB: OpenGL called");
}

 
void glPointParameterfv (GLenum pname, const GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glPointParameteri (GLenum pname, GLint param) {
    if (verbose) puts("STUB: OpenGL called");
}

 
void glPointParameteriv (GLenum pname, const GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}



void glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    if (verbose) puts("STUB: OpenGL called");
}



void glMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount) {
    if (verbose) puts("STUB: OpenGL called");
}


void glMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei primcount) {
    if (verbose) puts("STUB: OpenGL called");
}



void glWindowPos2d (GLdouble x, GLdouble y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2f (GLfloat x, GLfloat y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2i (GLint x, GLint y) {
    if (verbose) puts("STUB: OpenGL called");
}

 
void glWindowPos2iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2s (GLshort x, GLshort y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos2sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3d (GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3dv (const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3f (GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3fv (const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3i (GLint x, GLint y, GLint z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3iv (const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3s (GLshort x, GLshort y, GLshort z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glWindowPos3sv (const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}



void glGenQueries (GLsizei n, GLuint *ids) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteQueries (GLsizei n, const GLuint *ids) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsQuery (GLuint id) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBeginQuery (GLenum target, GLuint id) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEndQuery (GLenum target) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetQueryiv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params) {
    if (verbose) puts("STUB: OpenGL called");
}



void glBindBuffer (GLenum target, GLuint buffer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteBuffers (GLsizei n, const GLuint *buffers) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGenBuffers (GLsizei n, GLuint *buffers) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsBuffer (GLuint buffer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data) {
    if (verbose) puts("STUB: OpenGL called");
}


GLvoid *glMapBuffer (GLenum target, GLenum access) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glUnmapBuffer (GLenum target) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetBufferPointerv (GLenum target, GLenum pname, GLvoid **params) {
    if (verbose) puts("STUB: OpenGL called");
}



void glDrawBuffers (GLsizei n, const GLenum *bufs) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1d (GLuint index, GLdouble x) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1dv (GLuint index, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1f (GLuint index, GLfloat x) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1fv (GLuint index, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1s (GLuint index, GLshort x) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib1sv (GLuint index, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2d (GLuint index, GLdouble x, GLdouble y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2dv (GLuint index, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2fv (GLuint index, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2s (GLuint index, GLshort x, GLshort y) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib2sv (GLuint index, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3d (GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3dv (GLuint index, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3fv (GLuint index, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3s (GLuint index, GLshort x, GLshort y, GLshort z) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib3sv (GLuint index, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nbv (GLuint index, const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Niv (GLuint index, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nsv (GLuint index, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nub (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nubv (GLuint index, const GLubyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nuiv (GLuint index, const GLuint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4Nusv (GLuint index, const GLushort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4bv (GLuint index, const GLbyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4d (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4dv (GLuint index, const GLdouble *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4fv (GLuint index, const GLfloat *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4iv (GLuint index, const GLint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4s (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4sv (GLuint index, const GLshort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4ubv (GLuint index, const GLubyte *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4uiv (GLuint index, const GLuint *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttrib4usv (GLuint index, const GLushort *v) {
    if (verbose) puts("STUB: OpenGL called");
}


void glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glEnableVertexAttribArray (GLuint index) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDisableVertexAttribArray (GLuint index) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetVertexAttribdv (GLuint index, GLenum pname, GLdouble *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid **pointer) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteShader (GLuint shader) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDetachShader (GLuint program, GLuint shader) {
    if (verbose) puts("STUB: OpenGL called");
}


GLuint glCreateShader (GLenum type) {
    if (verbose) puts("STUB: OpenGL called");
}


void glShaderSource (GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) {
    if (verbose) puts("STUB: OpenGL called");
}


void glCompileShader (GLuint shader) {
    if (verbose) puts("STUB: OpenGL called");
}


GLuint glCreateProgram (void) {
    if (verbose) puts("STUB: OpenGL called");
}


void glAttachShader (GLuint program, GLuint shader) {
    if (verbose) puts("STUB: OpenGL called");
}


void glLinkProgram (GLuint program) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUseProgram (GLuint program) {
    if (verbose) puts("STUB: OpenGL called");
}


void glDeleteProgram (GLuint program) {
    if (verbose) puts("STUB: OpenGL called");
}


void glValidateProgram (GLuint program) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform1f (GLint location, GLfloat v0) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform2f (GLint location, GLfloat v0, GLfloat v1) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform1i (GLint location, GLint v0) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform2i (GLint location, GLint v0, GLint v1) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform3i (GLint location, GLint v0, GLint v1, GLint v2) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform1fv (GLint location, GLsizei count, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform2fv (GLint location, GLsizei count, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform3fv (GLint location, GLsizei count, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform4fv (GLint location, GLsizei count, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform1iv (GLint location, GLsizei count, const GLint *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform2iv (GLint location, GLsizei count, const GLint *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform3iv (GLint location, GLsizei count, const GLint *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniform4iv (GLint location, GLsizei count, const GLint *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsShader (GLuint shader) {
    if (verbose) puts("STUB: OpenGL called");
}


GLboolean glIsProgram (GLuint program) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    if (verbose) puts("STUB: OpenGL called");
}


GLint glGetUniformLocation (GLuint program, const GLchar *name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetUniformfv (GLuint program, GLint location, GLfloat *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetUniformiv (GLuint program, GLint location, GLint *params) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
    if (verbose) puts("STUB: OpenGL called");
}


void glBindAttribLocation (GLuint program, GLuint index, const GLchar *name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
    if (verbose) puts("STUB: OpenGL called");
}


GLint glGetAttribLocation (GLuint program, const GLchar *name) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) {
    if (verbose) puts("STUB: OpenGL called");
}


void glStencilMaskSeparate (GLenum face, GLuint mask) {
    if (verbose) puts("STUB: OpenGL called");
}



void glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}


void glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    if (verbose) puts("STUB: OpenGL called");
}

CGLError CGLSetCurrentContext(CGLContextObj OPENGL_NULLABLE ctx) {
    if (verbose) puts("STUB: OpenGL called");
}
CGLContextObj OPENGL_NULLABLE CGLGetCurrentContext(void) {
    if (verbose) puts("STUB: OpenGL called");
}