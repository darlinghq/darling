// Copyright (C) 2026 Lubos Dolezel
//
// CGLTexImageIOSurface2D specifies a texture from an IOSurface. Whatever
// way the implementation takes (the surface's memory, GPU copies, or a copy
// of the pixels), the call must not leave a GL error behind for the
// application, must not disturb an error that was already pending, and the
// texture must show the surface's contents and render into the surface:
//  - no-error:  glGetError is GL_NO_ERROR after the call (also when the
//               same texture is specified again);
//  - pending:   an error pending before the call is still the first one
//               glGetError returns afterwards;
//  - contents:  the texture shows the surface's pixels;
//  - writeback: rendering into the texture reaches the surface after
//               glFlush.
// Each case runs for GL_TEXTURE_RECTANGLE and, where the implementation
// takes it (Darling does, macOS does not), GL_TEXTURE_2D.
//
// Needs an offscreen OpenGL context. AppKit is set up first so that the
// window system's display is the one used, unless the argument
// --no-window-system is given (an offscreen renderer of its own).
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_ARB
#endif

#define W 64
#define H 32
#define GREY 0xff808080u
#define RED 0xffff0000u

static int g_failures;

static void expect(int ok, const char *what, const char *target) {
	printf("%s: %s (%s)\n", ok ? "PASS" : "FAIL", what, target);
	if (!ok)
		g_failures++;
}

static void finish(int status) {
	fflush(stdout);
	_exit(status);
}

static void onAlarm(int sig) {
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value) {
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static IOSurfaceRef createSurface(uint32_t value) {
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	if (surface == NULL)
		return NULL;
	IOSurfaceLock(surface, 0, NULL);
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			*(uint32_t *) (base + y * bytesPerRow + x * 4) = value;
	IOSurfaceUnlock(surface, 0, NULL);
	return surface;
}

// The surface's pixels under a read-only lock: their common value, or
// 0xdeadbeef if the first and last differ.
static uint32_t cpuPixel(IOSurfaceRef surface) {
	IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL);
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	uint32_t first = *(uint32_t *) base;
	uint32_t last = *(uint32_t *) (base + (H - 1) * bytesPerRow + (W - 1) * 4);
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
	return first == last ? first : 0xdeadbeefu;
}

static CGLError specify(CGLContextObj context, GLenum target, IOSurfaceRef surface) {
	return CGLTexImageIOSurface2D(context, target, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
}

static void drainErrors(void) {
	for (int i = 0; i < 16 && glGetError() != GL_NO_ERROR; i++)
		;
}

// The common value of the pixels of the framebuffer, or 0xdeadbeef.
static uint32_t framebufferPixel(void) {
	static uint32_t pixels[W * H];
	glReadPixels(0, 0, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	return pixels[0] == pixels[W * H - 1] ? pixels[0] : 0xdeadbeefu;
}

static void runCases(CGLContextObj context, GLenum target) {
	const char *name = target == GL_TEXTURE_RECTANGLE ? "rectangle" : "2d";
	IOSurfaceRef surface = createSurface(GREY);
	GLuint texture = 0, framebuffer = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	drainErrors();

	CGLError error = specify(context, target, surface);
	expect(error == kCGLNoError, "CGLTexImageIOSurface2D succeeds", name);
	expect(glGetError() == GL_NO_ERROR, "no-error: the call leaves no error", name);

	// Specifying the same texture from the surface again.
	error = specify(context, target, surface);
	expect(error == kCGLNoError && glGetError() == GL_NO_ERROR, "no-error: specifying again", name);

	// An error pending before the call stays the application's.
	glActiveTexture(0xffff);
	error = specify(context, target, surface);
	expect(error == kCGLNoError, "pending: CGLTexImageIOSurface2D succeeds", name);
	expect(glGetError() == GL_INVALID_ENUM, "pending: an earlier error is still reported first", name);
	expect(glGetError() == GL_NO_ERROR, "pending: and the call added none", name);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
	expect(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer complete", name);
	glViewport(0, 0, W, H);

	expect(framebufferPixel() == GREY, "contents: the texture shows the surface", name);

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	expect(cpuPixel(surface) == RED, "writeback: rendering reaches the surface", name);
	expect(glGetError() == GL_NO_ERROR, "no-error: after rendering and flush", name);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	CFRelease(surface);
}

int main(int argc, char **argv) {
	signal(SIGALRM, onAlarm);
	alarm(50);

	if (argc < 2 || strcmp(argv[1], "--no-window-system") != 0) {
		@autoreleasepool {
			[NSApplication sharedApplication];
		}
	}

	CGLPixelFormatAttribute attrs[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
		kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
		(CGLPixelFormatAttribute) 0
	};
	GLint screens = 0;
	CGLPixelFormatObj pixelFormat = NULL;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &pixelFormat, &screens) != kCGLNoError || pixelFormat == NULL
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError || context == NULL) {
		printf("FAIL: no offscreen context\n");
		finish(1);
	}
	CGLSetCurrentContext(context);
	printf("renderer: %s\n", (const char *) glGetString(GL_RENDERER));

	const GLenum targets[] = { GL_TEXTURE_RECTANGLE, GL_TEXTURE_2D };
	for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
		// macOS rejects GL_TEXTURE_2D for IOSurfaces.
		if (targets[i] == GL_TEXTURE_2D) {
			GLuint probe = 0;
			IOSurfaceRef surface = createSurface(GREY);
			glGenTextures(1, &probe);
			glBindTexture(GL_TEXTURE_2D, probe);
			CGLError error = specify(context, GL_TEXTURE_2D, surface);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &probe);
			CFRelease(surface);
			drainErrors();
			if (error != kCGLNoError)
				continue;
		}
		runCases(context, targets[i]);
	}

	CGLSetCurrentContext(NULL);
	CGLReleaseContext(context);
	CGLReleasePixelFormat(pixelFormat);
	printf(g_failures == 0 ? "PASS\n" : "FAIL: %d\n", g_failures);
	finish(g_failures == 0 ? 0 : 1);
}
