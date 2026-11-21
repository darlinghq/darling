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

// CVOpenGLESTextureCache and CVOpenGLESTexture: GL_TEXTURE_2D textures of pixel buffers, for
// OpenGL ES 2 and 3 contexts (EAGLContext, from OpenGLES.framework).
//
// Apple's cache binds the pixel buffer's memory to the texture, so the texture follows later
// changes of the buffer. The GL driver of the host cannot bind that memory, so every call to
// CVOpenGLESTextureCacheCreateTextureFromImage copies the current pixels of the plane into the
// texture instead; a buffer that a camera or decoder refills is uploaded again by each call,
// which is when the application asks for its texture. Rows of the buffer start at the top of the
// image, so the textures are flipped: the first row of the texture is the top of the image, as
// CVOpenGLESTextureIsFlipped and the texture coordinates say.
//
// OpenGLES.framework exists only in 64-bit processes while CoreVideo also has 32-bit slices, so
// CoreVideo finds it, the EAGLContext class and the GL entry points at run time.

#include "CVInternal.h"
#include <CoreVideo/CVOpenGLESTextureCache.h>
#include <OpenGLES/gltypes.h>
#include <dispatch/dispatch.h>
#include <dlfcn.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GL_NO_ERROR 0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_RENDERBUFFER 0x8D41
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNSIGNED_BYTE 0x1401
#define GL_ALPHA 0x1906
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_RED 0x1903
#define GL_RG 0x8227
#define GL_BGRA_EXT 0x80E1

#define kOpenGLESPath "/System/iOSSupport/System/Library/Frameworks/OpenGLES.framework/OpenGLES"

struct __CVOpenGLESFunctions {
	void (*glGenTextures)(GLsizei, GLuint*);
	void (*glDeleteTextures)(GLsizei, const GLuint*);
	void (*glBindTexture)(GLenum, GLuint);
	void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
	void (*glGetIntegerv)(GLenum, GLint*);
	void (*glPixelStorei)(GLenum, GLint);
	GLenum (*glGetError)(void);
};

static struct __CVOpenGLESFunctions gl;
static Class eaglContextClass;

static Boolean openGLESLoad(void)
{
	static dispatch_once_t once;
	static Boolean loaded;
	dispatch_once(&once, ^{
		void* handle = dlopen(kOpenGLESPath, RTLD_LAZY);
		if (handle == NULL)
			return;
		gl.glGenTextures = dlsym(handle, "glGenTextures");
		gl.glDeleteTextures = dlsym(handle, "glDeleteTextures");
		gl.glBindTexture = dlsym(handle, "glBindTexture");
		gl.glTexImage2D = dlsym(handle, "glTexImage2D");
		gl.glGetIntegerv = dlsym(handle, "glGetIntegerv");
		gl.glPixelStorei = dlsym(handle, "glPixelStorei");
		gl.glGetError = dlsym(handle, "glGetError");
		eaglContextClass = objc_getClass("EAGLContext");
		loaded = gl.glGenTextures && gl.glDeleteTextures && gl.glBindTexture && gl.glTexImage2D
			&& gl.glGetIntegerv && gl.glPixelStorei && gl.glGetError && eaglContextClass != Nil;
	});
	return loaded;
}

// ── The EAGLContext, as the objects of the Objective-C runtime ──

static void* contextRetain(void* context)
{
	return ((void* (*)(void*, SEL)) objc_msgSend)(context, sel_registerName("retain"));
}

static void contextRelease(void* context)
{
	((void (*)(void*, SEL)) objc_msgSend)(context, sel_registerName("release"));
}

static void* contextCurrent(void)
{
	return ((void* (*)(Class, SEL)) objc_msgSend)(eaglContextClass, sel_registerName("currentContext"));
}

static Boolean contextMakeCurrent(void* context)
{
	return ((BOOL (*)(Class, SEL, void*)) objc_msgSend)(eaglContextClass, sel_registerName("setCurrentContext:"), context);
}

static unsigned long contextAPI(void* context)
{
	return ((unsigned long (*)(void*, SEL)) objc_msgSend)(context, sel_registerName("API"));
}

// ── Pixel formats and the formats the application asks for ──

