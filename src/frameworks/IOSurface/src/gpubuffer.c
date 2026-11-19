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

#include "gpubuffer.h"
#include "linux_dmabuf.h"
#include <elfcalls.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern struct elf_calls* _elfcalls;

// The host libgbm and libEGL are the layer that macOS's IOSurface driver
// corresponds to: GL and other GPU APIs sit on top of IOSurface, and
// IOSurface reaches the GPU through them, never through Darling's own GPU
// frameworks. They are loaded in the host's ELF loader, as dmabuf.c loads
// the host C library. Darling code is built against the Darwin SDK, so the
// few types and values used are spelled out here (from <EGL/egl.h>,
// <EGL/eglext.h>, <GL/gl.h>, <GL/glext.h> and <gbm.h>; stable ABI).

typedef void* HostEGLDisplay;
typedef void* HostEGLContext;
typedef void* HostEGLImage;
typedef int32_t HostEGLint;
typedef uint32_t HostEGLBoolean;
typedef uint32_t HostEGLenum;
typedef intptr_t HostEGLAttrib;

#define H_EGL_NONE 0x3038
#define H_EGL_EXTENSIONS 0x3055
#define H_EGL_WIDTH 0x3057
#define H_EGL_HEIGHT 0x3056
#define H_EGL_OPENGL_API 0x30A2
#define H_EGL_PLATFORM_GBM_KHR 0x31D7
#define H_EGL_PLATFORM_DEVICE_EXT 0x313F
#define H_EGL_DRM_RENDER_NODE_FILE_EXT 0x3377
#define H_EGL_LINUX_DMA_BUF_EXT 0x3270
#define H_EGL_LINUX_DRM_FOURCC_EXT 0x3271
#define H_EGL_DMA_BUF_PLANE0_FD_EXT 0x3272
#define H_EGL_DMA_BUF_PLANE0_OFFSET_EXT 0x3273
#define H_EGL_DMA_BUF_PLANE0_PITCH_EXT 0x3274
#define H_EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT 0x3443
#define H_EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT 0x3444

#define H_GL_NO_ERROR 0
#define H_GL_TEXTURE_2D 0x0DE1
#define H_GL_TEXTURE_WIDTH 0x1000
#define H_GL_TEXTURE_HEIGHT 0x1001
#define H_GL_TEXTURE_MAG_FILTER 0x2800
#define H_GL_TEXTURE_MIN_FILTER 0x2801
#define H_GL_NEAREST 0x2600
#define H_GL_UNSIGNED_BYTE 0x1401
#define H_GL_RGBA 0x1908
#define H_GL_BGRA 0x80E1
#define H_GL_PACK_ROW_LENGTH 0x0D02
#define H_GL_PACK_ALIGNMENT 0x0D05
#define H_GL_UNPACK_ROW_LENGTH 0x0CF2
#define H_GL_UNPACK_ALIGNMENT 0x0CF5
#define H_GL_FRAMEBUFFER 0x8D40
#define H_GL_COLOR_ATTACHMENT0 0x8CE0
#define H_GL_FRAMEBUFFER_COMPLETE 0x8CD5

#define H_GBM_BO_USE_RENDERING (1 << 2)
// DRM_FORMAT_MOD_INVALID: the driver chose a layout it does not name.
#define H_DRM_FORMAT_MOD_INVALID 0x00ffffffffffffffULL

struct gbm_device;
struct gbm_bo;

