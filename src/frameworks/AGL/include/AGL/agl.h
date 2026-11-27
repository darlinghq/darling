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

#ifndef _AGL_AGL_H_
#define _AGL_AGL_H_

#include <Carbon/Carbon.h>
#include <OpenGL/gl.h>
#include <ApplicationServices/ApplicationServices.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AGL_VERSION_2_0 1
#define AGL_VERSION_3_0 1

typedef struct __AGLContextRec *AGLContext;
typedef struct __AGLPixelFormatRec *AGLPixelFormat;
typedef struct __AGLRendererInfoRec *AGLRendererInfo;
typedef struct __AGLPBufferRec *AGLPbuffer;
typedef GDHandle AGLDevice;
typedef CGrafPtr AGLDrawable;

/* Pixel format attributes */
enum {
	AGL_NONE = 0,
	AGL_ALL_RENDERERS = 1,
	AGL_BUFFER_SIZE = 2,
	AGL_LEVEL = 3,
	AGL_RGBA = 4,
	AGL_DOUBLEBUFFER = 5,
	AGL_STEREO = 6,
	AGL_AUX_BUFFERS = 7,
	AGL_RED_SIZE = 8,
	AGL_GREEN_SIZE = 9,
	AGL_BLUE_SIZE = 10,
	AGL_ALPHA_SIZE = 11,
	AGL_DEPTH_SIZE = 12,
	AGL_STENCIL_SIZE = 13,
	AGL_ACCUM_RED_SIZE = 14,
	AGL_ACCUM_GREEN_SIZE = 15,
	AGL_ACCUM_BLUE_SIZE = 16,
	AGL_ACCUM_ALPHA_SIZE = 17,
	AGL_PIXEL_SIZE = 50,
	AGL_MINIMUM_POLICY = 51,
	AGL_MAXIMUM_POLICY = 52,
	AGL_OFFSCREEN = 53,
	AGL_FULLSCREEN = 54,
	AGL_SAMPLE_BUFFERS_ARB = 55,
	AGL_SAMPLES_ARB = 56,
	AGL_AUX_DEPTH_STENCIL = 57,
	AGL_COLOR_FLOAT = 58,
	AGL_MULTISAMPLE = 59,
	AGL_SUPERSAMPLE = 60,
	AGL_SAMPLE_ALPHA = 61,
	AGL_RENDERER_ID = 70,
	AGL_SINGLE_RENDERER = 71,
	AGL_NO_RECOVERY = 72,
	AGL_ACCELERATED = 73,
	AGL_CLOSEST_POLICY = 74,
	AGL_ROBUST = 75,
	AGL_BACKING_STORE = 76,
	AGL_MP_SAFE = 78,
	AGL_WINDOW = 80,
	AGL_MULTISCREEN = 81,
	AGL_VIRTUAL_SCREEN = 82,
	AGL_COMPLIANT = 83,
	AGL_PBUFFER = 90
};

/* Renderer properties */
enum {
	AGL_BUFFER_MODES = 100,
	AGL_MIN_LEVEL = 101,
	AGL_MAX_LEVEL = 102,
	AGL_COLOR_MODES = 103,
	AGL_ACCUM_MODES = 104,
	AGL_DEPTH_MODES = 105,
	AGL_STENCIL_MODES = 106,
	AGL_MAX_AUX_BUFFERS = 107,
	AGL_VIDEO_MEMORY = 120,
	AGL_TEXTURE_MEMORY = 121,
	AGL_RENDERER_COUNT = 128
};

/* Context options */
enum {
	AGL_SWAP_RECT = 200,
	AGL_BUFFER_RECT = 202,
	AGL_SWAP_LIMIT = 203,
	AGL_COLORMAP_TRACKING = 210,
	AGL_COLORMAP_ENTRY = 212,
	AGL_RASTERIZATION = 220,
	AGL_SWAP_INTERVAL = 222,
	AGL_STATE_VALIDATION = 230,
	AGL_BUFFER_NAME = 231,
	AGL_ORDER_CONTEXT_TO_FRONT = 232,
	AGL_CONTEXT_SURFACE_ID = 233,
	AGL_CONTEXT_DISPLAY_ID = 234,
	AGL_SURFACE_ORDER = 235,
	AGL_SURFACE_OPACITY = 236,
	AGL_CLIP_REGION = 254,
	AGL_FS_CAPTURE_SINGLE = 255,
	AGL_SURFACE_BACKING_SIZE = 304,
	AGL_ENABLE_SURFACE_BACKING_SIZE = 305,
	AGL_SURFACE_VOLATILE = 306,
	AGL_DISPLAY_MODE = 308
};

