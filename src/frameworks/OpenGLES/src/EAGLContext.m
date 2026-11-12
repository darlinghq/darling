/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

/*
 * EAGL on the host's EGL and OpenGL ES: Mesa on Linux, the phone's own driver
 * on Android.
 *
 * An EAGLContext is an EGL context with no window of its own (a 1x1 pbuffer,
 * or none). As on iOS, apps draw into a renderbuffer of their own, which
 * -renderbufferStorage:fromDrawable: sizes to a CAEAGLLayer. Presenting it
 * reads its pixels back into an image, which becomes the layer's contents,
 * and UIKit draws those as it draws a view.
 */

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/CALayer.h>
#import <objc/message.h>
#include <elfcalls.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <os/lock.h>

#include "gles-functions.h"

extern struct elf_calls *_elfcalls;

NSString *const kEAGLDrawablePropertyRetainedBacking = @"EAGLDrawablePropertyRetainedBacking";
NSString *const kEAGLDrawablePropertyColorFormat = @"EAGLDrawablePropertyColorFormat";
NSString *const kEAGLColorFormatRGBA8 = @"EAGLColorFormatRGBA8";
NSString *const kEAGLColorFormatRGB565 = @"EAGLColorFormat565";
NSString *const kEAGLColorFormatSRGBA8 = @"EAGLColorFormatSRGBA8";

void EAGLGetVersion(unsigned int *major, unsigned int *minor)
{
	if (major != NULL)
		*major = EAGL_MAJOR_VERSION;
	if (minor != NULL)
		*minor = EAGL_MINOR_VERSION;
}

#pragma mark The host's libraries

typedef void *EGLDisplay, *EGLConfig, *EGLContext, *EGLSurface;
typedef int EGLint;
typedef unsigned int EGLBoolean, EGLenum;

#define EGL_DEFAULT_DISPLAY ((void *)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_SURFACE ((EGLSurface)0)
#define EGL_NONE 0x3038
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_SURFACE_TYPE 0x3033
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_PBUFFER_BIT 0x0001
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_ES3_BIT 0x0040
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
#define EGL_EXTENSIONS 0x3055
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_OPENGL_ES_API 0x30A0
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD

