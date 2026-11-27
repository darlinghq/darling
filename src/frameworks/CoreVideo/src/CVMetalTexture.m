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

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreFoundation/CFRuntime.h>
#import <dispatch/dispatch.h>
#import <CoreVideo/CVBase.h>
#import <CoreVideo/CVReturn.h>
#import <CoreVideo/CVBuffer.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <CoreVideo/CVMetalTexture.h>
#import <CoreVideo/CVMetalTextureCache.h>

const CFStringRef kCVMetalTextureUsage = CFSTR("MetalTextureUsage");
const CFStringRef kCVMetalTextureStorageMode = CFSTR("MetalTextureStorageMode");
const CFStringRef kCVMetalTextureCacheMaximumTextureAgeKey = CFSTR("MaximumMetalTextureAge");

typedef struct {
    NSUInteger x, y, z;
} CVMTLOrigin;

typedef struct {
    NSUInteger width, height, depth;
} CVMTLSize;

typedef struct {
    CVMTLOrigin origin;
    CVMTLSize size;
} CVMTLRegion;

@interface NSObject (CVMTLTextureDescriptor)
+ (id)texture2DDescriptorWithPixelFormat:(NSUInteger)pixelFormat
                                   width:(NSUInteger)width
                                  height:(NSUInteger)height
                               mipmapped:(BOOL)mipmapped;
- (void)setUsage:(NSUInteger)usage;
- (void)setStorageMode:(NSUInteger)storageMode;
- (id)newTextureWithDescriptor:(id)descriptor;
- (void)replaceRegion:(CVMTLRegion)region
          mipmapLevel:(NSUInteger)level
            withBytes:(const void *)pixelBytes
          bytesPerRow:(NSUInteger)bytesPerRow;
@end

struct __CVMetalTexture {
    CFRuntimeBase _base;
    id<MTLTexture> texture;
    CVImageBufferRef sourceImage;
};

struct __CVMetalTextureCache {
    CFRuntimeBase _base;
    id<MTLDevice> device;
};

static void metalTextureFinalize(CFTypeRef cf)
{
    struct __CVMetalTexture *tex = (struct __CVMetalTexture *)cf;
    [(id)tex->texture release];
    tex->texture = nil;
    if (tex->sourceImage) {
        CFRelease(tex->sourceImage);
        tex->sourceImage = NULL;
    }
}

static void metalTextureCacheFinalize(CFTypeRef cf)
{
    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)cf;
    [(id)cache->device release];
    cache->device = nil;
}

static const CFRuntimeClass metalTextureClass = {
    .version = 0,
    .className = "CVMetalTexture",
    .finalize = metalTextureFinalize,
};

static const CFRuntimeClass metalTextureCacheClass = {
    .version = 0,
    .className = "CVMetalTextureCache",
    .finalize = metalTextureCacheFinalize,
};

CFTypeID CVMetalTextureGetTypeID(void)
{
    static CFTypeID typeID;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        typeID = _CFRuntimeRegisterClass(&metalTextureClass);
    });
    return typeID;
}

CFTypeID CVMetalTextureCacheGetTypeID(void)
{
    static CFTypeID typeID;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        typeID = _CFRuntimeRegisterClass(&metalTextureCacheClass);
    });
    return typeID;
}

id<MTLTexture> CVMetalTextureGetTexture(CVMetalTextureRef image)
{
    if (!image || CFGetTypeID(image) != CVMetalTextureGetTypeID())
        return nil;
    struct __CVMetalTexture *tex = (struct __CVMetalTexture *)image;
    return tex->texture;
}

Boolean CVMetalTextureIsFlipped(CVMetalTextureRef image)
{
    return false;
}

void CVMetalTextureGetCleanTexCoords(CVMetalTextureRef image,
    float lowerLeft[2], float lowerRight[2],
    float upperRight[2], float upperLeft[2])
{
    if (lowerLeft)  { lowerLeft[0] = 0.0f;  lowerLeft[1] = 1.0f; }
    if (lowerRight) { lowerRight[0] = 1.0f; lowerRight[1] = 1.0f; }
    if (upperRight) { upperRight[0] = 1.0f; upperRight[1] = 0.0f; }
    if (upperLeft)  { upperLeft[0] = 0.0f;  upperLeft[1] = 0.0f; }
}

