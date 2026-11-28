// Copyright (C) 2026 Lubos Dolezel
//
// CGL (CGL Reference, 2007-06-28): the global options of CGLSetOption and CGLGetOption (the cache size is a positive
// integer that starts at 5, the options that clear the cache and reset the library read as false, an option that does
// not exist is kCGLBadEnumeration), the string of CGLErrorString for a code that is not a result code, and the virtual
// screen of CGLGetVirtualScreen, which is -1 when the call fails. Needs an OpenGL context (a host EGL driver).
#include <OpenGL/OpenGL.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	GLint value = -1;
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, &value), kCGLNoError);
	CHECK_EQ(value, 5);
	CHECK_EQ(CGLSetOption(kCGLGOFormatCacheSize, 9), kCGLNoError);
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, &value), kCGLNoError);
	CHECK_EQ(value, 9);
	// a cache size is positive
	CHECK_EQ(CGLSetOption(kCGLGOFormatCacheSize, 0), kCGLBadValue);
	CHECK_EQ(CGLSetOption(kCGLGOFormatCacheSize, -3), kCGLBadValue);
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, &value), kCGLNoError);
	CHECK_EQ(value, 9);
	// the options that always read false
	CHECK_EQ(CGLSetOption(kCGLGOClearFormatCache, 1), kCGLNoError);
	CHECK_EQ(CGLGetOption(kCGLGOClearFormatCache, &value), kCGLNoError);
	CHECK_EQ(value, 0);
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, &value), kCGLNoError);
	CHECK_EQ(value, 9);
	CHECK_EQ(CGLSetOption(kCGLGORetainRenderers, 1), kCGLNoError);
	CHECK_EQ(CGLGetOption(kCGLGORetainRenderers, &value), kCGLNoError);
	CHECK_EQ(value, 1);
	CHECK_EQ(CGLSetOption(kCGLGOUseErrorHandler, 1), kCGLNoError);
	CHECK_EQ(CGLGetOption(kCGLGOUseErrorHandler, &value), kCGLNoError);
	CHECK_EQ(value, 1);
	// resetting puts the options back to the values they started with
	CHECK_EQ(CGLSetOption(kCGLGOResetLibrary, 1), kCGLNoError);
	CHECK_EQ(CGLGetOption(kCGLGOResetLibrary, &value), kCGLNoError);
	CHECK_EQ(value, 0);
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, &value), kCGLNoError);
	CHECK_EQ(value, 5);
	CHECK_EQ(CGLGetOption(kCGLGORetainRenderers, &value), kCGLNoError);
	CHECK_EQ(value, 0);
	CHECK_EQ(CGLGetOption(kCGLGOUseErrorHandler, &value), kCGLNoError);
	CHECK_EQ(value, 0);
	// bad arguments
	CHECK_EQ(CGLSetOption((CGLGlobalOption) 31337, 1), kCGLBadEnumeration);
	CHECK_EQ(CGLGetOption((CGLGlobalOption) 31337, &value), kCGLBadEnumeration);
	CHECK_EQ(CGLGetOption(kCGLGOFormatCacheSize, NULL), kCGLBadAddress);

	// the strings of the result codes are there, and differ; any other code has the one documented string
	for (int code = kCGLBadAttribute; code <= kCGLBadConnection; code++) {
		CHECK(strlen(CGLErrorString((CGLError) code)) > 0);
		CHECK(strcmp(CGLErrorString((CGLError) code), "No such error code.") != 0);
	}
	CHECK(strcmp(CGLErrorString(kCGLNoError), CGLErrorString(kCGLBadContext)) != 0);
	CHECK(strcmp(CGLErrorString((CGLError) 31337), "No such error code.") == 0);
	CHECK(strcmp(CGLErrorString((CGLError) -1), "No such error code.") == 0);

	// the virtual screen is 0, and -1 when the call fails
	CGLPixelFormatAttribute attributes[] = { kCGLPFAColorSize, 24, 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CGLContextObj ctx = NULL;
	CHECK_EQ(CGLChoosePixelFormat(attributes, &format, &screens), kCGLNoError);
	CHECK_EQ(CGLCreateContext(format, NULL, &ctx), kCGLNoError);
	GLint screen = 77;
	CHECK_EQ(CGLGetVirtualScreen(ctx, &screen), kCGLNoError);
	CHECK_EQ(screen, 0);
	screen = 77;
	CHECK_EQ(CGLGetVirtualScreen(NULL, &screen), kCGLBadContext);
	CHECK_EQ(screen, -1);
	CHECK_EQ(CGLGetVirtualScreen(ctx, NULL), kCGLBadAddress);
	CGLDestroyContext(ctx);
	CGLReleasePixelFormat(format);

	printf(g_failures ? "FAIL cgl_options\n" : "PASS cgl_options\n");
	return g_failures != 0;
}