// The bytes per pixel of a plane, or 0 when the pixel format has no texture for it.
static size_t texturePlaneBytesPerPixel(OSType pixelFormat, size_t plane)
{
	switch (pixelFormat) {
		case 'BGRA':
		case 'RGBA':
			return plane == 0 ? 4 : 0;
		case 'L008':
			return plane == 0 ? 1 : 0;
		case '420v':
		case '420f':
			return plane == 0 ? 1 : plane == 1 ? 2 : 0;
		default:
			return 0;
	}
}

// The bytes of a pixel in the format glTexImage2D is told (with type GL_UNSIGNED_BYTE); 0 when the
// format is not one a pixel buffer's pixels fit.
static size_t formatBytesPerPixel(GLenum format)
{
	switch (format) {
		case GL_RGBA:
		case GL_BGRA_EXT:
			return 4;
		case GL_LUMINANCE_ALPHA:
		case GL_RG:
			return 2;
		case GL_LUMINANCE:
		case GL_ALPHA:
		case GL_RED:
			return 1;
		default:
			return 0;
	}
}

// ── Objects ──

struct __CVOpenGLESTextureCacheEntry {
	CVPixelBufferRef buffer; // retained
	size_t plane;
	GLenum target;
	GLint internalFormat;
	GLsizei width, height;
	GLenum format, type;
	GLuint name;
	size_t users; // the textures that use the name
	uint64_t lastUse; // nanoseconds, monotonic
};

struct __CVOpenGLESTextureCache {
	CFRuntimeBase runtimeBase;
	void* context; // retained EAGLContext
	double maximumAge; // seconds; 0 keeps unused textures until a flush
	pthread_mutex_t lock;
	struct __CVOpenGLESTextureCacheEntry* entries;
	size_t entryCount;
	size_t entryCapacity;
};

// A CVBuffer first, so that attachments work on a texture like on any image buffer.
struct __CVOpenGLESTexture {
	struct __CVBuffer buffer;
	CVOpenGLESTextureCacheRef cache; // retained
	CVPixelBufferRef source; // retained
	GLuint name;
	GLenum target;
};

static uint64_t nowNanoseconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t) ts.tv_sec * 1000000000ull + (uint64_t) ts.tv_nsec;
}

// ── The texture ──

static void textureCacheEntryRelease(CVOpenGLESTextureCacheRef cache, GLuint name);

static void textureFinalize(CFTypeRef cf)
{
	struct __CVOpenGLESTexture* texture = (struct __CVOpenGLESTexture*) cf;
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
	.className = "CVOpenGLESTexture",
	.finalize = textureFinalize,
};

CFTypeID CVOpenGLESTextureGetTypeID(void)
{
	static CFTypeID typeID = _kCFRuntimeNotATypeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&textureClass);
	});
	return typeID;
}

GLenum CVOpenGLESTextureGetTarget(CVOpenGLESTextureRef image)
{
	return image ? ((struct __CVOpenGLESTexture*) image)->target : 0;
}

GLuint CVOpenGLESTextureGetName(CVOpenGLESTextureRef image)
{
	return image ? ((struct __CVOpenGLESTexture*) image)->name : 0;
}

Boolean CVOpenGLESTextureIsFlipped(CVOpenGLESTextureRef image)
{
	return true;
}

void CVOpenGLESTextureGetCleanTexCoords(CVOpenGLESTextureRef image, GLfloat lowerLeft[2], GLfloat lowerRight[2],
	GLfloat upperRight[2], GLfloat upperLeft[2])
{
	lowerLeft[0] = 0;
	lowerLeft[1] = 1;
	lowerRight[0] = 1;
	lowerRight[1] = 1;
	upperRight[0] = 1;
	upperRight[1] = 0;
	upperLeft[0] = 0;
	upperLeft[1] = 0;
}

// ── The cache ──

// Makes the cache's context current, as far as it is not, and gives back what has to be restored.
static Boolean cacheEnter(CVOpenGLESTextureCacheRef cache, void** previous)
{
	*previous = contextCurrent();
	return *previous == cache->context || contextMakeCurrent(cache->context);
}

static void cacheLeave(CVOpenGLESTextureCacheRef cache, void* previous)
{
	if (previous != cache->context)
		contextMakeCurrent(previous);
}

