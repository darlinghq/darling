// Copyright (C) 2026 Lubos Dolezel
//
// IOSurface contents shared between the CPU, OpenGL and another process,
// as the IOSurface and CGLTexImageIOSurface2D documentation describes them:
// CPU writes made under a lock are what GL reads from a texture specified
// from the surface, GL rendering into such a texture is what the CPU reads
// under the next lock, and a process that receives the surface through a
// Mach port sees and changes the same contents.
//
// Darling has two ways to do this. With DMA-BUF export (a host with
// /dev/udmabuf and an EGL driver that imports DMA-BUFs into GL_TEXTURE_2D
// textures), the texture uses the surface's memory, as on macOS. Otherwise
// the texture is a copy, made by CGLTexImageIOSurface2D and copied back
// before a lock. The test passes with both and prints which one it ran on.
// Where the two differ, a CPU write after CGLTexImageIOSurface2D shows in
// the texture only when it uses the surface's memory; that is how the test
// tells them apart.
//
// GL_TEXTURE_RECTANGLE textures cannot use the memory directly; with
// DMA-BUFs, GPU copies keep them in step with it instead: from it when the
// context is made current or locked after a CPU write, into it before a
// lock. Each case prints its path: dmabuf-2d, dmabuf-rect-gpucopy or
// cpu-copy.
//
// Covered: BGRA, RGBA and ARGB byte orders, as 2D and rectangle textures,
// and both planes of a bi-planar
// 4:2:0 surface, read and written by the CPU and by GL, and a BGRA surface
// sent to a forked child, which reads the parent's GL rendering, reads it
// through its own GL texture, and renders into it.
//
// Needs an offscreen OpenGL context (a host EGL driver).
#define GL_GLEXT_PROTOTYPES 1
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <IOSurface/IOSurface.h>
#include <IOSurface/IOSurfacePriv.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <dlfcn.h>
#include <mach/mach.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define W 64
#define H 32
#define TIMEOUT_MS 10000

static int g_failures = 0;
static volatile pid_t g_child = 0;
static const char* g_who = "";

#define CHECK(cond) do { if (!(cond)) { printf("%sFAIL %d: %s\n", g_who, __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	if (g_child > 0)
	{
		kill(g_child, SIGKILL);
		waitpid(g_child, NULL, 0);
	}
	_exit(1);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static IOSurfaceRef createSurface(int width, int height, int bytesPerElement, OSType format)
{
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, width);
	setNumber(props, kIOSurfaceHeight, height);
	setNumber(props, kIOSurfaceBytesPerElement, bytesPerElement);
	setNumber(props, kIOSurfacePixelFormat, (int) format);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	return surface;
}

