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

// CVOpenGLTextureCache and CVOpenGLTexture: GL_TEXTURE_RECTANGLE textures of pixel buffers.
//
// A pixel buffer backed by an IOSurface becomes the storage of its texture through
// CGLTexImageIOSurface2D, which is how OpenGL.framework shares the surface's memory with the
// texture. The pixels of a buffer in memory are copied into the texture. Rows of the buffer
// start at the top of the image, so the textures are flipped: the first row of the texture is
// the top of the image, as CVOpenGLTextureIsFlipped and the texture coordinates say.

#define GL_GLEXT_PROTOTYPES 1
#include "CVInternal.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>
#include <string.h>

struct __CVOpenGLTextureCacheEntry {
	CVPixelBufferRef buffer; // retained
	GLuint name;
	size_t users; // the textures that use the name
};

struct __CVOpenGLTextureCache {
	CFRuntimeBase runtimeBase;
	CGLContextObj context;
	CGLPixelFormatObj pixelFormat;
	pthread_mutex_t lock;
	struct __CVOpenGLTextureCacheEntry* entries;
	size_t entryCount;
	size_t entryCapacity;
};

// A CVBuffer first, so that attachments work on a texture like on any image buffer.
struct __CVOpenGLTexture {
	struct __CVBuffer buffer;
	CVOpenGLTextureCacheRef cache; // retained
	CVPixelBufferRef source; // retained
	GLuint name;
};

// How a pixel format is handed to OpenGL.
struct __CVTextureFormat {
	OSType pixelFormat;
	GLenum internalFormat;
	GLenum format;
	GLenum type;
	size_t bytesPerPixel;
};

static const struct __CVTextureFormat textureFormats[] = {
	{ 'BGRA', GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 4 },
	{ 0x00000020, GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, 4 }, // kCVPixelFormatType_32ARGB
	{ 'RGBA', GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 4 },
	{ 'ABGR', GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4 },
	{ 0x00000018, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3 }, // kCVPixelFormatType_24RGB
	{ '24BG', GL_RGB8, GL_BGR, GL_UNSIGNED_BYTE, 3 },
	{ 'L008', GL_R8, GL_RED, GL_UNSIGNED_BYTE, 1 },
	{ 'RGhA', GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 8 },
};

static const struct __CVTextureFormat* textureFormatFor(OSType pixelFormat)
{
	for (size_t i = 0; i < sizeof(textureFormats) / sizeof(textureFormats[0]); i++) {
		if (textureFormats[i].pixelFormat == pixelFormat)
			return &textureFormats[i];
	}
	return NULL;
}

// ── The texture ──

static void textureCacheEntryRelease(CVOpenGLTextureCacheRef cache, GLuint name);

static void textureFinalize(CFTypeRef cf)
{
	struct __CVOpenGLTexture* texture = (struct __CVOpenGLTexture*) cf;
	if (texture->cache) {
		textureCacheEntryRelease(texture->cache, texture->name);
		CFRelease(texture->cache);
		texture->cache = NULL;
	}
	if (texture->source) {
		CFRelease(texture->source);
		texture->source = NULL;
	}
	if (texture->buffer.propagatingAttachments)
		CFRelease(texture->buffer.propagatingAttachments);
	if (texture->buffer.localAttachments)
		CFRelease(texture->buffer.localAttachments);
	texture->buffer.propagatingAttachments = NULL;
	texture->buffer.localAttachments = NULL;
}

static const CFRuntimeClass textureClass = {
	.version = 0,
	.className = "CVOpenGLTexture",
	.finalize = textureFinalize,
};

CFTypeID CVOpenGLTextureGetTypeID(void)
{
	static CFTypeID typeID = _kCFRuntimeNotATypeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&textureClass);
	});
	return typeID;
}

CVOpenGLTextureRef CVOpenGLTextureRetain(CVOpenGLTextureRef texture)
{
	if (texture)
		CFRetain(texture);
	return texture;
}

void CVOpenGLTextureRelease(CVOpenGLTextureRef texture)
{
	if (texture)
		CFRelease(texture);
}

GLenum CVOpenGLTextureGetTarget(CVOpenGLTextureRef image)
{
	return GL_TEXTURE_RECTANGLE;
}

GLuint CVOpenGLTextureGetName(CVOpenGLTextureRef image)
{
	return image ? ((struct __CVOpenGLTexture*) image)->name : 0;
}

Boolean CVOpenGLTextureIsFlipped(CVOpenGLTextureRef image)
{
	return true;
}

void CVOpenGLTextureGetCleanTexCoords(CVOpenGLTextureRef image, GLfloat lowerLeft[2], GLfloat lowerRight[2],
	GLfloat upperRight[2], GLfloat upperLeft[2])
{
	GLfloat width = image ? (GLfloat) image->width : 0, height = image ? (GLfloat) image->height : 0;
	lowerLeft[0] = 0;
	lowerLeft[1] = height;
	lowerRight[0] = width;
	lowerRight[1] = height;
	upperRight[0] = width;
	upperRight[1] = 0;
	upperLeft[0] = 0;
	upperLeft[1] = 0;
}

