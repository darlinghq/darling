// Copyright (C) 2026 Lubos Dolezel
//
// Rendering into a texture specified from an IOSurface is in the surface once
// the producer has flushed: glFlush only submits the work, but the surface's
// memory already holds the finished rendering on macOS, and a reader that
// starts right after the producer's glFlush (another thread's CPU lock, or
// another process) cannot wait for the producer's context. The cases:
//  - thread: a lock in another thread, started after glFlush returned, sees
//            the whole frame and a new seed;
//  - child:  a forked child that looks the surface up from a mach port, locks
//            it after glFlush returned, sees the whole frame and a new seed.
// The frame is a clear and several passes of an expensive fragment shader, then
// a cheap pass that covers the surface with one color, so that a frame that is
// still being drawn shows other pixels somewhere.
// Each case runs for a GL_TEXTURE_2D texture (the surface's memory, skipped where
// the target is not accepted, as on macOS) and a GL_TEXTURE_RECTANGLE texture.
//
// Argument --measure: prints the time glFlush takes for such a frame, for a
// surface only this process holds (no wait expected) and then for one that the
// forked child holds too.
// Argument --no-window-system: an offscreen renderer of its own, without AppKit.
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <servers/bootstrap.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_ARB
#endif

#define W 1024
#define H 1024
#define FINAL 0xff3070c0u
#define PASSES 6
#define ITERATIONS 400

static int g_failures;

#define CHECK(cond, what) do { \
	int ok_ = (cond); \
	printf("%s: %s\n", ok_ ? "PASS" : "FAIL", what); \
	if (!ok_) g_failures++; \
} while (0)

static void finish(int status) {
	fflush(stdout);
	_exit(status);
}

static void onAlarm(int sig) {
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value) {
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static IOSurfaceRef createSurface(void) {
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, W);
	setNumber(props, kIOSurfaceHeight, H);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	return surface;
}

// The number of pixels of the surface, under a read-only lock, that are not
// FINAL, or -1 if the lock fails.
static long wrongPixels(IOSurfaceRef surface) {
	if (IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) != kIOReturnSuccess)
		return -1;
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	long wrong = 0;
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			if (*(uint32_t *) (base + y * bytesPerRow + x * 4) != FINAL)
				wrong++;
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);
	return wrong;
}

static GLuint compile(GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	return ok ? shader : 0;
}

static GLuint program(const char *fragment) {
	static const char *vertex =
		"#version 150\n"
		"void main() {\n"
		"  vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);\n"
		"  gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);\n"
		"}\n";
	GLuint vs = compile(GL_VERTEX_SHADER, vertex), fs = compile(GL_FRAGMENT_SHADER, fragment);
	if (!vs || !fs)
		return 0;
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	GLint ok = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	return ok ? prog : 0;
}

struct producer {
	CGLContextObj context;
	GLuint texture, framebuffer, vao, heavy, solid;
	GLint count;
};

static int setUpProducer(struct producer *p, IOSurfaceRef surface, GLenum target) {
	static const char *heavy =
		"#version 150\n"
		"uniform int count;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"  float v = gl_FragCoord.x * 0.001;\n"
		"  for (int i = 0; i < count; i++)\n"
		"    v = sin(v * 1.0001 + float(i)) + 0.5 * cos(v);\n"
		"  color = vec4(fract(v), 0.25, 0.75, 1.0);\n"
		"}\n";
	static const char *solid =
		"#version 150\n"
		"out vec4 color;\n"
		"void main() { color = vec4(0x30 / 255.0, 0x70 / 255.0, 0xc0 / 255.0, 1.0); }\n";
	if (!p->heavy) {
		p->heavy = program(heavy);
		p->solid = program(solid);
		if (!p->heavy || !p->solid)
			return 0;
		glGenVertexArrays(1, &p->vao);
		glBindVertexArray(p->vao);
	}
	glGenTextures(1, &p->texture);
	glBindTexture(target, p->texture);
	if (CGLTexImageIOSurface2D(p->context, target, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) != kCGLNoError)
		return 0;
	glGenFramebuffers(1, &p->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, p->texture, 0);
	glViewport(0, 0, W, H);
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

// Submits a frame without flushing it.
static void drawFrame(struct producer *p) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(p->heavy);
	glUniform1i(glGetUniformLocation(p->heavy, "count"), ITERATIONS);
	for (int i = 0; i < PASSES; i++)
		glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(p->solid);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

static uint64_t nanoseconds(uint64_t ticks) {
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	return ticks * info.numer / info.denom;
}

static CGLContextObj createContext(void) {
	CGLPixelFormatAttribute attrs[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
		kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
		(CGLPixelFormatAttribute) 0
	};
	GLint screens = 0;
	CGLPixelFormatObj pixelFormat = NULL;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &pixelFormat, &screens) != kCGLNoError || pixelFormat == NULL
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError)
		return NULL;
	return context;
}