CVReturn CVMetalTextureCacheCreate(CFAllocatorRef allocator, CFDictionaryRef cacheAttributes,
    id<MTLDevice> metalDevice, CFDictionaryRef textureAttributes, CVMetalTextureCacheRef *cacheOut)
{
    if (cacheOut == NULL || metalDevice == NULL)
        return kCVReturnInvalidArgument;

    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)_CFRuntimeCreateInstance(
        allocator, CVMetalTextureCacheGetTypeID(),
        sizeof(struct __CVMetalTextureCache) - sizeof(CFRuntimeBase), NULL);
    if (!cache) {
        *cacheOut = NULL;
        return kCVReturnAllocationFailed;
    }

    cache->device = [(id)metalDevice retain];
    *cacheOut = cache;
    return kCVReturnSuccess;
}

CVReturn CVMetalTextureCacheCreateTextureFromImage(CFAllocatorRef allocator,
    CVMetalTextureCacheRef textureCache,
    CVImageBufferRef sourceImage,
    CFDictionaryRef textureAttributes,
    unsigned long pixelFormat,
    size_t width,
    size_t height,
    size_t planeIndex,
    CVMetalTextureRef *textureOut)
{
    if (textureOut == NULL || textureCache == NULL || sourceImage == NULL || width == 0 || height == 0)
        return kCVReturnInvalidArgument;
    *textureOut = NULL;

    Class descClass = NSClassFromString(@"MTLTextureDescriptor");
    if (!descClass)
        return kCVReturnUnsupported;

    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)textureCache;
    id desc = [descClass texture2DDescriptorWithPixelFormat:pixelFormat
                                                      width:width
                                                     height:height
                                                  mipmapped:NO];
    if (!desc)
        return kCVReturnUnsupported;

    NSUInteger usage = 1; // MTLTextureUsageShaderRead
    if (textureAttributes) {
        NSNumber *usageNum = [(NSDictionary *)textureAttributes objectForKey:(id)kCVMetalTextureUsage];
        if (usageNum)
            usage = [usageNum unsignedIntegerValue];
    }
    [desc setUsage:usage];

    id<MTLTexture> mtlTex = [(id)cache->device newTextureWithDescriptor:desc];
    if (!mtlTex)
        return kCVReturnAllocationFailed;

    CVPixelBufferRef pb = (CVPixelBufferRef)sourceImage;
    if (CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess) {
        size_t planes = CVPixelBufferGetPlaneCount(pb);
        void *base = (planes > 0) ? CVPixelBufferGetBaseAddressOfPlane(pb, planeIndex) : CVPixelBufferGetBaseAddress(pb);
        size_t bpr = (planes > 0) ? CVPixelBufferGetBytesPerRowOfPlane(pb, planeIndex) : CVPixelBufferGetBytesPerRow(pb);
        if (base && bpr > 0) {
            CVMTLRegion region = { { 0, 0, 0 }, { width, height, 1 } };
            [(id)mtlTex replaceRegion:region mipmapLevel:0 withBytes:base bytesPerRow:bpr];
        }
        CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);
    }

    struct __CVMetalTexture *cvTex = (struct __CVMetalTexture *)_CFRuntimeCreateInstance(
        allocator, CVMetalTextureGetTypeID(),
        sizeof(struct __CVMetalTexture) - sizeof(CFRuntimeBase), NULL);
    if (!cvTex) {
        [(id)mtlTex release];
        return kCVReturnAllocationFailed;
    }

    cvTex->texture = mtlTex;
    cvTex->sourceImage = (CVImageBufferRef)CFRetain(sourceImage);
    *textureOut = (CVMetalTextureRef)cvTex;
    return kCVReturnSuccess;
}

void CVMetalTextureCacheFlush(CVMetalTextureCacheRef textureCache, CVOptionFlags options)
{
}