// A two-plane surface: full-size 1-byte luma, half-size 2-byte chroma.
static IOSurfaceRef createBiPlanar(void)
{
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfacePixelFormat, (int) kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
	CFMutableArrayRef planes = CFArrayCreateMutable(NULL, 2, &kCFTypeArrayCallBacks);
	for (int i = 0; i < 2; i++)
	{
		CFMutableDictionaryRef plane = CFDictionaryCreateMutable(NULL, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		setNumber(plane, kIOSurfacePlaneWidth, i == 0 ? W : W / 2);
		setNumber(plane, kIOSurfacePlaneHeight, i == 0 ? H : H / 2);
		setNumber(plane, kIOSurfacePlaneBytesPerElement, i == 0 ? 1 : 2);
		CFArrayAppendValue(planes, plane);
		CFRelease(plane);
	}
	CFDictionarySetValue(props, kIOSurfacePlaneInfo, planes);
	CFRelease(planes);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	return surface;
}

// The value of a pixel of the test pattern; salt tells patterns apart.
static uint8_t patternByte(uint32_t salt, int x, int y, int byte)
{
	return (uint8_t) (salt * 37 + x * 3 + y * 5 + byte * 11 + 1);
}

// Writes the pattern into a plane under a write lock.
static void writePattern(IOSurfaceRef surface, size_t plane, uint32_t salt)
{
	CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
	uint8_t* base = IOSurfaceGetBaseAddressOfPlane(surface, plane);
	size_t bytesPerRow = IOSurfaceGetBytesPerRowOfPlane(surface, plane);
	size_t bytesPerElement = IOSurfaceGetBytesPerElementOfPlane(surface, plane);
	for (int y = 0; y < (int) IOSurfaceGetHeightOfPlane(surface, plane); y++)
		for (int x = 0; x < (int) IOSurfaceGetWidthOfPlane(surface, plane); x++)
			for (int b = 0; b < (int) bytesPerElement; b++)
				base[y * bytesPerRow + x * bytesPerElement + b] = patternByte(salt, x, y, b);
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
}

// Whether packed client pixels (as glGetTexImage returns them, rows without
// padding) hold the pattern.
static bool pixelsHavePattern(const uint8_t* pixels, int width, int height, int bytesPerElement, uint32_t salt)
{
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			for (int b = 0; b < bytesPerElement; b++)
				if (pixels[(y * width + x) * bytesPerElement + b] != patternByte(salt, x, y, b))
					return false;
	return true;
}

// Reads a texture level 0 with no row padding.
static void readTargetTexture(GLenum target, GLuint texture, GLenum format, GLenum type, void* pixels)
{
	glBindTexture(target, texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glGetTexImage(target, 0, format, type, pixels);
}

static void readTexture(GLuint texture, int width, int height, GLenum format, GLenum type, void* pixels)
{
	readTargetTexture(GL_TEXTURE_2D, texture, format, type, pixels);
	(void) width;
	(void) height;
}

// The 32-bit word of a pixel read under a read-only lock.
static uint32_t surfaceWord(IOSurfaceRef surface, size_t plane, int x, int y)
{
	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	const uint8_t* base = IOSurfaceGetBaseAddressOfPlane(surface, plane);
	size_t bytesPerRow = IOSurfaceGetBytesPerRowOfPlane(surface, plane);
	size_t bytesPerElement = IOSurfaceGetBytesPerElementOfPlane(surface, plane);
	uint32_t word = 0;
	memcpy(&word, base + y * bytesPerRow + x * bytesPerElement, bytesPerElement);
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	return word;
}

static GLuint g_framebuffer;

// Renders into a texture: clears it to one color, then a rectangle in the
// middle to another, so that the CPU check sees GL's rendering and not a
// stale fill.
static void renderTarget(GLenum target, GLuint texture, int width, int height, const float outer[4], const float inner[4])
{
	glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
	CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glViewport(0, 0, width, height);
	glClearColor(outer[0], outer[1], outer[2], outer[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	glScissor(width / 4, height / 4, width / 2, height / 2);
	glClearColor(inner[0], inner[1], inner[2], inner[3]);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	// Submitted, not waited for: the lock waits.
	glFlush();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render(GLuint texture, int width, int height, const float outer[4], const float inner[4])
{
	renderTarget(GL_TEXTURE_2D, texture, width, height, outer, inner);
}

static const float RED[4] = { 1, 0, 0, 1 };
static const float GREEN[4] = { 0, 1, 0, 1 };
static const float BLUE[4] = { 0, 0, 1, 1 };
static const float WHITE[4] = { 1, 1, 1, 1 };

static CGLContextObj createContext(void)
{
	CGLPixelFormatAttribute attrs[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &pixelFormat, &screens) != kCGLNoError
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError
		|| CGLSetCurrentContext(context) != kCGLNoError)
	{
		printf("%sFAIL: no offscreen context\n", g_who);
		finish(1);
	}
	CGLReleasePixelFormat(pixelFormat);
	glGenFramebuffers(1, &g_framebuffer);
	return context;
}

// Whether IOSurface exports this surface as a DMA-BUF. Looked up at run time,
// so that the program also runs on an IOSurface without the call.
static bool exportsDMABuf(IOSurfaceRef surface)
{
	__typeof__(_IOSurfaceGetDMABuf)* get = (__typeof__(_IOSurfaceGetDMABuf)*) dlsym(RTLD_DEFAULT, "_IOSurfaceGetDMABuf");
	_IOSurfaceDMABuf buffer = { .version = _IOSURFACE_DMABUF_VERSION };
	return get && get(surface, 0, &buffer) && buffer.fd >= 0;
}

struct format
{
	const char* name;
	OSType pixelFormat;
	GLenum format, type;
	// Bytes of each primary in memory: red, green, blue, alpha.
	int red, green, blue, alpha;
};

static const struct format FORMATS[] = {
	{ "BGRA", kCVPixelFormatType_32BGRA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 2, 1, 0, 3 },
	{ "RGBA", kCVPixelFormatType_32RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 0, 1, 2, 3 },
	{ "ARGB", kCVPixelFormatType_32ARGB, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, 1, 2, 3, 0 },
};

static uint32_t expectedWord(const struct format* format, const float color[4])
{
	uint8_t bytes[4];
	bytes[format->red] = (uint8_t) (color[0] * 255);
	bytes[format->green] = (uint8_t) (color[1] * 255);
	bytes[format->blue] = (uint8_t) (color[2] * 255);
	bytes[format->alpha] = (uint8_t) (color[3] * 255);
	uint32_t word;
	memcpy(&word, bytes, 4);
	return word;
}

// Returns whether the texture follows CPU writes without being specified
// again: a 2D texture that uses the surface's memory, or a rectangle
// texture that GPU copies keep in step with it.
static bool checkFormat(CGLContextObj context, const struct format* format, GLenum target)
{
	IOSurfaceRef surface = createSurface(W, H, 4, format->pixelFormat);
	CHECK(surface != NULL);
	if (!surface)
		return false;
	static uint8_t pixels[W * H * 4];

	writePattern(surface, 0, 1);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	CHECK(CGLTexImageIOSurface2D(context, target, GL_RGBA8, W, H, format->format, format->type,
		surface, 0) == kCGLNoError);

	// GL reads what the CPU wrote.
	readTargetTexture(target, texture, format->format, format->type, pixels);
	CHECK(pixelsHavePattern(pixels, W, H, 4, 1));

	// A CPU write after the call reaches a 2D texture right away only if it
	// uses the surface's memory, and a rectangle texture once the context
	// is locked only if GPU copies keep it in step. Specifying the texture
	// again brings it in anyway.
	writePattern(surface, 0, 2);
	readTargetTexture(target, texture, format->format, format->type, pixels);
	bool direct = pixelsHavePattern(pixels, W, H, 4, 2);
	if (!direct)
		CHECK(pixelsHavePattern(pixels, W, H, 4, 1));
	CGLLockContext(context);
	CGLUnlockContext(context);
	readTargetTexture(target, texture, format->format, format->type, pixels);
	bool follows = pixelsHavePattern(pixels, W, H, 4, 2);
	if (target == GL_TEXTURE_RECTANGLE_ARB)
		CHECK(!direct);
	else
		CHECK(follows == direct);
	const char* path = !follows ? "cpu-copy" : target == GL_TEXTURE_2D ? "dmabuf-2d" : "dmabuf-rect-gpucopy";
	CHECK(CGLTexImageIOSurface2D(context, target, GL_RGBA8, W, H, format->format, format->type,
		surface, 0) == kCGLNoError);
	readTargetTexture(target, texture, format->format, format->type, pixels);
	CHECK(pixelsHavePattern(pixels, W, H, 4, 2));

	// The CPU reads what GL rendered.
	renderTarget(target, texture, W, H, RED, GREEN);
	uint32_t corner = surfaceWord(surface, 0, 0, 0), middle = surfaceWord(surface, 0, W / 2, H / 2);
	printf("%s %s: path %s, rendered %08x %08x\n", format->name,
		target == GL_TEXTURE_2D ? "2D" : "RECT", path, corner, middle);
	CHECK(corner == expectedWord(format, RED));
	CHECK(middle == expectedWord(format, GREEN));

	// Kept in step both ways: a CPU write, then rendering on top of it.
	if (follows)
	{
		writePattern(surface, 0, 3);
		CGLLockContext(context);
		CGLUnlockContext(context);
		readTargetTexture(target, texture, format->format, format->type, pixels);
		CHECK(pixelsHavePattern(pixels, W, H, 4, 3));
		renderTarget(target, texture, W, H, BLUE, WHITE);
		CHECK(surfaceWord(surface, 0, 0, 0) == expectedWord(format, BLUE));
		CHECK(surfaceWord(surface, 0, W / 2, H / 2) == expectedWord(format, WHITE));
	}

	glDeleteTextures(1, &texture);
	CFRelease(surface);
	return follows;
}

static void checkBiPlanar(CGLContextObj context)
{
	IOSurfaceRef surface = createBiPlanar();
	CHECK(surface != NULL);
	if (!surface)
		return;
	CHECK(IOSurfaceGetPlaneCount(surface) == 2);
	static uint8_t pixels[W * H * 2];
	static const struct { GLenum internalFormat, format; int width, height, bytesPerElement; } planes[2] = {
		{ GL_R8, GL_RED, W, H, 1 },
		{ GL_RG8, GL_RG, W / 2, H / 2, 2 },
	};

	GLuint textures[2];
	glGenTextures(2, textures);
	bool imported[2];
	for (int i = 0; i < 2; i++)
	{
		writePattern(surface, i, 3 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, planes[i].internalFormat, planes[i].width,
			planes[i].height, planes[i].format, GL_UNSIGNED_BYTE, surface, i) == kCGLNoError);
		readTexture(textures[i], planes[i].width, planes[i].height, planes[i].format, GL_UNSIGNED_BYTE, pixels);
		CHECK(pixelsHavePattern(pixels, planes[i].width, planes[i].height, planes[i].bytesPerElement, 3 + i));
		writePattern(surface, i, 5 + i);
		readTexture(textures[i], planes[i].width, planes[i].height, planes[i].format, GL_UNSIGNED_BYTE, pixels);
		imported[i] = pixelsHavePattern(pixels, planes[i].width, planes[i].height, planes[i].bytesPerElement, 5 + i);
		if (!imported[i])
			CHECK(pixelsHavePattern(pixels, planes[i].width, planes[i].height, planes[i].bytesPerElement, 3 + i));
	}
	// Specifying the textures again brings in the CPU writes, and makes a
	// copied texture's rendering newer than them.
	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, planes[i].internalFormat, planes[i].width,
			planes[i].height, planes[i].format, GL_UNSIGNED_BYTE, surface, i) == kCGLNoError);
	}

	// Rendering into the chroma plane leaves the luma plane alone.
	render(textures[1], W / 2, H / 2, WHITE, BLUE);
	uint32_t corner = surfaceWord(surface, 1, 0, 0), middle = surfaceWord(surface, 1, W / 4, H / 4);
	printf("420v: GL textures %s/%s, rendered chroma %04x %04x\n", imported[0] ? "imported" : "copied",
		imported[1] ? "imported" : "copied", corner, middle);
	CHECK(corner == 0xffff);
	CHECK(middle == 0x0000);
	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	const uint8_t* luma = IOSurfaceGetBaseAddressOfPlane(surface, 0);
	CHECK(luma[0] == patternByte(5, 0, 0, 0));
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);

	glDeleteTextures(2, textures);
	CFRelease(surface);
}