static struct
{
	struct gbm_device* (*create_device)(int fd);
	struct gbm_bo* (*bo_create)(struct gbm_device* device, uint32_t width, uint32_t height,
		uint32_t format, uint32_t flags);
	struct gbm_bo* (*bo_create_with_modifiers)(struct gbm_device* device, uint32_t width, uint32_t height,
		uint32_t format, const uint64_t* modifiers, unsigned int count);
	struct gbm_bo* (*bo_create_with_modifiers2)(struct gbm_device* device, uint32_t width, uint32_t height,
		uint32_t format, const uint64_t* modifiers, unsigned int count, uint32_t flags);
	int (*bo_get_fd)(struct gbm_bo* bo);
	int (*bo_get_plane_count)(struct gbm_bo* bo);
	uint32_t (*bo_get_stride_for_plane)(struct gbm_bo* bo, int plane);
	uint32_t (*bo_get_offset)(struct gbm_bo* bo, int plane);
	uint64_t (*bo_get_modifier)(struct gbm_bo* bo);
	void (*bo_destroy)(struct gbm_bo* bo);
} gbm;

static struct
{
	void* (*getProcAddress)(const char* name);
	HostEGLDisplay (*getPlatformDisplay)(HostEGLenum platform, void* nativeDisplay, const HostEGLAttrib* attributes);
	HostEGLBoolean (*initialize)(HostEGLDisplay display, HostEGLint* major, HostEGLint* minor);
	const char* (*queryString)(HostEGLDisplay display, HostEGLint name);
	HostEGLBoolean (*bindAPI)(HostEGLenum api);
	HostEGLContext (*createContext)(HostEGLDisplay display, void* config, HostEGLContext share,
		const HostEGLint* attributes);
	HostEGLBoolean (*makeCurrent)(HostEGLDisplay display, void* draw, void* read, HostEGLContext context);
	HostEGLImage (*createImage)(HostEGLDisplay display, HostEGLContext context, HostEGLenum target,
		void* buffer, const HostEGLint* attributes);
	HostEGLBoolean (*destroyImage)(HostEGLDisplay display, HostEGLImage image);
	// EGL_EXT_device_enumeration and EGL_EXT_device_query; may be NULL.
	HostEGLBoolean (*queryDevices)(HostEGLint max, void** devices, HostEGLint* count);
	const char* (*queryDeviceString)(void* device, HostEGLint name);
} egl;

static struct
{
	uint32_t (*getError)(void);
	void (*genTextures)(int32_t count, uint32_t* names);
	void (*deleteTextures)(int32_t count, const uint32_t* names);
	void (*bindTexture)(uint32_t target, uint32_t name);
	void (*texParameteri)(uint32_t target, uint32_t name, int32_t value);
	void (*getTexLevelParameteriv)(uint32_t target, int32_t level, uint32_t name, int32_t* value);
	void (*imageTargetTexture2D)(uint32_t target, void* image);
	void (*genFramebuffers)(int32_t count, uint32_t* names);
	void (*deleteFramebuffers)(int32_t count, const uint32_t* names);
	void (*bindFramebuffer)(uint32_t target, uint32_t name);
	void (*framebufferTexture2D)(uint32_t target, uint32_t attachment, uint32_t textureTarget,
		uint32_t texture, int32_t level);
	uint32_t (*checkFramebufferStatus)(uint32_t target);
	void (*pixelStorei)(uint32_t name, int32_t value);
	void (*readPixels)(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t format, uint32_t type,
		void* pixels);
	void (*texSubImage2D)(uint32_t target, int32_t level, int32_t x, int32_t y, int32_t width, int32_t height,
		uint32_t format, uint32_t type, const void* pixels);
	void (*finish)(void);
} gl;

static int (*host_open)(const char* path, int flags, ...);

struct _IOSurfaceGPUBo
{
	struct gbm_bo* bo;
	_IOSurfaceGPUBoDescription description;
	// The GL format of the pixels, with GL_UNSIGNED_BYTE components.
	uint32_t glFormat;
	// The buffer in the helper's context: a texture made from it, and a
	// framebuffer that has the texture as its color attachment.
	HostEGLImage image;
	uint32_t texture, framebuffer;
};

// The helper: a thread with a surfaceless GL context current on the render
// node of the first buffer, which carries out every call into the driver.
// State below is used only on that thread, apart from the flags.
static bool g_disabled;
static bool g_started;
static char g_renderNode[64];
static int g_deviceFD = -1;
static struct gbm_device* g_device;
static HostEGLDisplay g_display;
static HostEGLContext g_context;
static bool g_deviceFailed;