static struct {
	void *egl, *gles2, *gles1;
	void *(*getProcAddress)(const char *);
	EGLDisplay (*getDisplay)(void *);
	EGLDisplay (*getPlatformDisplay)(EGLenum, void *, const intptr_t *);
	EGLBoolean (*initialize)(EGLDisplay, EGLint *, EGLint *);
	const char *(*queryString)(EGLDisplay, EGLint);
	EGLBoolean (*bindAPI)(EGLenum);
	EGLBoolean (*chooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
	EGLContext (*createContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
	EGLBoolean (*destroyContext)(EGLDisplay, EGLContext);
	EGLSurface (*createPbufferSurface)(EGLDisplay, EGLConfig, const EGLint *);
	EGLBoolean (*destroySurface)(EGLDisplay, EGLSurface);
	EGLBoolean (*makeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
	EGLint (*getError)(void);
	EGLDisplay display;
	BOOL ready;
} egl;

static void *open_host(const char *const *names)
{
	for (; *names != NULL; names++) {
		void *handle = _elfcalls->dlopen(*names);
		if (handle != NULL)
			return handle;
	}
	return NULL;
}

static void *egl_symbol(const char *name)
{
	return egl.egl != NULL ? _elfcalls->dlsym(egl.egl, name) : NULL;
}

static BOOL egl_load(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		static const char *const eglNames[] = { "libEGL.so", "libEGL.so.1", NULL };
		static const char *const gles2Names[] = { "libGLESv2.so", "libGLESv2.so.2", NULL };
		static const char *const gles1Names[] = { "libGLESv1_CM.so", "libGLESv1_CM.so.1", NULL };
		egl.egl = open_host(eglNames);
		egl.gles2 = open_host(gles2Names);
		egl.gles1 = open_host(gles1Names);
		if (egl.egl == NULL) {
			NSLog(@"OpenGLES: the host has no libEGL");
			return;
		}
		egl.getProcAddress = egl_symbol("eglGetProcAddress");
		egl.getDisplay = egl_symbol("eglGetDisplay");
		egl.getPlatformDisplay = egl_symbol("eglGetPlatformDisplay");
		egl.initialize = egl_symbol("eglInitialize");
		egl.queryString = egl_symbol("eglQueryString");
		egl.bindAPI = egl_symbol("eglBindAPI");
		egl.chooseConfig = egl_symbol("eglChooseConfig");
		egl.createContext = egl_symbol("eglCreateContext");
		egl.destroyContext = egl_symbol("eglDestroyContext");
		egl.createPbufferSurface = egl_symbol("eglCreatePbufferSurface");
		egl.destroySurface = egl_symbol("eglDestroySurface");
		egl.makeCurrent = egl_symbol("eglMakeCurrent");
		egl.getError = egl_symbol("eglGetError");

		/* Mesa: no window system is needed (or wanted: there may be no X
		 * server, as under a phone's display). Android has only one. */
		const char *clientExtensions = egl.queryString(EGL_NO_CONTEXT, EGL_EXTENSIONS);
		if (egl.getPlatformDisplay != NULL && clientExtensions != NULL
				&& strstr(clientExtensions, "EGL_MESA_platform_surfaceless") != NULL)
			egl.display = egl.getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
		if (egl.display == NULL)
			egl.display = egl.getDisplay(EGL_DEFAULT_DISPLAY);
		EGLint major = 0, minor = 0;
		if (egl.display == NULL || !egl.initialize(egl.display, &major, &minor)) {
			NSLog(@"OpenGLES: cannot initialize EGL (error 0x%x)", egl.getError());
			return;
		}
		egl.bindAPI(EGL_OPENGL_ES_API);
		egl.ready = YES;
	});
	return egl.ready;
}

/* Called when a host function is missing: does nothing and returns 0. */
__asm__(".text\n.p2align 2\n_gles_missing:\n\tmov x0, #0\n\tmovi d0, #0\n\tret\n");
extern void gles_missing(void);

/* iOS names the host may know by another. */
static const char *const gles_aliases[][2] = {
	{ "glDiscardFramebufferEXT", "glInvalidateFramebuffer" },
	{ "glRenderbufferStorageMultisampleAPPLE", "glRenderbufferStorageMultisample" },
	{ "glRenderbufferStorageMultisampleAPPLE", "glRenderbufferStorageMultisampleEXT" },
	{ "glFenceSyncAPPLE", "glFenceSync" },
	{ "glClientWaitSyncAPPLE", "glClientWaitSync" },
	{ "glDeleteSyncAPPLE", "glDeleteSync" },
	{ "glMapBufferRangeEXT", "glMapBufferRange" },
	{ "glFlushMappedBufferRangeEXT", "glFlushMappedBufferRange" },
	{ "glBindVertexArrayOES", "glBindVertexArray" },
	{ "glGenVertexArraysOES", "glGenVertexArrays" },
	{ "glDeleteVertexArraysOES", "glDeleteVertexArrays" },
	{ "glIsVertexArrayOES", "glIsVertexArray" },
};

static void *gles_lookup(const char *name)
{
	void *address = NULL;
	if (egl.gles2 != NULL)
		address = _elfcalls->dlsym(egl.gles2, name);
	if (address == NULL && egl.gles1 != NULL)
		address = _elfcalls->dlsym(egl.gles1, name);
	if (address == NULL && egl.getProcAddress != NULL)
		address = egl.getProcAddress(name);
	return address;
}

static void gles_resolve_apple(void);

void *gles_resolve(struct gles_slot *slot)
{
	egl_load();
	void *address = NULL;
	if (strcmp(slot->name, "glResolveMultisampleFramebufferAPPLE") == 0)
		address = (void *)gles_resolve_apple;
	if (address == NULL)
		address = gles_lookup(slot->name);
	for (size_t i = 0; address == NULL && i < sizeof(gles_aliases) / sizeof(gles_aliases[0]); i++) {
		if (strcmp(slot->name, gles_aliases[i][0]) == 0)
			address = gles_lookup(gles_aliases[i][1]);
	}
	/* OES and EXT functions that became core ones. */
	size_t length = strlen(slot->name);
	if (address == NULL && length > 3 && (strcmp(slot->name + length - 3, "OES") == 0 || strcmp(slot->name + length - 3, "EXT") == 0)) {
		char core[128];
		if (length - 3 < sizeof(core)) {
			memcpy(core, slot->name, length - 3);
			core[length - 3] = 0;
			address = gles_lookup(core);
		}
	}
	if (address == NULL) {
		NSLog(@"OpenGLES: the host has no %s; it does nothing", slot->name);
		address = (void *)gles_missing;
	}
	__atomic_store_n(&slot->address, address, __ATOMIC_RELEASE);
	return address;
}

#pragma mark GL, as this file uses it

#define GL_RENDERBUFFER 0x8D41
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_FRAMEBUFFER 0x8D40
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_TEXTURE 0x1702
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RGBA8 0x8058
#define GL_RGB565 0x8D62
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_NEAREST 0x2600
#define GL_NO_ERROR 0
#define GL_EXTENSIONS 0x1F03
#define GL_BGRA_EXT 0x80E1

/* These are this framework's own entry points (gles-functions.c). */
extern void glGetIntegerv(GLenum, GLint *);
extern GLenum glGetError(void);
extern void glPixelStorei(GLenum, GLint);
extern void glReadPixels(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
extern const unsigned char *glGetString(GLenum);
extern void glBindBuffer(GLenum, GLuint);
extern void glBlitFramebuffer(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
extern void glGetFramebufferAttachmentParameteriv(GLenum, GLenum, GLenum, GLint *);
extern void glGenFramebuffers(GLsizei, GLuint *);
extern void glDeleteFramebuffers(GLsizei, const GLuint *);
extern void glBindFramebuffer(GLenum, GLuint);
extern void glFramebufferRenderbuffer(GLenum, GLenum, GLenum, GLuint);
extern void glBindRenderbuffer(GLenum, GLuint);
extern void glRenderbufferStorage(GLenum, GLenum, GLsizei, GLsizei);
extern void glGetRenderbufferParameteriv(GLenum, GLenum, GLint *);
extern void glGenFramebuffersOES(GLsizei, GLuint *);
extern void glDeleteFramebuffersOES(GLsizei, const GLuint *);
extern void glBindFramebufferOES(GLenum, GLuint);
extern void glFramebufferRenderbufferOES(GLenum, GLenum, GLenum, GLuint);
extern void glBindRenderbufferOES(GLenum, GLuint);
extern void glRenderbufferStorageOES(GLenum, GLenum, GLsizei, GLsizei);
extern void glGetRenderbufferParameterivOES(GLenum, GLenum, GLint *);

/* ES 1 has the framebuffer functions as OES ones; ES 2 and 3 as core. */
struct gles_framebuffers {
	void (*gen)(GLsizei, GLuint *);
	void (*delete)(GLsizei, const GLuint *);
	void (*bind)(GLenum, GLuint);
	void (*attach)(GLenum, GLenum, GLenum, GLuint);
	void (*bindRenderbuffer)(GLenum, GLuint);
	void (*storage)(GLenum, GLenum, GLsizei, GLsizei);
	void (*parameter)(GLenum, GLenum, GLint *);
};

static const struct gles_framebuffers gles_core = {
	glGenFramebuffers, glDeleteFramebuffers, glBindFramebuffer, glFramebufferRenderbuffer,
	glBindRenderbuffer, glRenderbufferStorage, glGetRenderbufferParameteriv,
};
static const struct gles_framebuffers gles_oes = {
	glGenFramebuffersOES, glDeleteFramebuffersOES, glBindFramebufferOES, glFramebufferRenderbufferOES,
	glBindRenderbufferOES, glRenderbufferStorageOES, glGetRenderbufferParameterivOES,
};

/* APPLE_framebuffer_multisample's resolve: a blit of the whole read
 * framebuffer into the draw one, as ES 3 has it. */
static void gles_resolve_apple(void)
{
	GLint type = 0, name = 0, renderbuffer = 0, width = 0, height = 0;
	glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
	glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
	if (type != GL_RENDERBUFFER)
		return;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, name);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

#pragma mark EAGLSharegroup

@implementation EAGLSharegroup {
@public
	/* The first context made in the group, which the others share with. */
	EGLContext _shared;
}
@synthesize debugLabel = _debugLabel;
@end

#pragma mark EAGLContext

static pthread_key_t sCurrentContextKey;

@implementation EAGLContext {
	EAGLRenderingAPI _api;
	EAGLSharegroup *_sharegroup;
	EGLContext _context;
	EGLSurface _surface;
	const struct gles_framebuffers *_framebuffers;
	GLuint _readFramebuffer;
	/* Whether frames are read back as BGRA, the window's own order: 1 yes,
	 * 0 no, -1 not known yet. */
	int _readsBGRA;
	/* Renderbuffer names to the layers they were sized for. */
	NSMapTable *_drawables;
	NSString *_debugLabel;
	BOOL _multiThreaded;
}

+ (void)initialize
{
	if (self == [EAGLContext class])
		pthread_key_create(&sCurrentContextKey, NULL);
}

- (instancetype)initWithAPI:(EAGLRenderingAPI)api
{
	return [self initWithAPI:api sharegroup:nil];
}

- (instancetype)initWithAPI:(EAGLRenderingAPI)api sharegroup:(EAGLSharegroup *)sharegroup
{
	self = [super init];
	if (self == nil)
		return nil;
	if (api < kEAGLRenderingAPIOpenGLES1 || api > kEAGLRenderingAPIOpenGLES3 || !egl_load())
		return nil;

	EGLint renderable = api == kEAGLRenderingAPIOpenGLES1 ? EGL_OPENGL_ES_BIT
		: api == kEAGLRenderingAPIOpenGLES2 ? EGL_OPENGL_ES2_BIT : EGL_OPENGL_ES3_BIT;
	EGLint pbufferAttributes[] = {
		EGL_RENDERABLE_TYPE, renderable, EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE,
	};
	EGLConfig config = NULL;
	EGLint count = 0;
	BOOL pbuffer = egl.chooseConfig(egl.display, pbufferAttributes, &config, 1, &count) && count > 0;
	if (!pbuffer) {
		/* Surfaceless platforms may have no pbuffers: then no surface at
		 * all (KHR_surfaceless_context), since apps draw into their own
		 * framebuffers anyway. */
		EGLint attributes[] = { EGL_RENDERABLE_TYPE, renderable, EGL_NONE };
		if (!egl.chooseConfig(egl.display, attributes, &config, 1, &count) || count == 0) {
			NSLog(@"OpenGLES: the host has no OpenGL ES %lu", (unsigned long)api);
			return nil;
		}
	}

	_sharegroup = sharegroup != nil ? sharegroup : [EAGLSharegroup new];
	EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, (EGLint)api, EGL_NONE };
	_context = egl.createContext(egl.display, config, _sharegroup->_shared, contextAttributes);
	if (_context == EGL_NO_CONTEXT) {
		NSLog(@"OpenGLES: cannot create an OpenGL ES %lu context (error 0x%x)", (unsigned long)api, egl.getError());
		return nil;
	}
	if (_sharegroup->_shared == EGL_NO_CONTEXT)
		_sharegroup->_shared = _context;
	if (pbuffer) {
		EGLint size[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
		_surface = egl.createPbufferSurface(egl.display, config, size);
	}
	_api = api;
	_framebuffers = api == kEAGLRenderingAPIOpenGLES1 ? &gles_oes : &gles_core;
	_drawables = [NSMapTable strongToWeakObjectsMapTable];
	_readsBGRA = -1;
	return self;
}

- (void)dealloc
{
	if ((__bridge void *)self == pthread_getspecific(sCurrentContextKey))
		[EAGLContext setCurrentContext:nil];
	if (_surface != EGL_NO_SURFACE)
		egl.destroySurface(egl.display, _surface);
	/* The group's first context stays while others share with it. */
	if (_context != EGL_NO_CONTEXT && _context != _sharegroup->_shared)
		egl.destroyContext(egl.display, _context);
}

+ (BOOL)setCurrentContext:(EAGLContext *)context
{
	void *previous = pthread_getspecific(sCurrentContextKey);
	if (previous == (__bridge void *)context)
		return YES;
	BOOL made;
	if (context != nil)
		made = egl.makeCurrent(egl.display, context->_surface, context->_surface, context->_context);
	else
		made = !egl.ready || egl.makeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (!made)
		return NO;
	/* As on iOS, the current context is retained. */
	if (context != nil)
		CFBridgingRetain(context);
	pthread_setspecific(sCurrentContextKey, (__bridge void *)context);
	if (previous != NULL)
		CFBridgingRelease(previous);
	return YES;
}

+ (EAGLContext *)currentContext
{
	return (__bridge EAGLContext *)pthread_getspecific(sCurrentContextKey);
}

- (EAGLRenderingAPI)API { return _api; }
- (EAGLSharegroup *)sharegroup { return _sharegroup; }
- (NSString *)debugLabel { return _debugLabel; }
- (void)setDebugLabel:(NSString *)label { _debugLabel = [label copy]; }
- (BOOL)isMultiThreaded { return _multiThreaded; }
- (void)setMultiThreaded:(BOOL)multiThreaded { _multiThreaded = multiThreaded; }

/* The size a layer is shown at, in pixels: its view's, when it has one,
 * since UIKit sizes views rather than their layers. */
static CGSize drawable_pixel_size(CALayer *drawable)
{
	CGRect bounds = [drawable bounds];
	CGFloat scale = [drawable contentsScale];
	id delegate = [drawable delegate];
	if (delegate != nil && [delegate respondsToSelector:@selector(contentScaleFactor)]) {
		bounds = ((CGRect (*)(id, SEL))objc_msgSend)(delegate, @selector(bounds));
		scale = ((CGFloat (*)(id, SEL))objc_msgSend)(delegate, @selector(contentScaleFactor));
	}
	if (scale <= 0)
		scale = 1;
	return CGSizeMake(floor(bounds.size.width * scale), floor(bounds.size.height * scale));
}

- (BOOL)renderbufferStorage:(NSUInteger)target fromDrawable:(id<EAGLDrawable>)drawable
{
	if (target != GL_RENDERBUFFER || drawable == nil)
		return NO;
	GLint renderbuffer = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
	if (renderbuffer == 0)
		return NO;

	NSString *format = [[drawable drawableProperties] objectForKey:kEAGLDrawablePropertyColorFormat];
	GLenum internalFormat = [format isEqualToString:kEAGLColorFormatRGB565] ? GL_RGB565
		: [format isEqualToString:kEAGLColorFormatSRGBA8] && _api >= kEAGLRenderingAPIOpenGLES3 ? GL_SRGB8_ALPHA8
		: GL_RGBA8;
	CGSize size = drawable_pixel_size((CALayer *)drawable);
	while (glGetError() != GL_NO_ERROR)
		;
	_framebuffers->storage(GL_RENDERBUFFER, internalFormat, (GLsizei)size.width, (GLsizei)size.height);
	if (glGetError() != GL_NO_ERROR)
		return NO;
	[_drawables setObject:drawable forKey:@(renderbuffer)];
	return YES;
}

/* Frames' pixels are reused: a frame is megabytes, and new memory is
 * zeroed a page at a time. A frame's comes back when its image goes (the
 * layer shows the next one). */
static os_unfair_lock sSpareLock = OS_UNFAIR_LOCK_INIT;
static void *sSpare[2];
static size_t sSpareSize[2];

static void *take_pixels(size_t size)
{
	void *pixels = NULL;
	os_unfair_lock_lock(&sSpareLock);
	for (int i = 0; i < 2 && pixels == NULL; i++) {
		if (sSpare[i] != NULL && sSpareSize[i] == size) {
			pixels = sSpare[i];
			sSpare[i] = NULL;
		}
	}
	os_unfair_lock_unlock(&sSpareLock);
	return pixels != NULL ? pixels : malloc(size);
}

static void release_pixels(void *info, const void *data, size_t size)
{
	void *stale = (void *)data;
	os_unfair_lock_lock(&sSpareLock);
	/* An empty slot, or one of another size (the view's old one). */
	for (int i = 0; i < 2 && stale == data; i++) {
		if (sSpare[i] == NULL || sSpareSize[i] != size) {
			stale = sSpare[i];
			sSpare[i] = (void *)data;
			sSpareSize[i] = size;
		}
	}
	os_unfair_lock_unlock(&sSpareLock);
	free(stale);
}

/* The bound renderbuffer's pixels, bottom row first, as GL has them. */
- (CGImageRef)copyRenderbufferImage:(GLuint)renderbuffer opaque:(BOOL)opaque CF_RETURNS_RETAINED
{
	const struct gles_framebuffers *fb = _framebuffers;
	GLint width = 0, height = 0;
	fb->parameter(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	fb->parameter(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	if (width <= 0 || height <= 0)
		return NULL;

	GLint framebuffer = 0, packBuffer = 0, alignment = 4, rowLength = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
	glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	if (_api >= kEAGLRenderingAPIOpenGLES3) {
		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &packBuffer);
		glGetIntegerv(GL_PACK_ROW_LENGTH, &rowLength);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	}
	if (_readFramebuffer == 0)
		fb->gen(1, &_readFramebuffer);
	fb->bind(GL_FRAMEBUFFER, _readFramebuffer);
	fb->attach(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	size_t bytesPerRow = (size_t)width * 4;
	void *pixels = take_pixels(bytesPerRow * height);
	if (_readsBGRA < 0) {
		const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
		_readsBGRA = extensions != NULL && strstr(extensions, "GL_EXT_read_format_bgra") != NULL;
	}
	BOOL bgra = NO;
	if (pixels != NULL && _readsBGRA) {
		while (glGetError() != GL_NO_ERROR)
			;
		glReadPixels(0, 0, width, height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
		bgra = glGetError() == GL_NO_ERROR;
		if (!bgra)
			_readsBGRA = 0;
	}
	if (pixels != NULL && !bgra)
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	fb->bind(GL_FRAMEBUFFER, framebuffer);
	glPixelStorei(GL_PACK_ALIGNMENT, alignment);
	if (_api >= kEAGLRenderingAPIOpenGLES3) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
		glPixelStorei(GL_PACK_ROW_LENGTH, rowLength);
	}
	if (pixels == NULL)
		return NULL;

	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, pixels, bytesPerRow * height, release_pixels);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	/* An opaque layer's alpha is whatever the app left there. BGRA is the
	 * window's order, so drawing the frame there copies it as it is. */
	CGBitmapInfo info = bgra
		? (opaque ? kCGImageAlphaNoneSkipFirst : kCGImageAlphaPremultipliedFirst) | kCGBitmapByteOrder32Little
		: (opaque ? kCGImageAlphaNoneSkipLast : kCGImageAlphaPremultipliedLast) | kCGBitmapByteOrder32Big;
	CGImageRef image = CGImageCreate(width, height, 8, 32, bytesPerRow, space, info, provider, NULL, false, kCGRenderingIntentDefault);
	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);
	return image;
}

/* DARLING_EAGL_TRACE=1: every second, how long reading the frame back and
 * showing it took. */
static double trace_now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

/* Shows the image: the layer's contents, drawn by its view. */
static void show_on_layer(CALayer *layer, CGImageRef image)
{
	void (^show)(void) = ^{
		[layer setContents:(__bridge id)image];
		id view = [layer delegate];
		if (view == nil || ![view respondsToSelector:@selector(setNeedsDisplay:)])
			return;
		((void (*)(id, SEL, BOOL))objc_msgSend)(view, @selector(setNeedsDisplay:), YES);
		/* Apps that present from their own loop (SDL's, say) seldom let
		 * the run loop wait, which is when UIKit would draw: draw now. */
		/* The AppKit window that hosts the view (a UIView's -window is its UIWindow). */
		SEL hostWindow = [view respondsToSelector:@selector(_appKitWindow)] ? @selector(_appKitWindow) : @selector(window);
		id window = [view respondsToSelector:hostWindow] ? ((id (*)(id, SEL))objc_msgSend)(view, hostWindow) : nil;
		if (window != nil && [window respondsToSelector:@selector(displayIfNeeded)])
			((void (*)(id, SEL))objc_msgSend)(window, @selector(displayIfNeeded));
	};
	if ([NSThread isMainThread]) {
		show();
	} else {
		CGImageRetain(image);
		dispatch_async(dispatch_get_main_queue(), ^{
			show();
			CGImageRelease(image);
		});
	}
}

static void trace_present(double read, double show)
{
	static int tracing = -1, count;
	static double reading, showing, since;
	if (tracing < 0)
		tracing = getenv("DARLING_EAGL_TRACE") != NULL;
	if (!tracing)
		return;
	reading += read;
	showing += show;
	count++;
	double now = trace_now();
	if (since == 0)
		since = now;
	if (now - since < 1000)
		return;
	NSLog(@"OpenGLES: %d presents, %.2f ms reading back, %.2f ms showing", count, reading / count, showing / count);
	count = 0;
	reading = showing = 0;
	since = now;
}

- (BOOL)presentRenderbuffer:(NSUInteger)target
{
	if (target != GL_RENDERBUFFER)
		return NO;
	GLint renderbuffer = 0;
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
	CALayer *layer = [_drawables objectForKey:@(renderbuffer)];
	if (layer == nil)
		return NO;
	double started = trace_now();
	CGImageRef image = [self copyRenderbufferImage:renderbuffer opaque:[layer isOpaque]];
	if (image == NULL)
		return NO;
	double read = trace_now();
	show_on_layer(layer, image);
	CGImageRelease(image);
	trace_present(read - started, trace_now() - read);
	return YES;
}

- (BOOL)presentRenderbuffer:(NSUInteger)target atTime:(CFTimeInterval)presentationTime
{
	return [self presentRenderbuffer:target];
}

- (BOOL)presentRenderbuffer:(NSUInteger)target afterMinimumDuration:(CFTimeInterval)duration
{
	return [self presentRenderbuffer:target];
}

@end
