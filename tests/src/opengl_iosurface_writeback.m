// Copyright (C) 2026 Lubos Dolezel
//
// An IOSurface is shared memory: GL rendering into a texture that
// CGLTexImageIOSurface2D specified from a surface is in the surface for the
// CPU and for other contexts once the producer has flushed it, and stays
// there whatever then happens to the texture (on macOS the surface is the
// texture's storage). The cases:
//  - keep:      render, glFlush, read the surface;
//  - delete:    render, glFlush, delete the texture and framebuffer, read;
//  - delete-unflushed: render, delete the texture, glFlush, read;
//  - consumer:  render, glFlush, another context specifies a texture from
//               the surface, render again, glFlush: the second rendering
//               is what the surface holds;
//  - respecify: render, glTexImage2D on the texture (same size, then
//               smaller), glFlush: the rendering is in the surface, and so is
//               rendering after that, over the whole surface (on macOS
//               26.7.3 the texture stays the surface's; glTexImage2D fails
//               with GL_INVALID_OPERATION there);
//  - rebind:    render, glFlush, CGLTexImageIOSurface2D of the texture from
//               another surface: the first surface keeps the rendering and
//               later rendering goes to the second;
//  - threads:   a producer context on one thread renders, flushes and
//               keeps the context current while a consumer context on a
//               second thread reads the surface.
// Each case runs for GL_TEXTURE_RECTANGLE and, where the implementation
// takes it (Darling does, macOS does not), GL_TEXTURE_2D. Reads are by the
// CPU under a read-only lock and through a texture of another context.
// Where Darling gives the surface a GPU buffer (a driver that cannot render
// into the surface's memory), each case also prints whether it has one and
// the copies made, through the private statistics.
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
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if __has_include(<IOSurface/IOSurfacePriv.h>)
#include <IOSurface/IOSurfacePriv.h>
#define HAVE_PRIVATE_HEADER 1
#endif

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_ARB
#endif

#define W 64
#define H 32

#define GREY 0xff808080u
#define RED 0xffff0000u
#define GREEN 0xff00ff00u
#define BLUE 0xff0000ffu
#define YELLOW 0xffffff00u
#define MAGENTA 0xffff00ffu
#define CYAN 0xff00ffffu

static int g_failures;
static CGLPixelFormatObj g_pixelFormat;

static void expectPixel(const char *what, const char *target, uint32_t got, uint32_t expected) {
	if (got == expected) {
		printf("PASS: %s (%s)\n", what, target);
	} else {
		printf("FAIL: %s (%s): %08x, expected %08x\n", what, target, got, expected);
		g_failures++;
	}
}

// Prints whether Darling gave the surface a GPU buffer, and its copies.
static void printGPUBuffer(const char *what, const char *target, IOSurfaceRef surface) {
#ifdef HAVE_PRIVATE_HEADER
	typedef bool (*statistics_proc)(IOSurfaceRef, _IOSurfaceGPUBufferStatistics *);
	statistics_proc statistics = (statistics_proc) dlsym(RTLD_DEFAULT, "_IOSurfaceGetGPUBufferStatistics");
	_IOSurfaceGPUBufferStatistics s = { .version = _IOSURFACE_GPU_BUFFER_VERSION };
	if (statistics && statistics(surface, &s))
		printf("gpu buffer (%s, %s): %s, downloads %llu, uploads %llu\n", what, target, s.hasBuffer ? "yes" : "no",
			(unsigned long long) s.downloads, (unsigned long long) s.uploads);
#endif
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

static void fill(IOSurfaceRef surface, uint32_t value) {
	IOSurfaceLock(surface, 0, NULL);
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			*(uint32_t *) (base + y * bytesPerRow + x * 4) = value;
	IOSurfaceUnlock(surface, 0, NULL);
}

static IOSurfaceRef createSurface(void) {
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	if (surface != NULL)
		fill(surface, GREY);
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

static CGLContextObj createContext(void) {
	CGLContextObj context = NULL;
	if (CGLCreateContext(g_pixelFormat, NULL, &context) != kCGLNoError)
		return NULL;
	return context;
}

// The surface's contents as a texture of consumer specified from it shows
// them, read through a framebuffer: their common value, or 0xdeadbeef if
// the first and last pixel differ. Restores the calling thread's current
// context.
static uint32_t consumerPixel(CGLContextObj consumer, IOSurfaceRef surface) {
	static uint32_t pixels[W * H];
	CGLContextObj previous = CGLGetCurrentContext();
	CGLSetCurrentContext(consumer);
	GLuint texture = 0, framebuffer = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, texture);
	uint32_t result = 0xdeadbeefu;
	if (specify(consumer, GL_TEXTURE_RECTANGLE, surface) == kCGLNoError) {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, texture, 0);
		glReadPixels(0, 0, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
		result = pixels[0] == pixels[W * H - 1] ? pixels[0] : 0xdeadbeefu;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &framebuffer);
	}
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	glDeleteTextures(1, &texture);
	CGLSetCurrentContext(previous);
	return result;
}

struct producer {
	GLenum target;
	GLuint texture, framebuffer;
};

