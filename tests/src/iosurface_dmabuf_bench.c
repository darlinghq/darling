// Copyright (C) 2026 Lubos Dolezel
//
// Times the round trip that IOSurface-based rendering makes each frame: the
// CPU writes the surface under a lock, GL reads it through a texture
// specified from it with CGLTexImageIOSurface2D and renders into that
// texture, and the CPU reads the result under a read-only lock. With
// DMA-BUF export and import the texture uses the surface's memory, so there
// is no upload and no copy back; otherwise both happen every frame.
//
// Usage: iosurface_dmabuf_bench [width height [frames]]
// Prints the time per frame and per step, and which way the texture went.
// Every frame's result is checked, so a fast but wrong path fails.
//
// Needs an offscreen OpenGL context (a host EGL driver).
#define GL_GLEXT_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <mach/mach_time.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static mach_timebase_info_data_t g_timebase;

static double microseconds(uint64_t ticks)
{
	return (double) ticks * g_timebase.numer / g_timebase.denom / 1000.0;
}

int main(int argc, char** argv)
{
	signal(SIGALRM, on_alarm);
	alarm(300);
	mach_timebase_info(&g_timebase);

	int width = argc > 2 ? atoi(argv[1]) : 1920;
	int height = argc > 2 ? atoi(argv[2]) : 1080;
	int frames = argc > 3 ? atoi(argv[3]) : 100;
	if (width <= 0 || height <= 0 || frames <= 0)
	{
		printf("usage: %s [width height [frames]]\n", argv[0]);
		finish(2);
	}

	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, width);
	setNumber(props, kIOSurfaceHeight, height);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, (int) kCVPixelFormatType_32BGRA);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	if (!surface)
	{
		printf("FAIL: no surface\n");
		finish(1);
	}

	CGLPixelFormatAttribute attrs[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &pixelFormat, &screens) != kCGLNoError
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError
		|| CGLSetCurrentContext(context) != kCGLNoError)
	{
		printf("FAIL: no offscreen context\n");
		finish(1);
	}
	printf("GL renderer: %s\n", (const char*) glGetString(GL_RENDERER));

	GLuint texture, framebuffer;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glGenFramebuffers(1, &framebuffer);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);

	uint64_t write = 0, upload = 0, render = 0, read = 0;
	int failures = 0, imported = -1;
	for (int frame = 0; frame < frames; frame++)
	{
		// The CPU fills the surface.
		uint64_t start = mach_absolute_time();
		IOSurfaceLock(surface, 0, NULL);
		uint8_t* base = IOSurfaceGetBaseAddress(surface);
		for (int y = 0; y < height; y++)
			memset(base + y * bytesPerRow, frame & 0xff, (size_t) width * 4);
		IOSurfaceUnlock(surface, 0, NULL);
		uint64_t written = mach_absolute_time();

		// GL takes the new contents.
		CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, width, height, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
		if (imported < 0)
		{
			// The first frame finds out whether a later CPU write shows in
			// the texture, which only one that uses the surface's memory
			// does.
			IOSurfaceLock(surface, 0, NULL);
			base[0] = (uint8_t) ~frame;
			IOSurfaceUnlock(surface, 0, NULL);
			uint8_t texel[4];
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			GLint previous = 0;
			glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			glReadPixels(0, 0, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texel);
			glBindFramebuffer(GL_FRAMEBUFFER, (GLuint) previous);
			imported = texel[0] == (uint8_t) ~frame;
			printf("GL texture: %s\n", imported ? "imported (DMA-BUF)" : "copied");
			CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, width, height, GL_BGRA,
				GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
			written = mach_absolute_time();
		}
		uint64_t uploaded = mach_absolute_time();

		// GL renders into it: a color that depends on the CPU's fill shows
		// that the texture had it.
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glViewport(0, 0, width, height);
		uint8_t sample[4];
		glReadPixels(0, 0, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sample);
		if (sample[0] != (frame & 0xff))
			failures++;
		glClearColor(0.0f, (float) ((frame + 1) & 0xff) / 255.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		uint64_t rendered = mach_absolute_time();

		// The CPU reads the result.
		IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL);
		uint8_t green = base[(height - 1) * bytesPerRow + (width - 1) * 4 + 1];
		IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
		uint64_t done = mach_absolute_time();
		if (green != ((frame + 1) & 0xff))
			failures++;

		write += written - start;
		upload += uploaded - written;
		render += rendered - uploaded;
		read += done - rendered;
	}

	double total = microseconds(write + upload + render + read) / frames;
	printf("%dx%d BGRA, %d frames: %.1f us/frame (CPU write %.1f, CGLTexImageIOSurface2D %.1f, "
		"GL read+render %.1f, lock and CPU read %.1f)\n", width, height, frames, total,
		microseconds(write) / frames, microseconds(upload) / frames, microseconds(render) / frames,
		microseconds(read) / frames);
	printf("backend: %s\n", imported ? "dmabuf" : "copy");

	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	CGLReleasePixelFormat(pixelFormat);
	CFRelease(surface);

	if (failures)
		printf("FAIL: %d frame(s) with wrong contents\n", failures);
	else
		printf("PASS\n");
	finish(failures ? 1 : 0);
}