/* Library options */
enum {
	AGL_FORMAT_CACHE_SIZE = 501,
	AGL_CLEAR_FORMAT_CACHE = 502,
	AGL_RETAIN_RENDERERS = 503
};

/* Renderer, buffer, color and depth mode bits */
enum {
	AGL_MONOSCOPIC_BIT = 0x00000001,
	AGL_STEREOSCOPIC_BIT = 0x00000002,
	AGL_SINGLEBUFFER_BIT = 0x00000004,
	AGL_DOUBLEBUFFER_BIT = 0x00000008,
	AGL_0_BIT = 0x00000001,
	AGL_1_BIT = 0x00000002,
	AGL_2_BIT = 0x00000004,
	AGL_3_BIT = 0x00000008,
	AGL_4_BIT = 0x00000010,
	AGL_5_BIT = 0x00000020,
	AGL_6_BIT = 0x00000040,
	AGL_8_BIT = 0x00000080,
	AGL_10_BIT = 0x00000100,
	AGL_12_BIT = 0x00000200,
	AGL_16_BIT = 0x00000400,
	AGL_24_BIT = 0x00000800,
	AGL_32_BIT = 0x00001000,
	AGL_48_BIT = 0x00002000,
	AGL_64_BIT = 0x00004000,
	AGL_96_BIT = 0x00008000,
	AGL_128_BIT = 0x00010000,
	AGL_RGB8_BIT = 0x00000001,
	AGL_RGB8_A8_BIT = 0x00000002,
	AGL_BGR233_BIT = 0x00000004,
	AGL_BGR233_A8_BIT = 0x00000008,
	AGL_RGB332_BIT = 0x00000010,
	AGL_RGB332_A8_BIT = 0x00000020,
	AGL_RGB444_BIT = 0x00000040,
	AGL_ARGB4444_BIT = 0x00000080,
	AGL_RGB444_A8_BIT = 0x00000100,
	AGL_RGB555_BIT = 0x00000200,
	AGL_ARGB1555_BIT = 0x00000400,
	AGL_RGB555_A8_BIT = 0x00000800,
	AGL_RGB565_BIT = 0x00001000,
	AGL_RGB565_A8_BIT = 0x00002000,
	AGL_RGB888_BIT = 0x00004000,
	AGL_ARGB8888_BIT = 0x00008000,
	AGL_RGB888_A8_BIT = 0x00010000,
	AGL_RGB101010_BIT = 0x00020000,
	AGL_ARGB2101010_BIT = 0x00040000,
	AGL_RGB101010_A8_BIT = 0x00080000,
	AGL_RGB121212_BIT = 0x00100000,
	AGL_ARGB12121212_BIT = 0x00200000,
	AGL_RGB161616_BIT = 0x00400000,
	AGL_ARGB16161616_BIT = 0x00800000,
	AGL_INDEX8_BIT = 0x20000000,
	AGL_INDEX16_BIT = 0x40000000
};

/* Error codes */
enum {
	AGL_NO_ERROR = 0,
	AGL_BAD_ATTRIBUTE = 10000,
	AGL_BAD_PROPERTY = 10001,
	AGL_BAD_PIXELFMT = 10002,
	AGL_BAD_RENDINFO = 10003,
	AGL_BAD_CONTEXT = 10004,
	AGL_BAD_DRAWABLE = 10005,
	AGL_BAD_GDEV = 10006,
	AGL_BAD_STATE = 10007,
	AGL_BAD_VALUE = 10008,
	AGL_BAD_MATCH = 10009,
	AGL_BAD_ENUM = 10010,
	AGL_BAD_OFFSCREEN = 10011,
	AGL_BAD_FULLSCREEN = 10012,
	AGL_BAD_WINDOW = 10013,
	AGL_BAD_POINTER = 10014,
	AGL_BAD_MODULE = 10015,
	AGL_BAD_ALLOC = 10016,
	AGL_BAD_CONNECTION = 10017
};

