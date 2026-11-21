// Copyright (C) 2026 Lubos Dolezel
//
// OpenGL rendering into IOSurfaces, as the CPU and other contexts see it,
// with timings. On a host whose GPU driver cannot render into the linear
// layout of a surface's memory, Darling gives a surface that GL renders into
// a GPU buffer in the driver's own layout and copies between the two on the
// GPU (IOSurfacePriv.h); elsewhere the texture uses the memory or a copy of
// it. The public-API checks hold on every path:
//  - flush:     rendering does not change the seed; glFlush does, and the
//               memory then holds the rendering, so a lock, also one with
//               kIOSurfaceLockAvoidSync, sees it and changes nothing;
//  - unflushed: a lock of rendering that has not been flushed sees it
//               (Darling waits for it at the lock); an AvoidSync lock either
//               succeeds or fails with kIOReturnCannotLock;
//  - upload:    where the surface has a GPU buffer, a CPU write under a
//               lock is what GL reads from the texture after the unlock,
//               without specifying the texture again (a texture that holds
//               a copy is specified again instead);
//  - consumer:  another context sees the rendering through a texture of its
//               own;
//  - fork:      a child process sees the flushed rendering in the memory.
// Where the surface has a GPU buffer, the copies are counted through the
// private statistics: one copy into the memory per flushed rendering, made
// at the flush or, where no other process reads the memory (as here), at
// the first lock after it, and none at later locks; one copy into the
// buffer per CPU write, and none for a consumer context. A 1920x1080 surface is timed (printed, not checked):
// glFlush after a clear (which includes the copy into the memory where there
// is a GPU buffer), a lock after that flush, and a lock of rendering that
// has not been flushed.
//
// Forcing the fallback without GPU buffers needs a switch Darling does not
// have; that case is skipped. Hosts without the libraries or with a driver
// that renders into the linear layout run the other paths.
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
#include <mach/mach_time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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
#define BIG_W 1920
#define BIG_H 1080
#define TIMED_RUNS 10

#define GREY 0xff808080u
#define RED 0xffff0000u
#define GREEN 0xff00ff00u
#define BLUE 0xff0000ffu
#define YELLOW 0xffffff00u

static int g_failures;
static CGLPixelFormatObj g_pixelFormat;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define EXPECT_PIXEL(actual, expected) do { uint32_t a_ = (actual), e_ = (expected); \
	if (a_ != e_) { printf("FAIL %d: %s is %08x, expected %08x\n", __LINE__, #actual, a_, e_); g_failures++; } } while (0)

static void onAlarm(int sig) {
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	_exit(1);
}

// Whether this is Darling, which has the private IOSurface interface.
static bool onDarling(void) {
	return dlsym(RTLD_DEFAULT, "_IOSurfaceGetDMABuf") != NULL;
}

struct copies {
	bool known, hasBuffer;
	uint64_t downloads, uploads;
};

// The surface's GPU buffer and its copies, where Darling reports them.
static struct copies copiesOf(IOSurfaceRef surface) {
	struct copies result = { false, false, 0, 0 };
#ifdef HAVE_PRIVATE_HEADER
	typedef bool (*statistics_proc)(IOSurfaceRef, _IOSurfaceGPUBufferStatistics *);
	statistics_proc statistics = (statistics_proc) dlsym(RTLD_DEFAULT, "_IOSurfaceGetGPUBufferStatistics");
	_IOSurfaceGPUBufferStatistics s = { .version = _IOSURFACE_GPU_BUFFER_VERSION };
	if (statistics && statistics(surface, &s)) {
		result.known = true;
		result.hasBuffer = s.hasBuffer;
		result.downloads = s.downloads;
		result.uploads = s.uploads;
	}
#endif
	return result;
}

static double nowMs(void) {
	static mach_timebase_info_data_t timebase;
	if (timebase.denom == 0)
		mach_timebase_info(&timebase);
	return (double) mach_absolute_time() * timebase.numer / timebase.denom / 1e6;
}

static int compareDoubles(const void *a, const void *b) {
	double x = *(const double *) a, y = *(const double *) b;
	return x < y ? -1 : x > y;
}

static double median(double *values, int count) {
	qsort(values, (size_t) count, sizeof(double), compareDoubles);
	return values[count / 2];
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
	size_t width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface);
	for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width; x++)
			*(uint32_t *) (base + y * bytesPerRow + x * 4) = value;
	IOSurfaceUnlock(surface, 0, NULL);
}

static IOSurfaceRef createSurface(int width, int height) {
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, width);
	setNumber(props, kIOSurfaceHeight, height);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	if (surface != NULL)
		fill(surface, GREY);
	return surface;
}

