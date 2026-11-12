// Copyright (C) 2026 Lubos Dolezel
//
// CGLQueryRendererInfo() and CGLDescribeRenderer(): there is at least one
// renderer; its ID, accelerated flag and video memory are described; the
// error codes for a bad display mask, index, property and addresses; the
// answers agree with the GL context an application gets; and querying does
// not change the current context.
//
// There are two renderers, as on macOS: the host's and a software one. Needs an
// offscreen OpenGL context (a host EGL driver). On a software renderer
// (llvmpipe) the host's renderer is not accelerated either, and its memory is
// what the IORegistry's IOAccelerator says (iokitd's share of system memory).
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/gl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

static int is_software_name(const char* renderer)
{
	static const char* const names[] = { "llvmpipe", "lavapipe", "softpipe", "swrast", "Software Rasterizer", "Apple Software Renderer" };
	for (size_t i = 0; renderer && i < sizeof(names) / sizeof(names[0]); i++)
		if (strstr(renderer, names[i]))
			return 1;
	return 0;
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
	CGLContextObj context = NULL;
	CHECK(CGLCreateContext(format, NULL, &context) == kCGLNoError);
	CGLDestroyPixelFormat(format);
	if (!context) {
		printf("FAIL: no context\n");
		return 1;
	}
	CHECK(CGLSetCurrentContext(context) == kCGLNoError);
	const char* glRenderer = (const char*) glGetString(GL_RENDERER);
	printf("GL_RENDERER: %s\n", glRenderer ? glRenderer : "(null)");

	// the renderers
	CGLRendererInfoObj info = NULL;
	GLint count = -1;
	CHECK(CGLQueryRendererInfo(0xFFFFFFFFu, &info, &count) == kCGLNoError);
	CHECK(info != NULL);
	// the host's renderer and a software renderer, as on macOS
	CHECK(count >= 2);
	GLint reported = -1;
	CHECK(CGLDescribeRenderer(info, 0, kCGLRPRendererCount, &reported) == kCGLNoError);
	CHECK(reported == count);

	int accelerated_seen = 0;
	for (GLint i = 0; i < count; i++) {
		GLint id = 0, accelerated = -1, mask = 0, window = 0, megabytes = -1, textureMegabytes = -1;
		GLint video = -1, texture = -1, online = -1;
		CHECK(CGLDescribeRenderer(info, i, kCGLRPRendererID, &id) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPAccelerated, &accelerated) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPDisplayMask, &mask) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPWindow, &window) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPOnline, &online) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPVideoMemoryMegabytes, &megabytes) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPTextureMemoryMegabytes, &textureMegabytes) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPVideoMemory, &video) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, i, kCGLRPTextureMemory, &texture) == kCGLNoError);
		printf("renderer %d: id=0x%x accelerated=%d mask=0x%x megabytes=%d texture megabytes=%d\n",
		       i, id, accelerated, (unsigned) mask, megabytes, textureMegabytes);

		CHECK(id != 0);
		CHECK((id & ~kCGLRendererIDMatchingMask & ~0x01000000) == 0);
		CHECK(accelerated == 0 || accelerated == 1);
		// the software renderers are the generic ones (macOS sets one more bit above the matching mask)
		CHECK(accelerated == 1 || (id & kCGLRendererIDMatchingMask) == kCGLRendererGenericFloatID
		      || (id & kCGLRendererIDMatchingMask) == kCGLRendererAppleSWID
		      || (id & kCGLRendererIDMatchingMask) == kCGLRendererGenericID);
		CHECK((mask & 1) != 0);
		CHECK(window == 1);
		CHECK(online == 1);
		// the size is known for an accelerated renderer; the software renderer may have none
		if (accelerated)
			CHECK(megabytes > 0);
		CHECK(megabytes >= 0);
		CHECK(textureMegabytes >= 0 && textureMegabytes <= megabytes);
		// the deprecated properties give the sizes in bytes, as far as an int goes
		CHECK(video >= 0 && texture >= 0);
		if (megabytes < 2048)
			CHECK(video / (1024 * 1024) == megabytes || video / (1024 * 1024) == megabytes - 1 || video / (1024 * 1024) == megabytes + 1);
		else
			CHECK(video == INT_MAX);
		accelerated_seen += accelerated;
	}

	// the last renderer is a software one with no video memory
	{
		GLint accelerated = -1, id = 0, megabytes = -1;
		CHECK(CGLDescribeRenderer(info, count - 1, kCGLRPAccelerated, &accelerated) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, count - 1, kCGLRPRendererID, &id) == kCGLNoError);
		CHECK(CGLDescribeRenderer(info, count - 1, kCGLRPVideoMemoryMegabytes, &megabytes) == kCGLNoError);
		CHECK(accelerated == 0);
		CHECK((id & kCGLRendererIDMatchingMask) == kCGLRendererGenericFloatID);
		CHECK(megabytes == 0);
	}

	// the renderer the context got is one of the renderers
	if (is_software_name(glRenderer)) {
		GLint accelerated = -1;
		CHECK(CGLDescribeRenderer(info, 0, kCGLRPAccelerated, &accelerated) == kCGLNoError);
		printf("software renderer: renderer 0 accelerated=%d\n", accelerated);
	} else {
		CHECK(accelerated_seen >= 1);
	}

	// errors
	GLint value = 0x5a5a;
	CHECK(CGLDescribeRenderer(info, count, kCGLRPRendererID, &value) == kCGLBadValue);
	CHECK(CGLDescribeRenderer(info, -1, kCGLRPRendererID, &value) == kCGLBadValue);
	CHECK(CGLDescribeRenderer(info, 0, (CGLRendererProperty) 99999, &value) == kCGLBadProperty);
	CGLRendererInfoObj other = NULL;
	GLint otherCount = -1;
	CHECK(CGLQueryRendererInfo(0, &other, &otherCount) == kCGLBadDisplay);
	CHECK(other == NULL && otherCount == 0);
	CHECK(CGLQueryRendererInfo(0xFFFFFFFFu, NULL, &otherCount) == kCGLBadAddress);
	CHECK(CGLQueryRendererInfo(0xFFFFFFFFu, &other, NULL) == kCGLBadAddress);
	CHECK(CGLDestroyRendererInfo(NULL) == kCGLNoError);

	// a smaller mask is accepted, and the renderer reports the displays it was asked about
	CHECK(CGLQueryRendererInfo(1, &other, &otherCount) == kCGLNoError);
	CHECK(otherCount >= 2);
	GLint otherMask = 0;
	CHECK(CGLDescribeRenderer(other, 0, kCGLRPDisplayMask, &otherMask) == kCGLNoError);
	CHECK((otherMask & 1) != 0);
	CHECK(CGLDestroyRendererInfo(other) == kCGLNoError);

	// asking does not change the current context
	CHECK(CGLGetCurrentContext() == context);

	// with no context current, the query works and leaves none current
	CHECK(CGLSetCurrentContext(NULL) == kCGLNoError);
	CHECK(CGLQueryRendererInfo(0xFFFFFFFFu, &other, &otherCount) == kCGLNoError);
	CHECK(otherCount == count);
	CHECK(CGLGetCurrentContext() == NULL);
	CHECK(CGLDestroyRendererInfo(other) == kCGLNoError);

	CHECK(CGLDestroyRendererInfo(info) == kCGLNoError);
	CGLDestroyContext(context);
	puts(g_failures ? "FAILED" : "PASSED");
	return g_failures != 0;
}
