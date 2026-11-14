// Copyright (C) 2026 Lubos Dolezel
//
// A CGL context made current on one thread and then on another, the threads
// taking turns under CGLLockContext (the pattern of an application whose
// main thread and render thread share one context). The second thread must
// be able to make the context current once the first has unlocked it, GL
// calls must work on both, and a context used by one thread only keeps its
// binding after CGLUnlockContext. Needs an offscreen OpenGL context (a host
// EGL driver).
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

static pthread_mutex_t g_turnLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_turnCond = PTHREAD_COND_INITIALIZER;
static int g_step;

static void signal_step(int step)
{
	pthread_mutex_lock(&g_turnLock);
	g_step = step;
	pthread_cond_broadcast(&g_turnCond);
	pthread_mutex_unlock(&g_turnLock);
}

static void wait_step(int step)
{
	pthread_mutex_lock(&g_turnLock);
	while (g_step < step)
		pthread_cond_wait(&g_turnCond, &g_turnLock);
	pthread_mutex_unlock(&g_turnLock);
}

static CGLContextObj g_context;

static void* other_thread(void* arg)
{
	(void) arg;
	// 1: first user of the context
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);
	CHECK(glGetString(GL_VERSION) != NULL);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	signal_step(1);

	// 2: the main thread was refused; the next unlock releases the context
	wait_step(2);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	signal_step(3);

	// 4: the context is current on this thread again after locking
	wait_step(4);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(glGetString(GL_VERSION) != NULL);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	signal_step(5);
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
	CHECK(CGLCreateContext(format, NULL, &g_context) == kCGLNoError);
	if (!g_context) {
		printf("FAIL: no context\n");
		return 1;
	}

	CHECK(CGLLockContext(NULL) == kCGLBadContext);
	CHECK(CGLUnlockContext(NULL) == kCGLBadContext);

	pthread_t thread;
	pthread_create(&thread, NULL, other_thread, NULL);

	// The other thread has the context current and has not released it.
	// macOS lets this thread have it too; the framework here may refuse
	// once (EGL binds a context to one thread), which also tells it that
	// the context is shared between threads.
	wait_step(1);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CGLError first = CGLSetCurrentContext(g_context);
	CHECK(first == kCGLNoError || first == kCGLBadContext);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	signal_step(2);

	// 3: now the context can move here
	wait_step(3);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLSetCurrentContext(g_context) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(glGetString(GL_VERSION) != NULL);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	signal_step(4);

	// 5: nested locks release the context only at the outermost unlock
	wait_step(5);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLLockContext(g_context) == kCGLNoError);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == g_context);
	CHECK(glGetString(GL_VERSION) != NULL);
	CHECK(CGLUnlockContext(g_context) == kCGLNoError);
	pthread_join(thread, NULL);

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

	CGLDestroyContext(g_context);
	CGLDestroyPixelFormat(format);
	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
