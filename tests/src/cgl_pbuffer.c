// Copyright (C) 2026 Lubos Dolezel
//
// CGL pixel buffers, from the CGL Reference: CGLCreatePBuffer, CGLDescribePBuffer, CGLRetainPBuffer,
// CGLReleasePBuffer, CGLGetPBufferRetainCount, CGLDestroyPBuffer, CGLSetPBuffer, CGLGetPBuffer and
// CGLTexImagePBuffer. A context renders into the buffer after CGLSetPBuffer (face and mipmap level chosen there),
// CGLTexImagePBuffer makes the buffer's contents the image of the texture bound in another context, and the
// documented errors are returned for bad arguments. Needs an offscreen OpenGL context (a host EGL driver).
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long) (a), _b = (long) (b); if (_a != _b) { printf("FAIL %d: %s = %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void fillQuadrants(int width, int height, const float lower[3], const float upper[3])
{
	// the lower half of the buffer (GL's first rows) and the upper half in two colors
	glDisable(GL_DITHER);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, width, height / 2);
	glClearColor(lower[0], lower[1], lower[2], 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glScissor(0, height / 2, width, height - height / 2);
	glClearColor(upper[0], upper[1], upper[2], 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

static int isColor(const unsigned char *p, int r, int g, int b)
{
	return p[0] >= r - 2 && p[0] <= r + 2 && p[1] >= g - 2 && p[1] <= g + 2 && p[2] >= b - 2 && p[2] <= b + 2;
}

static void arguments(void)
{
	CGLPBufferObj pbuffer = (CGLPBufferObj) 1;
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_2D, GL_RGBA, 0, NULL), kCGLBadAddress);
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_1D, GL_RGBA, 0, &pbuffer), kCGLBadEnumeration);
	CHECK(pbuffer == NULL);
	CHECK_EQ(CGLCreatePBuffer(16, 16, 0, GL_RGBA, 0, &pbuffer), kCGLBadEnumeration);
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_2D, GL_RGBA16, 0, &pbuffer), kCGLBadEnumeration);
	CHECK_EQ(CGLCreatePBuffer(0, 16, GL_TEXTURE_2D, GL_RGBA, 0, &pbuffer), kCGLBadValue);
	CHECK_EQ(CGLCreatePBuffer(16, -1, GL_TEXTURE_2D, GL_RGBA, 0, &pbuffer), kCGLBadValue);
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_2D, GL_RGBA, -1, &pbuffer), kCGLBadValue);
	// a 16x16 buffer has the levels down to 1x1: 0 to 4
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_2D, GL_RGBA, 5, &pbuffer), kCGLBadValue);
	// a cube map has square faces
	CHECK_EQ(CGLCreatePBuffer(16, 8, GL_TEXTURE_CUBE_MAP, GL_RGBA, 0, &pbuffer), kCGLBadValue);
	CHECK(pbuffer == NULL);

	CHECK_EQ(CGLCreatePBuffer(100, 50, GL_TEXTURE_RECTANGLE_EXT, GL_RGB, 0, &pbuffer), kCGLNoError);
	CHECK(pbuffer != NULL);
	GLsizei width = 0, height = 0;
	GLenum target = 0, format = 0;
	GLint mipmap = -1;
	CHECK_EQ(CGLDescribePBuffer(pbuffer, &width, &height, &target, &format, &mipmap), kCGLNoError);
	CHECK(width == 100 && height == 50 && target == GL_TEXTURE_RECTANGLE_EXT && format == GL_RGB && mipmap == 0);
	CHECK_EQ(CGLDescribePBuffer(NULL, &width, &height, &target, &format, &mipmap), kCGLBadAddress);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 1);
	CHECK(CGLRetainPBuffer(pbuffer) == pbuffer);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 2);
	CGLReleasePBuffer(pbuffer);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 1);
	CHECK_EQ(CGLDestroyPBuffer(pbuffer), kCGLNoError);
	CHECK_EQ(CGLDestroyPBuffer(NULL), kCGLBadAddress);
	CHECK_EQ(CGLGetPBufferRetainCount(NULL), 0);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	arguments();

	CGLPixelFormatAttribute attributes[] = { kCGLPFAColorSize, 24, kCGLPFAAlphaSize, 8, kCGLPFADepthSize, 16, 0 };
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK_EQ(CGLChoosePixelFormat(attributes, &format, &screens), kCGLNoError);
	CGLContextObj drawing = NULL, texturing = NULL;
	CHECK_EQ(CGLCreateContext(format, NULL, &drawing), kCGLNoError);
	CHECK_EQ(CGLCreateContext(format, NULL, &texturing), kCGLNoError);
	if (!drawing || !texturing) {
		printf("FAIL: contexts not created\n");
		return 1;
	}

	enum { W = 64, H = 32 };
	CGLPBufferObj pbuffer = NULL;
	CHECK_EQ(CGLCreatePBuffer(W, H, GL_TEXTURE_2D, GL_RGBA, 2, &pbuffer), kCGLNoError);

	// bad arguments of the context calls
	CHECK_EQ(CGLSetPBuffer(NULL, pbuffer, 0, 0, 0), kCGLBadContext);
	CHECK_EQ(CGLSetPBuffer(drawing, NULL, 0, 0, 0), kCGLBadAddress);
	CHECK_EQ(CGLSetPBuffer(drawing, pbuffer, 0, 3, 0), kCGLBadValue);
	CHECK_EQ(CGLSetPBuffer(drawing, pbuffer, 0, -1, 0), kCGLBadValue);
	CHECK_EQ(CGLSetPBuffer(drawing, pbuffer, 0, 0, 1), kCGLBadValue);
	CHECK_EQ(CGLGetPBuffer(NULL, &pbuffer, NULL, NULL, NULL), kCGLBadContext);
	CHECK_EQ(CGLTexImagePBuffer(NULL, pbuffer, GL_BACK), kCGLBadContext);
	CHECK_EQ(CGLTexImagePBuffer(texturing, NULL, GL_BACK), kCGLBadAddress);
	CHECK_EQ(CGLTexImagePBuffer(texturing, pbuffer, GL_RGBA), kCGLBadEnumeration);
	// no context has drawn to the buffer yet
	CHECK_EQ(CGLTexImagePBuffer(texturing, pbuffer, GL_BACK), kCGLBadValue);

	// a context without a pixel buffer reports none
	CGLPBufferObj attached = (CGLPBufferObj) 1;
	GLenum face = 77;
	GLint level = 77, screen = 77;
	CHECK_EQ(CGLGetPBuffer(drawing, &attached, &face, &level, &screen), kCGLNoError);
	CHECK(attached == NULL);

	// render the base level: green in the lower half, blue in the upper
	CHECK_EQ(CGLSetPBuffer(drawing, pbuffer, 0, 0, 0), kCGLNoError);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 2);
	CHECK_EQ(CGLGetPBuffer(drawing, &attached, &face, &level, &screen), kCGLNoError);
	CHECK(attached == pbuffer && face == 0 && level == 0 && screen == 0);
	CHECK_EQ(CGLSetCurrentContext(drawing), kCGLNoError);
	glViewport(0, 0, W, H);
	const float green[3] = { 0, 1, 0 }, blue[3] = { 0, 0, 1 };
	fillQuadrants(W, H, green, blue);
	unsigned char pixel[4];
	glReadPixels(5, 5, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	CHECK(isColor(pixel, 0, 255, 0));
	glReadPixels(W - 5, H - 5, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
	CHECK(isColor(pixel, 0, 0, 255));
	CHECK_EQ(CGLFlushDrawable(drawing), kCGLNoError);

	// the other context takes the buffer as its texture
	CHECK_EQ(CGLSetCurrentContext(texturing), kCGLNoError);
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK_EQ(CGLTexImagePBuffer(texturing, pbuffer, GL_BACK), kCGLNoError);
	CHECK(CGLGetCurrentContext() == texturing);
	GLint textureWidth = 0, textureHeight = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
	CHECK(textureWidth == W && textureHeight == H);
	static unsigned char image[W * H * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	CHECK(isColor(&image[(5 * W + 5) * 4], 0, 255, 0));                       // the first rows are the lower half
	CHECK(isColor(&image[((H - 5) * W + W - 5) * 4], 0, 0, 255));
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	// the buffer still holds what was drawn once the texture exists; drawing again changes the next copy only
	CHECK_EQ(CGLSetCurrentContext(drawing), kCGLNoError);
	const float red[3] = { 1, 0, 0 }, yellow[3] = { 1, 1, 0 };
	fillQuadrants(W, H, red, yellow);
	glFinish();
	CHECK_EQ(CGLSetCurrentContext(texturing), kCGLNoError);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	CHECK(isColor(&image[(5 * W + 5) * 4], 0, 255, 0));
	CHECK_EQ(glGetError(), GL_NO_ERROR);
	CHECK_EQ(CGLTexImagePBuffer(texturing, pbuffer, GL_FRONT), kCGLNoError);
	CHECK_EQ(glGetError(), GL_NO_ERROR);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	CHECK(isColor(&image[(5 * W + 5) * 4], 255, 0, 0));
	CHECK(isColor(&image[((H - 5) * W + W - 5) * 4], 255, 255, 0));

	// a mipmap level is a drawable of its own, of half the size: the copy lands in that level of the texture
	CHECK_EQ(CGLSetPBuffer(drawing, pbuffer, 0, 1, 0), kCGLNoError);
	CHECK_EQ(CGLGetPBuffer(drawing, &attached, &face, &level, &screen), kCGLNoError);
	CHECK(attached == pbuffer && level == 1);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 2);
	CHECK_EQ(CGLSetCurrentContext(drawing), kCGLNoError);
	glViewport(0, 0, W / 2, H / 2);
	const float white[3] = { 1, 1, 1 };
	fillQuadrants(W / 2, H / 2, white, white);
	glFinish();
	CHECK_EQ(CGLSetCurrentContext(texturing), kCGLNoError);
	CHECK_EQ(CGLTexImagePBuffer(texturing, pbuffer, GL_BACK), kCGLNoError);
	CHECK_EQ(glGetError(), GL_NO_ERROR);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 1, GL_TEXTURE_WIDTH, &textureWidth);
	CHECK_EQ(textureWidth, W / 2);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE, image);
	CHECK(isColor(&image[0], 255, 255, 255));
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	// clearing the drawable lets go of the buffer
	CHECK_EQ(CGLClearDrawable(drawing), kCGLNoError);
	CHECK_EQ(CGLGetPBuffer(drawing, &attached, &face, &level, &screen), kCGLNoError);
	CHECK(attached == NULL);
	CHECK_EQ(CGLGetPBufferRetainCount(pbuffer), 1);
	CHECK_EQ(CGLSetCurrentContext(drawing), kCGLNoError);
	CHECK_EQ(glGetError(), GL_NO_ERROR);

	// a cube map buffer: each face is its own drawable, and a copy goes to the texture's face
	CGLPBufferObj cube = NULL;
	CHECK_EQ(CGLCreatePBuffer(16, 16, GL_TEXTURE_CUBE_MAP, GL_RGBA, 0, &cube), kCGLNoError);
	CHECK_EQ(CGLSetPBuffer(drawing, cube, 0, 0, 0), kCGLBadEnumeration);
	CHECK_EQ(CGLSetPBuffer(drawing, cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0), kCGLNoError);
	CHECK_EQ(CGLGetPBuffer(drawing, &attached, &face, &level, &screen), kCGLNoError);
	CHECK(attached == cube && face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	CHECK_EQ(CGLSetCurrentContext(drawing), kCGLNoError);
	glViewport(0, 0, 16, 16);
	fillQuadrants(16, 16, green, green);
	glFinish();
	CHECK_EQ(CGLSetCurrentContext(texturing), kCGLNoError);
	GLuint cubeTexture = 0;
	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	CHECK_EQ(CGLTexImagePBuffer(texturing, cube, GL_BACK), kCGLNoError);
	CHECK_EQ(glGetError(), GL_NO_ERROR);
	unsigned char small[16 * 16 * 4];
	glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, small);
	CHECK(isColor(&small[(8 * 16 + 8) * 4], 0, 255, 0));

	// destroying a context that draws to a buffer lets go of it
	CHECK_EQ(CGLGetPBufferRetainCount(cube), 2);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(drawing);
	CHECK_EQ(CGLGetPBufferRetainCount(cube), 1);
	CGLReleasePBuffer(cube);
	CGLReleasePBuffer(pbuffer);
	CGLDestroyContext(texturing);
	CGLReleasePixelFormat(format);

	printf(g_failures ? "FAIL cgl_pbuffer\n" : "PASS cgl_pbuffer\n");
	return g_failures != 0;
}
