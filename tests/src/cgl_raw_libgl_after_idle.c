// Copyright (C) 2026 Lubos Dolezel
//
// An application may load libGL.dylib from OpenGL.framework's Libraries directory itself and look the GL
// functions up there, instead of using the framework's own gl* exports. Its calls then reach the driver
// without the framework's help, so a context that is current on the main thread and used by that thread
// only must stay usable while the main run loop idles: the framework must not give its binding up.
// The library re-exports the framework, so the same lookup gives the framework's own entry points,
// which also hold the binding of a context that several threads share: with two threads using the
// context, a call through the library outside CGLLockContext, right after the other thread used it,
// must work on either thread.
// Needs an offscreen OpenGL context (a host EGL driver).
#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
static const char *const kLibGL = "/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib";

static CGLContextObj g_ctx;
static const GLubyte *(*g_getString)(GLenum);
static atomic_int g_phase;     // the step the two threads have reached: 1 helper used the context, 2 main did, 3 done
static atomic_int g_helper_bad;

static void spin_main(int phase) {
	// the main run loop runs so that the helper thread can have the context handed to it
	while (atomic_load(&g_phase) < phase)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
}

static void *helper(void *arg) {
	(void) arg;
	// phase 1: the helper uses the shared context inside the lock, then leaves it
	CGLSetCurrentContext(g_ctx);
	CGLLockContext(g_ctx);
	const char *s = (const char *) glGetString(GL_VERSION);
	CGLUnlockContext(g_ctx);
	printf("helper in lock: %s\n", s ? "version" : "NULL");
	if (!s) atomic_fetch_add(&g_helper_bad, 1);
	atomic_store(&g_phase, 1);
	// phase 2 (the main thread used the context in its own turn): a raw call outside the lock
	while (atomic_load(&g_phase) < 2)
		usleep(1000);
	s = g_getString ? (const char *) g_getString(GL_VERSION) : NULL;
	printf("helper outside lock through libGL: %s\n", s ? "version" : "NULL");
	if (!s) atomic_fetch_add(&g_helper_bad, 1);
	atomic_store(&g_phase, 3);
	return NULL;
}

int main(int c, char **v) {
	CGLPixelFormatAttribute at[] = { 0 }; CGLPixelFormatObj pf; GLint n;
	CGLChoosePixelFormat(at, &pf, &n); CGLCreateContext(pf, NULL, &g_ctx); CGLSetCurrentContext(g_ctx);
	void *lib = dlopen(kLibGL, RTLD_LAZY | RTLD_LOCAL);
	const GLubyte *(*getString)(GLenum) = lib ? dlsym(lib, "glGetString") : NULL;
	g_getString = getString;
	int bad = 0;
	for (int i = 0; i < 3; i++) {
		const char *s = getString ? (const char *) getString(GL_VERSION) : NULL;
		printf("before idle %d: %s\n", i, s ? "version" : "NULL"); if (!s) bad++;
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.02, false);
		s = getString ? (const char *) getString(GL_VERSION) : NULL;
		printf("after idle %d: %s\n", i, s ? "version" : "NULL"); if (!s) bad++;
		(void) glGetError();  /* the framework's own export */
	}

	// The context is now shared: a second thread uses it, and then each thread calls through the
	// library outside the lock, right after the other one used the context.
	pthread_t t;
	pthread_create(&t, NULL, helper, NULL);
	spin_main(1);
	const char *s = getString ? (const char *) getString(GL_VERSION) : NULL;
	printf("main outside lock through libGL after the helper: %s\n", s ? "version" : "NULL"); if (!s) bad++;
	atomic_store(&g_phase, 2);
	while (atomic_load(&g_phase) < 3)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
	pthread_join(t, NULL);
	bad += atomic_load(&g_helper_bad);
	// and the main thread's own call after that
	s = getString ? (const char *) getString(GL_VERSION) : NULL;
	printf("main outside lock through libGL after its own idle: %s\n", s ? "version" : "NULL"); if (!s) bad++;

	printf(bad ? "FAIL\n" : "PASS\n");
	return bad != 0;
}
