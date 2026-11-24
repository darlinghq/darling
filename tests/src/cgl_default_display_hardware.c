// Copyright (C) 2026 Lubos Dolezel
//
// A process that never opens a window (a GPU helper, an offscreen renderer)
// and creates an OpenGL context on a host that has a GPU gets the GPU's
// renderer, also when the session it runs in has a window system: DISPLAY or
// WAYLAND_DISPLAY set must not turn the context into one on the host's
// software rasterizer. The program queries the displays first, as such
// processes do, and renders into an IOSurface-backed texture.
//
// Where the host has no GPU device (no NVIDIA device, no DRM render node) or
// the process has no window system in its environment, any renderer is
// accepted; the test needs a window system session on a GPU host to be able
// to fail. Run it with DISPLAY and/or WAYLAND_DISPLAY pointing at a server
// (an X server on the NVIDIA driver, a Wayland compositor on a GPU).
#define GL_GLEXT_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static int is_software_name(const char *renderer)
{
	static const char *const names[] = { "llvmpipe", "lavapipe", "softpipe", "swrast", "Software Rasterizer" };
	for (size_t i = 0; renderer && i < sizeof(names) / sizeof(names[0]); i++)
		if (strstr(renderer, names[i]))
			return 1;
	return 0;
}

// The host's file system is visible under /Volumes/SystemRoot.
static int host_has_gpu(void)
{
	return access("/Volumes/SystemRoot/dev/nvidiactl", F_OK) == 0
		|| access("/Volumes/SystemRoot/dev/dri/renderD128", F_OK) == 0;
}

static int has_window_system(void)
{
	const char *x = getenv("DISPLAY"), *wl = getenv("WAYLAND_DISPLAY");
	return (x && *x) || (wl && *wl);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(60);

	// What a GPU process does before it creates its context.
	CGDirectDisplayID displays[8];
	uint32_t count = 0;
	CHECK(CGGetActiveDisplayList(8, displays, &count) == kCGErrorSuccess);

	CGLPixelFormatAttribute attrs[] = { kCGLPFAAllowOfflineRenderers, 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
	CGLContextObj context = NULL;
	CHECK(CGLCreateContext(format, NULL, &context) == kCGLNoError);
	CGLDestroyPixelFormat(format);
	if (!context) {
		printf("FAIL: no context\n");
		return 1;
	}
	CHECK(CGLSetCurrentContext(context) == kCGLNoError);
	const char *renderer = (const char *) glGetString(GL_RENDERER);
	printf("renderer: %s (window system in the environment: %d, GPU on the host: %d)\n",
		renderer ? renderer : "(none)", has_window_system(), host_has_gpu());
	CHECK(renderer != NULL);
	if (host_has_gpu() && has_window_system())
		CHECK(!is_software_name(renderer));

	// The context works: render into an IOSurface-backed texture.
	int value = 16;
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	int four = 4;
	CFNumberRef bytes = CFNumberCreate(NULL, kCFNumberIntType, &four);
	int format_code = 'BGRA';
	CFNumberRef pixel_format = CFNumberCreate(NULL, kCFNumberIntType, &format_code);
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(props, kIOSurfaceWidth, number);
	CFDictionarySetValue(props, kIOSurfaceHeight, number);
	CFDictionarySetValue(props, kIOSurfaceBytesPerElement, bytes);
	CFDictionarySetValue(props, kIOSurfacePixelFormat, pixel_format);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	CFRelease(number);
	CFRelease(bytes);
	CFRelease(pixel_format);
	CHECK(surface != NULL);
	if (surface) {
		GLuint texture = 0, framebuffer = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
		CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, 16, 16, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError);
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, texture, 0);
		CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		unsigned char pixel[4] = { 0, 0, 0, 0 };
		glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		CHECK(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0);
		CHECK(glGetError() == GL_NO_ERROR);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &texture);
		CFRelease(surface);
	}

	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	if (g_failures) {
		printf("%d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
