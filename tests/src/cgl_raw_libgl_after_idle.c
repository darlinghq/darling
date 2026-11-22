// Copyright (C) 2026 Lubos Dolezel
//
// An application may load libGL.dylib from OpenGL.framework's Libraries directory itself and look the GL
// functions up there, instead of using the framework's own gl* exports. Its calls then reach the driver
// without the framework's help, so a context that is current on the main thread and used by that thread
// only must stay usable while the main run loop idles: the framework must not give its binding up.
// Needs an offscreen OpenGL context (a host EGL driver).
#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
int main(int c, char **v) {
	CGLContextObj ctx; CGLPixelFormatAttribute at[] = { 0 }; CGLPixelFormatObj pf; GLint n;
	CGLChoosePixelFormat(at, &pf, &n); CGLCreateContext(pf, NULL, &ctx); CGLSetCurrentContext(ctx);
	void *lib = dlopen("/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib", RTLD_LAZY | RTLD_LOCAL);
	const GLubyte *(*getString)(GLenum) = lib ? dlsym(lib, "glGetString") : NULL;
	int bad = 0;
	for (int i = 0; i < 3; i++) {
		const char *s = getString ? (const char *) getString(GL_VERSION) : NULL;
		printf("before idle %d: %s\n", i, s ? "version" : "NULL"); if (!s) bad++;
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.02, false);
		s = getString ? (const char *) getString(GL_VERSION) : NULL;
		printf("after idle %d: %s\n", i, s ? "version" : "NULL"); if (!s) bad++;
		(void) glGetError();  /* the framework's own export */
	}
	printf(bad ? "FAIL\n" : "PASS\n");
	return bad != 0;
}
