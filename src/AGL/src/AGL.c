
#pragma GCC visibility push(hidden)

#include <OpenGL/gl.h>
#include <Carbon/Carbon.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FW_EXPORT __attribute__ ((visibility ("default")))

typedef void* AGLContext;
typedef void* GLMfunctions;
typedef void* AGLRendererInfo;
typedef void* AGLPixelFormat;
typedef void* AGLPbuffer;
typedef void* GDHandle;
typedef void* HIViewRef;
typedef void* WindowRef;
typedef uint32_t AGLDevice;

typedef void* AGLDrawable;

uint32_t gFakeContext;
uint32_t gFakePBuffer = 0;
uint32_t gFakePixelFormat = 0;
uint32_t gFakeRenderInfo = 0;
uint32_t gFakeDrawable = 0;
uint32_t gFakeViewRef = 0;

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

FW_EXPORT AGLPixelFormat aglChoosePixelFormat(const AGLDevice *dev, GLint a, const GLint *b)
{
   if (verbose) puts("STUB: aglChoosePixelFormat called");
   return (AGLPixelFormat)&gFakePixelFormat;
}

FW_EXPORT AGLPixelFormat aglChoosePixelFormatCFM(const AGLDevice *dev, GLint a, const GLint *b)
{
   if (verbose) puts("STUB: aglChoosePixelFormatCFM called");
   return (AGLPixelFormat)&gFakePixelFormat;
}

FW_EXPORT void aglConfigure(GLenum a, GLuint b)
{
   if (verbose) puts("STUB: aglConfigure called");
}

FW_EXPORT void aglConfigureCFM(GLenum a, GLuint b)
{
   if (verbose) puts("STUB: aglConfigureCFM called");
}

FW_EXPORT GLboolean aglCopyContext(AGLContext ctx, AGLContext ctx2, GLuint a)
{
   if (verbose) puts("STUB: aglCopyContext called");
   return 0;
}

FW_EXPORT AGLContext aglCreateContext(AGLPixelFormat fmt, AGLContext ctx)
{
   if (verbose) puts("STUB: aglCreateContext called");
   return &gFakeContext;
}

FW_EXPORT AGLContext aglCreateContextCFM(AGLPixelFormat fmt, AGLContext ctx)
{
   if (verbose) puts("STUB: aglCreateContextCFM called");
   return &gFakeContext;
}

FW_EXPORT GLboolean aglCreatePBuffer(GLint a, GLint b , GLenum c, GLenum d, GLint e, AGLPbuffer *out)
{
   if (verbose) puts("STUB: aglCreatePBuffer called");
   *out = &gFakePBuffer;
   return GL_TRUE;
}

FW_EXPORT AGLPixelFormat aglCreatePixelFormat(const GLint *a)
{
   if (verbose) puts("STUB: aglCreatePixelFormat called");
   return (AGLPixelFormat)&gFakePixelFormat;
}

FW_EXPORT AGLPixelFormat aglCreatePixelFormatCFM(const GLint *a)
{
   if (verbose) puts("STUB: aglCreatePixelFormatCFM called");
   return (AGLPixelFormat)&gFakePixelFormat;
}

FW_EXPORT GLboolean aglDescribePBuffer(AGLPbuffer buf, GLint *w, GLint *h, GLenum *t, GLenum *fmt, GLint *ml)
{
   if (verbose) puts("STUB: aglDescribePBuffer called");
   *w = 640;
   *h = 480;
   *t = GL_NONE;
   *fmt = GL_NONE;
   *ml = 0;
   
   return GL_TRUE;
}

FW_EXPORT GLboolean aglDescribePixelFormat(AGLPixelFormat fmt, GLint a, GLint *out)
{
   if (verbose) puts("STUB: aglDescribePixelFormat called");
   *out = 0;
   return GL_TRUE;
}

FW_EXPORT GLboolean aglDescribeRenderer(AGLRendererInfo a, GLint b, GLint *out)
{
   if (verbose) puts("STUB: aglDescribeRenderer called");
   *out = 0;
   return GL_TRUE;
}

