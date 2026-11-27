// Copyright (C) 2026 Lubos Dolezel
//
// CVMetalTextureCache. Argument errors are kCVReturnInvalidArgument and leave the
// output untouched (a nil device and a NULL cache were observed on macOS 26.7.3).
// With a Metal device: a texture made from an IOSurface-backed pixel buffer is a
// view of the buffer's memory, so a CPU write to the buffer shows in the texture
// without a new texture; planes of a planar buffer get their own textures; a pixel
// buffer without an IOSurface cannot be used (kCVReturnPixelBufferNotMetalCompatible);
// out-of-range planes and sizes are invalid arguments. Prints SKIP for the device
// part where no Metal device exists.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <CoreVideo/CoreVideo.h>
#import <IOSurface/IOSurface.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static CVPixelBufferRef makeBuffer(size_t w, size_t h, OSType format, BOOL ioSurface)
{
	NSDictionary *attrs = ioSurface ? @{ (id)kCVPixelBufferIOSurfacePropertiesKey: @{} } : nil;
	CVPixelBufferRef pb = NULL;
	if (CVPixelBufferCreate(NULL, w, h, format, (CFDictionaryRef)attrs, &pb) != kCVReturnSuccess)
		return NULL;
	return pb;
}

int main(void)
{
	@autoreleasepool {
		// Argument errors, which need no device.
		CVMetalTextureCacheRef cache = (CVMetalTextureCacheRef)(uintptr_t)1;
		CHECK(CVMetalTextureCacheCreate(NULL, NULL, nil, NULL, &cache) == kCVReturnInvalidArgument);
		CHECK(cache == (CVMetalTextureCacheRef)(uintptr_t)1);
		CHECK(CVMetalTextureCacheCreate(NULL, NULL, nil, NULL, NULL) == kCVReturnInvalidArgument);

		CVPixelBufferRef plain = makeBuffer(5, 3, kCVPixelFormatType_32BGRA, NO);
		CHECK(plain != NULL);
		CVMetalTextureRef texture = (CVMetalTextureRef)(uintptr_t)1;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, NULL, plain, NULL, 80, 5, 3, 0, &texture) == kCVReturnInvalidArgument);
		CHECK(texture == (CVMetalTextureRef)(uintptr_t)1);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, NULL, NULL, NULL, 80, 5, 3, 0, &texture) == kCVReturnInvalidArgument);
		CHECK(CVMetalTextureGetTexture(NULL) == nil);
		CHECK(CVMetalTextureGetTypeID() != CVMetalTextureCacheGetTypeID());

		id<MTLDevice> device = MTLCreateSystemDefaultDevice();
		if (device == nil) {
			printf("SKIP: no Metal device\n");
			CVPixelBufferRelease(plain);
			printf("PASS\n");
			return g_failures ? 1 : 0;
		}

		CHECK(CVMetalTextureCacheCreate(NULL, NULL, device, NULL, &cache) == kCVReturnSuccess && cache != NULL);
		CHECK(CFGetTypeID(cache) == CVMetalTextureCacheGetTypeID());
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, plain, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, NULL) == kCVReturnInvalidArgument);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, NULL, plain, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture) == kCVReturnInvalidArgument);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, NULL, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture) == kCVReturnInvalidArgument);

		// No IOSurface, no texture.
		texture = (CVMetalTextureRef)(uintptr_t)1;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, plain, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture)
			== kCVReturnPixelBufferNotMetalCompatible);
		CHECK(texture == (CVMetalTextureRef)(uintptr_t)1);

		// A texture that is a view of the pixel buffer.
		CVPixelBufferRef pb = makeBuffer(5, 3, kCVPixelFormatType_32BGRA, YES);
		CHECK(pb != NULL && CVPixelBufferGetIOSurface(pb) != NULL);
		CVPixelBufferLockBaseAddress(pb, 0);
		uint8_t *base = CVPixelBufferGetBaseAddress(pb);
		size_t rowBytes = CVPixelBufferGetBytesPerRow(pb);
		for (int y = 0; y < 3; y++)
			for (int x = 0; x < 5; x++) {
				uint8_t *p = base + y * rowBytes + x * 4;
				p[0] = x; p[1] = y; p[2] = 9; p[3] = 255;
			}
		CVPixelBufferUnlockBaseAddress(pb, 0);

		texture = NULL;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture) == kCVReturnSuccess);
		CHECK(texture != NULL && CFGetTypeID(texture) == CVMetalTextureGetTypeID());
		id<MTLTexture> mt = CVMetalTextureGetTexture(texture);
		CHECK(mt != nil);
		if (mt) {
			CHECK(mt.width == 5 && mt.height == 3 && mt.pixelFormat == MTLPixelFormatBGRA8Unorm);
			CHECK(mt.textureType == MTLTextureType2D);
			CHECK(mt.usage == MTLTextureUsageShaderRead);
			CHECK(mt.iosurface == CVPixelBufferGetIOSurface(pb));
			// Single-pixel reads in the first row (the Metal layer's getBytes does not yet address the later rows of an
			// IOSurface texture correctly): the last pixel of the row, then the first pixel after a CPU write.
			uint8_t px[4] = { 0 };
			[mt getBytes:px bytesPerRow:4 fromRegion:MTLRegionMake2D(4, 0, 1, 1) mipmapLevel:0];
			CHECK(px[0] == 4 && px[1] == 0 && px[2] == 9 && px[3] == 255);
			[mt getBytes:px bytesPerRow:4 fromRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0];
			CHECK(px[0] == 0 && px[1] == 0);

			// The texture shows the buffer's memory, not a copy of it.
			CVPixelBufferLockBaseAddress(pb, 0);
			((uint8_t *)CVPixelBufferGetBaseAddress(pb))[0] = 77;
			CVPixelBufferUnlockBaseAddress(pb, 0);
			[mt getBytes:px bytesPerRow:4 fromRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0];
			CHECK(px[0] == 77);
		}
		CHECK(!CVMetalTextureIsFlipped(texture));
		float ll[2], lr[2], ur[2], ul[2];
		CVMetalTextureGetCleanTexCoords(texture, ll, lr, ur, ul);
		CHECK(ll[0] == 0 && ll[1] == 1 && lr[0] == 1 && lr[1] == 1 && ur[0] == 1 && ur[1] == 0 && ul[0] == 0 && ul[1] == 0);
		CFRelease(texture);

		// Texture usage from the call's attributes; sizes and planes out of range.
		NSDictionary *attrs = @{ (id)kCVMetalTextureUsage: @(MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget) };
		texture = NULL;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, (CFDictionaryRef)attrs, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture) == kCVReturnSuccess);
		CHECK(texture != NULL && CVMetalTextureGetTexture(texture).usage == (MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget));
		if (texture) CFRelease(texture);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, NULL, MTLPixelFormatBGRA8Unorm, 6, 3, 0, &texture) == kCVReturnInvalidArgument);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, NULL, MTLPixelFormatBGRA8Unorm, 5, 0, 0, &texture) == kCVReturnInvalidArgument);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 1, &texture) == kCVReturnInvalidArgument);

		// A texture keeps the pixel buffer alive.
		CFIndex before = CFGetRetainCount(pb);
		texture = NULL;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, pb, NULL, MTLPixelFormatBGRA8Unorm, 5, 3, 0, &texture) == kCVReturnSuccess);
		CHECK(CFGetRetainCount(pb) == before + 1);
		CFRelease(texture);
		CHECK(CFGetRetainCount(pb) == before);
		CVPixelBufferRelease(pb);

		// The planes of a biplanar buffer.
		CVPixelBufferRef nv = makeBuffer(8, 4, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange, YES);
		CHECK(nv != NULL && CVPixelBufferGetPlaneCount(nv) == 2);
		texture = NULL;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, nv, NULL, MTLPixelFormatR8Unorm, 8, 4, 0, &texture) == kCVReturnSuccess);
		CHECK(texture != NULL && CVMetalTextureGetTexture(texture).width == 8 && CVMetalTextureGetTexture(texture).height == 4);
		if (texture) CFRelease(texture);
		texture = NULL;
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, nv, NULL, MTLPixelFormatRG8Unorm, 4, 2, 1, &texture) == kCVReturnSuccess);
		CHECK(texture != NULL && CVMetalTextureGetTexture(texture).width == 4 && CVMetalTextureGetTexture(texture).height == 2);
		CHECK(texture != NULL && CVMetalTextureGetTexture(texture).iosurfacePlane == 1);
		if (texture) CFRelease(texture);
		CHECK(CVMetalTextureCacheCreateTextureFromImage(NULL, cache, nv, NULL, MTLPixelFormatR8Unorm, 8, 4, 2, &texture) == kCVReturnInvalidArgument);
		CVPixelBufferRelease(nv);

		CVMetalTextureCacheFlush(cache, 0);
		CFRelease(cache);
		CVPixelBufferRelease(plain);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