// The first and last pixel of the memory, without a lock: their common
// value, or 0xdeadbeef if they differ.
static uint32_t memoryPixel(IOSurfaceRef surface) {
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	size_t width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface);
	uint32_t first = *(uint32_t *) base;
	uint32_t last = *(uint32_t *) (base + (height - 1) * bytesPerRow + (width - 1) * 4);
	return first == last ? first : 0xdeadbeefu;
}

// The pixel under a lock with options; 0 if the lock fails, with its result
// in *result.
static uint32_t lockedPixel(IOSurfaceRef surface, IOSurfaceLockOptions options, kern_return_t *result) {
	kern_return_t locked = IOSurfaceLock(surface, options, NULL);
	if (result)
		*result = locked;
	if (locked != kIOReturnSuccess)
		return 0;
	uint32_t pixel = memoryPixel(surface);
	IOSurfaceUnlock(surface, options, NULL);
	return pixel;
}

static void writePixels(IOSurfaceRef surface, uint32_t value) {
	fill(surface, value);
}

static CGLContextObj createContext(void) {
	CGLContextObj context = NULL;
	if (CGLCreateContext(g_pixelFormat, NULL, &context) != kCGLNoError)
		return NULL;
	return context;
}

static const char *targetName(GLenum target) {
	return target == GL_TEXTURE_RECTANGLE ? "rectangle" : "2d";
}

struct producer {
	GLenum target;
	GLuint texture, framebuffer;
	int width, height;
};

