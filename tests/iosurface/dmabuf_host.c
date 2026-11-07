// Copyright (C) 2026 Lubos Dolezel
//
// Host-side checks for IOSurface's DMA-BUF export
// (src/frameworks/IOSurface/src/dmabuf.c).
//
// "abi": the Linux constants and structures that IOSurface spells out in
// linux_dmabuf.h, and the DRM formats that OpenGL.framework spells out in
// drm_formats.h, because both are built against the Darwin SDK, equal the
// kernel's own headers.
//
// "gl": the host can do what IOSurface and OpenGL.framework do with a
// surface: a sealed memfd with a header page and pixel pages after it, a
// udmabuf DMA-BUF of the pixel pages, imported with
// EGL_EXT_image_dma_buf_import into a GL texture. CPU writes through the
// memfd mapping are seen by GL, and GL rendering is seen by the CPU, with no
// copy. Exits 77 (skipped) when /dev/udmabuf, a surfaceless EGL display or
// dma-buf import is missing; that is a host that runs IOSurface on its
// shared memory fallback.
#define _GNU_SOURCE
#include "linux_dmabuf.h"
#include "drm_formats.h"

#include <drm/drm_fourcc.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <linux/udmabuf.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

_Static_assert(LINUX_MFD_CLOEXEC == MFD_CLOEXEC, "MFD_CLOEXEC");
_Static_assert(LINUX_MFD_ALLOW_SEALING == MFD_ALLOW_SEALING, "MFD_ALLOW_SEALING");
_Static_assert(LINUX_F_ADD_SEALS == F_ADD_SEALS, "F_ADD_SEALS");
_Static_assert(LINUX_F_SEAL_SHRINK == F_SEAL_SHRINK, "F_SEAL_SHRINK");
_Static_assert(LINUX_O_RDWR == O_RDWR, "O_RDWR");
_Static_assert(LINUX_O_CLOEXEC == O_CLOEXEC, "O_CLOEXEC");
_Static_assert(LINUX_EINTR == EINTR, "EINTR");
_Static_assert(LINUX_EAGAIN == EAGAIN, "EAGAIN");
_Static_assert(LINUX_UDMABUF_FLAGS_CLOEXEC == UDMABUF_FLAGS_CLOEXEC, "UDMABUF_FLAGS_CLOEXEC");
_Static_assert(LINUX_UDMABUF_CREATE == UDMABUF_CREATE, "UDMABUF_CREATE");
_Static_assert(sizeof(struct linux_udmabuf_create) == sizeof(struct udmabuf_create), "udmabuf_create");
_Static_assert(__builtin_offsetof(struct linux_udmabuf_create, offset) == __builtin_offsetof(struct udmabuf_create, offset), "udmabuf_create");
_Static_assert(__builtin_offsetof(struct linux_udmabuf_create, size) == __builtin_offsetof(struct udmabuf_create, size), "udmabuf_create");
_Static_assert(LINUX_DMA_BUF_SYNC_READ == DMA_BUF_SYNC_READ, "DMA_BUF_SYNC_READ");
_Static_assert(LINUX_DMA_BUF_SYNC_WRITE == DMA_BUF_SYNC_WRITE, "DMA_BUF_SYNC_WRITE");
_Static_assert(LINUX_DMA_BUF_SYNC_RW == DMA_BUF_SYNC_RW, "DMA_BUF_SYNC_RW");
_Static_assert(LINUX_DMA_BUF_SYNC_START == DMA_BUF_SYNC_START, "DMA_BUF_SYNC_START");
_Static_assert(LINUX_DMA_BUF_SYNC_END == DMA_BUF_SYNC_END, "DMA_BUF_SYNC_END");
_Static_assert(LINUX_DMA_BUF_IOCTL_SYNC == DMA_BUF_IOCTL_SYNC, "DMA_BUF_IOCTL_SYNC");
_Static_assert(sizeof(struct linux_dma_buf_sync) == sizeof(struct dma_buf_sync), "dma_buf_sync");

