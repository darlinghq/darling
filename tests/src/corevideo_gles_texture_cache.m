/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/*
 * CVOpenGLESTextureCache and CVOpenGLESTexture on pixel buffers: the contract
 * is the public header documentation (error results, targets, planes, the
 * flip and the texture coordinates), checked by drawing the textures into a
 * framebuffer and reading it back. 64-bit only, as iOS's OpenGL ES is.
 *
 * Where the documentation is silent this test pins the choices of the
 * implementation, which are marked INFERENCE: the result code for each kind
 * of bad argument, and that the first row of a texture is the top row of the
 * image (as for CVOpenGLTexture on macOS).
 */

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <CoreVideo/CVOpenGLESTextureCache.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static GLuint program, fbo, rbo;

static void setup_target(int w, int h)
{
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
	const GLchar *vs = "attribute vec2 p; varying vec2 uv; void main(){ uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }";
	const GLchar *fs = "precision mediump float; varying vec2 uv; uniform sampler2D t; void main(){ gl_FragColor = texture2D(t, uv); }";
	GLuint v = glCreateShader(GL_VERTEX_SHADER), f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v, 1, &vs, NULL);
	glShaderSource(f, 1, &fs, NULL);
	glCompileShader(v);
	glCompileShader(f);
	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glBindAttribLocation(program, 0, "p");
	glLinkProgram(program);
}

/* Draws the texture over the whole w x h framebuffer, nearest-filtered, and reads it back. */
static void sample(GLuint name, int w, int h, unsigned char *out)
{
	static const GLfloat quad[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glViewport(0, 0, w, h);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "t"), 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quad);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, out);
}

static CVPixelBufferRef make_buffer(OSType format, size_t w, size_t h, BOOL surface)
{
	NSDictionary *attrs = surface
		? @{ (id)kCVPixelBufferIOSurfacePropertiesKey: @{}, (id)kCVPixelBufferBytesPerRowAlignmentKey: @64 }
		: @{ (id)kCVPixelBufferBytesPerRowAlignmentKey: @64 };
	CVPixelBufferRef buffer = NULL;
	CVReturn r = CVPixelBufferCreate(kCFAllocatorDefault, w, h, format, (__bridge CFDictionaryRef)attrs, &buffer);
	return r == kCVReturnSuccess ? buffer : NULL;
}

/* Fills the BGRA buffer: row 0 gets (b0,g0,r0), the others (b1,g1,r1); alpha 255. */
static void fill_bgra(CVPixelBufferRef buffer, const uint8_t *c0, const uint8_t *c1)
{
	CVPixelBufferLockBaseAddress(buffer, 0);
	uint8_t *base = CVPixelBufferGetBaseAddress(buffer);
	size_t stride = CVPixelBufferGetBytesPerRow(buffer);
	for (size_t y = 0; y < CVPixelBufferGetHeight(buffer); y++)
		for (size_t x = 0; x < CVPixelBufferGetWidth(buffer); x++) {
			uint8_t *p = base + y * stride + x * 4;
			const uint8_t *c = y == 0 ? c0 : c1;
			p[0] = c[0]; p[1] = c[1]; p[2] = c[2]; p[3] = 255;
		}
	CVPixelBufferUnlockBaseAddress(buffer, 0);
}