/* Library version, pixel formats and renderer information */
void aglGetVersion(GLint *major, GLint *minor);
AGLPixelFormat aglChoosePixelFormat(const AGLDevice *gdevs, GLint ndev, const GLint *attribs);
void aglDestroyPixelFormat(AGLPixelFormat pix);
AGLPixelFormat aglNextPixelFormat(AGLPixelFormat pix);
GLboolean aglDescribePixelFormat(AGLPixelFormat pix, GLint attrib, GLint *value);
AGLDevice *aglDevicesOfPixelFormat(AGLPixelFormat pix, GLint *ndevs);
AGLRendererInfo aglQueryRendererInfo(const AGLDevice *gdevs, GLint ndev);
void aglDestroyRendererInfo(AGLRendererInfo rend);
AGLRendererInfo aglNextRendererInfo(AGLRendererInfo rend);
GLboolean aglDescribeRenderer(AGLRendererInfo rend, GLint prop, GLint *value);

/* Contexts */
AGLContext aglCreateContext(AGLPixelFormat pix, AGLContext share);
GLboolean aglDestroyContext(AGLContext ctx);
GLboolean aglCopyContext(AGLContext src, AGLContext dst, GLuint mask);
GLboolean aglUpdateContext(AGLContext ctx);

/* Current context and drawable */
GLboolean aglSetCurrentContext(AGLContext ctx);
AGLContext aglGetCurrentContext(void);
GLboolean aglSetDrawable(AGLContext ctx, AGLDrawable draw);
GLboolean aglSetOffScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei rowbytes, GLvoid *baseaddr);
GLboolean aglSetFullScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei freq, GLint device);
AGLDrawable aglGetDrawable(AGLContext ctx);
GLboolean aglSetWindowRef(AGLContext ctx, WindowRef window);
WindowRef aglGetWindowRef(AGLContext ctx);
GLboolean aglSetHIViewRef(AGLContext ctx, HIViewRef hiview);
HIViewRef aglGetHIViewRef(AGLContext ctx);

/* Virtual screens */
GLboolean aglSetVirtualScreen(AGLContext ctx, GLint screen);
GLint aglGetVirtualScreen(AGLContext ctx);

/* Swapping, options and queries */
void aglSwapBuffers(AGLContext ctx);
GLboolean aglEnable(AGLContext ctx, GLenum pname);
GLboolean aglDisable(AGLContext ctx, GLenum pname);
GLboolean aglIsEnabled(AGLContext ctx, GLenum pname);
GLboolean aglSetInteger(AGLContext ctx, GLenum pname, const GLint *params);
GLboolean aglGetInteger(AGLContext ctx, GLenum pname, GLint *params);
GLboolean aglConfigure(GLenum pname, GLuint param);
void aglResetLibrary(void);

/* Bitmap fonts */
GLboolean aglUseFont(AGLContext ctx, GLint fontID, Style face, GLint size, GLint first, GLint count, GLint base);

/* Errors */
GLenum aglGetError(void);
const GLubyte *aglErrorString(GLenum code);

/* Pixel buffers */
GLboolean aglCreatePBuffer(GLint width, GLint height, GLenum target, GLenum internalFormat, long max_level, AGLPbuffer *pbuffer);
GLboolean aglDestroyPBuffer(AGLPbuffer pbuffer);
GLboolean aglDescribePBuffer(AGLPbuffer pbuffer, GLint *width, GLint *height, GLenum *target, GLenum *internalFormat, GLint *max_level);
GLboolean aglTexImagePBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint source);
GLboolean aglSetPBuffer(AGLContext ctx, AGLPbuffer pbuffer, GLint face, GLint level, GLint screen);
GLboolean aglGetPBuffer(AGLContext ctx, AGLPbuffer *pbuffer, GLint *face, GLint *level, GLint *screen);

/* Surface textures */
GLboolean aglSurfaceTexture(AGLContext context, GLenum target, GLenum internalformat, AGLContext surfacecontext);

/* Access to the CGL context */
GLboolean aglGetCGLContext(AGLContext ctx, void **cgl_ctx);
GLboolean aglGetCGLPixelFormat(AGLPixelFormat pix, void **cgl_pix);

#ifdef __cplusplus
}
#endif

#endif
