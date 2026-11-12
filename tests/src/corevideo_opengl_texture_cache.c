// Copyright (C) 2026 Lubos Dolezel
//
// CVOpenGLTextureCache turns pixel buffers into GL_TEXTURE_RECTANGLE textures, as observed on
// macOS 27.0.1: the texture of a buffer backed by an IOSurface and that of a buffer in memory
// are flipped, with clean texture coordinates running from the bottom edge (t = height) to the
// top edge (t = 0), and their first row is the top row of the image. The same buffer gets the
// same texture name from a cache while the cache keeps it, planar formats are not OpenGL
// compatible, and the current context is left as it was. Needs an offscreen OpenGL context
// (a host EGL driver on Darling). Passes on macOS.
#define GL_GLEXT_PROTOTYPES 1
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_EXT 0x84F6
#endif

#define W 8
#define H 4

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void onAlarm(int sig)
{
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	_exit(1);
}

static CFDictionaryRef surfaceAttributes(void)
{
	CFDictionaryRef empty = CFDictionaryCreate(NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFStringRef key = kCVPixelBufferIOSurfacePropertiesKey;
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, (const void**) &key, (const void**) &empty, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFRelease(empty);
	return attributes;
}

// Blue, green and red bytes are functions of the pixel's position; alpha is opaque.
static uint8_t blue(int x) { return (uint8_t) (x * 30); }
static uint8_t green(int y) { return (uint8_t) (y * 60); }

static CVPixelBufferRef makeBuffer(CFDictionaryRef attributes)
{
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreate(NULL, W, H, kCVPixelFormatType_32BGRA, attributes, &buffer) == kCVReturnSuccess);
	if (!buffer)
		return NULL;
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);
	for (int y = 0; base && y < H; y++)
		for (int x = 0; x < W; x++) {
			uint8_t* pixel = base + y * bytesPerRow + x * 4;
			pixel[0] = blue(x);
			pixel[1] = green(y);
			pixel[2] = 200;
			pixel[3] = 255;
		}
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	return buffer;
}

// Reads the texture through a framebuffer: BGRA words, the first row of the texture first.
static void readTexture(GLenum target, GLuint name, uint8_t* pixels)
{
	GLuint framebuffer = 0;
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, name, 0);
	CHECK(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
	memset(pixels, 0x77, W * H * 4);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &framebuffer);
}

static void checkTexture(CVOpenGLTextureCacheRef cache, CVPixelBufferRef buffer, const char* what)
{
	CVOpenGLTextureRef texture = NULL;
	CVReturn result = CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, buffer, NULL, &texture);
	if (result != kCVReturnSuccess || !texture) {
		printf("FAIL %d: %s: texture creation returned %d\n", __LINE__, what, result);
		g_failures++;
		return;
	}
	CHECK(CVOpenGLTextureGetTarget(texture) == GL_TEXTURE_RECTANGLE_EXT);
	GLuint name = CVOpenGLTextureGetName(texture);
	CHECK(name != 0);
	CHECK(CVOpenGLTextureIsFlipped(texture));
	CHECK(CFGetTypeID(texture) == CVOpenGLTextureGetTypeID());

	GLfloat lowerLeft[2], lowerRight[2], upperRight[2], upperLeft[2];
	CVOpenGLTextureGetCleanTexCoords(texture, lowerLeft, lowerRight, upperRight, upperLeft);
	CHECK(lowerLeft[0] == 0 && lowerLeft[1] == H);
	CHECK(lowerRight[0] == W && lowerRight[1] == H);
	CHECK(upperRight[0] == W && upperRight[1] == 0);
	CHECK(upperLeft[0] == 0 && upperLeft[1] == 0);

	// The first row of the texture is the top row of the image.
	uint8_t pixels[W * H * 4];
	readTexture(CVOpenGLTextureGetTarget(texture), name, pixels);
	int wrong = 0;
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++) {
			const uint8_t* pixel = pixels + (y * W + x) * 4;
			if (pixel[0] != blue(x) || pixel[1] != green(y) || pixel[2] != 200 || pixel[3] != 255)
				wrong++;
		}
	if (wrong)
		printf("%s: %d of %d pixels differ\n", what, wrong, W * H);
	CHECK(wrong == 0);

	// The texture parameters are those of a texture that can be sampled at once.
	GLint previous = 0, filter = 0, wrap = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_EXT, &previous);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
	glGetTexParameteriv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, &filter);
	CHECK(filter == GL_LINEAR);
	glGetTexParameteriv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, &wrap);
	CHECK(wrap == GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, (GLuint) previous);

	// The same buffer gets the same name, in another texture object.
	CVOpenGLTextureRef second = NULL;
	CHECK(CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, buffer, NULL, &second) == kCVReturnSuccess);
	CHECK(second != NULL && second != texture && CVOpenGLTextureGetName(second) == name);
	CVOpenGLTextureRelease(second);
	CVOpenGLTextureRelease(texture);
}

