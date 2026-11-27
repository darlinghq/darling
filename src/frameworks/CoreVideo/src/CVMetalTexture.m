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
#import <Metal/Metal.h>
#import <IOSurface/IOSurface.h>
#import <CoreVideo/CVBase.h>
#import <CoreVideo/CVReturn.h>
#import <CoreVideo/CVBuffer.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <CoreVideo/CVPixelBufferIOSurface.h>
#import <CoreVideo/CVMetalTexture.h>
#import <CoreVideo/CVMetalTextureCache.h>

// A texture of the cache's Metal device that is made of an IOSurface plane of the pixel buffer,
// so it shows the pixel buffer's memory and not a copy. Pixel buffers that are not backed by an
// IOSurface cannot be used: they fail with kCVReturnPixelBufferNotMetalCompatible.

const CFStringRef kCVMetalTextureUsage = CFSTR("MetalTextureUsage");
const CFStringRef kCVMetalTextureStorageMode = CFSTR("MetalTextureStorageMode");
const CFStringRef kCVMetalTextureCacheMaximumTextureAgeKey = CFSTR("MaximumMetalTextureAge");

struct __CVMetalTexture {
    CFRuntimeBase _base;
    id<MTLTexture> texture;
    CVImageBufferRef sourceImage;
};

struct __CVMetalTextureCache {
    CFRuntimeBase _base;
    id<MTLDevice> device;
    CFDictionaryRef textureAttributes;
};

static void metalTextureFinalize(CFTypeRef cf)
{
    struct __CVMetalTexture *tex = (struct __CVMetalTexture *)cf;
    [tex->texture release];
    tex->texture = nil;
    if (tex->sourceImage) {
        CFRelease(tex->sourceImage);
        tex->sourceImage = NULL;
    }
}

static void metalTextureCacheFinalize(CFTypeRef cf)
{
    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)cf;
    [cache->device release];
    cache->device = nil;
    if (cache->textureAttributes) {
        CFRelease(cache->textureAttributes);
        cache->textureAttributes = NULL;
    }
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
    if (cacheOut == NULL || metalDevice == nil)
        return kCVReturnInvalidArgument;

    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)_CFRuntimeCreateInstance(
        allocator, CVMetalTextureCacheGetTypeID(),
        sizeof(struct __CVMetalTextureCache) - sizeof(CFRuntimeBase), NULL);
    if (!cache)
        return kCVReturnAllocationFailed;

    cache->device = [metalDevice retain];
    cache->textureAttributes = textureAttributes ? CFDictionaryCreateCopy(allocator, textureAttributes) : NULL;
    *cacheOut = cache;
    return kCVReturnSuccess;
}

// The attribute the call passes wins over the one the cache was created with.
static id textureAttribute(struct __CVMetalTextureCache *cache, CFDictionaryRef callAttributes, CFStringRef key)
{
    id value = callAttributes ? (id)CFDictionaryGetValue(callAttributes, key) : nil;
    if (value == nil && cache->textureAttributes)
        value = (id)CFDictionaryGetValue(cache->textureAttributes, key);
    return [value isKindOfClass:[NSNumber class]] ? value : nil;
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
    if (textureOut == NULL || textureCache == NULL || sourceImage == NULL
        || CFGetTypeID(textureCache) != CVMetalTextureCacheGetTypeID()
        || CFGetTypeID(sourceImage) != CVPixelBufferGetTypeID())
        return kCVReturnInvalidArgument;

    struct __CVMetalTextureCache *cache = (struct __CVMetalTextureCache *)textureCache;
    CVPixelBufferRef pb = (CVPixelBufferRef)sourceImage;

    size_t planes = CVPixelBufferGetPlaneCount(pb);
    if (planes == 0 ? planeIndex != 0 : planeIndex >= planes)
        return kCVReturnInvalidArgument;
    size_t planeWidth = planes ? CVPixelBufferGetWidthOfPlane(pb, planeIndex) : CVPixelBufferGetWidth(pb);
    size_t planeHeight = planes ? CVPixelBufferGetHeightOfPlane(pb, planeIndex) : CVPixelBufferGetHeight(pb);
    if (width == 0 || height == 0 || width > planeWidth || height > planeHeight)
        return kCVReturnInvalidArgument;

    IOSurfaceRef surface = CVPixelBufferGetIOSurface(pb);
    if (surface == NULL)
        return kCVReturnPixelBufferNotMetalCompatible;

    MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat
                                                                                     width:width
                                                                                    height:height
                                                                                 mipmapped:NO];
    NSNumber *usage = textureAttribute(cache, textureAttributes, kCVMetalTextureUsage);
    desc.usage = usage ? (MTLTextureUsage)[usage unsignedIntegerValue] : MTLTextureUsageShaderRead;
    NSNumber *storage = textureAttribute(cache, textureAttributes, kCVMetalTextureStorageMode);
    if (storage)
        desc.storageMode = (MTLStorageMode)[storage unsignedIntegerValue];

    id<MTLTexture> mtlTex = [cache->device newTextureWithDescriptor:desc iosurface:surface plane:planeIndex];
    if (!mtlTex)
        return kCVReturnError;

    struct __CVMetalTexture *cvTex = (struct __CVMetalTexture *)_CFRuntimeCreateInstance(
        allocator, CVMetalTextureGetTypeID(),
        sizeof(struct __CVMetalTexture) - sizeof(CFRuntimeBase), NULL);
    if (!cvTex) {
        [mtlTex release];
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
