// Copyright (C) 2026 Lubos Dolezel
//
// What a GPU process does with CGL: a context is created and made current on
// a thread that is not the main thread (in the "shared" mode the main thread
// made it current first, as set-up code does, so the context is one that
// several threads use), an IOSurface-backed texture is specified with
// CGLTexImageIOSurface2D, rendered into through a framebuffer and flushed,
// and the context is asked whether it was lost. Nothing is locked with
// CGLLockContext. After every step glGetError must be GL_NO_ERROR, the
// graphics reset status (when the driver has the extension) must be
// GL_NO_ERROR (no reset), the framebuffer complete and the surface must hold
// what was drawn; the main run loop idles between the frames, and the context
// must still be the thread's current context afterwards.
//
// Modes: "shared" (context made current on the main thread, then used on a
// worker), "single" (created and used on the worker only), "unlocked" (shared,
// and the worker takes and gives up CGLLockContext at the start of each frame,
// so that it works on the context outside the lock, as the thread of a context
// that other threads use does), "main" (everything on the main thread, which
// runs its run loop between the frames).
// Needs an offscreen OpenGL context (a host EGL driver).
#define GL_GLEXT_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define W 32
#define H 16
#define FRAMES 25

static int g_failures;
static volatile int g_done;
static volatile int g_ready;
static volatile int g_contact;
static int g_shared;
static int g_unlock;
static int g_main;
static CGLContextObj g_context;
static CGLPixelFormatObj g_format;

static int g_frame;
static const char *g_step = "";

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s (frame %d, step %s)\n", __LINE__, #cond, g_frame, g_step); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

typedef GLenum (*reset_status_fn)(void);
// The entry points an application may use for the reset status: the ARB name
// macOS has, and the KHR and EXT names a driver offers (applications look the
// name up for the extension the driver lists).
static const char *const g_reset_names[] = {
	"glGetGraphicsResetStatusARB", "glGetGraphicsResetStatusKHR", "glGetGraphicsResetStatusEXT",
	"glGetGraphicsResetStatus",
};
#define RESET_NAMES (sizeof(g_reset_names) / sizeof(g_reset_names[0]))
static reset_status_fn g_reset_status[RESET_NAMES];

// glGetError and the reset status after a step; the status is the one the
// context has when nothing was lost.
static void state(const char *step)
{
	g_step = step;
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		printf("frame %d after %s: glGetError 0x%x\n", g_frame, step, error);
	CHECK(error == GL_NO_ERROR);
	for (unsigned i = 0; i < RESET_NAMES; i++) {
		if (!g_reset_status[i])
			continue;
		GLenum status = g_reset_status[i]();
		if (status != 0)
			printf("frame %d after %s: %s returned 0x%x\n", g_frame, step, g_reset_names[i], status);
		CHECK(status == 0);
	}
	error = glGetError();
	CHECK(error == GL_NO_ERROR);
	CHECK(CGLGetCurrentContext() == g_context);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static IOSurfaceRef createSurface(void)
{
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	return surface;
}

static void frame(int index)
{
	g_frame = index;
	GLenum target = (index & 1) ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE_ARB;

	g_step = "make current";
	if (g_unlock)
		CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);
	if (g_unlock) {
		CHECK(glGetString(GL_VERSION) != NULL);
		CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	}
	CHECK(CGLGetCurrentContext() == g_context);
	state("make current");
	CHECK(glGetString(GL_VERSION) != NULL);

	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (surface == NULL)
		return;
	IOSurfaceLock(surface, 0, NULL);
	memset(IOSurfaceGetBaseAddress(surface), 0x40, IOSurfaceGetBytesPerRow(surface) * H);
	IOSurfaceUnlock(surface, 0, NULL);

	GLuint texture = 0, framebuffer = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	state("bind texture");
	CHECK(glIsTexture(texture));

	g_step = "CGLTexImageIOSurface2D";
	CHECK(CGLTexImageIOSurface2D(g_context, target, GL_RGBA8, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		surface, 0) == kCGLNoError);
	state("CGLTexImageIOSurface2D");
	GLint width = 0;
	glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &width);
	CHECK(width == W);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
	state("framebuffer");
	CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	GLint bound = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound);
	CHECK((GLuint) bound == framebuffer);

	glViewport(0, 0, W, H);
	float red = (index % 3) == 0 ? 1.0f : 0.0f, green = (index % 3) == 1 ? 1.0f : 0.0f, blue = 1.0f - red - green;
	glClearColor(red, green, blue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	state("draw");
	glFlush();
	state("glFlush");

	// What was drawn is in the surface and readable from the framebuffer.
	unsigned char pixel[4] = { 0, 0, 0, 0 };
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	state("glReadPixels");
	CHECK(pixel[0] == (red != 0 ? 255 : 0) && pixel[1] == (green != 0 ? 255 : 0) && pixel[2] == (blue != 0 ? 255 : 0));
	IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL);
	const unsigned char *base = IOSurfaceGetBaseAddress(surface);
	g_step = "surface contents";
	CHECK(base[0] == (blue != 0 ? 255 : 0) && base[1] == (green != 0 ? 255 : 0) && base[2] == (red != 0 ? 255 : 0));
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
	state("lock surface");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	state("delete");
	CFRelease(surface);

	// Entry points a driver offers under the extension's name only, and the
	// context parameter calls, work on the context that is current here.
	g_step = "extension names";
	GLuint query = 0;
	void (*genQueries)(GLsizei, GLuint *) = (void (*)(GLsizei, GLuint *)) dlsym(RTLD_DEFAULT, "glGenQueriesEXT");
	if (genQueries) {
		genQueries(1, &query);
		CHECK(query != 0);
		glDeleteQueries(1, &query);
	}
	// A host name with no declared function behind it (OpenGL ES's fixed-point
	// glClearDepthx) must work on this context as well, with arguments of the
	// kinds the call has.
	void (*clearDepthx)(GLint) = (void (*)(GLint)) dlsym(RTLD_DEFAULT, "glClearDepthx");
	if (clearDepthx) {
		glClearDepth(1.0);
		clearDepthx(0x8000);
		GLfloat depth = 0;
		glGetFloatv(GL_DEPTH_CLEAR_VALUE, &depth);
		CHECK(depth == 0.5f);
	}
	GLint interval = 0;
	CHECK(CGLSetParameter(g_context, kCGLCPSwapInterval, &interval) == kCGLNoError);
	GLint value = 7;
	CHECK(CGLGetParameter(g_context, kCGLCPSwapInterval, &value) == kCGLNoError && value == 0);
	state("extension names");
}