// One job at a time is handed to the helper and waited for.
enum JobKind
{
	JOB_CREATE,
	JOB_IMPORT,
	JOB_DESTROY,
	JOB_DOWNLOAD,
	JOB_UPLOAD,
};

struct Job
{
	enum JobKind kind;
	_IOSurfaceGPUBo* bo;
	// JOB_CREATE, JOB_IMPORT
	const char* renderNode;
	const uint64_t* modifiers;
	size_t modifierCount;
	// JOB_DOWNLOAD, JOB_UPLOAD
	void* memory;
	size_t bytesPerRow;
	bool ok;
	bool done;
};

static pthread_mutex_t g_submitLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_jobLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_jobCond = PTHREAD_COND_INITIALIZER;
static struct Job* g_job;

static bool hasExtension(const char* extensions, const char* name)
{
	size_t length = strlen(name);
	for (const char* p = extensions; p && (p = strstr(p, name)) != NULL; p += length)
	{
		if ((p == extensions || p[-1] == ' ') && (p[length] == ' ' || p[length] == '\0'))
			return true;
	}
	return false;
}

static bool loadLibraries(void)
{
	if (_elfcalls == NULL)
		return false;
	void* libc = _elfcalls->dlopen("libc.so.6");
	void* libgbm = _elfcalls->dlopen("libgbm.so.1");
	void* libEGL = _elfcalls->dlopen("libEGL.so.1");
	if (!libc || !libgbm || !libEGL)
		return false;

	host_open = _elfcalls->dlsym(libc, "open");

	gbm.create_device = _elfcalls->dlsym(libgbm, "gbm_create_device");
	gbm.bo_create = _elfcalls->dlsym(libgbm, "gbm_bo_create");
	gbm.bo_create_with_modifiers = _elfcalls->dlsym(libgbm, "gbm_bo_create_with_modifiers");
	gbm.bo_create_with_modifiers2 = _elfcalls->dlsym(libgbm, "gbm_bo_create_with_modifiers2");
	gbm.bo_get_fd = _elfcalls->dlsym(libgbm, "gbm_bo_get_fd");
	gbm.bo_get_plane_count = _elfcalls->dlsym(libgbm, "gbm_bo_get_plane_count");
	gbm.bo_get_stride_for_plane = _elfcalls->dlsym(libgbm, "gbm_bo_get_stride_for_plane");
	gbm.bo_get_offset = _elfcalls->dlsym(libgbm, "gbm_bo_get_offset");
	gbm.bo_get_modifier = _elfcalls->dlsym(libgbm, "gbm_bo_get_modifier");
	gbm.bo_destroy = _elfcalls->dlsym(libgbm, "gbm_bo_destroy");

	egl.getProcAddress = _elfcalls->dlsym(libEGL, "eglGetProcAddress");
	egl.getPlatformDisplay = _elfcalls->dlsym(libEGL, "eglGetPlatformDisplay");
	egl.initialize = _elfcalls->dlsym(libEGL, "eglInitialize");
	egl.queryString = _elfcalls->dlsym(libEGL, "eglQueryString");
	egl.bindAPI = _elfcalls->dlsym(libEGL, "eglBindAPI");
	egl.createContext = _elfcalls->dlsym(libEGL, "eglCreateContext");
	egl.makeCurrent = _elfcalls->dlsym(libEGL, "eglMakeCurrent");

	if (!host_open || !gbm.create_device || !gbm.bo_get_fd || !gbm.bo_get_plane_count
		|| !gbm.bo_get_stride_for_plane || !gbm.bo_get_offset || !gbm.bo_get_modifier || !gbm.bo_destroy
		|| (!gbm.bo_create_with_modifiers && !gbm.bo_create_with_modifiers2))
		return false;
	if (!egl.getProcAddress || !egl.getPlatformDisplay || !egl.initialize || !egl.queryString
		|| !egl.bindAPI || !egl.createContext || !egl.makeCurrent)
		return false;

	egl.createImage = egl.getProcAddress("eglCreateImageKHR");
	egl.destroyImage = egl.getProcAddress("eglDestroyImageKHR");
	egl.queryDevices = egl.getProcAddress("eglQueryDevicesEXT");
	egl.queryDeviceString = egl.getProcAddress("eglQueryDeviceStringEXT");
	return egl.createImage && egl.destroyImage;
}

