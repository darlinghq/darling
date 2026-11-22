// Copyright (C) 2026 Lubos Dolezel
//
// A display link thread and the main thread share a context and an application
// lock, and both take the application lock first and the context's lock second.
// The render thread, every frame: lock the application, CGLLockContext, make the
// context current, draw, flush, unlock both. The main thread, between run loop
// passes (a view reshape): lock the application, CGLLockContext, use GL (the
// context is its current context), unlock both. With an OpenGL where a thread
// can only use a context after being handed it by the thread holding it, the
// main thread waits for the render thread, which is waiting for the
// application lock the main thread holds, until a timeout. Every GL use must
// get the context without a wait of seconds, and the whole run must be fast.
// Needs an offscreen OpenGL context (a host EGL driver; on macOS without a GPU
// session build with -DTEST_GENERIC_FLOAT_RENDERER).
#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static double now_seconds(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

static CGLContextObj g_context;
static pthread_mutex_t g_app_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_done;
static volatile double g_worst_wait;

static void note_wait(double start)
{
	double waited = now_seconds() - start;
	if (waited > g_worst_wait)
		g_worst_wait = waited;
}

static void* render_thread(void* arg)
{
	(void) arg;
	for (int frame = 0; frame < 200; frame++) {
		pthread_mutex_lock(&g_app_lock);
		CHECK(CGLLockContext(g_context) == kCGLNoError);
		double start = now_seconds();
		CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);
		CHECK(glGetString(GL_VERSION) != NULL);
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		note_wait(start);
		CHECK(CGLFlushDrawable(g_context) == kCGLNoError);
		CHECK(CGLUnlockContext(g_context) == kCGLNoError);
		pthread_mutex_unlock(&g_app_lock);
		// GL calls after the unlock still work on this thread.
		CHECK(glGetString(GL_VERSION) != NULL);
		usleep(1000);
	}
	g_done = 1;
	return NULL;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(120);

#ifdef TEST_GENERIC_FLOAT_RENDERER
	CGLPixelFormatAttribute attrs[] = { kCGLPFARendererID, (CGLPixelFormatAttribute) 0x00020400, 0 };
#else
	CGLPixelFormatAttribute attrs[] = { 0 };
#endif
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
	CHECK(CGLCreateContext(format, NULL, &g_context) == kCGLNoError);
	if (!g_context) {
		printf("FAIL: no context\n");
		return 1;
	}
	CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);

	// The main thread is back in its run loop after the set-up.
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);

	double start = now_seconds();
	pthread_t thread;
	pthread_create(&thread, NULL, render_thread, NULL);
	int passes = 0;
	while (!g_done) {
		pthread_mutex_lock(&g_app_lock);
		CHECK(CGLLockContext(g_context) == kCGLNoError);
		double use = now_seconds();
		CHECK(CGLGetCurrentContext() == g_context);
		CHECK(glGetString(GL_VERSION) != NULL);
		GLint size = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
		CHECK(size >= 1024);
		note_wait(use);
		CHECK(CGLUnlockContext(g_context) == kCGLNoError);
		pthread_mutex_unlock(&g_app_lock);
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.002, false);
		passes++;
	}
	pthread_join(thread, NULL);
	double total = now_seconds() - start;
	printf("200 frames, %d main passes in %.2f s, worst wait for the context %.3f s\n", passes, total, g_worst_wait);
	CHECK(g_worst_wait < 3.0);
	CHECK(total < 30.0);

	CGLSetCurrentContext(NULL);
	CGLDestroyContext(g_context);
	CGLDestroyPixelFormat(format);
	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