// The message that carries the surface (or, from the child, its inbox).
struct message
{
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t port;
	mach_msg_port_descriptor_t bootstrap;
};

struct receivedMessage
{
	struct message message;
	mach_msg_trailer_t trailer;
};

static kern_return_t sendPort(mach_port_t destination, mach_port_t port, mach_msg_type_name_t disposition,
	mach_port_t bootstrap)
{
	struct message message;
	memset(&message, 0, sizeof(message));
	message.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX;
	message.header.msgh_size = sizeof(message);
	message.header.msgh_remote_port = destination;
	message.body.msgh_descriptor_count = 2;
	message.port.name = port;
	message.port.disposition = disposition;
	message.port.type = MACH_MSG_PORT_DESCRIPTOR;
	message.bootstrap.name = bootstrap;
	message.bootstrap.disposition = MACH_MSG_TYPE_COPY_SEND;
	message.bootstrap.type = MACH_MSG_PORT_DESCRIPTOR;
	return mach_msg(&message.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(message), 0,
		MACH_PORT_NULL, TIMEOUT_MS, MACH_PORT_NULL);
}

static bool receivePort(mach_port_t inbox, struct message* out)
{
	struct receivedMessage buffer;
	memset(&buffer, 0, sizeof(buffer));
	if (mach_msg(&buffer.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(buffer), inbox,
			TIMEOUT_MS, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return false;
	if (!(buffer.message.header.msgh_bits & MACH_MSGH_BITS_COMPLEX) || buffer.message.body.msgh_descriptor_count != 2)
		return false;
	*out = buffer.message;
	return MACH_PORT_VALID(out->port.name);
}

static const struct format* const BGRA = &FORMATS[0];

// The child gets the bootstrap special port pointing at the parent's
// rendezvous port, sends the parent its inbox, and receives the surface's
// port and the real bootstrap port. It touches no framework before that.
static void child(void)
{
	g_who = "child: ";
	alarm(60);
	mach_port_t task = mach_task_self();
	mach_port_t parent = MACH_PORT_NULL, inbox = MACH_PORT_NULL;
	if (task_get_special_port(task, TASK_BOOTSTRAP_PORT, &parent) != KERN_SUCCESS
		|| mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &inbox) != KERN_SUCCESS
		|| sendPort(parent, inbox, MACH_MSG_TYPE_MAKE_SEND, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
	{
		printf("child: FAIL: no connection to the parent\n");
		finish(1);
	}
	struct message message;
	if (!receivePort(inbox, &message))
	{
		printf("child: FAIL: no surface from the parent\n");
		finish(1);
	}
	if (MACH_PORT_VALID(message.bootstrap.name))
	{
		task_set_special_port(task, TASK_BOOTSTRAP_PORT, message.bootstrap.name);
		bootstrap_port = message.bootstrap.name;
	}

	IOSurfaceRef surface = IOSurfaceLookupFromMachPort(message.port.name);
	CHECK(surface != NULL);
	if (!surface)
		finish(1);
	printf("child: IOSurface DMA-BUF: %s\n", exportsDMABuf(surface) ? "yes" : "no");

	// The parent's GL rendering, made before it created the port.
	CHECK(surfaceWord(surface, 0, 0, 0) == expectedWord(BGRA, BLUE));
	CHECK(surfaceWord(surface, 0, W / 2, H / 2) == expectedWord(BGRA, WHITE));

	// Read through this process's own texture, then render into it.
	CGLContextObj context = createContext();
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		surface, 0) == kCGLNoError);
	static uint32_t pixels[W * H];
	readTexture(texture, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
	CHECK(pixels[0] == expectedWord(BGRA, BLUE));
	CHECK(pixels[(H / 2) * W + W / 2] == expectedWord(BGRA, WHITE));
	render(texture, W, H, GREEN, RED);
	// The lock brings the rendering into the surface for the parent.
	uint32_t corner = surfaceWord(surface, 0, 0, 0);
	printf("child: rendered %08x\n", corner);
	CHECK(corner == expectedWord(BGRA, GREEN));

	glDeleteTextures(1, &texture);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	CFRelease(surface);
	mach_port_deallocate(task, message.port.name);
	finish(g_failures == 0 ? 0 : 1);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(120);

	// Fork before any framework is used; the child reaches the parent
	// through the bootstrap special port it inherits.
	mach_port_t task = mach_task_self();
	mach_port_t rendezvous = MACH_PORT_NULL, bootstrap = MACH_PORT_NULL;
	CHECK(mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &rendezvous) == KERN_SUCCESS);
	CHECK(mach_port_insert_right(task, rendezvous, rendezvous, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS);
	task_get_special_port(task, TASK_BOOTSTRAP_PORT, &bootstrap);
	CHECK(task_set_special_port(task, TASK_BOOTSTRAP_PORT, rendezvous) == KERN_SUCCESS);
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0)
		child();
	g_child = pid;
	task_set_special_port(task, TASK_BOOTSTRAP_PORT, bootstrap);
	CHECK(pid > 0);
	if (pid < 0)
		finish(1);
	struct message hello;
	bool connected = receivePort(rendezvous, &hello);
	CHECK(connected);

	CGLContextObj context = createContext();
	const GLubyte* renderer = glGetString(GL_RENDERER);
	printf("GL renderer: %s\n", renderer ? (const char*) renderer : "?");

	bool imported = false;
	for (size_t i = 0; i < sizeof(FORMATS) / sizeof(FORMATS[0]); i++)
	{
		bool formatImported = checkFormat(context, &FORMATS[i], GL_TEXTURE_2D);
		if (i == 0)
			imported = formatImported;
		checkFormat(context, &FORMATS[i], GL_TEXTURE_RECTANGLE_ARB);
	}
	checkBiPlanar(context);

	// Another process sees the GL rendering and renders into the surface.
	IOSurfaceRef surface = createSurface(W, H, 4, BGRA->pixelFormat);
	CHECK(surface != NULL);
	if (!surface)
		finish(1);
	bool exported = exportsDMABuf(surface);
	printf("IOSurface DMA-BUF: %s\n", exported ? "yes" : "no");
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK(CGLTexImageIOSurface2D(context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
		surface, 0) == kCGLNoError);
	render(texture, W, H, BLUE, WHITE);
	// IOSurfaceCreateMachPort brings the rendering into the surface.
	mach_port_t port = IOSurfaceCreateMachPort(surface);
	CHECK(MACH_PORT_VALID(port));
	if (connected && MACH_PORT_VALID(port))
		CHECK(sendPort(hello.port.name, port, MACH_MSG_TYPE_MOVE_SEND, bootstrap) == MACH_MSG_SUCCESS);

	int status = 0;
	pid_t reaped = waitpid(pid, &status, 0);
	g_child = 0;
	CHECK(reaped == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	// The child's rendering, which it brought into the surface with a lock.
	uint32_t corner = surfaceWord(surface, 0, 0, 0), middle = surfaceWord(surface, 0, W / 2, H / 2);
	printf("after the child: %08x %08x\n", corner, middle);
	CHECK(corner == expectedWord(BGRA, GREEN));
	CHECK(middle == expectedWord(BGRA, RED));

	printf("backend: %s\n", exported && imported ? "dmabuf" : "copy");
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &g_framebuffer);
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	CFRelease(surface);

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