// ── The cache ──

static void textureCacheFinalize(CFTypeRef cf)
{
	CVOpenGLTextureCacheRef cache = (CVOpenGLTextureCacheRef) cf;
	CGLContextObj previous = CGLGetCurrentContext();
	Boolean switched = previous != cache->context && CGLSetCurrentContext(cache->context) == kCGLNoError;

	for (size_t i = 0; i < cache->entryCount; i++) {
		glDeleteTextures(1, &cache->entries[i].name);
		CFRelease(cache->entries[i].buffer);
	}
	if (switched)
		CGLSetCurrentContext(previous);
	free(cache->entries);
	cache->entries = NULL;
	cache->entryCount = 0;
	if (cache->context)
		CGLReleaseContext(cache->context);
	if (cache->pixelFormat)
		CGLReleasePixelFormat(cache->pixelFormat);
	pthread_mutex_destroy(&cache->lock);
}

static const CFRuntimeClass textureCacheClass = {
	.version = 0,
	.className = "CVOpenGLTextureCache",
	.finalize = textureCacheFinalize,
};

CFTypeID CVOpenGLTextureCacheGetTypeID(void)
{
	static CFTypeID typeID = _kCFRuntimeNotATypeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&textureCacheClass);
	});
	return typeID;
}

CVReturn CVOpenGLTextureCacheCreate(CFAllocatorRef allocator, CFDictionaryRef cacheAttributes, CGLContextObj cglContext,
	CGLPixelFormatObj cglPixelFormat, CFDictionaryRef textureAttributes, CVOpenGLTextureCacheRef* cacheOut)
{
	if (cacheOut == NULL || cglContext == NULL)
		return kCVReturnInvalidArgument;
	*cacheOut = NULL;

	CVOpenGLTextureCacheRef cache = (CVOpenGLTextureCacheRef) _CFRuntimeCreateInstance(allocator,
		CVOpenGLTextureCacheGetTypeID(), sizeof(struct __CVOpenGLTextureCache) - sizeof(CFRuntimeBase), NULL);
	if (cache == NULL)
		return kCVReturnAllocationFailed;
	memset((char*) cache + sizeof(CFRuntimeBase), 0, sizeof(struct __CVOpenGLTextureCache) - sizeof(CFRuntimeBase));
	pthread_mutex_init(&cache->lock, NULL);
	cache->context = CGLRetainContext(cglContext);
	cache->pixelFormat = cglPixelFormat ? CGLRetainPixelFormat(cglPixelFormat) : NULL;

	*cacheOut = cache;
	return kCVReturnSuccess;
}

CVOpenGLTextureCacheRef CVOpenGLTextureCacheRetain(CVOpenGLTextureCacheRef textureCache)
{
	if (textureCache)
		CFRetain(textureCache);
	return textureCache;
}

void CVOpenGLTextureCacheRelease(CVOpenGLTextureCacheRef textureCache)
{
	if (textureCache)
		CFRelease(textureCache);
}

// Finds the entry of a buffer, making one (with a new texture name) when the buffer has none.
// The context of the cache is current. The lock of the cache is held.
static struct __CVOpenGLTextureCacheEntry* textureCacheFindEntryLocked(CVOpenGLTextureCacheRef cache, CVPixelBufferRef buffer)
{
	for (size_t i = 0; i < cache->entryCount; i++) {
		if (cache->entries[i].buffer == buffer)
			return &cache->entries[i];
	}

	if (cache->entryCount == cache->entryCapacity) {
		size_t capacity = cache->entryCapacity ? cache->entryCapacity * 2 : 8;
		struct __CVOpenGLTextureCacheEntry* entries = realloc(cache->entries, capacity * sizeof(*entries));
		if (entries == NULL)
			return NULL;
		cache->entries = entries;
		cache->entryCapacity = capacity;
	}

	GLuint name = 0;
	glGenTextures(1, &name);
	if (name == 0)
		return NULL;
	struct __CVOpenGLTextureCacheEntry* entry = &cache->entries[cache->entryCount++];
	entry->buffer = (CVPixelBufferRef) CFRetain(buffer);
	entry->name = name;
	entry->users = 0;
	return entry;
}

static void textureCacheEntryRelease(CVOpenGLTextureCacheRef cache, GLuint name)
{
	pthread_mutex_lock(&cache->lock);
	for (size_t i = 0; i < cache->entryCount; i++) {
		if (cache->entries[i].name == name) {
			if (cache->entries[i].users > 0)
				cache->entries[i].users--;
			break;
		}
	}
	pthread_mutex_unlock(&cache->lock);
}