// In the current context: a texture specified from surface, attached to a
// framebuffer that is bound.
static void startProducer(struct producer *p, GLenum target, IOSurfaceRef surface) {
	p->target = target;
	glGenTextures(1, &p->texture);
	glBindTexture(target, p->texture);
	if (specify(CGLGetCurrentContext(), target, surface) != kCGLNoError) {
		printf("FAIL: CGLTexImageIOSurface2D\n");
		g_failures++;
	}
	glGenFramebuffers(1, &p->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, p->texture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("FAIL: framebuffer incomplete\n");
		g_failures++;
	}
	glViewport(0, 0, W, H);
}

static void render(uint32_t color) {
	glClearColor(((color >> 16) & 0xff) / 255.0f, ((color >> 8) & 0xff) / 255.0f, (color & 0xff) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void stopProducer(struct producer *p) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (p->framebuffer != 0)
		glDeleteFramebuffers(1, &p->framebuffer);
	if (p->texture != 0)
		glDeleteTextures(1, &p->texture);
	p->framebuffer = p->texture = 0;
}

static const char *targetName(GLenum target) {
	return target == GL_TEXTURE_RECTANGLE ? "rectangle" : "2d";
}

// Prints how this implementation keeps a texture of the target in step with
// its surface, as the results may differ by path: "shared" when a CPU write
// shows in the texture (the texture uses the surface's memory, or is kept in
// step by GPU copies when the context is locked), else "copy".
static void printPath(CGLContextObj context, GLenum target) {
	static uint32_t texels[W * H];
	CGLSetCurrentContext(context);
	IOSurfaceRef surface = createSurface();
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	specify(context, target, surface);
	fill(surface, GREEN);
	CGLLockContext(context);
	CGLUnlockContext(context);
	glGetTexImage(target, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texels);
	printf("path (%s): %s\n", targetName(target), texels[0] == GREEN ? "shared" : "copy");
	glDeleteTextures(1, &texture);
	CFRelease(surface);
	CGLSetCurrentContext(NULL);
}

// Whether CGLTexImageIOSurface2D takes the target.
static int targetSupported(CGLContextObj context, GLenum target) {
	CGLSetCurrentContext(context);
	IOSurfaceRef surface = createSurface();
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	int supported = specify(context, target, surface) == kCGLNoError;
	glDeleteTextures(1, &texture);
	CFRelease(surface);
	while (glGetError() != GL_NO_ERROR)
		;
	CGLSetCurrentContext(NULL);
	if (!supported)
		printf("SKIP: CGLTexImageIOSurface2D does not take %s textures\n", targetName(target));
	return supported;
}

static void runSingleThreadCases(CGLContextObj producer, CGLContextObj consumer, GLenum target) {
	const char *name = targetName(target);
	struct producer p;
	CGLSetCurrentContext(producer);

	// keep
	IOSurfaceRef s = createSurface();
	startProducer(&p, target, s);
	render(RED);
	glFlush();
	expectPixel("keep: CPU sees the flushed rendering", name, cpuPixel(s), RED);
	expectPixel("keep: another context sees the flushed rendering", name, consumerPixel(consumer, s), RED);
	printGPUBuffer("keep", name, s);
	stopProducer(&p);
	CFRelease(s);

	// delete
	s = createSurface();
	startProducer(&p, target, s);
	render(RED);
	glFlush();
	stopProducer(&p);
	expectPixel("delete: CPU sees the rendering after the texture is deleted", name, cpuPixel(s), RED);
	expectPixel("delete: another context sees it", name, consumerPixel(consumer, s), RED);
	printGPUBuffer("delete", name, s);
	CFRelease(s);

	// delete-unflushed
	s = createSurface();
	startProducer(&p, target, s);
	render(RED);
	glDeleteTextures(1, &p.texture);
	p.texture = 0;
	glFlush();
	expectPixel("delete-unflushed: rendering before the delete is in the surface", name, cpuPixel(s), RED);
	printGPUBuffer("delete-unflushed", name, s);
	stopProducer(&p);
	CFRelease(s);

	// consumer
	s = createSurface();
	startProducer(&p, target, s);
	render(RED);
	glFlush();
	CGLSetCurrentContext(consumer);
	GLuint consumerTexture = 0;
	glGenTextures(1, &consumerTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, consumerTexture);
	specify(consumer, GL_TEXTURE_RECTANGLE, s);
	CGLSetCurrentContext(producer);
	render(GREEN);
	glFlush();
	expectPixel("consumer: the producer's second rendering is in the surface", name, cpuPixel(s), GREEN);
	expectPixel("consumer: another context sees the second rendering", name, consumerPixel(consumer, s), GREEN);
	printGPUBuffer("consumer", name, s);
	stopProducer(&p);
	CGLSetCurrentContext(consumer);
	glDeleteTextures(1, &consumerTexture);
	CGLSetCurrentContext(producer);
	CFRelease(s);

	// respecify
	s = createSurface();
	startProducer(&p, target, s);
	render(BLUE);
	glTexImage2D(target, 0, GL_RGBA8, W, H, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	while (glGetError() != GL_NO_ERROR)
		;
	glFlush();
	expectPixel("respecify: rendering before glTexImage2D is in the surface", name, cpuPixel(s), BLUE);
	render(YELLOW);
	glFlush();
	expectPixel("respecify: rendering after glTexImage2D is in the surface", name, cpuPixel(s), YELLOW);
	glTexImage2D(target, 0, GL_RGBA8, W / 2, H / 2, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	while (glGetError() != GL_NO_ERROR)
		;
	glFlush();
	expectPixel("respecify: a smaller glTexImage2D keeps the surface's contents", name, cpuPixel(s), YELLOW);
	render(GREEN);
	glFlush();
	expectPixel("respecify: rendering after it covers the whole surface", name, cpuPixel(s), GREEN);
	printGPUBuffer("respecify", name, s);
	stopProducer(&p);
	CFRelease(s);

	// rebind
	IOSurfaceRef first = createSurface(), second = createSurface();
	startProducer(&p, target, first);
	render(MAGENTA);
	glFlush();
	specify(producer, target, second);
	expectPixel("rebind: rendering before the rebind is in the first surface", name, cpuPixel(first), MAGENTA);
	expectPixel("rebind: the second surface is unchanged", name, cpuPixel(second), GREY);
	render(CYAN);
	glFlush();
	expectPixel("rebind: rendering after the rebind is in the second surface", name, cpuPixel(second), CYAN);
	expectPixel("rebind: and not in the first", name, cpuPixel(first), MAGENTA);
	printGPUBuffer("rebind, first", name, first);
	printGPUBuffer("rebind, second", name, second);
	stopProducer(&p);
	CFRelease(first);
	CFRelease(second);

	CGLSetCurrentContext(NULL);
}

// Two threads, each with its own context.
struct handoff {
	pthread_mutex_t lock;
	pthread_cond_t cond;
	int rendered, read;
	GLenum target;
	IOSurfaceRef surface;
};

static void *producerThread(void *arg) {
	struct handoff *h = arg;
	CGLContextObj context = createContext();
	struct producer p = { 0 };
	if (context != NULL && CGLSetCurrentContext(context) == kCGLNoError) {
		startProducer(&p, h->target, h->surface);
		render(RED);
		glFlush();
	}
	// The context stays current here, with the framebuffer bound, while the
	// other thread reads.
	pthread_mutex_lock(&h->lock);
	h->rendered = 1;
	pthread_cond_broadcast(&h->cond);
	while (!h->read)
		pthread_cond_wait(&h->cond, &h->lock);
	pthread_mutex_unlock(&h->lock);
	if (context != NULL) {
		stopProducer(&p);
		CGLSetCurrentContext(NULL);
		CGLReleaseContext(context);
	}
	return NULL;
}

static void *consumerThread(void *arg) {
	struct handoff *h = arg;
	pthread_mutex_lock(&h->lock);
	while (!h->rendered)
		pthread_cond_wait(&h->cond, &h->lock);
	pthread_mutex_unlock(&h->lock);
	CGLContextObj context = createContext();
	const char *name = targetName(h->target);
	if (context == NULL) {
		printf("FAIL: no consumer context\n");
		g_failures++;
	} else {
		expectPixel("threads: CPU on another thread sees the flushed rendering", name, cpuPixel(h->surface), RED);
		expectPixel("threads: a context on another thread sees it", name, consumerPixel(context, h->surface), RED);
		printGPUBuffer("threads", name, h->surface);
		CGLReleaseContext(context);
	}
	pthread_mutex_lock(&h->lock);
	h->read = 1;
	pthread_cond_broadcast(&h->cond);
	pthread_mutex_unlock(&h->lock);
	return NULL;
}

static void runThreadCase(GLenum target) {
	struct handoff h = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, target, createSurface() };
	pthread_t producer, consumer;
	pthread_create(&producer, NULL, producerThread, &h);
	pthread_create(&consumer, NULL, consumerThread, &h);
	pthread_join(consumer, NULL);
	pthread_join(producer, NULL);
	CFRelease(h.surface);
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
	CGLContextObj producer = NULL, consumer = NULL;
	if (CGLChoosePixelFormat(attrs, &g_pixelFormat, &screens) != kCGLNoError || g_pixelFormat == NULL
		|| (producer = createContext()) == NULL || (consumer = createContext()) == NULL) {
		printf("FAIL: no offscreen context\n");
		finish(1);
	}
	CGLSetCurrentContext(producer);
	printf("renderer: %s\n", (const char *) glGetString(GL_RENDERER));
	CGLSetCurrentContext(NULL);

	const GLenum targets[] = { GL_TEXTURE_RECTANGLE, GL_TEXTURE_2D };
	for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
		if (!targetSupported(producer, targets[i]))
			continue;
		printPath(producer, targets[i]);
		runSingleThreadCases(producer, consumer, targets[i]);
		runThreadCase(targets[i]);
	}

	CGLReleaseContext(producer);
	CGLReleaseContext(consumer);
	CGLReleasePixelFormat(g_pixelFormat);

	printf(g_failures == 0 ? "PASS\n" : "FAIL: %d\n", g_failures);
	finish(g_failures == 0 ? 0 : 1);
}