static bool loadGLFunctions(void)
{
#define LOAD(field, name) if (!(*(void**) &gl.field = egl.getProcAddress(name))) return false;
	LOAD(getError, "glGetError")
	LOAD(genTextures, "glGenTextures")
	LOAD(deleteTextures, "glDeleteTextures")
	LOAD(bindTexture, "glBindTexture")
	LOAD(texParameteri, "glTexParameteri")
	LOAD(getTexLevelParameteriv, "glGetTexLevelParameteriv")
	LOAD(imageTargetTexture2D, "glEGLImageTargetTexture2DOES")
	LOAD(genFramebuffers, "glGenFramebuffers")
	LOAD(deleteFramebuffers, "glDeleteFramebuffers")
	LOAD(bindFramebuffer, "glBindFramebuffer")
	LOAD(framebufferTexture2D, "glFramebufferTexture2D")
	LOAD(checkFramebufferStatus, "glCheckFramebufferStatus")
	LOAD(pixelStorei, "glPixelStorei")
	LOAD(readPixels, "glReadPixels")
	LOAD(texSubImage2D, "glTexSubImage2D")
	LOAD(finish, "glFinish")
#undef LOAD
	return true;
}

// The EGL display of the EGL device whose DRM render node is renderNode
// (EGL_EXT_platform_device), or NULL.
static HostEGLDisplay deviceDisplay(const char* renderNode)
{
	if (!egl.queryDevices || !egl.queryDeviceString)
		return NULL;
	void* devices[16];
	HostEGLint count = 0;
	if (!egl.queryDevices(16, devices, &count))
		return NULL;
	for (HostEGLint i = 0; i < count && i < 16; i++)
	{
		const char* extensions = egl.queryDeviceString(devices[i], H_EGL_EXTENSIONS);
		if (!hasExtension(extensions, "EGL_EXT_device_drm_render_node"))
			continue;
		const char* node = egl.queryDeviceString(devices[i], H_EGL_DRM_RENDER_NODE_FILE_EXT);
		if (node && strcmp(node, renderNode) == 0)
			return egl.getPlatformDisplay(H_EGL_PLATFORM_DEVICE_EXT, devices[i], NULL);
	}
	return NULL;
}

// Opens the render node, and makes a surfaceless GL context current on
// this thread on an EGL display of it: that of the node's buffer manager,
// or, where the host EGL has no buffer manager platform (as some 32-bit
// driver installations lack it), that of the node's EGL device. Once.
static bool openDevice(const char* renderNode)
{
	if (g_device != NULL)
		return strcmp(g_renderNode, renderNode) == 0;
	if (g_deviceFailed || strlen(renderNode) >= sizeof(g_renderNode))
		return false;
	g_deviceFailed = true;

	if (!loadLibraries())
		return false;
	g_deviceFD = host_open(renderNode, LINUX_O_RDWR | LINUX_O_CLOEXEC);
	if (g_deviceFD < 0)
		return false;
	g_device = gbm.create_device(g_deviceFD);
	if (g_device == NULL)
		return false;

	g_display = egl.getPlatformDisplay(H_EGL_PLATFORM_GBM_KHR, g_device, NULL);
	HostEGLint major = 0, minor = 0;
	if (g_display == NULL || !egl.initialize(g_display, &major, &minor))
	{
		g_display = deviceDisplay(renderNode);
		if (g_display == NULL || !egl.initialize(g_display, &major, &minor))
			return false;
	}
	const char* extensions = egl.queryString(g_display, H_EGL_EXTENSIONS);
	if (!hasExtension(extensions, "EGL_KHR_no_config_context")
		|| !hasExtension(extensions, "EGL_KHR_surfaceless_context")
		|| !hasExtension(extensions, "EGL_EXT_image_dma_buf_import")
		|| !hasExtension(extensions, "EGL_EXT_image_dma_buf_import_modifiers"))
		return false;
	if (!egl.bindAPI(H_EGL_OPENGL_API))
		return false;
	static const HostEGLint attributes[] = { H_EGL_NONE };
	g_context = egl.createContext(g_display, NULL, NULL, attributes);
	if (g_context == NULL || !egl.makeCurrent(g_display, NULL, NULL, g_context) || !loadGLFunctions())
		return false;

	strcpy(g_renderNode, renderNode);
	g_deviceFailed = false;
	return true;
}