static void *worker(void *arg)
{
	(void) arg;
	if (!g_shared) {
		GLint screens = 0;
		CGLPixelFormatAttribute attrs[] = { 0 };
		if (CGLChoosePixelFormat(attrs, &g_format, &screens) != kCGLNoError
			|| CGLCreateContext(g_format, NULL, &g_context) != kCGLNoError) {
			printf("FAIL: no offscreen context\n");
			g_failures++;
			g_done = 1;
			return NULL;
		}
	}
	g_ready = 1;
	if (g_unlock) {
		// First contact: locking the context while the main thread has it
		// current tells the framework that another thread uses it.
		CGLLockContext(g_context);
		CGLUnlockContext(g_context);
	}
	g_contact = 1;
	CGLSetCurrentContext(g_context);
	printf("renderer: %s\n", (const char *) glGetString(GL_RENDERER));
	for (int i = 0; i < FRAMES; i++) {
		frame(i);
		// The main thread's run loop goes idle here.
		usleep(15000);
		g_step = "after idle";
		CHECK(CGLGetCurrentContext() == g_context);
		state("after idle");
	}
	CGLSetCurrentContext(NULL);
	g_done = 1;
	CFRunLoopStop(CFRunLoopGetMain());
	return NULL;
}

int main(int argc, char **argv)
{
	signal(SIGALRM, on_alarm);
	alarm(90);
	g_shared = argc > 1 && strcmp(argv[1], "single") != 0;
	g_unlock = argc > 1 && strcmp(argv[1], "unlocked") == 0;
	g_main = argc > 1 && strcmp(argv[1], "main") == 0;

	if (g_shared) {
		GLint screens = 0;
		CGLPixelFormatAttribute attrs[] = { 0 };
		if (CGLChoosePixelFormat(attrs, &g_format, &screens) != kCGLNoError
			|| CGLCreateContext(g_format, NULL, &g_context) != kCGLNoError
			|| CGLSetCurrentContext(g_context) != kCGLNoError) {
			printf("FAIL: no offscreen context\n");
			return 1;
		}
	}
	printf("mode %s\n", argc > 1 ? argv[1] : "shared");
	for (unsigned i = 0; i < RESET_NAMES; i++) {
		g_reset_status[i] = (reset_status_fn) dlsym(RTLD_DEFAULT, g_reset_names[i]);
		printf("%s: %s\n", g_reset_names[i], g_reset_status[i] ? "found" : "not found");
	}

	if (g_main) {
		for (int i = 0; i < FRAMES; i++) {
			frame(i);
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.015, false);
			g_step = "after idle";
			CHECK(CGLGetCurrentContext() == g_context);
			state("after idle");
		}
	} else {
		pthread_t thread;
		pthread_create(&thread, NULL, worker, NULL);
		// The main thread keeps the context bound until the worker has made contact.
		while (g_shared && !g_contact)
			usleep(1000);
		while (!g_done)
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
		pthread_join(thread, NULL);
	}

	if (g_failures == 0)
		printf("PASS\n");
	fflush(stdout);
	_exit(g_failures == 0 ? 0 : 1);
}