FW_EXPORT GLboolean aglDestroyContext(AGLContext ctx)
{
   if (verbose) puts("STUB: aglDestroyContext called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglDestroyPBuffer(AGLPbuffer buf)
{
   if (verbose) puts("STUB: aglDestroyPBuffer called");
   return GL_TRUE;
}

FW_EXPORT void aglDestroyPixelFormat(AGLPixelFormat fmt)
{
   if (verbose) puts("STUB: aglDestroyPixelFormat called");
}

FW_EXPORT void aglDestroyRendererInfo(AGLRendererInfo a)
{
   if (verbose) puts("STUB: aglDestroyRendererInfo called");
}

FW_EXPORT GDHandle * aglDevicesOfPixelFormat(AGLPixelFormat fmt, GLint *a)
{
   if (verbose) puts("STUB: aglDevicesOfPixelFormat called");
   return NULL;
}

FW_EXPORT GLboolean aglDisable(AGLContext ctx, GLenum a)
{
   if (verbose) puts("STUB: aglDisable called");
   return GL_TRUE;
}

FW_EXPORT CGDirectDisplayID * aglDisplaysOfPixelFormat(AGLPixelFormat fmt, GLint *a)
{
   if (verbose) puts("STUB: aglDisplaysOfPixelFormat called");
   return NULL;
}

FW_EXPORT GLboolean aglEnable(AGLContext ctx, GLenum a)
{
   if (verbose) puts("STUB: aglEnable called");
   return GL_TRUE;
}

FW_EXPORT const GLubyte * aglErrorString(GLenum a)
{
   if (verbose) puts("STUB: aglErrorString called");
   return (const GLubyte*)"";
}

FW_EXPORT GLboolean aglGetCGLContext(AGLContext ctx, void **out)
{
   if (verbose) puts("STUB: aglGetCGLContext called");
   return GL_TRUE;
}

GLboolean aglGetCGLPixelFormat(AGLPixelFormat fmt, void **out)
{
   if (verbose) puts("STUB: aglGetCGLPixelFormat called");
   return GL_FALSE;
}

AGLContext aglGetCurrentContext(void)
{
   if (verbose) puts("STUB: aglGetCurrentContext called");
   return NULL;
}

FW_EXPORT AGLDrawable aglGetDrawable(AGLContext ctx)
{
   if (verbose) puts("STUB: aglGetDrawable called");
   return (AGLDrawable)&gFakeDrawable;
}

FW_EXPORT GLenum aglGetError(void)
{
   if (verbose) puts("STUB: aglGetError called");
   return GL_NONE;
}

FW_EXPORT HIViewRef aglGetHIViewRef(AGLContext ctx)
{
   if (verbose) puts("STUB: aglGetHIViewRef called");
   return (HIViewRef)&gFakeViewRef;
}

FW_EXPORT GLboolean aglGetInteger(AGLContext ctx, GLenum a, GLint *out)
{
   if (verbose) puts("STUB: aglGetInteger called");
   *out = 0;
   return GL_FALSE;
}

FW_EXPORT GLboolean aglGetPBuffer(AGLContext ctx, AGLPbuffer *buf, GLint *a, GLint *b, GLint *c)
{
   if (verbose) puts("STUB: aglGetPBuffer called");
   return GL_FALSE;
}

FW_EXPORT void aglGetVersion(GLint *major, GLint *minor)
{
   *major = 2;
   *minor = 0;
}

FW_EXPORT GLint aglGetVirtualScreen(AGLContext ctx)
{
   if (verbose) puts("STUB: aglGetVirtualScreen called");
   return 0;
}

FW_EXPORT WindowRef aglGetWindowRef(AGLContext ctx)
{
   if (verbose) puts("STUB: aglGetWindowRef called");
   return 0;
}

FW_EXPORT GLboolean aglIsEnabled(AGLContext ctx, GLenum a)
{
   if (verbose) puts("STUB: aglIsEnabled called");
   return GL_FALSE;
}

FW_EXPORT AGLPixelFormat aglNextPixelFormat(AGLPixelFormat fmt)
{
   if (verbose) puts("STUB: aglNextPixelFormat called");
   return NULL;
}

FW_EXPORT AGLRendererInfo aglNextRendererInfo(AGLRendererInfo a)
{
   if (verbose) puts("STUB: aglNextRendererInfo called");
   return NULL;
}

FW_EXPORT AGLRendererInfo aglQueryRendererInfo(const AGLDevice *dev, GLint a)
{
   if (verbose) puts("STUB: aglQueryRendererInfo called");
   return (AGLRendererInfo)&gFakeRenderInfo;
}

FW_EXPORT AGLRendererInfo aglQueryRendererInfoCFM(const AGLDevice *dev, GLint a)
{
   if (verbose) puts("STUB: aglQueryRendererInfoCFM called");
   return (AGLRendererInfo)&gFakeRenderInfo;
}

FW_EXPORT AGLRendererInfo aglQueryRendererInfoForCGDirectDisplayIDs(const CGDirectDisplayID *a, GLint b)
{
   if (verbose) puts("STUB: aglQueryRendererInfoForCGDirectDisplayIDs called");
   return (AGLRendererInfo)&gFakeRenderInfo;
}

FW_EXPORT void aglResetLibrary(void)
{
   if (verbose) puts("STUB: aglResetLibrary called");
}

FW_EXPORT GLboolean aglSetCurrentContext(AGLContext ctx)
{
   if (verbose) puts("STUB: aglSetCurrentContext called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetDrawable(AGLContext ctx, AGLDrawable a)
{
   if (verbose) puts("STUB: aglSetDrawable called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetFullScreen(AGLContext ctx, GLsizei a, GLsizei b, GLsizei c, GLint d)
{
   if (verbose) puts("STUB: aglSetFullScreen called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetHIViewRef(AGLContext ctx, HIViewRef a)
{
   if (verbose) puts("STUB: aglSetHIViewRef called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetInteger(AGLContext ctx, GLenum a, const GLint *b)
{
   if (verbose) puts("STUB: aglSetInteger called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetOffScreen(AGLContext ctx, GLsizei a, GLsizei b, GLsizei c, GLvoid *d)
{
   if (verbose) puts("STUB: aglSetOffScreen called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetPBuffer(AGLContext ctx, AGLPbuffer buf, GLint a, GLint b, GLint c)
{
   if (verbose) puts("STUB: aglSetPBuffer called");
   return GL_TRUE;
}

FW_EXPORT GLboolean aglSetVirtualScreen(AGLContext ctx, GLint a)
{
   if (verbose) puts("STUB: aglSetVirtualScreen called");
   return GL_TRUE;
}

GLboolean aglSetWindowRef(AGLContext ctx, WindowRef a)
{
   if (verbose) puts("STUB: aglSetWindowRef called");
   return GL_TRUE;
}

FW_EXPORT void aglSurfaceTexture(AGLContext ctx, GLenum a, GLenum b, AGLContext ctx2)
{
   if (verbose) puts("STUB: aglSurfaceTexture called");
}

FW_EXPORT void aglSwapBuffers(AGLContext ctx)
{
   if (verbose) puts("STUB: aglSwapBuffers called");
}

FW_EXPORT GLboolean aglTexImagePBuffer(AGLContext ctx, AGLPbuffer buf, GLint a)
{
   if (verbose) puts("STUB: aglTexImagePBuffer called");
   return GL_TRUE;
}

FW_EXPORT int8_t aglUpdateContext(AGLContext ctx)
{
   if (verbose) puts("STUB: aglUpdateContext called");
   return 0;
}

FW_EXPORT int8_t aglUseFont(AGLContext ctx, int a, int b, int c, int d, int e, void* f)
{
   if (verbose) puts("STUB: aglUseFont called");
   return 0;
}

FW_EXPORT GLvoid * glmCalloc(GLsizei a, GLsizei b)
{
   if (verbose) puts("STUB: glmCalloc called");
   return NULL;
}

FW_EXPORT void glmCopy(const GLubyte a, GLubyte b, GLsizei c, GLsizei d, GLsizei e, GLsizei f)
{
   if (verbose) puts("STUB: glmCopy called");
}

FW_EXPORT GLvoid * glmDCBAlloc(GLsizei a)
{
   if (verbose) puts("STUB: glmDCBAlloc called");
   return NULL;
}

FW_EXPORT void glmDCBFree(GLvoid * a)
{
   if (verbose) puts("STUB: glmDCBFree called");
}

FW_EXPORT GLvoid * glmDCBRealloc(GLvoid *a, GLsizei b)
{
   if (verbose) puts("STUB: glmDCBRealloc called");
   return NULL;
}

FW_EXPORT void glmFree(GLvoid *a)
{
   if (verbose) puts("STUB: glmFree called");
}

FW_EXPORT GLenum glmGetError(void)
{
   if (verbose) puts("STUB: glmGetError called");
   return GL_NONE;
}

FW_EXPORT GLint glmGetInteger(GLenum a)
{
   if (verbose) puts("STUB: glmGetInteger called");
   return 0;
}

FW_EXPORT GLvoid * glmMalloc(GLsizei a)
{
   if (verbose) puts("STUB: glmMalloc called");
   return NULL;
}

FW_EXPORT void glmPageFreeAll(void)
{
   if (verbose) puts("STUB: glmPageFreeAll called");
}

FW_EXPORT GLvoid * glmRealloc(GLvoid *a, GLsizei b)
{
   if (verbose) puts("STUB: glmRealloc called");
   return NULL;
}

FW_EXPORT void glmReportMemoryStatus()
{
   if (verbose) puts("STUB: glmReportMemoryStatus called");
}

FW_EXPORT void glmResetMemoryStatus()
{
   if (verbose) puts("STUB: glmResetMemoryStatus called");
}

FW_EXPORT void glmSetDebugInfo()
{
   if (verbose) puts("STUB: glmSetDebugInfo called");
}

FW_EXPORT void glmSetDouble(GLdouble *a, GLsizei b, GLsizei c, GLsizei d, GLdouble e)
{
   if (verbose) puts("STUB: glmSetDouble called");
}

FW_EXPORT void glmSetFunc(GLenum a, GLMfunctions b)
{
   if (verbose) puts("STUB: glmSetFunc called");
}

FW_EXPORT void glmSetInteger(GLenum a, GLint b)
{
   if (verbose) puts("STUB: glmSetInteger called");
}

FW_EXPORT void glmSetMode(GLenum a)
{
   if (verbose) puts("STUB: glmSetMode called");
}

FW_EXPORT void glmSetUByte(GLubyte *a, GLsizei b, GLsizei c, GLsizei d, GLubyte e)
{
   if (verbose) puts("STUB: glmSetUByte called");
}

FW_EXPORT void glmSetUInt(GLuint *a, GLsizei b, GLsizei c, GLsizei d, GLuint e)
{
   if (verbose) puts("STUB: glmSetUInt called");
}

FW_EXPORT void glmSetUShort(GLushort *a, GLsizei b, GLsizei c, GLsizei d, GLushort e)
{
   if (verbose) puts("STUB: glmSetUShort called");
}

FW_EXPORT GLvoid * glmVecAlloc(GLsizei a)
{
   if (verbose) puts("STUB: glmVecAlloc called");
   return NULL;
}

FW_EXPORT void glmVecFree(GLvoid *a)
{
   if (verbose) puts("STUB: glmVecFree called");
}

FW_EXPORT GLvoid * glmVecRealloc(GLvoid *a, GLsizei b)
{
   if (verbose) puts("STUB: glmVecRealloc called");
   return NULL;
}

FW_EXPORT void glmZero(GLubyte* a, GLsizei b, GLsizei c, GLsizei d)
{
   if (verbose) puts("STUB: glmZero called");
}