static void clearGLErrors(void)
{
	for (int i = 0; i < 16 && gl.getError() != H_GL_NO_ERROR; i++)
		;
}

static void releaseBo(_IOSurfaceGPUBo* bo)
{
	if (bo->framebuffer != 0)
		gl.deleteFramebuffers(1, &bo->framebuffer);
	if (bo->texture != 0)
		gl.deleteTextures(1, &bo->texture);
	if (bo->image != NULL)
		egl.destroyImage(g_display, bo->image);
	if (bo->bo != NULL)
		gbm.bo_destroy(bo->bo);
	if (bo->description.fd >= 0)
		close(bo->description.fd);
	free(bo);
}

static bool listed(uint64_t modifier, const uint64_t* modifiers, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		if (modifiers[i] == modifier)
			return true;
	}
	return false;
}

static bool bindBuffer(_IOSurfaceGPUBo* bo);

static bool createBo(struct Job* job)
{
	_IOSurfaceGPUBo* bo = job->bo;
	_IOSurfaceGPUBoDescription* d = &bo->description;
	if (!openDevice(job->renderNode))
		return false;

	if (gbm.bo_create_with_modifiers2)
		bo->bo = gbm.bo_create_with_modifiers2(g_device, d->width, d->height, d->fourcc, job->modifiers,
			(unsigned int) job->modifierCount, H_GBM_BO_USE_RENDERING);
	if (bo->bo == NULL && gbm.bo_create_with_modifiers)
		bo->bo = gbm.bo_create_with_modifiers(g_device, d->width, d->height, d->fourcc, job->modifiers,
			(unsigned int) job->modifierCount);
	if (bo->bo == NULL)
		return false;

	// One plane at offset 0, in a layout the caller can render into: a
	// linear layout is what the surface's memory already has.
	d->modifier = gbm.bo_get_modifier(bo->bo);
	if (gbm.bo_get_plane_count(bo->bo) != 1 || gbm.bo_get_offset(bo->bo, 0) != 0
		|| d->modifier == IOSURFACE_DRM_FORMAT_MOD_LINEAR || d->modifier == H_DRM_FORMAT_MOD_INVALID
		|| !listed(d->modifier, job->modifiers, job->modifierCount))
		return false;
	d->offset = 0;
	d->stride = gbm.bo_get_stride_for_plane(bo->bo, 0);
	d->fd = gbm.bo_get_fd(bo->bo);
	if (d->fd < 0 || d->stride == 0 || d->stride > INT32_MAX)
		return false;
	return bindBuffer(bo);
}