_Static_assert(CGL_DRM_FORMAT_ARGB8888 == DRM_FORMAT_ARGB8888, "DRM_FORMAT_ARGB8888");
_Static_assert(CGL_DRM_FORMAT_XRGB8888 == DRM_FORMAT_XRGB8888, "DRM_FORMAT_XRGB8888");
_Static_assert(CGL_DRM_FORMAT_ABGR8888 == DRM_FORMAT_ABGR8888, "DRM_FORMAT_ABGR8888");
_Static_assert(CGL_DRM_FORMAT_XBGR8888 == DRM_FORMAT_XBGR8888, "DRM_FORMAT_XBGR8888");
_Static_assert(CGL_DRM_FORMAT_BGRA8888 == DRM_FORMAT_BGRA8888, "DRM_FORMAT_BGRA8888");
_Static_assert(CGL_DRM_FORMAT_BGRX8888 == DRM_FORMAT_BGRX8888, "DRM_FORMAT_BGRX8888");
_Static_assert(CGL_DRM_FORMAT_RGBA8888 == DRM_FORMAT_RGBA8888, "DRM_FORMAT_RGBA8888");
_Static_assert(CGL_DRM_FORMAT_RGBX8888 == DRM_FORMAT_RGBX8888, "DRM_FORMAT_RGBX8888");
_Static_assert(CGL_DRM_FORMAT_R8 == DRM_FORMAT_R8, "DRM_FORMAT_R8");
_Static_assert(CGL_DRM_FORMAT_GR88 == DRM_FORMAT_GR88, "DRM_FORMAT_GR88");
_Static_assert(CGL_DRM_FORMAT_ABGR16161616F == DRM_FORMAT_ABGR16161616F, "DRM_FORMAT_ABGR16161616F");
_Static_assert(CGL_DRM_FORMAT_XBGR16161616F == DRM_FORMAT_XBGR16161616F, "DRM_FORMAT_XBGR16161616F");
_Static_assert(CGL_DRM_FORMAT_MOD_LINEAR == DRM_FORMAT_MOD_LINEAR, "DRM_FORMAT_MOD_LINEAR");

#define SKIPPED 77

#ifdef HAVE_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>

#define W 64
#define H 32
#define ROW 256

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static void cpuAccess(int dmabuf, uint64_t flags)
{
	struct linux_dma_buf_sync sync = { .flags = flags };
	CHECK(ioctl(dmabuf, LINUX_DMA_BUF_IOCTL_SYNC, &sync) == 0);
}