static void textureCacheFinalize(CFTypeRef cf)
{
	CVOpenGLESTextureCacheRef cache = (CVOpenGLESTextureCacheRef) cf;
	void* previous;
	Boolean entered = cacheEnter(cache, &previous);
	for (size_t i = 0; i < cache->entryCount; i++) {
		if (entered)
			gl.glDeleteTextures(1, &cache->entries[i].name);
		CFRelease(cache->entries[i].buffer);
	}
	if (entered)
		cacheLeave(cache, previous);
	free(cache->entries);
	cache->entries = NULL;
	cache->entryCount = 0;
	if (cache->context)
		contextRelease(cache->context);
	pthread_mutex_destroy(&cache->lock);
}

static const CFRuntimeClass textureCacheClass = {
	.version = 0,
	.className = "CVOpenGLESTextureCache",
	.finalize = textureCacheFinalize,
};

CFTypeID CVOpenGLESTextureCacheGetTypeID(void)
{
	static CFTypeID typeID = _kCFRuntimeNotATypeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&textureCacheClass);
	});
	return typeID;
}

CVReturn CVOpenGLESTextureCacheCreate(CFAllocatorRef allocator, CFDictionaryRef cacheAttributes,
	CVEAGLContext eaglContext, CFDictionaryRef textureAttributes, CVOpenGLESTextureCacheRef* cacheOut)
{
	if (cacheOut == NULL)
		return kCVReturnInvalidArgument;
	*cacheOut = NULL;
	void* context = (void*) eaglContext;
	// OpenGL ES 1 contexts are not supported.
	if (context == NULL || !openGLESLoad() || contextAPI(context) < 2)
		return kCVReturnInvalidArgument;

	double maximumAge = 1.0;
	if (cacheAttributes != NULL) {
		CFTypeRef age = CFDictionaryGetValue(cacheAttributes, kCVOpenGLESTextureCacheMaximumTextureAgeKey);
		if (age != NULL && CFGetTypeID(age) == CFNumberGetTypeID() && !CFNumberGetValue(age, kCFNumberDoubleType, &maximumAge))
			return kCVReturnInvalidArgument;
		if (maximumAge < 0)
			return kCVReturnInvalidArgument;
	}

	CVOpenGLESTextureCacheRef cache = (CVOpenGLESTextureCacheRef) _CFRuntimeCreateInstance(allocator,
		CVOpenGLESTextureCacheGetTypeID(), sizeof(struct __CVOpenGLESTextureCache) - sizeof(CFRuntimeBase), NULL);
	if (cache == NULL)
		return kCVReturnAllocationFailed;
	memset((char*) cache + sizeof(CFRuntimeBase), 0, sizeof(struct __CVOpenGLESTextureCache) - sizeof(CFRuntimeBase));
	pthread_mutex_init(&cache->lock, NULL);
	cache->context = contextRetain(context);
	cache->maximumAge = maximumAge;

	*cacheOut = cache;
	return kCVReturnSuccess;
}

// Deletes the entries nobody uses that are older than the limit (every one of them when the limit
// is negative). The cache's context is current, and the lock is held.
static void textureCacheEvictLocked(CVOpenGLESTextureCacheRef cache, double limitSeconds)
{
	uint64_t now = nowNanoseconds();
	for (size_t i = 0; i < cache->entryCount; ) {
		struct __CVOpenGLESTextureCacheEntry* entry = &cache->entries[i];
		Boolean old = limitSeconds < 0 || (now - entry->lastUse) > (uint64_t) (limitSeconds * 1e9);
		if (entry->users == 0 && old) {
			gl.glDeleteTextures(1, &entry->name);
			CFRelease(entry->buffer);
			*entry = cache->entries[--cache->entryCount];
		} else {
			i++;
		}
	}
}

static void textureCacheEntryRelease(CVOpenGLESTextureCacheRef cache, GLuint name)
{
	pthread_mutex_lock(&cache->lock);
	for (size_t i = 0; i < cache->entryCount; i++) {
		if (cache->entries[i].name == name) {
			if (cache->entries[i].users > 0)
				cache->entries[i].users--;
			cache->entries[i].lastUse = nowNanoseconds();
			break;
		}
	}
	pthread_mutex_unlock(&cache->lock);
}

void CVOpenGLESTextureCacheFlush(CVOpenGLESTextureCacheRef textureCache, CVOptionFlags options)
{
	if (textureCache == NULL)
		return;
	void* previous;
	if (!cacheEnter(textureCache, &previous))
		return;
	pthread_mutex_lock(&textureCache->lock);
	textureCacheEvictLocked(textureCache, -1);
	pthread_mutex_unlock(&textureCache->lock);
	cacheLeave(textureCache, previous);
}