// Makes the buffer's DMA-BUF a texture and a framebuffer of the helper's
// context, and checks that they are complete and of the buffer's size.
static bool bindBuffer(_IOSurfaceGPUBo* bo)
{
	_IOSurfaceGPUBoDescription* d = &bo->description;
	HostEGLint attributes[] = {
		H_EGL_WIDTH, (HostEGLint) d->width,
		H_EGL_HEIGHT, (HostEGLint) d->height,
		H_EGL_LINUX_DRM_FOURCC_EXT, (HostEGLint) d->fourcc,
		H_EGL_DMA_BUF_PLANE0_FD_EXT, d->fd,
		H_EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		H_EGL_DMA_BUF_PLANE0_PITCH_EXT, (HostEGLint) d->stride,
		H_EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, (HostEGLint) (d->modifier & 0xffffffff),
		H_EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, (HostEGLint) (d->modifier >> 32),
		H_EGL_NONE,
	};
	bo->image = egl.createImage(g_display, NULL, H_EGL_LINUX_DMA_BUF_EXT, NULL, attributes);
	if (bo->image == NULL)
		return false;

	clearGLErrors();
	gl.genTextures(1, &bo->texture);
	gl.bindTexture(H_GL_TEXTURE_2D, bo->texture);
	gl.imageTargetTexture2D(H_GL_TEXTURE_2D, bo->image);
	gl.texParameteri(H_GL_TEXTURE_2D, H_GL_TEXTURE_MIN_FILTER, H_GL_NEAREST);
	gl.texParameteri(H_GL_TEXTURE_2D, H_GL_TEXTURE_MAG_FILTER, H_GL_NEAREST);
	int32_t width = 0, height = 0;
	gl.getTexLevelParameteriv(H_GL_TEXTURE_2D, 0, H_GL_TEXTURE_WIDTH, &width);
	gl.getTexLevelParameteriv(H_GL_TEXTURE_2D, 0, H_GL_TEXTURE_HEIGHT, &height);
	gl.bindTexture(H_GL_TEXTURE_2D, 0);
	if (gl.getError() != H_GL_NO_ERROR || width != (int32_t) d->width || height != (int32_t) d->height)
		return false;

	gl.genFramebuffers(1, &bo->framebuffer);
	gl.bindFramebuffer(H_GL_FRAMEBUFFER, bo->framebuffer);
	gl.framebufferTexture2D(H_GL_FRAMEBUFFER, H_GL_COLOR_ATTACHMENT0, H_GL_TEXTURE_2D, bo->texture, 0);
	uint32_t status = gl.checkFramebufferStatus(H_GL_FRAMEBUFFER);
	gl.bindFramebuffer(H_GL_FRAMEBUFFER, 0);
	return status == H_GL_FRAMEBUFFER_COMPLETE && gl.getError() == H_GL_NO_ERROR;
}

// A buffer another process made: only its DMA-BUF is used, no buffer
// object of this process's buffer manager.
static bool importBo(struct Job* job)
{
	_IOSurfaceGPUBo* bo = job->bo;
	if (!openDevice(job->renderNode))
		return false;
	return bindBuffer(bo);
}

static bool download(struct Job* job)
{
	_IOSurfaceGPUBo* bo = job->bo;
	clearGLErrors();
	gl.bindFramebuffer(H_GL_FRAMEBUFFER, bo->framebuffer);
	gl.pixelStorei(H_GL_PACK_ROW_LENGTH, (int32_t) (job->bytesPerRow / 4));
	gl.pixelStorei(H_GL_PACK_ALIGNMENT, 4);
	gl.readPixels(0, 0, (int32_t) bo->description.width, (int32_t) bo->description.height, bo->glFormat,
		H_GL_UNSIGNED_BYTE, job->memory);
	gl.bindFramebuffer(H_GL_FRAMEBUFFER, 0);
	return gl.getError() == H_GL_NO_ERROR;
}

static bool upload(struct Job* job)
{
	_IOSurfaceGPUBo* bo = job->bo;
	clearGLErrors();
	gl.bindTexture(H_GL_TEXTURE_2D, bo->texture);
	gl.pixelStorei(H_GL_UNPACK_ROW_LENGTH, (int32_t) (job->bytesPerRow / 4));
	gl.pixelStorei(H_GL_UNPACK_ALIGNMENT, 4);
	gl.texSubImage2D(H_GL_TEXTURE_2D, 0, 0, 0, (int32_t) bo->description.width,
		(int32_t) bo->description.height, bo->glFormat, H_GL_UNSIGNED_BYTE, job->memory);
	gl.bindTexture(H_GL_TEXTURE_2D, 0);
	// Other contexts use the buffer next; they see the upload once it has
	// completed.
	gl.finish();
	return gl.getError() == H_GL_NO_ERROR;
}