static int gl(void)
{
	long page = sysconf(_SC_PAGESIZE);
	size_t pixels = (size_t) ROW * H;
	size_t size = (pixels + page - 1) / page * page;

	int device = open("/dev/udmabuf", LINUX_O_RDWR | LINUX_O_CLOEXEC);
	if (device < 0)
	{
		printf("SKIP: /dev/udmabuf: %s\n", strerror(errno));
		return SKIPPED;
	}

	int memory = memfd_create("IOSurface", LINUX_MFD_CLOEXEC | LINUX_MFD_ALLOW_SEALING);
	CHECK(memory >= 0);
	CHECK(ftruncate(memory, page + size) == 0);
	CHECK(fcntl(memory, LINUX_F_ADD_SEALS, LINUX_F_SEAL_SHRINK) == 0);
	// Sealing again, as each process that exports the surface does.
	CHECK(fcntl(memory, LINUX_F_ADD_SEALS, LINUX_F_SEAL_SHRINK) == 0);
	struct linux_udmabuf_create create = {
		.memfd = (uint32_t) memory, .flags = LINUX_UDMABUF_FLAGS_CLOEXEC, .offset = page, .size = size,
	};
	int dmabuf = ioctl(device, LINUX_UDMABUF_CREATE, &create);
	if (dmabuf < 0)
	{
		printf("SKIP: UDMABUF_CREATE: %s\n", strerror(errno));
		return SKIPPED;
	}
	unsigned char* mapping = mmap(NULL, page + size, PROT_READ | PROT_WRITE, MAP_SHARED, memory, 0);
	CHECK(mapping != MAP_FAILED);
	if (mapping == MAP_FAILED)
		return 1;
	unsigned char* base = mapping + page;

	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
		(PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
	EGLDisplay display = getPlatformDisplay
		? getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL) : EGL_NO_DISPLAY;
	if (display == EGL_NO_DISPLAY || !eglInitialize(display, NULL, NULL) || !eglBindAPI(EGL_OPENGL_API))
	{
		printf("SKIP: no surfaceless EGL display\n");
		return SKIPPED;
	}
	const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
	if (!extensions || !strstr(extensions, "EGL_EXT_image_dma_buf_import"))
	{
		printf("SKIP: no EGL_EXT_image_dma_buf_import\n");
		return SKIPPED;
	}
	EGLContext context = eglCreateContext(display, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, NULL);
	if (context == EGL_NO_CONTEXT || !eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context))
	{
		printf("SKIP: no configless EGL context\n");
		return SKIPPED;
	}
	printf("GL renderer: %s\n", (const char*) glGetString(GL_RENDERER));

	// The CPU writes before the import and after it.
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			memcpy(base + y * ROW + x * 4, (unsigned char[]) { x, y, 7, 255 }, 4);

	PFNEGLCREATEIMAGEKHRPROC createImage = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
	PFNEGLDESTROYIMAGEKHRPROC destroyImage = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC targetTexture =
		(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
	CHECK(createImage && destroyImage && targetTexture);
	EGLint attributes[] = {
		EGL_WIDTH, W, EGL_HEIGHT, H,
		EGL_LINUX_DRM_FOURCC_EXT, (EGLint) CGL_DRM_FORMAT_ARGB8888,
		EGL_DMA_BUF_PLANE0_FD_EXT, dmabuf,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, ROW,
		EGL_NONE,
	};
	EGLImageKHR image = createImage(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attributes);
	if (image == EGL_NO_IMAGE_KHR)
	{
		printf("SKIP: the driver does not import the DMA-BUF (EGL error %#x)\n", eglGetError());
		return SKIPPED;
	}
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	targetTexture(GL_TEXTURE_2D, image);
	// The texture keeps the memory after the image is gone.
	destroyImage(display, image);
	CHECK(glGetError() == GL_NO_ERROR);

	memcpy(base + 3 * ROW + 5 * 4, (unsigned char[]) { 11, 22, 33, 44 }, 4);
	static unsigned char read[W * H * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, read);
	const unsigned char* texel = read + (3 * W + 5) * 4;
	printf("GL sees the CPU write: %u %u %u %u\n", texel[0], texel[1], texel[2], texel[3]);
	CHECK(texel[0] == 11 && texel[1] == 22 && texel[2] == 33 && texel[3] == 44);
	CHECK(read[(31 * W + 63) * 4 + 0] == 63 && read[(31 * W + 63) * 4 + 1] == 31);

	PFNGLGENFRAMEBUFFERSPROC genFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) eglGetProcAddress("glGenFramebuffers");
	PFNGLBINDFRAMEBUFFERPROC bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) eglGetProcAddress("glBindFramebuffer");
	PFNGLFRAMEBUFFERTEXTURE2DPROC framebufferTexture =
		(PFNGLFRAMEBUFFERTEXTURE2DPROC) eglGetProcAddress("glFramebufferTexture2D");
	PFNGLCHECKFRAMEBUFFERSTATUSPROC status =
		(PFNGLCHECKFRAMEBUFFERSTATUSPROC) eglGetProcAddress("glCheckFramebufferStatus");
	GLuint framebuffer;
	genFramebuffers(1, &framebuffer);
	bindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	framebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	CHECK(status(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glViewport(0, 0, W, H);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	cpuAccess(dmabuf, LINUX_DMA_BUF_SYNC_START | LINUX_DMA_BUF_SYNC_READ);
	uint32_t first, last;
	memcpy(&first, base, 4);
	memcpy(&last, base + (H - 1) * ROW + (W - 1) * 4, 4);
	cpuAccess(dmabuf, LINUX_DMA_BUF_SYNC_END | LINUX_DMA_BUF_SYNC_READ);
	printf("CPU sees the GL clear: %08x %08x\n", first, last);
	CHECK(first == 0xffff0000u && last == 0xffff0000u);
	// The header page in front of the exported range is untouched.
	CHECK(mapping[0] == 0 && mapping[page - 1] == 0);

	// A second DMA-BUF of the same memory, as a process that received the
	// surface makes, shows the same pixels.
	int second = ioctl(device, LINUX_UDMABUF_CREATE, &create);
	CHECK(second >= 0);
	unsigned char* view = second >= 0 ? mmap(NULL, size, PROT_READ, MAP_SHARED, second, 0) : MAP_FAILED;
	CHECK(view != MAP_FAILED);
	if (view != MAP_FAILED)
		CHECK(memcmp(view, base, size) == 0);

	if (failures == 0)
		printf("PASS\n");
	return failures == 0 ? 0 : 1;
}
#endif

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "gl") == 0)
	{
#ifdef HAVE_EGL
		return gl();
#else
		printf("SKIP: built without EGL\n");
		return SKIPPED;
#endif
	}
	// The ABI is checked at compile time.
	printf("PASS\n");
	return 0;
}