// In the current context: a texture specified from surface and a
// framebuffer with it attached. Returns false if the target is not taken.
static bool startProducer(struct producer *p, GLenum target, IOSurfaceRef surface) {
	p->target = target;
	p->width = (int) IOSurfaceGetWidth(surface);
	p->height = (int) IOSurfaceGetHeight(surface);
	glGenTextures(1, &p->texture);
	glBindTexture(target, p->texture);
	CGLError error = CGLTexImageIOSurface2D(CGLGetCurrentContext(), target, GL_RGBA8, p->width, p->height,
		GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
	if (error != kCGLNoError) {
		glDeleteTextures(1, &p->texture);
		p->texture = 0;
		while (glGetError() != GL_NO_ERROR)
			;
		return false;
	}
	glGenFramebuffers(1, &p->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, p->texture, 0);
	CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glViewport(0, 0, p->width, p->height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

static void stopProducer(struct producer *p) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (p->framebuffer != 0)
		glDeleteFramebuffers(1, &p->framebuffer);
	if (p->texture != 0)
		glDeleteTextures(1, &p->texture);
	p->framebuffer = p->texture = 0;
}

// Renders a colour into the texture; with unbind, the framebuffer is
// unbound afterwards (Darling counts a texture still attached to the draw
// framebuffer as rendered into again at a later lock).
static void render(struct producer *p, uint32_t color, bool unbind) {
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glClearColor(((color >> 16) & 0xff) / 255.0f, ((color >> 8) & 0xff) / 255.0f, (color & 0xff) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	if (unbind)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// What GL reads from the texture: the common value of the first and last
// texel, or 0xdeadbeef.
static uint32_t texturePixel(struct producer *p) {
	uint32_t first = 0, last = 0;
	// The read framebuffer only: a texture attached to the draw framebuffer
	// counts as rendered into in Darling.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, p->framebuffer);
	glReadPixels(0, 0, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &first);
	glReadPixels(p->width - 1, p->height - 1, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &last);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	return first == last ? first : 0xdeadbeefu;
}

// The surface's contents as a rectangle texture of consumer shows them,
// read without a framebuffer (one would count as rendering in Darling).
static uint32_t consumerPixel(CGLContextObj consumer, IOSurfaceRef surface) {
	static uint32_t texels[W * H];
	CGLContextObj previous = CGLGetCurrentContext();
	CGLSetCurrentContext(consumer);
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, texture);
	uint32_t result = 0xdeadbeefu;
	if (CGLTexImageIOSurface2D(consumer, GL_TEXTURE_RECTANGLE, GL_RGBA8, W, H, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError) {
		glGetTexImage(GL_TEXTURE_RECTANGLE, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texels);
		result = texels[0] == texels[W * H - 1] ? texels[0] : 0xdeadbeefu;
	}
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	glDeleteTextures(1, &texture);
	CGLSetCurrentContext(previous);
	return result;
}

static void printCopies(const char *what, IOSurfaceRef surface) {
	struct copies c = copiesOf(surface);
	if (c.known)
		printf("%s: GPU buffer %s, downloads %llu, uploads %llu\n", what, c.hasBuffer ? "yes" : "no",
			(unsigned long long) c.downloads, (unsigned long long) c.uploads);
}

static void functionalCases(CGLContextObj producer, CGLContextObj consumer, GLenum target) {
	const char *name = targetName(target);
	CGLSetCurrentContext(producer);
	IOSurfaceRef s = createSurface(W, H);
	struct producer p;
	if (!startProducer(&p, target, s)) {
		printf("SKIP: CGLTexImageIOSurface2D does not take %s textures\n", name);
		CFRelease(s);
		CGLSetCurrentContext(NULL);
		return;
	}
	printf("== %s\n", name);

	// flush
	uint32_t seed = IOSurfaceGetSeed(s);
	render(&p, RED, true);
	CHECK(IOSurfaceGetSeed(s) == seed);
	glFlush();
	uint32_t flushed = IOSurfaceGetSeed(s);
	CHECK(flushed != seed);
	struct copies afterFlush = copiesOf(s);
	printCopies("after the first flush", s);
	kern_return_t result = 0;
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &result), RED);
	CHECK(result == kIOReturnSuccess);
	struct copies afterFirstLock = copiesOf(s);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), RED);
	CHECK(IOSurfaceGetSeed(s) == flushed);
	struct copies afterLocks = copiesOf(s);
	if (afterFlush.hasBuffer) {
		// The rendering was copied once, at the flush or at the first lock;
		// the second lock copies nothing.
		CHECK(afterFirstLock.downloads >= 1 && afterFirstLock.downloads <= afterFlush.downloads + 1);
		CHECK(afterLocks.downloads == afterFirstLock.downloads);
		CHECK(afterLocks.uploads == afterFlush.uploads);
	}

	// Each further flush of new rendering is one copy into the memory.
	render(&p, YELLOW, true);
	glFlush();
	CHECK(IOSurfaceGetSeed(s) != flushed);
	flushed = IOSurfaceGetSeed(s);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL), YELLOW);
	struct copies afterSecond = copiesOf(s);
	if (afterLocks.hasBuffer)
		CHECK(afterSecond.downloads == afterLocks.downloads + 1);

	// unflushed: AvoidSync copies nothing; a plain lock waits and sees it.
	render(&p, GREEN, false);
	struct copies beforeAvoid = copiesOf(s);
	uint32_t pixel = lockedPixel(s, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &result);
	printf("AvoidSync lock of unflushed rendering: 0x%x, pixel %08x\n", result, pixel);
	CHECK(result == kIOReturnSuccess || result == kIOReturnCannotLock);
	if (beforeAvoid.hasBuffer)
		CHECK(copiesOf(s).downloads == beforeAvoid.downloads);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), GREEN);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFlush();

	// upload
	struct copies beforeWrite = copiesOf(s);
	writePixels(s, BLUE);
	struct copies afterWrite = copiesOf(s);
	// A texture that holds a copy of the surface (rectangle textures on
	// macOS; textures Darling cannot give the memory or a GPU buffer) is
	// not expected to see it.
	bool kept = afterWrite.hasBuffer || (target == GL_TEXTURE_RECTANGLE && !onDarling());
	if (target == GL_TEXTURE_RECTANGLE)
		CGLSetCurrentContext(producer);
	pixel = texturePixel(&p);
	printf("GL reads %08x from the texture after a CPU write of %08x\n", pixel, BLUE);
	if (kept)
		EXPECT_PIXEL(pixel, BLUE);
	else {
		// The copy is brought up to date by specifying it again, so that
		// the rendering below is not of an older copy.
		printf("SKIP: the %s texture holds a copy here\n", name);
		glBindTexture(target, p.texture);
		CHECK(CGLTexImageIOSurface2D(producer, target, GL_RGBA8, p.width, p.height, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV, s, 0) == kCGLNoError);
		glBindTexture(target, 0);
	}
	if (beforeWrite.hasBuffer)
		CHECK(afterWrite.uploads == beforeWrite.uploads + 1);

	// consumer
	render(&p, RED, true);
	glFlush();
	struct copies beforeConsumer = copiesOf(s);
	EXPECT_PIXEL(consumerPixel(consumer, s), RED);
	struct copies afterConsumer = copiesOf(s);
	if (beforeConsumer.hasBuffer) {
		// The consumer uses the same GPU buffer: nothing is copied.
		CHECK(afterConsumer.uploads == beforeConsumer.uploads);
		CHECK(afterConsumer.downloads == beforeConsumer.downloads);
	}
	printCopies("at the end", s);

	// fork: the child sees the flushed rendering in the memory and has no
	// GPU buffer of its own. It makes no GL calls.
	CGLSetCurrentContext(NULL);
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0) {
		int status = 0;
		if (memoryPixel(s) != RED)
			status |= 1;
		struct copies child = copiesOf(s);
		if (child.hasBuffer)
			status |= 2;
		IOSurfaceRef other = createSurface(8, 8);
		if (other == NULL || lockedPixel(other, kIOSurfaceLockReadOnly, NULL) != GREY)
			status |= 4;
		_exit(status);
	}
	int status = -1;
	CHECK(pid > 0 && waitpid(pid, &status, 0) == pid);
	printf("fork: child status %d\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);

	CGLSetCurrentContext(producer);
	stopProducer(&p);
	CFRelease(s);
	CGLSetCurrentContext(NULL);
}