// Finds the entry for a plane of a buffer with these texture parameters, making one (with a new
// texture name) when there is none. The cache's context is current, and the lock is held.
static struct __CVOpenGLESTextureCacheEntry* textureCacheFindEntryLocked(CVOpenGLESTextureCacheRef cache,
	CVPixelBufferRef buffer, size_t plane, GLenum target, GLint internalFormat, GLsizei width, GLsizei height,
	GLenum format, GLenum type)
{
	for (size_t i = 0; i < cache->entryCount; i++) {
		struct __CVOpenGLESTextureCacheEntry* entry = &cache->entries[i];
		if (entry->buffer == buffer && entry->plane == plane && entry->target == target
				&& entry->internalFormat == internalFormat && entry->width == width && entry->height == height
				&& entry->format == format && entry->type == type)
			return entry;
	}

	if (cache->entryCount == cache->entryCapacity) {
		size_t capacity = cache->entryCapacity ? cache->entryCapacity * 2 : 8;
		struct __CVOpenGLESTextureCacheEntry* entries = realloc(cache->entries, capacity * sizeof(*entries));
		if (entries == NULL)
			return NULL;
		cache->entries = entries;
		cache->entryCapacity = capacity;
	}

	GLuint name = 0;
	gl.glGenTextures(1, &name);
	if (name == 0)
		return NULL;
	struct __CVOpenGLESTextureCacheEntry* entry = &cache->entries[cache->entryCount++];
	entry->buffer = (CVPixelBufferRef) CFRetain(buffer);
	entry->plane = plane;
	entry->target = target;
	entry->internalFormat = internalFormat;
	entry->width = width;
	entry->height = height;
	entry->format = format;
	entry->type = type;
	entry->name = name;
	entry->users = 0;
	entry->lastUse = nowNanoseconds();
	return entry;
}

// Copies the pixels of one plane into the texture that is bound to GL_TEXTURE_2D. Rows are handed
// to GL tightly packed, since ES 2 cannot skip the padding at the end of a row itself; a BGRA
// plane that is read as GL_RGBA has its red and blue swapped on the way, so that the texture has
// the colours the format names.
static Boolean uploadPlane(CVPixelBufferRef buffer, size_t plane, size_t bytesPerPixel, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type)
{
	if (CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
		return false;
	const uint8_t* base = CVPixelBufferIsPlanar(buffer) ? CVPixelBufferGetBaseAddressOfPlane(buffer, plane)
		: CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferIsPlanar(buffer) ? CVPixelBufferGetBytesPerRowOfPlane(buffer, plane)
		: CVPixelBufferGetBytesPerRow(buffer);
	size_t rowBytes = (size_t) width * bytesPerPixel;
	Boolean swap = CVPixelBufferGetPixelFormatType(buffer) == 'BGRA' && format == GL_RGBA;

	const void* pixels = base;
	uint8_t* tight = NULL;
	if (swap || bytesPerRow != rowBytes) {
		tight = malloc(rowBytes * (size_t) height);
		if (tight == NULL) {
			CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
			return false;
		}
		for (GLsizei y = 0; y < height; y++) {
			const uint8_t* row = base + (size_t) y * bytesPerRow;
			uint8_t* out = tight + (size_t) y * rowBytes;
			if (swap) {
				for (size_t x = 0; x < rowBytes; x += 4) {
					out[x] = row[x + 2];
					out[x + 1] = row[x + 1];
					out[x + 2] = row[x];
					out[x + 3] = row[x + 3];
				}
			} else {
				memcpy(out, row, rowBytes);
			}
		}
		pixels = tight;
	}

	GLint alignment = 4;
	gl.glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Errors the application left unread are not this call's.
	while (gl.glGetError() != GL_NO_ERROR)
		;
	gl.glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, pixels);
	Boolean ok = gl.glGetError() == GL_NO_ERROR;
	gl.glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

	free(tight);
	CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
	return ok;
}