static void test_bgra(CVOpenGLESTextureCacheRef cache, BOOL surface)
{
	CVPixelBufferRef buffer = make_buffer(kCVPixelFormatType_32BGRA, 4, 2, surface);
	CHECK(buffer != NULL);
	if (buffer == NULL)
		return;
	const uint8_t top[3] = { 10, 20, 30 }, bottom[3] = { 100, 110, 120 };
	fill_bgra(buffer, top, bottom);

	CVOpenGLESTextureRef tex = NULL;
	CVReturn r = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL,
		GL_TEXTURE_2D, GL_RGBA, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0, &tex);
	CHECK(r == kCVReturnSuccess);
	CHECK(tex != NULL);
	if (tex == NULL) {
		CFRelease(buffer);
		return;
	}
	CHECK(CVOpenGLESTextureGetTarget(tex) == GL_TEXTURE_2D);
	CHECK(CVOpenGLESTextureGetName(tex) != 0);
	CHECK(CFGetTypeID(tex) == CVOpenGLESTextureGetTypeID());

	/* INFERENCE: first row of the texture is the top of the image. */
	CHECK(CVOpenGLESTextureIsFlipped(tex));
	GLfloat ll[2], lr[2], ur[2], ul[2];
	CVOpenGLESTextureGetCleanTexCoords(tex, ll, lr, ur, ul);
	CHECK(ul[0] == 0 && ul[1] == 0 && ur[0] == 1 && ur[1] == 0);
	CHECK(ll[0] == 0 && ll[1] == 1 && lr[0] == 1 && lr[1] == 1);

	/* The format GL_RGBA names red first: the buffer's BGRA bytes come out as red, green, blue. */
	unsigned char px[4 * 2 * 4];
	sample(CVOpenGLESTextureGetName(tex), 4, 2, px);
	CHECK(px[0] == 30 && px[1] == 20 && px[2] == 10 && px[3] == 255);                 /* texture row 0 */
	CHECK(px[4 * 4] == 120 && px[4 * 4 + 1] == 110 && px[4 * 4 + 2] == 100);          /* texture row 1 */

	/* The pixels are those of the buffer when the texture is asked for. */
	const uint8_t other[3] = { 1, 2, 3 };
	fill_bgra(buffer, other, other);
	CVOpenGLESTextureRef again = NULL;
	r = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL,
		GL_TEXTURE_2D, GL_RGBA, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0, &again);
	CHECK(r == kCVReturnSuccess && again != NULL);
	if (again != NULL) {
		sample(CVOpenGLESTextureGetName(again), 4, 2, px);
		CHECK(px[0] == 3 && px[1] == 2 && px[2] == 1);
		CFRelease(again);
	}

	/* An unused texture is deleted by a flush; one in use is not. */
	GLuint name = CVOpenGLESTextureGetName(tex);
	CVOpenGLESTextureCacheFlush(cache, 0);
	CHECK(glIsTexture(name));
	CFRelease(tex);
	CVOpenGLESTextureCacheFlush(cache, 0);
	CHECK(!glIsTexture(name));
	CFRelease(buffer);
}

static void test_planar(CVOpenGLESTextureCacheRef cache)
{
	CVPixelBufferRef buffer = make_buffer(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange, 4, 4, NO);
	CHECK(buffer != NULL);
	if (buffer == NULL)
		return;
	CVPixelBufferLockBaseAddress(buffer, 0);
	uint8_t *luma = CVPixelBufferGetBaseAddressOfPlane(buffer, 0), *chroma = CVPixelBufferGetBaseAddressOfPlane(buffer, 1);
	size_t ls = CVPixelBufferGetBytesPerRowOfPlane(buffer, 0), cs = CVPixelBufferGetBytesPerRowOfPlane(buffer, 1);
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			luma[y * ls + x] = (uint8_t)(16 + y * 10 + x);
	for (int y = 0; y < 2; y++)
		for (int x = 0; x < 2; x++) {
			chroma[y * cs + x * 2] = (uint8_t)(100 + y * 4 + x);       /* Cb */
			chroma[y * cs + x * 2 + 1] = (uint8_t)(200 + y * 4 + x);   /* Cr */
		}
	CVPixelBufferUnlockBaseAddress(buffer, 0);

	CVOpenGLESTextureRef y = NULL, uv = NULL;
	CVReturn r0 = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL,
		GL_TEXTURE_2D, GL_LUMINANCE, 4, 4, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0, &y);
	CVReturn r1 = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL,
		GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, 2, 2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 1, &uv);
	CHECK(r0 == kCVReturnSuccess && r1 == kCVReturnSuccess);
	if (y != NULL && uv != NULL) {
		/* Each plane has a texture of its own. */
		CHECK(CVOpenGLESTextureGetName(y) != CVOpenGLESTextureGetName(uv));
		unsigned char px[4 * 4 * 4];
		sample(CVOpenGLESTextureGetName(y), 4, 4, px);
		CHECK(px[0] == 16 && px[1] == 16 && px[2] == 16 && px[3] == 255);     /* luminance: rgb, opaque */
		CHECK(px[4 * 5] == 16 + 10 + 1);                                       /* row 1, column 1 */
		sample(CVOpenGLESTextureGetName(uv), 4, 4, px);
		CHECK(px[0] == 100 && px[3] == 200);                                   /* Cb in rgb, Cr in alpha */
		CHECK(px[4 * (2 * 4 + 2)] == 105 && px[4 * (2 * 4 + 2) + 3] == 205);   /* row 1, column 1 of the plane */
	}
	/* The plane index is ignored for a buffer that is not planar: see test_errors. */
	if (y != NULL) CFRelease(y);
	if (uv != NULL) CFRelease(uv);
	CFRelease(buffer);
}