static void timedCase(CGLContextObj producer, GLenum target) {
	CGLSetCurrentContext(producer);
	IOSurfaceRef s = createSurface(BIG_W, BIG_H);
	struct producer p;
	if (s == NULL || !startProducer(&p, target, s)) {
		if (s)
			CFRelease(s);
		CGLSetCurrentContext(NULL);
		return;
	}
	double flush[TIMED_RUNS], lockFlushed[TIMED_RUNS], lockUnflushed[TIMED_RUNS];
	static const uint32_t colors[] = { RED, GREEN, BLUE, YELLOW };
	for (int i = 0; i < TIMED_RUNS; i++) {
		render(&p, colors[i % 4], true);
		double t0 = nowMs();
		glFlush();
		double t1 = nowMs();
		uint32_t pixel = lockedPixel(s, kIOSurfaceLockReadOnly, NULL);
		double t2 = nowMs();
		EXPECT_PIXEL(pixel, colors[i % 4]);
		flush[i] = t1 - t0;
		lockFlushed[i] = t2 - t1;

		render(&p, colors[(i + 1) % 4], false);
		t0 = nowMs();
		pixel = lockedPixel(s, kIOSurfaceLockReadOnly, NULL);
		t1 = nowMs();
		EXPECT_PIXEL(pixel, colors[(i + 1) % 4]);
		lockUnflushed[i] = t1 - t0;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glFlush();
	}
	struct copies c = copiesOf(s);
	printf("%dx%d %s, medians of %d: glFlush after a clear %.2f ms, lock after the flush %.2f ms, "
		"lock of unflushed rendering %.2f ms%s\n", BIG_W, BIG_H, targetName(target), TIMED_RUNS,
		median(flush, TIMED_RUNS), median(lockFlushed, TIMED_RUNS), median(lockUnflushed, TIMED_RUNS),
		c.hasBuffer ? " (GPU buffer)" : "");
	stopProducer(&p);
	CFRelease(s);
	CGLSetCurrentContext(NULL);
}

int main(int argc, char **argv) {
	signal(SIGALRM, onAlarm);
	alarm(50);
	setvbuf(stdout, NULL, _IOLBF, 0);

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
		return 1;
	}
	CGLSetCurrentContext(producer);
	printf("renderer: %s\n", (const char *) glGetString(GL_RENDERER));
	CGLSetCurrentContext(NULL);
	printf("on Darling: %s\n", onDarling() ? "yes" : "no");
	printf("SKIP: fallback without GPU buffers (no switch to force it)\n");

	const GLenum targets[] = { GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE };
	for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++)
		functionalCases(producer, consumer, targets[i]);
	for (size_t i = 0; i < sizeof(targets) / sizeof(targets[0]); i++)
		timedCase(producer, targets[i]);

	CGLReleaseContext(producer);
	CGLReleaseContext(consumer);
	CGLReleasePixelFormat(g_pixelFormat);
	printf(g_failures == 0 ? "PASS\n" : "FAIL: %d\n", g_failures);
	fflush(stdout);
	_exit(g_failures == 0 ? 0 : 1);
}