int main(void)
{
	signal(SIGALRM, onAlarm);
	alarm(60);

	CGLPixelFormatAttribute formatAttributes[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(formatAttributes, &pixelFormat, &screens) != kCGLNoError
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError
		|| CGLSetCurrentContext(context) != kCGLNoError) {
		printf("FAIL: no offscreen OpenGL context\n");
		return 1;
	}

	// Creating a cache.
	CVOpenGLTextureCacheRef cache = NULL;
	CHECK(CVOpenGLTextureCacheCreate(NULL, NULL, context, pixelFormat, NULL, &cache) == kCVReturnSuccess);
	CHECK(cache != NULL);
	if (!cache)
		return 1;
	CHECK(CFGetTypeID(cache) == CVOpenGLTextureCacheGetTypeID());
	CHECK(CVOpenGLTextureCacheGetTypeID() != CVOpenGLTextureGetTypeID());
	CVOpenGLTextureCacheRef unused = NULL;
	CHECK(CVOpenGLTextureCacheCreate(NULL, NULL, NULL, pixelFormat, NULL, &unused) == kCVReturnInvalidArgument);
	CHECK(CVOpenGLTextureCacheCreate(NULL, NULL, context, pixelFormat, NULL, NULL) == kCVReturnInvalidArgument);

	// A buffer in a surface and a buffer in memory.
	CFDictionaryRef withSurface = surfaceAttributes();
	CVPixelBufferRef inSurface = makeBuffer(withSurface);
	CVPixelBufferRef inMemory = makeBuffer(NULL);
	CHECK(inSurface != NULL && CVPixelBufferGetIOSurface(inSurface) != NULL);
	CHECK(inMemory != NULL && CVPixelBufferGetIOSurface(inMemory) == NULL);
	if (inSurface)
		checkTexture(cache, inSurface, "IOSurface-backed buffer");
	if (inMemory)
		checkTexture(cache, inMemory, "buffer in memory");

	// Release and flush: textures can be made again.
	CVOpenGLTextureCacheFlush(cache, 0);
	CVOpenGLTextureRef again = NULL;
	CHECK(inSurface && CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, inSurface, NULL, &again) == kCVReturnSuccess);
	CHECK(again != NULL && CVOpenGLTextureGetName(again) != 0);
	CVOpenGLTextureRelease(again);

	// A change made to the pixels shows in the next texture of the buffer.
	if (inSurface) {
		CHECK(CVPixelBufferLockBaseAddress(inSurface, 0) == kCVReturnSuccess);
		uint8_t* base = CVPixelBufferGetBaseAddress(inSurface);
		if (base)
			base[0] = 99;
		CHECK(CVPixelBufferUnlockBaseAddress(inSurface, 0) == kCVReturnSuccess);
		CVOpenGLTextureRef changed = NULL;
		CHECK(CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, inSurface, NULL, &changed) == kCVReturnSuccess);
		if (changed) {
			uint8_t pixels[W * H * 4];
			readTexture(CVOpenGLTextureGetTarget(changed), CVOpenGLTextureGetName(changed), pixels);
			CHECK(pixels[0] == 99);
			CVOpenGLTextureRelease(changed);
		}
	}

	// A planar buffer is not OpenGL compatible; missing arguments are invalid.
	CVPixelBufferRef planar = NULL;
	CHECK(CVPixelBufferCreate(NULL, W, H, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, withSurface, &planar) == kCVReturnSuccess);
	CVOpenGLTextureRef texture = (CVOpenGLTextureRef) 1;
	CHECK(planar && CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, planar, NULL, &texture) == kCVReturnPixelBufferNotOpenGLCompatible);
	CHECK(texture == NULL);
	texture = (CVOpenGLTextureRef) 1;
	CHECK(CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, NULL, NULL, &texture) == kCVReturnInvalidArgument);
	CHECK(inSurface && CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, inSurface, NULL, NULL) == kCVReturnInvalidArgument);
	CVPixelBufferRelease(planar);

	// Other packed formats work too.
	OSType formats[] = { kCVPixelFormatType_32ARGB, kCVPixelFormatType_24RGB, kCVPixelFormatType_OneComponent8 };
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
		CVPixelBufferRef other = NULL;
		CHECK(CVPixelBufferCreate(NULL, W, H, formats[i], withSurface, &other) == kCVReturnSuccess);
		texture = NULL;
		CHECK(other && CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, other, NULL, &texture) == kCVReturnSuccess);
		CHECK(texture != NULL && CVOpenGLTextureGetTarget(texture) == GL_TEXTURE_RECTANGLE_EXT);
		CVOpenGLTextureRelease(texture);
		CVPixelBufferRelease(other);
	}

	// The cache did not change the current context.
	CHECK(CGLGetCurrentContext() == context);

	CVPixelBufferRelease(inSurface);
	CVPixelBufferRelease(inMemory);
	CFRelease(withSurface);
	CVOpenGLTextureCacheRetain(cache);
	CVOpenGLTextureCacheRelease(cache);
	CVOpenGLTextureCacheRelease(cache);
	CGLSetCurrentContext(NULL);
	CGLReleasePixelFormat(pixelFormat);
	CGLDestroyContext(context);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