static void test_errors(CVOpenGLESTextureCacheRef cache)
{
	CVPixelBufferRef bgra = make_buffer(kCVPixelFormatType_32BGRA, 4, 2, NO);
	CVPixelBufferRef planar = make_buffer(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange, 4, 4, NO);
	CVPixelBufferRef argb = make_buffer(kCVPixelFormatType_32ARGB, 4, 2, NO);
	CVOpenGLESTextureRef tex = NULL;
#define CREATE(cache_, src, target, w, h, fmt, type, plane) \
	CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache_, src, NULL, target, fmt, w, h, fmt, type, plane, &tex)

	/* GL_TEXTURE_2D and GL_RENDERBUFFER are the documented targets. */
	CHECK(CREATE(cache, bgra, 0x1234, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidArgument && tex == NULL);
	CHECK(CREATE(cache, bgra, GL_RENDERBUFFER, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnUnsupported && tex == NULL);
	/* INFERENCE for the codes below. */
	CHECK(CREATE(cache, bgra, GL_TEXTURE_2D, 4, 2, GL_RGBA, GL_UNSIGNED_SHORT, 0) == kCVReturnInvalidArgument);
	CHECK(CREATE(cache, bgra, GL_TEXTURE_2D, 4, 2, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidArgument);
	CHECK(CREATE(cache, bgra, GL_TEXTURE_2D, 5, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidSize);
	CHECK(CREATE(cache, bgra, GL_TEXTURE_2D, 0, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidSize);
	CHECK(CREATE(cache, planar, GL_TEXTURE_2D, 4, 4, GL_LUMINANCE, GL_UNSIGNED_BYTE, 2) == kCVReturnInvalidArgument);
	CHECK(CREATE(cache, argb, GL_TEXTURE_2D, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidPixelFormat);
	CHECK(CREATE(cache, (CVImageBufferRef)CFSTR("not a buffer"), GL_TEXTURE_2D, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidArgument);
	CHECK(CREATE(NULL, bgra, GL_TEXTURE_2D, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0) == kCVReturnInvalidArgument);
	CHECK(CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, bgra, NULL, GL_TEXTURE_2D, GL_RGBA, 4, 2,
		GL_RGBA, GL_UNSIGNED_BYTE, 0, NULL) == kCVReturnInvalidArgument);
	/* The plane index is ignored for a buffer that is not planar. */
	CHECK(CREATE(cache, bgra, GL_TEXTURE_2D, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 7) == kCVReturnSuccess && tex != NULL);
	if (tex != NULL) CFRelease(tex);
	if (bgra) CFRelease(bgra);
	if (planar) CFRelease(planar);
	if (argb) CFRelease(argb);
}

int main(void)
{
	@autoreleasepool {
		EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		if (context == nil)
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		CHECK(context != nil);
		if (context == nil)
			return 1;

		/* Creating a cache. */
		CVOpenGLESTextureCacheRef cache = NULL;
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, &cache) == kCVReturnSuccess);
		CHECK(cache != NULL);
		if (cache == NULL)
			return 1;
		CHECK(CFGetTypeID(cache) == CVOpenGLESTextureCacheGetTypeID());
		CHECK(CVOpenGLESTextureCacheGetTypeID() != CVOpenGLESTextureGetTypeID());
		CHECK(CVOpenGLESTextureCacheGetTypeID() != 0 && CVOpenGLESTextureGetTypeID() != 0);
		CVOpenGLESTextureCacheRef none = (CVOpenGLESTextureCacheRef)1;
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, nil, NULL, &none) == kCVReturnInvalidArgument);
		CHECK(none == NULL);
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, NULL) == kCVReturnInvalidArgument);
		CFRelease(cache);
		NSDictionary *attrs = @{ (id)kCVOpenGLESTextureCacheMaximumTextureAgeKey: @0 };
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, (__bridge CFDictionaryRef)attrs, context, NULL, &cache) == kCVReturnSuccess);
		CFRelease(cache);
		cache = NULL;
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, &cache) == kCVReturnSuccess);
		if (cache == NULL)
			return 1;

		/* The cache works in its context whichever is current, and puts the current one back. */
		[EAGLContext setCurrentContext:context];
		setup_target(4, 4);
		test_bgra(cache, NO);
		test_bgra(cache, YES);
		test_planar(cache);
		test_errors(cache);
		[EAGLContext setCurrentContext:nil];
		CVPixelBufferRef buffer = make_buffer(kCVPixelFormatType_32BGRA, 4, 2, NO);
		CVOpenGLESTextureRef tex = NULL;
		CHECK(CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL, GL_TEXTURE_2D, GL_RGBA,
			4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0, &tex) == kCVReturnSuccess);
		CHECK([EAGLContext currentContext] == nil);
		if (tex != NULL) CFRelease(tex);
		CVOpenGLESTextureCacheFlush(cache, 0);
		CHECK([EAGLContext currentContext] == nil);
		CFRelease(buffer);
		CFRelease(cache);
	}
	if (failures != 0) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
