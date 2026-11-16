// Copyright (C) 2026 Lubos Dolezel
//
// What a context reports about itself for each OpenGL profile, through CGL and
// through NSOpenGL: legacy contexts are OpenGL 2.1 (GL_VERSION "2.1 ...", GLSL
// 1.20, an extension string of 2.1 extensions only, no GL 3 version and
// extension count queries), core contexts report at most 4.1 (GLSL at most
// 4.10) and list their extensions through glGetStringi and GL_NUM_EXTENSIONS
// only. A shader of the profile's language compiles in each. Functions of
// newer versions stay resolvable in every profile.
//
// The core-profile checks that both 3.2 and 4.1 requests report the same
// version are quirk tests: macOS gives a 3.2 core request the newest core
// version, 4.1 (observed on Mojave 10.14.4 and macOS 26.7.3).
#define GL_GLEXT_PROTOTYPES 1
#import <AppKit/AppKit.h>
#import <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declared here: the SDK's gl.h stops at 2.1 and gl3.h pulls in GLES headers.
#define CHECK_GL_MAJOR_VERSION 0x821B
#define CHECK_GL_MINOR_VERSION 0x821C
#define CHECK_GL_NUM_EXTENSIONS 0x821D
const GLubyte *glGetStringi(GLenum name, GLuint index);

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

enum { LEGACY, CORE32, CORE41 };

static int has_word(const char *list, const char *name)
{
	size_t length = strlen(name);
	for (const char *p = list; (p = strstr(p, name)) != NULL; p += length) {
		if ((p == list || p[-1] == ' ') && (p[length] == ' ' || p[length] == '\0'))
			return 1;
	}
	return 0;
}

static int compiles(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[512] = "";
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		printf("shader log: %s\n", log);
	}
	glDeleteShader(shader);
	return status == GL_TRUE;
}