static void* helperThread(void* unused)
{
	(void) unused;
	pthread_setname_np("IOSurface GPU buffers");
	pthread_mutex_lock(&g_jobLock);
	for (;;)
	{
		while (g_job == NULL)
			pthread_cond_wait(&g_jobCond, &g_jobLock);
		struct Job* job = g_job;
		g_job = NULL;
		pthread_mutex_unlock(&g_jobLock);

		switch (job->kind)
		{
			case JOB_CREATE:
				job->ok = createBo(job);
				break;
			case JOB_IMPORT:
				job->ok = importBo(job);
				break;
			case JOB_DESTROY:
				releaseBo(job->bo);
				job->ok = true;
				break;
			case JOB_DOWNLOAD:
				job->ok = download(job);
				break;
			case JOB_UPLOAD:
				job->ok = upload(job);
				break;
		}

		pthread_mutex_lock(&g_jobLock);
		job->done = true;
		pthread_cond_broadcast(&g_jobCond);
	}
	return NULL;
}

static bool runJob(struct Job* job)
{
	if (g_disabled)
		return false;

	pthread_mutex_lock(&g_submitLock);
	if (!g_started)
	{
		pthread_t thread;
		pthread_attr_t attributes;
		pthread_attr_init(&attributes);
		pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
		g_started = pthread_create(&thread, &attributes, helperThread, NULL) == 0;
		pthread_attr_destroy(&attributes);
		if (!g_started)
		{
			pthread_mutex_unlock(&g_submitLock);
			return false;
		}
	}

	pthread_mutex_lock(&g_jobLock);
	job->done = false;
	g_job = job;
	pthread_cond_broadcast(&g_jobCond);
	while (!job->done)
		pthread_cond_wait(&g_jobCond, &g_jobLock);
	pthread_mutex_unlock(&g_jobLock);
	pthread_mutex_unlock(&g_submitLock);
	return job->ok;
}

// A new buffer record for width x height pixels in DRM format fourcc, or
// NULL if the format or size is not one buffers are made in.
static _IOSurfaceGPUBo* newBo(uint32_t width, uint32_t height, uint32_t fourcc)
{
	if (width == 0 || height == 0 || width > INT32_MAX || height > INT32_MAX)
		return NULL;

	uint32_t glFormat;
	switch (fourcc)
	{
		case IOSURFACE_DRM_FORMAT_ARGB8888:
			glFormat = H_GL_BGRA;
			break;
		case IOSURFACE_DRM_FORMAT_ABGR8888:
			glFormat = H_GL_RGBA;
			break;
		default:
			return NULL;
	}

	_IOSurfaceGPUBo* bo = calloc(1, sizeof(*bo));
	if (bo == NULL)
		return NULL;
	bo->glFormat = glFormat;
	bo->description = (_IOSurfaceGPUBoDescription) {
		.fd = -1,
		.fourcc = fourcc,
		.width = width,
		.height = height,
	};
	return bo;
}

// Runs a job that sets up bo, and releases bo if it fails.
static _IOSurfaceGPUBo* runSetup(struct Job* job)
{
	_IOSurfaceGPUBo* bo = job->bo;
	if (runJob(job))
		return bo;

	// A buffer that could not be made complete is released on the
	// helper, where its parts were made.
	if (g_started && !g_disabled)
	{
		struct Job destroy = { .kind = JOB_DESTROY, .bo = bo };
		runJob(&destroy);
	}
	else
	{
		if (bo->description.fd >= 0)
			close(bo->description.fd);
		free(bo);
	}
	return NULL;
}