void CVOpenGLTextureCacheFlush(CVOpenGLTextureCacheRef textureCache, CVOptionFlags options)
{
	if (textureCache == NULL)
		return;

	CGLContextObj previous = CGLGetCurrentContext();
	Boolean switched = previous != textureCache->context && CGLSetCurrentContext(textureCache->context) == kCGLNoError;

	pthread_mutex_lock(&textureCache->lock);
	for (size_t i = 0; i < textureCache->entryCount; ) {
		if (textureCache->entries[i].users == 0) {
			glDeleteTextures(1, &textureCache->entries[i].name);
			CFRelease(textureCache->entries[i].buffer);
			textureCache->entries[i] = textureCache->entries[--textureCache->entryCount];
		} else {
			i++;
		}
	}
	pthread_mutex_unlock(&textureCache->lock);

	if (switched)
		CGLSetCurrentContext(previous);
}

// Copies the pixels of a buffer in memory into the texture that is bound to the rectangle
// target, row by row because the rows of the buffer need not be a whole number of pixels long.
static Boolean uploadFromMemory(CVPixelBufferRef buffer, const struct __CVTextureFormat* format)
{
	if (CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
		return false;
	const uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);
	GLsizei width = (GLsizei) CVPixelBufferGetWidth(buffer), height = (GLsizei) CVPixelBufferGetHeight(buffer);

	GLint alignment = 4, rowLength = 0, skipPixels = 0, skipRows = 0, unpackBuffer = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
	glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, (GLint) format->internalFormat, width, height, 0, format->format, format->type, NULL);
	for (GLsizei y = 0; y < height; y++)
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, y, width, 1, format->format, format->type, base + y * bytesPerRow);

	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint) unpackBuffer);

	CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
	return true;
}

CVReturn CVOpenGLTextureCacheCreateTextureFromImage(CFAllocatorRef allocator, CVOpenGLTextureCacheRef textureCache,
	CVImageBufferRef sourceImage, CFDictionaryRef attributes, CVOpenGLTextureRef* textureOut)
{
	if (textureCache == NULL || sourceImage == NULL || textureOut == NULL)
		return kCVReturnInvalidArgument;
	if (CFGetTypeID(sourceImage) != CVPixelBufferGetTypeID())
		return kCVReturnInvalidArgument;
	*textureOut = NULL;

	// Planar formats (and the packed YCbCr ones) cannot be used as one texture.
	const struct __CVTextureFormat* format = textureFormatFor(CVPixelBufferGetPixelFormatType(sourceImage));
	if (format == NULL || CVPixelBufferIsPlanar(sourceImage))
		return kCVReturnPixelBufferNotOpenGLCompatible;

	CGLContextObj previous = CGLGetCurrentContext();
	if (previous != textureCache->context && CGLSetCurrentContext(textureCache->context) != kCGLNoError)
		return kCVReturnError;

	CVReturn result = kCVReturnSuccess;
	pthread_mutex_lock(&textureCache->lock);
	struct __CVOpenGLTextureCacheEntry* entry = textureCacheFindEntryLocked(textureCache, sourceImage);
	if (entry == NULL) {
		result = kCVReturnAllocationFailed;
	} else {
		GLint previousBinding = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &previousBinding);
		glBindTexture(GL_TEXTURE_RECTANGLE, entry->name);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		IOSurfaceRef surface = CVPixelBufferGetIOSurface(sourceImage);
		if (surface) {
			CGLError error = CGLTexImageIOSurface2D(textureCache->context, GL_TEXTURE_RECTANGLE, format->internalFormat,
				(GLsizei) CVPixelBufferGetWidth(sourceImage), (GLsizei) CVPixelBufferGetHeight(sourceImage),
				format->format, format->type, surface, 0);
			if (error != kCGLNoError)
				result = kCVReturnPixelBufferNotOpenGLCompatible;
		} else if (!uploadFromMemory(sourceImage, format)) {
			result = kCVReturnError;
		}
		glBindTexture(GL_TEXTURE_RECTANGLE, (GLuint) previousBinding);

		if (result == kCVReturnSuccess) {
			struct __CVOpenGLTexture* texture = (struct __CVOpenGLTexture*) _CFRuntimeCreateInstance(allocator,
				CVOpenGLTextureGetTypeID(), sizeof(struct __CVOpenGLTexture) - sizeof(CFRuntimeBase), NULL);
			if (texture == NULL) {
				result = kCVReturnAllocationFailed;
			} else {
				memset((char*) texture + sizeof(CFRuntimeBase), 0, sizeof(struct __CVOpenGLTexture) - sizeof(CFRuntimeBase));
				texture->buffer.width = CVPixelBufferGetWidth(sourceImage);
				texture->buffer.height = CVPixelBufferGetHeight(sourceImage);
				texture->buffer.pixelFormat = CVPixelBufferGetPixelFormatType(sourceImage);
				texture->cache = (CVOpenGLTextureCacheRef) CFRetain(textureCache);
				texture->source = (CVPixelBufferRef) CFRetain(sourceImage);
				texture->name = entry->name;
				entry->users++;
				*textureOut = (CVOpenGLTextureRef) texture;
			}
		}
	}
	pthread_mutex_unlock(&textureCache->lock);

	if (previous != textureCache->context)
		CGLSetCurrentContext(previous);
	return result;
}