// the checks that every profile has in common run in the profile's current context
static void check_context(int kind, const char *label)
{
	printf("-- %s\n", label);
	while (glGetError() != GL_NO_ERROR) {}

	const char *version = (const char *) glGetString(GL_VERSION);
	const char *glsl = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
	const char *renderer = (const char *) glGetString(GL_RENDERER);
	const char *vendor = (const char *) glGetString(GL_VENDOR);
	printf("GL_VERSION=[%s] GLSL=[%s] RENDERER=[%s] VENDOR=[%s]\n", version ? version : "(null)",
		glsl ? glsl : "(null)", renderer ? renderer : "(null)", vendor ? vendor : "(null)");
	CHECK(version && glsl && renderer && vendor);
	if (!version || !glsl)
		return;
	CHECK(renderer[0] != '\0' && vendor[0] != '\0');
	int major = 0, minor = 0, glslMajor = 0, glslMinor = 0;
	CHECK(sscanf(version, "%d.%d", &major, &minor) == 2);
	CHECK(sscanf(glsl, "%d.%d", &glslMajor, &glslMinor) == 2);
	// "major.minor" then a space and the driver's text, as on macOS
	CHECK(version[1] == '.' && (version[3] == '\0' || version[3] == ' '));
	CHECK(strstr(version, "Profile") == NULL);
	CHECK(glGetError() == GL_NO_ERROR);

	if (kind == LEGACY) {
		CHECK(major == 2 && minor == 1);
		CHECK(glslMajor == 1 && glslMinor == 20);
		CHECK(strcmp(glsl, "1.20") == 0);

		const char *extensions = (const char *) glGetString(GL_EXTENSIONS);
		CHECK(extensions && extensions[0] != '\0');
		CHECK(glGetError() == GL_NO_ERROR);
		if (extensions) {
			CHECK(has_word(extensions, "GL_ARB_vertex_buffer_object"));
			CHECK(has_word(extensions, "GL_ARB_multitexture"));
			// extensions of later versions are not part of a 2.1 context's list
			CHECK(!has_word(extensions, "GL_ARB_compute_shader"));
			CHECK(!has_word(extensions, "GL_ARB_direct_state_access"));
			CHECK(!has_word(extensions, "GL_ARB_shader_storage_buffer_object"));
			CHECK(!has_word(extensions, "GL_KHR_debug"));
			CHECK(!has_word(extensions, "GL_ARB_buffer_storage"));
			// no duplicates, well-formed names
			char *copy = strdup(extensions);
			int count = 0;
			for (char *name = strtok(copy, " "); name; name = strtok(NULL, " ")) {
				CHECK(strncmp(name, "GL_", 3) == 0);
				char needle[256];
				snprintf(needle, sizeof(needle), "%s ", name);
				const char *first = strstr(extensions, needle);
				const char *again = first ? strstr(first + strlen(needle), needle) : NULL;
				CHECK(again == NULL);
				count++;
			}
			free(copy);
			CHECK(count > 20);
		}

		// OpenGL 3.0 queries do not exist in 2.1
		GLint value = 12345;
		glGetIntegerv(CHECK_GL_MAJOR_VERSION, &value);
		CHECK(glGetError() == GL_INVALID_ENUM);
		CHECK(value == 12345);
		glGetIntegerv(CHECK_GL_MINOR_VERSION, &value);
		CHECK(glGetError() == GL_INVALID_ENUM);
		glGetIntegerv(CHECK_GL_NUM_EXTENSIONS, &value);
		CHECK(glGetError() == GL_INVALID_ENUM);
		CHECK(value == 12345);
		CHECK(glGetStringi(GL_EXTENSIONS, 0) == NULL);
		CHECK(glGetError() != GL_NO_ERROR);

		// a shader of the 2.1 language compiles
		CHECK(compiles(GL_VERTEX_SHADER,
			"#version 120\nattribute vec4 position;\nvarying vec4 color;\n"
			"void main() { color = gl_Color; gl_Position = gl_ModelViewProjectionMatrix * position; }\n"));
		CHECK(compiles(GL_FRAGMENT_SHADER,
			"#version 120\nvarying vec4 color;\nuniform sampler2D tex;\n"
			"void main() { gl_FragColor = color * texture2D(tex, gl_TexCoord[0].xy); }\n"));
		CHECK(glGetError() == GL_NO_ERROR);
	} else {
		// at most the newest core version a Mac reports, and at least what was asked for
		CHECK(major == 4 || (major == 3 && minor >= 2));
		CHECK(major < 4 || minor <= 1);
		CHECK(glslMajor * 100 + glslMinor >= 150 && glslMajor * 100 + glslMinor <= 410);
		CHECK(strlen(glsl) == 4);

		GLint ints[2] = { -1, -1 };
		glGetIntegerv(CHECK_GL_MAJOR_VERSION, &ints[0]);
		glGetIntegerv(CHECK_GL_MINOR_VERSION, &ints[1]);
		CHECK(glGetError() == GL_NO_ERROR);
		CHECK(ints[0] == major && ints[1] == minor);

		CHECK(glGetString(GL_EXTENSIONS) == NULL);
		CHECK(glGetError() == GL_INVALID_ENUM);

		GLint count = -1;
		glGetIntegerv(CHECK_GL_NUM_EXTENSIONS, &count);
		CHECK(glGetError() == GL_NO_ERROR);
		CHECK(count > 10);
		int sawCompute = 0, sawBuffers = 0, malformed = 0;
		for (GLint i = 0; i < count; i++) {
			const char *name = (const char *) glGetStringi(GL_EXTENSIONS, (GLuint) i);
			if (!name || strncmp(name, "GL_", 3) != 0) {
				malformed++;
				continue;
			}
			sawCompute |= strcmp(name, "GL_ARB_compute_shader") == 0;
			sawBuffers |= strcmp(name, "GL_ARB_buffer_storage") == 0;
			for (GLint j = 0; j < i; j++) {
				const char *other = (const char *) glGetStringi(GL_EXTENSIONS, (GLuint) j);
				if (other && strcmp(other, name) == 0)
					malformed++;
			}
		}
		CHECK(malformed == 0);
		// extensions of versions after 4.1 are not listed
		CHECK(!sawCompute && !sawBuffers);
		CHECK(glGetError() == GL_NO_ERROR);
		CHECK(glGetStringi(GL_EXTENSIONS, (GLuint) count) == NULL);
		CHECK(glGetError() == GL_INVALID_VALUE);

		CHECK(compiles(GL_VERTEX_SHADER,
			"#version 150\nin vec4 position;\nvoid main() { gl_Position = position; }\n"));
		CHECK(compiles(GL_FRAGMENT_SHADER, "#version 150\nout vec4 c;\nvoid main() { c = vec4(1.0); }\n"));
		CHECK(glGetError() == GL_NO_ERROR);
	}
}