_IOSurfaceGPUBo* _IOSurfaceGPUBoCreate(const char* renderNode, uint32_t width, uint32_t height,
	uint32_t fourcc, const uint64_t* modifiers, size_t modifierCount)
{
	if (g_disabled || renderNode == NULL || modifiers == NULL || modifierCount == 0)
		return NULL;
	_IOSurfaceGPUBo* bo = newBo(width, height, fourcc);
	if (bo == NULL)
		return NULL;

	struct Job job = {
		.kind = JOB_CREATE,
		.bo = bo,
		.renderNode = renderNode,
		.modifiers = modifiers,
		.modifierCount = modifierCount,
	};
	return runSetup(&job);
}

_IOSurfaceGPUBo* _IOSurfaceGPUBoImport(const char* renderNode, const _IOSurfaceGPUBoDescription* description)
{
	if (g_disabled || renderNode == NULL || description == NULL || description->fd < 0)
		return NULL;
	// One plane at offset 0, in a layout that is not linear (see createBo).
	if (description->offset != 0 || description->stride == 0 || description->stride > INT32_MAX
		|| description->modifier == IOSURFACE_DRM_FORMAT_MOD_LINEAR
		|| description->modifier == H_DRM_FORMAT_MOD_INVALID)
		return NULL;
	_IOSurfaceGPUBo* bo = newBo(description->width, description->height, description->fourcc);
	if (bo == NULL)
		return NULL;
	bo->description.modifier = description->modifier;
	bo->description.stride = description->stride;
	bo->description.fd = fcntl(description->fd, F_DUPFD_CLOEXEC, 0);
	if (bo->description.fd < 0)
	{
		free(bo);
		return NULL;
	}

	struct Job job = {
		.kind = JOB_IMPORT,
		.bo = bo,
		.renderNode = renderNode,
	};
	return runSetup(&job);
}

void _IOSurfaceGPUBoDestroy(_IOSurfaceGPUBo* bo)
{
	if (bo == NULL || g_disabled)
		return;
	struct Job job = { .kind = JOB_DESTROY, .bo = bo };
	runJob(&job);
}

void _IOSurfaceGPUBoDescribe(const _IOSurfaceGPUBo* bo, _IOSurfaceGPUBoDescription* description)
{
	*description = bo->description;
}

bool _IOSurfaceGPUBoOnRenderNode(const _IOSurfaceGPUBo* bo, const char* renderNode)
{
	(void) bo;
	return renderNode != NULL && strcmp(g_renderNode, renderNode) == 0;
}

bool _IOSurfaceGPUBoDownload(_IOSurfaceGPUBo* bo, void* memory, size_t bytesPerRow)
{
	if (bo == NULL || memory == NULL || bytesPerRow % 4 != 0 || bytesPerRow / 4 > INT32_MAX)
		return false;
	struct Job job = { .kind = JOB_DOWNLOAD, .bo = bo, .memory = memory, .bytesPerRow = bytesPerRow };
	return runJob(&job);
}

bool _IOSurfaceGPUBoUpload(_IOSurfaceGPUBo* bo, const void* memory, size_t bytesPerRow)
{
	if (bo == NULL || memory == NULL || bytesPerRow % 4 != 0 || bytesPerRow / 4 > INT32_MAX)
		return false;
	struct Job job = { .kind = JOB_UPLOAD, .bo = bo, .memory = (void*) memory, .bytesPerRow = bytesPerRow };
	return runJob(&job);
}

void _IOSurfaceGPUBoForked(void)
{
	// The helper thread does not exist in the child. The locks were free at
	// the fork: every job is handed over under IOSurface's own lock, which
	// the fork handler held.
	pthread_mutex_init(&g_submitLock, NULL);
	pthread_mutex_init(&g_jobLock, NULL);
	pthread_cond_init(&g_jobCond, NULL);
	g_job = NULL;
	if (g_started)
		g_disabled = true;
}
