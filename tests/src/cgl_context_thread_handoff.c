// Copyright (C) 2026 Lubos Dolezel
//
// The common macOS pattern for a context shared by two threads: the main
// thread makes the context current at set-up and then only runs its run
// loop (it never locks, unlocks or makes the context current again), while
// a render thread locks the context with CGLLockContext, makes it current,
// draws, flushes and unlocks, and keeps issuing GL calls after unlocking.
// The render thread must get the context on its first attempt, within a
// bounded time, on every frame. Also checked: nested locks, errors for a
// NULL context, and that a context used by a single thread keeps its
// binding after CGLUnlockContext. The main thread may also be busy with GL
// calls for longer than a short grace period before it returns to its run
// loop, or take the context's lock itself while the render thread waits. Needs an offscreen OpenGL context (a host
// EGL driver).
#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static CGLContextObj g_context;
static volatile int g_done;

// One frame of a render thread: lock, make current, draw, flush, unlock,
// then more GL work outside the lock.
static void render_frame(int frame)
{
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CGLError error = CGLSetCurrentContext(g_context);
	if (error != kCGLNoError)
		printf("frame %d: CGLSetCurrentContext failed with %d\n", frame, (int) error);
	CHECK(error == kCGLNoError);
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(glGetString(GL_VERSION) != NULL);
	CHECK(CGLFlushDrawable(g_context) == kCGLNoError);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	// GL after the unlock must still work on this thread.
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(glGetString(GL_VERSION) != NULL);
}

static void* render_thread(void* arg)
{
	(void) arg;
	for (int frame = 0; frame < 20; frame++) {
		render_frame(frame);
		usleep(2000);
	}
	g_done = 1;
	CFRunLoopStop(CFRunLoopGetMain());
	return NULL;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(60);

	CGLPixelFormatAttribute attrs[] = { 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
	CHECK(CGLLockContext(NULL) == kCGLBadContext);
	CHECK(CGLUnlockContext(NULL) == kCGLBadContext);

	// Main thread: set-up, then (1) only the run loop, (2) a long stretch of
	// GL work before the run loop, (3) a CGLLockContext while the render
	// thread already holds the lock and waits for the main thread.
	for (int mode = 0; mode < 3; mode++) {
		g_done = 0;
		CHECK(CGLCreateContext(format, NULL, &g_context) == kCGLNoError);
		if (!g_context) {
			printf("FAIL: no context\n");
			return 1;
		}
		CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);
		CHECK(glGetString(GL_VERSION) != NULL);

		pthread_t thread;
		pthread_create(&thread, NULL, render_thread, NULL);
		if (mode == 1) {
			// Longer than any short grace period: busy with GL calls, not
			// back in the run loop yet.
			for (int i = 0; i < 60; i++) {
				CHECK(glGetString(GL_VERSION) != NULL);
				usleep(10000);
			}
		} else if (mode == 2) {
			usleep(100000);
			CHECK(CGLLockContext(g_context) == kCGLNoError);
			CHECK(CGLUnlockContext(g_context) == kCGLNoError);
		}
		while (!g_done)
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.05, false);
		pthread_join(thread, NULL);

		// Nested locks on one thread.
		CHECK(CGLLockContext(g_context) == kCGLNoError);
		CHECK(CGLLockContext(g_context) == kCGLNoError);
		CHECK(CGLUnlockContext(g_context) == kCGLNoError);
		CHECK(CGLUnlockContext(g_context) == kCGLNoError);
		CGLSetCurrentContext(NULL);
		CGLDestroyContext(g_context);
	}

	// A context used by a single thread keeps its binding after unlocking.
	CGLContextObj single = NULL;
	CHECK(CGLCreateContext(format, NULL, &single) == kCGLNoError);
	CHECK(CGLLockContext(single) == kCGLNoError);
	CHECK(CGLSetCurrentContext(single) == kCGLNoError);
	CHECK(CGLUnlockContext(single) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == single);
	CHECK(glGetString(GL_VERSION) != NULL);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(single);

	CGLDestroyPixelFormat(format);
	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