// the version and shading language of a profile, to compare profiles
static void describe(char *out, size_t size)
{
	const char *version = (const char *) glGetString(GL_VERSION);
	const char *glsl = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint count = -1;
	glGetIntegerv(CHECK_GL_NUM_EXTENSIONS, &count);
	glGetError();
	snprintf(out, size, "%s|%s|%d", version ? version : "", glsl ? glsl : "", count);
}

static const char *const g_labels[] = { "legacy 2.1", "core 3.2", "core 4.1" };
static const GLint g_profiles[] = { kCGLOGLPVersion_Legacy, kCGLOGLPVersion_3_2_Core, kCGLOGLPVersion_GL4_Core };

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		[NSApplication sharedApplication];
		char described[2][3][256];
		memset(described, 0, sizeof(described));

		// CGL
		for (int kind = 0; kind < 3; kind++) {
			CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) g_profiles[kind], 0 };
			CGLPixelFormatObj format = NULL;
			GLint screens = 0;
			if (CGLChoosePixelFormat(attrs, &format, &screens) != kCGLNoError || !format) {
				printf("FAIL: no pixel format for %s\n", g_labels[kind]);
				g_failures++;
				continue;
			}
			CGLContextObj context = NULL;
			CGLError error = CGLCreateContext(format, NULL, &context);
			CGLDestroyPixelFormat(format);
			if (error != kCGLNoError || !context) {
				printf("FAIL: no context for %s (%d)\n", g_labels[kind], error);
				g_failures++;
				continue;
			}
			CGLSetCurrentContext(context);
			char label[64];
			snprintf(label, sizeof(label), "CGL %s", g_labels[kind]);
			check_context(kind, label);
			describe(described[0][kind], sizeof(described[0][kind]));
			// functions of later versions resolve in every profile
			CHECK(dlsym(RTLD_DEFAULT, "glGenVertexArrays") != NULL);
			CGLSetCurrentContext(NULL);
			CGLDestroyContext(context);
		}

		// a CGL pixel format without a profile attribute is a legacy context
		{
			CGLPixelFormatAttribute attrs[] = { kCGLPFAColorSize, (CGLPixelFormatAttribute) 24, 0 };
			CGLPixelFormatObj format = NULL;
			GLint screens = 0;
			CGLContextObj context = NULL;
			CHECK(CGLChoosePixelFormat(attrs, &format, &screens) == kCGLNoError);
			CHECK(CGLCreateContext(format, NULL, &context) == kCGLNoError);
			CGLDestroyPixelFormat(format);
			if (context) {
				CGLSetCurrentContext(context);
				check_context(LEGACY, "CGL default profile");
				CGLSetCurrentContext(NULL);
				CGLDestroyContext(context);
			}
		}

		// NSOpenGL
		for (int kind = 0; kind < 3; kind++) {
			NSOpenGLPixelFormatAttribute attrs[] = {
				NSOpenGLPFAOpenGLProfile, (NSOpenGLPixelFormatAttribute) g_profiles[kind], 0 };
			NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
			NSOpenGLContext *context = format ? [[NSOpenGLContext alloc] initWithFormat: format shareContext: nil] : nil;
			if (!context) {
				printf("FAIL: no NSOpenGLContext for %s\n", g_labels[kind]);
				g_failures++;
				continue;
			}
			[context makeCurrentContext];
			CHECK(CGLGetCurrentContext() == [context CGLContextObj]);
			char label[64];
			snprintf(label, sizeof(label), "NSOpenGL %s", g_labels[kind]);
			check_context(kind, label);
			describe(described[1][kind], sizeof(described[1][kind]));
			[NSOpenGLContext clearCurrentContext];
		}

		// both entry points give the same answers
		for (int kind = 0; kind < 3; kind++)
			CHECK(strcmp(described[0][kind], described[1][kind]) == 0);
		// QUIRK test: a 3.2 core request reports the same as a 4.1 request
		CHECK(strcmp(described[0][CORE32], described[0][CORE41]) == 0);
	}
	printf("cgl profile strings: %s\n", g_failures ? "FAIL" : "PASS");
	return g_failures != 0;
}
