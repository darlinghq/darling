// Copyright (C) 2026 Lubos Dolezel
//
// CGLSetOffScreen() and CGLGetOffScreen(): a context whose pixel format asked for kCGLPFAOffScreen renders into a
// buffer that is copied into the client's memory (32 bit pixels, rowbytes apart) when the rendering is flushed or
// finished; other contexts and bad arguments give the documented errors; CGLClearDrawable() leaves the buffer.
//
// The pixel layout (alpha, red, green, blue bytes in memory, the top row of the image
// first) is not documented by the CGL reference and could not be observed (the macOS releases available for
// comparison refuse off-screen drawables), so the test pins what this implementation gives. Needs an offscreen
// OpenGL context (a host EGL driver).
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

enum { W = 16, H = 8, ROWBYTES = W * 4 + 12 };

static int closeTo(unsigned char value, int expected) { return value >= expected - 2 && value <= expected + 2; }

static int pixelIs(const unsigned char *memory, int x, int y, int a, int r, int g, int b)
{
	const unsigned char *p = memory + y * ROWBYTES + x * 4;
	return closeTo(p[0], a) && closeTo(p[1], r) && closeTo(p[2], g) && closeTo(p[3], b);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	CGLPixelFormatAttribute plainAttributes[] = { kCGLPFAColorSize, 24, 0 };
	CGLPixelFormatAttribute offscreenAttributes[] = { kCGLPFAOffScreen, kCGLPFAColorSize, 32, kCGLPFAAlphaSize, 8,
		kCGLPFADepthSize, 16, 0 };
	CGLPixelFormatObj plainFormat = NULL, offscreenFormat = NULL;
	GLint screens = 0;
	CHECK_EQ(CGLChoosePixelFormat(plainAttributes, &plainFormat, &screens), kCGLNoError);
	CHECK_EQ(CGLChoosePixelFormat(offscreenAttributes, &offscreenFormat, &screens), kCGLNoError);
	CGLContextObj plain = NULL, offscreen = NULL;
	CHECK_EQ(CGLCreateContext(plainFormat, NULL, &plain), kCGLNoError);
	CHECK_EQ(CGLCreateContext(offscreenFormat, NULL, &offscreen), kCGLNoError);
	if (!plain || !offscreen) {
		printf("FAIL: contexts not created\n");
		return 1;
	}

	static unsigned char memory[ROWBYTES * H + 16];
	// argument errors; a context whose format did not ask for off-screen drawing has none
	CHECK_EQ(CGLSetOffScreen(NULL, W, H, ROWBYTES, memory), kCGLBadContext);
	CHECK_EQ(CGLSetOffScreen(plain, W, H, ROWBYTES, memory), kCGLBadOffScreen);
	CHECK_EQ(CGLSetOffScreen(offscreen, W, H, ROWBYTES, NULL), kCGLBadAddress);
	CHECK_EQ(CGLSetOffScreen(offscreen, 0, H, ROWBYTES, memory), kCGLBadValue);
	CHECK_EQ(CGLSetOffScreen(offscreen, W, 0, ROWBYTES, memory), kCGLBadValue);
	CHECK_EQ(CGLSetOffScreen(offscreen, W, H, W * 4 - 1, memory), kCGLBadValue);
	CHECK_EQ(CGLGetOffScreen(NULL, NULL, NULL, NULL, NULL), kCGLBadContext);

	GLsizei width = 5, height = 5;
	GLint rowbytes = 5;
	void *base = memory;
	CHECK_EQ(CGLGetOffScreen(offscreen, &width, &height, &rowbytes, &base), kCGLNoError);
	CHECK(width == 0 && height == 0 && rowbytes == 0 && base == NULL);

	memset(memory, 0xEE, sizeof(memory));
	CHECK_EQ(CGLSetOffScreen(offscreen, W, H, ROWBYTES, memory), kCGLNoError);
	CHECK_EQ(CGLGetOffScreen(offscreen, &width, &height, &rowbytes, &base), kCGLNoError);
	CHECK(width == W && height == H && rowbytes == ROWBYTES && base == memory);

	CHECK_EQ(CGLSetCurrentContext(offscreen), kCGLNoError);
	glViewport(0, 0, W, H);
	glDisable(GL_DITHER);
	// the lower half (GL's first rows) in one color, one pixel in another
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, W, H / 2);
	glClearColor(0.2f, 0.4f, 0.6f, 0.8f);
	glClear(GL_COLOR_BUFFER_BIT);
	glScissor(0, H / 2, W, H / 2);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glScissor(3, 0, 1, 1);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	// the rendering reaches the memory with the flush of the context: the rows of the image top first, rowbytes apart
	CHECK_EQ(CGLFlushDrawable(offscreen), kCGLNoError);
	CHECK(pixelIs(memory, 0, 0, 255, 255, 0, 0));                  // the top row of the image: the upper half
	CHECK(pixelIs(memory, W - 1, H / 2 - 1, 255, 255, 0, 0));
	CHECK(pixelIs(memory, 8, H / 2, 204, 51, 102, 153));           // the lower half: alpha, red, green, blue
	CHECK(pixelIs(memory, 3, H - 1, 255, 0, 255, 0));              // the bottom row of the image: GL's first row
	CHECK(pixelIs(memory, 4, H - 1, 204, 51, 102, 153));
	// the bytes after the pixels of each row and after the last row are not written
	for (int y = 0; y < H; y++)
		for (int x = W * 4; x < ROWBYTES; x++)
			CHECK(memory[y * ROWBYTES + x] == 0xEE);
	for (int x = ROWBYTES * H; x < (int) sizeof(memory); x++)
		CHECK(memory[x] == 0xEE);

	// glFinish and glFlush copy too
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
	CHECK(pixelIs(memory, 5, 2, 255, 0, 0, 255));
	glClearColor(1, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	CHECK(pixelIs(memory, 5, 2, 255, 255, 255, 0));
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	// a new buffer replaces the old one
	static unsigned char other[ROWBYTES * H];
	memset(other, 0xEE, sizeof(other));
	CHECK_EQ(CGLSetOffScreen(offscreen, W, H, ROWBYTES, other), kCGLNoError);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
	CHECK(pixelIs(other, 1, 1, 255, 255, 0, 255));
	CHECK(pixelIs(memory, 1, 1, 255, 255, 255, 0));

	// clearing the drawable leaves the buffer; the context stays usable
	CHECK_EQ(CGLClearDrawable(offscreen), kCGLNoError);
	CHECK_EQ(CGLGetOffScreen(offscreen, &width, &height, &rowbytes, &base), kCGLNoError);
	CHECK(width == 0 && height == 0 && rowbytes == 0 && base == NULL);
	CHECK_EQ(CGLSetCurrentContext(offscreen), kCGLNoError);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();
	CHECK(pixelIs(other, 1, 1, 255, 255, 0, 255));
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	CGLSetCurrentContext(NULL);
	CGLDestroyContext(offscreen);
	CGLDestroyContext(plain);
	CGLReleasePixelFormat(offscreenFormat);
	CGLReleasePixelFormat(plainFormat);

	printf(g_failures ? "FAIL cgl_offscreen\n" : "PASS cgl_offscreen\n");
	return g_failures != 0;
}
