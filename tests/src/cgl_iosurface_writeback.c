// Copyright (C) 2026 Darling Developers
//
// On macOS an IOSurface is the storage of the texture that
// CGLTexImageIOSurface2D binds to it, so locking the surface shows the GPU's
// rendering into that texture. Darling's texture is the same when the host
// imports the surface's memory as a DMA-BUF. Otherwise it is a copy, which
// OpenGL.framework writes back into the surface when the surface is
// locked. The writeback must not overwrite CPU writes made to the surface
// since the texture was last specified from it.
//
// Needs an offscreen OpenGL context (a host EGL driver).
#define GL_GLEXT_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/CGLInternal.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define W 16
#define H 8

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

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

// Fills the surface with one BGRA value (as a little-endian 32-bit word).
static void fill(IOSurfaceRef surface, uint32_t value)
{
	CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
	unsigned char* base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			*(uint32_t*) (base + y * bytesPerRow + x * 4) = value;
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
}

// The first and last pixel, read under a read-only lock.
static void corners(IOSurfaceRef surface, uint32_t* first, uint32_t* last)
{
	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	unsigned char* base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	*first = *(uint32_t*) base;
	*last = *(uint32_t*) (base + (H - 1) * bytesPerRow + (W - 1) * 4);
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
}

static void clearTexture(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
}

#define RED 0xffff0000u
#define GREEN 0xff00ff00u
#define BLUE 0xff0000ffu
#define GREY 0xff808080u

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	CHECK(surface != NULL);
	if (surface == NULL)
		finish(1);
	fill(surface, GREY);

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

	GLuint texture = 0, framebuffer = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError);

	// A CPU write after the call shows in the texture only if the texture
	// uses the surface's memory; specifying it again brings it in anyway.
	fill(surface, GREEN);
	static uint32_t texels[W * H];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texels);
	int imported = texels[0] == GREEN;
	printf("GL texture: %s\n", imported ? "imported" : "copied");
	CHECK(imported || texels[0] == GREY);
	CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glViewport(0, 0, W, H);

	// Rendering into the texture is visible under the next lock.
	uint32_t first = 0, last = 0;
	clearTexture(1.0f, 0.0f, 0.0f);
	corners(surface, &first, &last);
	printf("after rendering red: %08x %08x\n", first, last);
	CHECK(first == RED && last == RED);

	// So is rendering after that.
	clearTexture(0.0f, 0.0f, 1.0f);
	corners(surface, &first, &last);
	printf("after rendering blue: %08x %08x\n", first, last);
	CHECK(first == BLUE && last == BLUE);

	// A CPU write is newer than a copied texture: rendering that the
	// texture has not been specified from since does not replace it. A
	// texture that uses the surface's memory renders into it, as on macOS.
	fill(surface, GREEN);
	clearTexture(1.0f, 0.0f, 0.0f);
	corners(surface, &first, &last);
	printf("after a CPU write: %08x %08x\n", first, last);
	if (imported)
		CHECK(first == RED && last == RED);
	else
		CHECK(first == GREEN && last == GREEN);

	// Specifying the texture from the surface again resumes the writeback.
	CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	clearTexture(0.0f, 0.0f, 1.0f);
	corners(surface, &first, &last);
	printf("after specifying again: %08x %08x\n", first, last);
	CHECK(first == BLUE && last == BLUE);

	// The explicit readback still works and marks the surface modified.
	clearTexture(1.0f, 0.0f, 0.0f);
	uint32_t seed = IOSurfaceGetSeed(surface);
	CHECK(CGLReadbackIOSurface(surface) == kCGLNoError);
	CHECK(IOSurfaceGetSeed(surface) != seed);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	CGLReleasePixelFormat(pixelFormat);
	CFRelease(surface);

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
