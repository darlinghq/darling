// Copyright (C) 2026 Lubos Dolezel
//
// CGLMacro.h: code that calls OpenGL through a context's dispatch table.
// The first four members of the context object a CGLContextObj points to are
// public (rend, disp, priv, stak), the table has one pointer per slot of
// GLIFunctionDispatch, and the gl* macros call table entries with the context's
// renderer as the first argument. Checks the layout, that macro calls work for
// the current context and for a context that is not current, and that the
// table entries agree with the plain functions. Needs an offscreen OpenGL
// context (a host EGL driver); on macOS without a GPU session build it with
// -DTEST_GENERIC_FLOAT_RENDERER.
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLMacro.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// The number of slots of the public table: 975 pointers.
#define SLOTS 975

static void use_macros(CGLContextObj context, GLuint *texture_out, const GLubyte *pixels)
{
	CGLContextObj cgl_ctx = context;
	CGL_MACRO_DECLARE_RENDERER();
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	CHECK(glGetError() == GL_NO_ERROR);
	GLubyte back[16];
	memset(back, 0, sizeof back);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, back);
	CHECK(memcmp(back, pixels, 16) == 0);
	glEnable(GL_BLEND);
	CHECK(glIsEnabled(GL_BLEND) == GL_TRUE);
	glDisable(GL_BLEND);
	CHECK(glIsEnabled(GL_BLEND) == GL_FALSE);
	GLint size = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	CHECK(size >= 1024);
	glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
	GLfloat color[4] = { 0, 0, 0, 0 };
	glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
	CHECK(color[0] == 0.25f && color[1] == 0.5f && color[2] == 0.75f && color[3] == 1.0f);
	*texture_out = texture;
}

int main(void)
{
	CGLContextObj cgl_ctx = NULL;
	setvbuf(stdout, NULL, _IONBF, 0);

	// Layout.
	CHECK(sizeof(GLIFunctionDispatch) == SLOTS * sizeof(void *));
	CHECK(offsetof(struct _CGLContextObject, rend) == 0);
	CHECK(offsetof(struct _CGLContextObject, disp) == sizeof(void *));
	CHECK(offsetof(struct _CGLContextObject, priv) == sizeof(void *) + sizeof(GLIFunctionDispatch));
	CHECK(offsetof(struct _CGLContextObject, stak) == 2 * sizeof(void *) + sizeof(GLIFunctionDispatch));

#ifdef TEST_GENERIC_FLOAT_RENDERER
	CGLPixelFormatAttribute attrs[] = { kCGLPFARendererID, (CGLPixelFormatAttribute) 0x00020400, 0 };
#else
	CGLPixelFormatAttribute attrs[] = { 0 };
#endif
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
	CGLContextObj first = NULL, second = NULL;
	CHECK(CGLCreateContext(format, NULL, &first) == kCGLNoError);
	CHECK(CGLCreateContext(format, NULL, &second) == kCGLNoError);
	if (!first || !second) {
		printf("FAIL: no context\n");
		return 1;
	}
	CHECK(first->rend != NULL);
	CHECK(first->rend != second->rend);
	CHECK(first->disp.get_string != NULL && first->disp.get_error != NULL && first->disp.tex_image2D != NULL);

	CHECK(CGLSetCurrentContext(first) == kCGLNoError);
	CHECK(CGLGetCurrentContext() == first);
	cgl_ctx = first;

	// The renderer is the first argument, the entries do what the functions do.
	const GLubyte *version = (*first->disp.get_string)(first->rend, GL_VERSION);
	CHECK(version != NULL);
	CHECK(version != NULL && strcmp((const char *) version, (const char *) glGetString(GL_VERSION)) == 0);

	// Macro calls on the current context.
	static const GLubyte pixels[16] = { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 9, 8, 7, 255 };
	GLuint texture_first = 0, texture_second = 0;
	use_macros(first, &texture_first, pixels);
	CHECK(texture_first != 0);

	// Macro calls on a context that is not the current one act on that context.
	use_macros(second, &texture_second, pixels);
	CHECK(texture_second != 0);
	CHECK(CGLSetCurrentContext(second) == kCGLNoError);
	cgl_ctx = second;
	CHECK(glIsTexture(texture_second) == GL_TRUE);
	GLfloat color[4] = { 0, 0, 0, 0 };
	glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
	CHECK(color[2] == 0.75f);
	CGLSetCurrentContext(NULL);

	CGLDestroyContext(first);
	CGLDestroyContext(second);
	CGLDestroyPixelFormat(format);
	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