CVReturn CVOpenGLESTextureCacheCreateTextureFromImage(CFAllocatorRef allocator, CVOpenGLESTextureCacheRef textureCache,
	CVImageBufferRef sourceImage, CFDictionaryRef textureAttributes, GLenum target, GLint internalFormat,
	GLsizei width, GLsizei height, GLenum format, GLenum type, size_t planeIndex, CVOpenGLESTextureRef* textureOut)
{
	if (textureOut == NULL)
		return kCVReturnInvalidArgument;
	*textureOut = NULL;
	if (textureCache == NULL || sourceImage == NULL || CFGetTypeID(sourceImage) != CVPixelBufferGetTypeID())
		return kCVReturnInvalidArgument;

	// GL_TEXTURE_2D and GL_RENDERBUFFER are the targets Apple's cache takes. A renderbuffer would
	// have to write through to the buffer's memory, which the host's GL cannot do.
	if (target == GL_RENDERBUFFER)
		return kCVReturnUnsupported;
	if (target != GL_TEXTURE_2D)
		return kCVReturnInvalidArgument;

	// The plane index is ignored for a buffer that is not planar.
	CVPixelBufferRef buffer = (CVPixelBufferRef) sourceImage;
	size_t plane = CVPixelBufferIsPlanar(buffer) ? planeIndex : 0;
	if (CVPixelBufferIsPlanar(buffer) && plane >= CVPixelBufferGetPlaneCount(buffer))
		return kCVReturnInvalidArgument;
	size_t bytesPerPixel = texturePlaneBytesPerPixel(CVPixelBufferGetPixelFormatType(buffer), plane);
	if (bytesPerPixel == 0)
		return kCVReturnInvalidPixelFormat;

	// The format the application asks for has to describe the plane's pixels, and the image may be
	// a part of the plane at most.
	if (type != GL_UNSIGNED_BYTE || formatBytesPerPixel(format) != bytesPerPixel)
		return kCVReturnInvalidArgument;
	size_t planeWidth = CVPixelBufferIsPlanar(buffer) ? CVPixelBufferGetWidthOfPlane(buffer, plane) : CVPixelBufferGetWidth(buffer);
	size_t planeHeight = CVPixelBufferIsPlanar(buffer) ? CVPixelBufferGetHeightOfPlane(buffer, plane) : CVPixelBufferGetHeight(buffer);
	if (width <= 0 || height <= 0 || (size_t) width > planeWidth || (size_t) height > planeHeight)
		return kCVReturnInvalidSize;

	void* previous;
	if (!cacheEnter(textureCache, &previous))
		return kCVReturnError;

	CVReturn result = kCVReturnSuccess;
	pthread_mutex_lock(&textureCache->lock);
	if (textureCache->maximumAge > 0)
		textureCacheEvictLocked(textureCache, textureCache->maximumAge);
	struct __CVOpenGLESTextureCacheEntry* entry = textureCacheFindEntryLocked(textureCache, buffer, plane, target,
		internalFormat, width, height, format, type);
	if (entry == NULL) {
		result = kCVReturnAllocationFailed;
	} else {
		GLint previousBinding = 0;
		gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);
		gl.glBindTexture(GL_TEXTURE_2D, entry->name);
		if (!uploadPlane(buffer, plane, bytesPerPixel, internalFormat, width, height, format, type))
			result = kCVReturnInvalidArgument;
		gl.glBindTexture(GL_TEXTURE_2D, (GLuint) previousBinding);

		if (result == kCVReturnSuccess) {
			struct __CVOpenGLESTexture* texture = (struct __CVOpenGLESTexture*) _CFRuntimeCreateInstance(allocator,
				CVOpenGLESTextureGetTypeID(), sizeof(struct __CVOpenGLESTexture) - sizeof(CFRuntimeBase), NULL);
			if (texture == NULL) {
				result = kCVReturnAllocationFailed;
			} else {
				memset((char*) texture + sizeof(CFRuntimeBase), 0, sizeof(struct __CVOpenGLESTexture) - sizeof(CFRuntimeBase));
				texture->buffer.width = (size_t) width;
				texture->buffer.height = (size_t) height;
				texture->buffer.pixelFormat = CVPixelBufferGetPixelFormatType(buffer);
				texture->cache = (CVOpenGLESTextureCacheRef) CFRetain(textureCache);
				texture->source = (CVPixelBufferRef) CFRetain(buffer);
				texture->name = entry->name;
				texture->target = target;
				entry->users++;
				*textureOut = (CVOpenGLESTextureRef) texture;
			}
		}
	}
	pthread_mutex_unlock(&textureCache->lock);

	cacheLeave(textureCache, previous);
	return result;
}