#define TIMEOUT_MS 30000

struct message {
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t port;
	mach_msg_port_descriptor_t bootstrap;
};

struct receivedMessage {
	struct message message;
	mach_msg_trailer_t trailer;
};

static kern_return_t sendPort(mach_port_t destination, mach_port_t port, mach_msg_type_name_t disposition, mach_port_t bootstrapPort) {
	struct message message;
	memset(&message, 0, sizeof(message));
	message.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX;
	message.header.msgh_size = sizeof(message);
	message.header.msgh_remote_port = destination;
	message.body.msgh_descriptor_count = 2;
	message.port.name = port;
	message.port.disposition = disposition;
	message.port.type = MACH_MSG_PORT_DESCRIPTOR;
	message.bootstrap.name = bootstrapPort;
	message.bootstrap.disposition = MACH_MSG_TYPE_COPY_SEND;
	message.bootstrap.type = MACH_MSG_PORT_DESCRIPTOR;
	return mach_msg(&message.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(message), 0, MACH_PORT_NULL,
		TIMEOUT_MS, MACH_PORT_NULL);
}

static bool receivePort(mach_port_t inbox, mach_port_t *out, mach_port_t *bootstrapOut) {
	struct receivedMessage buffer;
	memset(&buffer, 0, sizeof(buffer));
	if (mach_msg(&buffer.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(buffer), inbox, TIMEOUT_MS,
		MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return false;
	*out = buffer.message.port.name;
	if (bootstrapOut)
		*bootstrapOut = buffer.message.bootstrap.name;
	return MACH_PORT_VALID(*out);
}

// The child: for each surface the parent sends through a mach port (a
// message without a port ends it), tells the parent it is ready, and when
// the parent has flushed, locks and reads the surface and reports a status
// with a bit set for each problem: 1 partial frame, 2 seed unchanged,
// 8 no surface. Exits 4 if the connection to the parent fails.
static void child(int ready, int go, int result) {
	alarm(55);
	mach_port_t task = mach_task_self(), parent = MACH_PORT_NULL, inbox = MACH_PORT_NULL, port = MACH_PORT_NULL,
		realBootstrap = MACH_PORT_NULL;
	if (task_get_special_port(task, TASK_BOOTSTRAP_PORT, &parent) != KERN_SUCCESS
		|| mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &inbox) != KERN_SUCCESS
		|| sendPort(parent, inbox, MACH_MSG_TYPE_MAKE_SEND, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		_exit(4);
	for (int first = 1; ; first = 0) {
		if (!receivePort(inbox, &port, &realBootstrap))
			_exit(0);
		if (first && MACH_PORT_VALID(realBootstrap)) {
			task_set_special_port(task, TASK_BOOTSTRAP_PORT, realBootstrap);
			bootstrap_port = realBootstrap;
		}
		IOSurfaceRef shared = IOSurfaceLookupFromMachPort(port);
		char status = 0;
		if (shared == NULL) {
			status = 8;
		} else {
			uint32_t before = IOSurfaceGetSeed(shared);
			char byte = 0;
			if (write(ready, &byte, 1) != 1 || read(go, &byte, 1) != 1)
				_exit(4);
			if (wrongPixels(shared) != 0)
				status |= 1;
			if (IOSurfaceGetSeed(shared) == before)
				status |= 2;
			CFRelease(shared);
		}
		if (write(result, &status, 1) != 1)
			_exit(4);
	}
}

struct reader {
	IOSurfaceRef surface;
	uint32_t seedBefore;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int go;
	long wrong;
	uint32_t seedAfter;
};

static void *threadReader(void *arg) {
	struct reader *r = arg;
	pthread_mutex_lock(&r->mutex);
	while (!r->go)
		pthread_cond_wait(&r->cond, &r->mutex);
	pthread_mutex_unlock(&r->mutex);
	r->wrong = wrongPixels(r->surface);
	r->seedAfter = IOSurfaceGetSeed(r->surface);
	return NULL;
}

static void runCase(struct producer *p, IOSurfaceRef surface, GLenum target, const char *name, mach_port_t rendezvous,
	mach_port_t child, mach_port_t bootstrap, int readyFd, int goFd, int resultFd) {
	char what[128];
	if (!setUpProducer(p, surface, target)) {
		snprintf(what, sizeof(what), "%s: the texture is specified from the surface", name);
		CHECK(0, what);
		return;
	}

	// child: the surface goes to it through a mach port.
	mach_port_t port = IOSurfaceCreateMachPort(surface);
	CHECK(MACH_PORT_VALID(port), "IOSurfaceCreateMachPort");
	CHECK(sendPort(child, port, MACH_MSG_TYPE_MOVE_SEND, bootstrap) == MACH_MSG_SUCCESS, "the child got the surface");
	char byte = 0;
	CHECK(read(readyFd, &byte, 1) == 1, "the child is ready");
	drawFrame(p);
	glFlush();
	CHECK(write(goFd, "x", 1) == 1, "release the child");
	char status = -1;
	CHECK(read(resultFd, &status, 1) == 1, "the child reported");
	printf("%s: child status %d\n", name, status);
	snprintf(what, sizeof(what), "%s: child: a lock right after glFlush sees the whole frame", name);
	CHECK(!(status & 1), what);
	snprintf(what, sizeof(what), "%s: child: the seed changed", name);
	CHECK(!(status & 2), what);
	CHECK(!(status & ~3), "child: the surface was found");

	// thread
	struct reader r = { .surface = surface, .seedBefore = IOSurfaceGetSeed(surface),
		.mutex = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER };
	pthread_t thread;
	pthread_create(&thread, NULL, threadReader, &r);
	drawFrame(p);
	glFlush();
	pthread_mutex_lock(&r.mutex);
	r.go = 1;
	pthread_cond_signal(&r.cond);
	pthread_mutex_unlock(&r.mutex);
	pthread_join(thread, NULL);
	printf("%s: thread: %ld wrong pixels, seed %u -> %u\n", name, r.wrong, r.seedBefore, r.seedAfter);
	snprintf(what, sizeof(what), "%s: thread: a lock right after glFlush sees the whole frame", name);
	CHECK(r.wrong == 0, what);
	snprintf(what, sizeof(what), "%s: thread: the seed changed", name);
	CHECK(r.seedAfter != r.seedBefore, what);
	(void) rendezvous;
}

int main(int argc, char **argv) {
	signal(SIGALRM, onAlarm);
	alarm(58);
	int measure = 0, windowSystem = 1;
	for (int i = 1; i < argc; i++) {
		measure |= strcmp(argv[i], "--measure") == 0;
		if (strcmp(argv[i], "--no-window-system") == 0)
			windowSystem = 0;
	}

	// Fork before any framework is used, the child's bootstrap port being
	// the parent's rendezvous port.
	mach_port_t task = mach_task_self(), rendezvous = MACH_PORT_NULL, bootstrap = MACH_PORT_NULL;
	int readyPipe[2] = { -1, -1 }, goPipe[2] = { -1, -1 }, resultPipe[2] = { -1, -1 };
	pid_t pid = -1;
	{
		CHECK(mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &rendezvous) == KERN_SUCCESS, "rendezvous port");
		CHECK(mach_port_insert_right(task, rendezvous, rendezvous, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS,
			"rendezvous send right");
		task_get_special_port(task, TASK_BOOTSTRAP_PORT, &bootstrap);
		CHECK(task_set_special_port(task, TASK_BOOTSTRAP_PORT, rendezvous) == KERN_SUCCESS, "rendezvous bootstrap");
		CHECK(pipe(readyPipe) == 0 && pipe(goPipe) == 0 && pipe(resultPipe) == 0, "pipes");
		fflush(stdout);
		pid = fork();
		if (pid == 0)
			child(readyPipe[1], goPipe[0], resultPipe[1]);
		task_set_special_port(task, TASK_BOOTSTRAP_PORT, bootstrap);
		CHECK(pid > 0, "fork");
		if (pid < 0)
			finish(1);
	}

	if (windowSystem) {
		@autoreleasepool {
			[NSApplication sharedApplication];
		}
	}

	struct producer p = { 0 };
	p.context = createContext();
	IOSurfaceRef surface = createSurface();
	if (surface == NULL || p.context == NULL) {
		printf("FAIL: no surface or offscreen context\n");
		finish(1);
	}
	CGLSetCurrentContext(p.context);
	printf("renderer: %s\n", (const char *) glGetString(GL_RENDERER));

	if (measure) {
		if (!setUpProducer(&p, surface, GL_TEXTURE_2D)) {
			printf("FAIL: setup\n");
			finish(1);
		}
		// A surface that no other process can reach: glFlush does not wait
		// for the rendering (the finish at the flush is for readers in other
		// processes). Then the same for a surface with a mach port.
		for (int shared = 0; shared < 2; shared++) {
			const char *label = shared ? "shared" : "unshared";
			if (shared) {
				// A second process holds the surface.
				mach_port_t hello = MACH_PORT_NULL, port = IOSurfaceCreateMachPort(surface);
				char byte = 0;
				CHECK(MACH_PORT_VALID(port), "IOSurfaceCreateMachPort");
				CHECK(receivePort(rendezvous, &hello, NULL), "the child connected");
				CHECK(sendPort(hello, port, MACH_MSG_TYPE_MOVE_SEND, bootstrap) == MACH_MSG_SUCCESS,
					"the child got the surface");
				CHECK(read(readyPipe[0], &byte, 1) == 1, "the child is ready");
			}
			drawFrame(&p);
			glFinish();
			uint64_t flushTotal = 0, restTotal = 0;
			for (int i = 0; i < 10; i++) {
				drawFrame(&p);
				uint64_t start = mach_absolute_time();
				glFlush();
				uint64_t flushed = mach_absolute_time();
				glFinish();
				flushTotal += flushed - start;
				restTotal += mach_absolute_time() - flushed;
			}
			printf("%s: glFlush of a heavy frame: %llu us, the rest of the frame: %llu us\n", label,
				nanoseconds(flushTotal) / 10 / 1000, nanoseconds(restTotal) / 10 / 1000);
			// Only where the frame takes long enough to tell.
			if (!shared && nanoseconds(restTotal) / 10 > 20000000ull) {
				CHECK(flushTotal * 4 < restTotal, "unshared: glFlush does not wait for the rendering");
			}
			if (shared && nanoseconds(flushTotal + restTotal) / 10 > 20000000ull) {
				CHECK(restTotal * 4 < flushTotal, "shared: glFlush waits for the rendering");
			}
			uint64_t total = 0;
			for (int i = 0; i < 1000; i++) {
				glClear(GL_COLOR_BUFFER_BIT);
				uint64_t start = mach_absolute_time();
				glFlush();
				total += mach_absolute_time() - start;
			}
			printf("%s: glFlush of a clear: %llu ns\n", label, nanoseconds(total) / 1000);
		}
		finish(g_failures ? 1 : 0);
	}

	// The child connects first; its inbox receives every surface.
	mach_port_t hello = MACH_PORT_NULL;
	CHECK(receivePort(rendezvous, &hello, NULL), "the child connected");

	// A 2D texture uses the surface's memory (macOS rejects the target for
	// IOSurfaces, and the case is then skipped); a rectangle texture holds
	// a copy or uses the memory through a shadow.
	const GLenum targets[] = { GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE };
	const char *names[] = { "2d", "rectangle" };
	for (int i = 0; i < 2; i++) {
		if (targets[i] == GL_TEXTURE_2D) {
			GLuint probe = 0;
			glGenTextures(1, &probe);
			glBindTexture(GL_TEXTURE_2D, probe);
			CGLError error = CGLTexImageIOSurface2D(p.context, GL_TEXTURE_2D, GL_RGBA8, W, H, GL_BGRA,
				GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &probe);
			if (error != kCGLNoError) {
				printf("SKIP: GL_TEXTURE_2D is not accepted for IOSurfaces\n");
				continue;
			}
		}
		IOSurfaceRef caseSurface = createSurface();
		runCase(&p, caseSurface, targets[i], names[i], rendezvous, hello, bootstrap, readyPipe[0], goPipe[1],
			resultPipe[0]);
	}
	CHECK(sendPort(hello, MACH_PORT_NULL, MACH_MSG_TYPE_COPY_SEND, MACH_PORT_NULL) == MACH_MSG_SUCCESS, "end the child");
	int status = -1;
	CHECK(waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0, "the child exited");

	CGLSetCurrentContext(NULL);
	printf(g_failures ? "FAILED\n" : "PASSED\n");
	finish(g_failures ? 1 : 0);
}
