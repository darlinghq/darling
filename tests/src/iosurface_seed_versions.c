// Copyright (C) 2026 Lubos Dolezel
//
// When an IOSurface's seed changes, and when a lock with
// kIOSurfaceLockAvoidSync fails.
//
// The seed rules were observed on macOS 10.14.4 (x86_64, i386) and 26.7.3
// (x86_64): a new surface has seed 0; a lock for writing changes the seed
// when it is taken (nested ones too), the lock reports the new value and the
// unlock leaves it; read-only locks change nothing; an AvoidSync lock of an
// idle surface succeeds. OpenGL rendering into a texture specified from the
// surface does not change the seed until a flush, and does then, so a seed
// poller sees GL rendering without locking.
//
// In Darling a GPU API that keeps its own copy of a surface reports
// rendering into it (IOSurfacePriv.h); until the copy is written back, a lock
// with kIOSurfaceLockAvoidSync fails with kIOReturnCannotLock. That part uses
// the private interface and runs only on Darling. The state lives in the
// surface's shared memory, so a child process sees what the parent did and
// the reverse.
//
// The OpenGL part needs an offscreen context (a host EGL driver in Darling).
#define GL_SILENCE_DEPRECATION 1
#define GL_GLEXT_PROTOTYPES 1
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLIOSurface.h>
#include <dlfcn.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#if __has_include(<IOSurface/IOSurfacePriv.h>)
#include <IOSurface/IOSurfacePriv.h>
#define HAVE_PRIVATE_HEADER 1
#endif

#define W 16
#define H 8
#define TIMEOUT_MS 10000

#define RED 0xffff0000u
#define GREEN 0xff00ff00u
#define BLUE 0xff0000ffu

static int g_failures = 0;
static volatile pid_t g_child = 0;
static const char* g_who = "";

#define CHECK(cond) do { if (!(cond)) { printf("%sFAIL %d: %s\n", g_who, __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void onAlarm(int sig)
{
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	if (g_child > 0)
		kill(g_child, SIGKILL);
	_exit(1);
}

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static IOSurfaceRef createSurface(void)
{
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

static void fill(IOSurfaceRef surface, uint32_t value)
{
	CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
	unsigned char* base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			*(uint32_t*) (base + y * bytesPerRow + x * 4) = value;
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
}

// Locks with options, reads the first pixel and unlocks; returns the lock's
// result.
static kern_return_t lockAndRead(IOSurfaceRef surface, IOSurfaceLockOptions options, uint32_t* pixel)
{
	kern_return_t result = IOSurfaceLock(surface, options, NULL);
	if (result != kIOReturnSuccess)
		return result;
	if (pixel)
		*pixel = *(uint32_t*) IOSurfaceGetBaseAddress(surface);
	CHECK(IOSurfaceUnlock(surface, options, NULL) == kIOReturnSuccess);
	return result;
}

// The seed rules, through the public API only.
static void seedRules(void)
{
	printf("== seed rules\n");
	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (!surface)
		return;
	CHECK(IOSurfaceGetSeed(surface) == 0);

	// Read-only locks change nothing.
	uint32_t lockSeed = 99, unlockSeed = 99;
	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, &lockSeed) == kIOReturnSuccess);
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, &unlockSeed) == kIOReturnSuccess);
	CHECK(lockSeed == 0 && unlockSeed == 0 && IOSurfaceGetSeed(surface) == 0);

	// QUIRK test: the seed changes at the write lock, not at the unlock
	// (the header comment says the unlock increments it). The lock reports
	// the new value, and the unlock reports it unchanged.
	uint32_t seed = IOSurfaceGetSeed(surface);
	CHECK(IOSurfaceLock(surface, 0, &lockSeed) == kIOReturnSuccess);
	CHECK(lockSeed == seed + 1);
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);
	CHECK(IOSurfaceUnlock(surface, 0, &unlockSeed) == kIOReturnSuccess);
	CHECK(unlockSeed == seed + 1);
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);

	// Each nested write lock changes it.
	seed = IOSurfaceGetSeed(surface);
	uint32_t lock2 = 0, unlock2 = 0;
	CHECK(IOSurfaceLock(surface, 0, &lockSeed) == kIOReturnSuccess);
	CHECK(IOSurfaceLock(surface, 0, &lock2) == kIOReturnSuccess);
	CHECK(IOSurfaceUnlock(surface, 0, &unlockSeed) == kIOReturnSuccess);
	CHECK(IOSurfaceUnlock(surface, 0, &unlock2) == kIOReturnSuccess);
	printf("nested: %u %u %u %u (from %u)\n", lockSeed, lock2, unlockSeed, unlock2, seed);
	CHECK(lockSeed == seed + 1 && lock2 == seed + 2 && unlockSeed == seed + 2 && unlock2 == seed + 2);

	// An idle surface needs no synchronisation: AvoidSync locks succeed,
	// and only a lock for writing changes the seed.
	seed = IOSurfaceGetSeed(surface);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	CHECK(IOSurfaceGetSeed(surface) == seed);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync, NULL) == kIOReturnSuccess);
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);
	CFRelease(surface);
}

// The private interface, looked up at run time so that the program also
// runs on a Darling without it (and reports the failure there).
static void (*p_didWrite)(IOSurfaceRef);
static void (*p_didWriteMemory)(IOSurfaceRef);

static bool onDarling(void)
{
	return dlsym(RTLD_DEFAULT, "_IOSurfaceGetDMABuf") != NULL;
}

#ifdef HAVE_PRIVATE_HEADER
static IOSurfaceRef g_watched;
static int g_reportingWritebacks, g_legacyWritebacks;
static uint32_t g_copyValue;

// A GPU API that reports its writes: its writeback copies its contents in
// under a lock of its own, as OpenGL.framework's does.
static void reportingWriteback(IOSurfaceRef surface)
{
	if (surface != g_watched)
		return;
	g_reportingWritebacks++;
	if (g_copyValue != 0)
	{
		fill(surface, g_copyValue);
		g_copyValue = 0;
	}
}

// A GPU API that registers the original version and reports nothing.
static void legacyWriteback(IOSurfaceRef surface)
{
	if (surface == g_watched)
		g_legacyWritebacks++;
}

static void registerCallbacks(void)
{
	static const _IOSurfaceGPUCallbacks reporting = {
		.version = _IOSURFACE_GPU_CALLBACKS_VERSION_REPORTS_WRITES,
		.writeback = reportingWriteback,
		.forget = NULL,
	};
	static const _IOSurfaceGPUCallbacks legacy = {
		.version = _IOSURFACE_GPU_CALLBACKS_VERSION,
		.writeback = legacyWriteback,
		.forget = NULL,
	};
	_IOSurfaceSetGPUCallbacks(&reporting);
	_IOSurfaceSetGPUCallbacks(&legacy);
}
#endif

// GPU copies newer than the memory, and AvoidSync.
static void versions(void)
{
	printf("== versions\n");
#ifndef HAVE_PRIVATE_HEADER
	printf("private interface not in this SDK, skipped\n");
#else
	CHECK(p_didWrite != NULL && p_didWriteMemory != NULL);
	if (!p_didWrite || !p_didWriteMemory)
		return;
	registerCallbacks();
	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (!surface)
		return;
	g_watched = surface;
	fill(surface, GREEN);

	// GPU rendering into a copy: the memory has not changed, so the seed
	// has not, but an AvoidSync lock would need the copy written back.
	uint32_t seed = IOSurfaceGetSeed(surface);
	int reporting = g_reportingWritebacks, legacy = g_legacyWritebacks;
	p_didWrite(surface);
	CHECK(IOSurfaceGetSeed(surface) == seed);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnCannotLock);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync, NULL) == kIOReturnCannotLock);
	CHECK(IOSurfaceGetSeed(surface) == seed);
	CHECK(g_reportingWritebacks == reporting && g_legacyWritebacks == legacy);

	// A plain lock writes the copy back (under the writeback's own lock,
	// which changes the seed), and then the memory is current.
	uint32_t pixel = 0;
	g_copyValue = RED;
	CHECK(lockAndRead(surface, kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == RED);
	CHECK(g_reportingWritebacks == reporting + 1 && g_legacyWritebacks == legacy + 1);
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);

	// Now AvoidSync locks succeed. They skip the writeback of the API that
	// reports its writes, not that of the one that does not.
	seed = IOSurfaceGetSeed(surface);
	reporting = g_reportingWritebacks;
	legacy = g_legacyWritebacks;
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == RED && IOSurfaceGetSeed(surface) == seed);
	CHECK(g_reportingWritebacks == reporting && g_legacyWritebacks == legacy + 1);

	// A copy that the API chose not to write back (nothing copied) does not
	// keep AvoidSync locks failing after a plain lock.
	p_didWrite(surface);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnCannotLock);
	CHECK(lockAndRead(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);

	// A CPU write is newer than any GPU copy reported before it.
	p_didWrite(surface);
	fill(surface, BLUE);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == BLUE);

	// GPU rendering into the memory itself changes the seed and needs no
	// writeback.
	seed = IOSurfaceGetSeed(surface);
	p_didWriteMemory(surface);
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	// ... and is newer than a copy reported before it.
	p_didWrite(surface);
	p_didWriteMemory(surface);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);

	g_watched = NULL;
	CFRelease(surface);
#endif
}

// Cross-process: the parent sends the surface's Mach port to a child forked
// before any framework was used; the child reaches the parent through the
// bootstrap special port it inherits.

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

static kern_return_t sendPort(mach_port_t destination, mach_port_t port, mach_msg_type_name_t disposition, mach_port_t bootstrapPort)
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
	message.bootstrap.name = bootstrapPort;
	message.bootstrap.disposition = MACH_MSG_TYPE_COPY_SEND;
	message.bootstrap.type = MACH_MSG_PORT_DESCRIPTOR;
	return mach_msg(&message.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(message), 0, MACH_PORT_NULL,
		TIMEOUT_MS, MACH_PORT_NULL);
}

static bool receivePort(mach_port_t inbox, struct message* out)
{
	struct receivedMessage buffer;
	memset(&buffer, 0, sizeof(buffer));
	if (mach_msg(&buffer.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(buffer), inbox, TIMEOUT_MS,
			MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return false;
	if (!(buffer.message.header.msgh_bits & MACH_MSGH_BITS_COMPLEX) || buffer.message.body.msgh_descriptor_count != 2)
		return false;
	*out = buffer.message;
	return MACH_PORT_VALID(out->port.name);
}

// The child sees the parent's seed (which the parent wrote into the first
// pixel), takes a write lock (seed + 1), writes, and on Darling reports a
// GPU copy that the parent then sees as pending.
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
	uint32_t pixel = 0;
	CHECK(lockAndRead(surface, kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	uint32_t seed = IOSurfaceGetSeed(surface);
	CHECK(seed == pixel);

	uint32_t lockSeed = 0;
	CHECK(IOSurfaceLock(surface, 0, &lockSeed) == kIOReturnSuccess);
	CHECK(lockSeed == seed + 1);
	*(uint32_t*) IOSurfaceGetBaseAddress(surface) = GREEN;
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);

	if (onDarling())
	{
		CHECK(p_didWrite != NULL);
		if (p_didWrite)
			p_didWrite(surface);
	}

	CFRelease(surface);
	mach_port_deallocate(task, message.port.name);
	finish(g_failures == 0 ? 0 : 1);
}

static void crossProcess(pid_t pid, mach_port_t rendezvous, mach_port_t bootstrap)
{
	printf("== cross-process\n");
	struct message hello;
	bool connected = receivePort(rendezvous, &hello);
	CHECK(connected);

	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (!surface)
		finish(1);
	// The first pixel holds the seed the write lock set.
	uint32_t seed = 0;
	CHECK(IOSurfaceLock(surface, 0, &seed) == kIOReturnSuccess);
	*(uint32_t*) IOSurfaceGetBaseAddress(surface) = seed;
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);

	mach_port_t port = IOSurfaceCreateMachPort(surface);
	CHECK(MACH_PORT_VALID(port));
	if (connected && MACH_PORT_VALID(port))
		CHECK(sendPort(hello.port.name, port, MACH_MSG_TYPE_MOVE_SEND, bootstrap) == MACH_MSG_SUCCESS);

	int status = 0;
	pid_t reaped = waitpid(pid, &status, 0);
	g_child = 0;
	CHECK(reaped == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	// The child's write lock changed the seed once.
	CHECK(IOSurfaceGetSeed(surface) == seed + 1);
	uint32_t pixel = 0;
	if (onDarling())
	{
		// The child's GPU copy is newer than the memory here too.
		CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnCannotLock);
		CHECK(IOSurfaceGetSeed(surface) == seed + 1);
	}
	CHECK(lockAndRead(surface, kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == GREEN);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	CFRelease(surface);
}

// OpenGL rendering into a texture specified from the surface.
static void glCase(CGLContextObj context, GLenum target, const char* name)
{
	printf("== GL %s\n", name);
	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (!surface)
		return;
	fill(surface, GREEN);

	GLuint texture = 0, framebuffer = 0;
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	uint32_t seed = IOSurfaceGetSeed(surface);
	CGLError error = CGLTexImageIOSurface2D(context, target, GL_RGBA8, W, H, GL_BGRA,
		GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
	if (error != kCGLNoError)
	{
		// macOS takes only rectangle textures.
		CHECK(target == GL_TEXTURE_2D);
		printf("CGLTexImageIOSurface2D: %d, skipped\n", error);
		glDeleteTextures(1, &texture);
		CFRelease(surface);
		return;
	}
	CHECK(IOSurfaceGetSeed(surface) == seed);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
	CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glViewport(0, 0, W, H);

	// Rendering does not change the seed by itself, nor does unbinding the
	// framebuffer; the flush does. (The framebuffer is unbound before the
	// flush: Darling cannot tell whether a texture still attached to the
	// draw framebuffer has been drawn into since a flush, and counts it as
	// rendered into again at a later unbind or lock.)
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK(IOSurfaceGetSeed(surface) == seed);
	glFlush();
	uint32_t flushed = IOSurfaceGetSeed(surface);
	printf("seed: %u, after rendering red and glFlush: %u\n", seed, flushed);
	CHECK(flushed != seed);
	uint32_t pixel = 0;
	CHECK(lockAndRead(surface, kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == RED);
	CHECK(IOSurfaceGetSeed(surface) == flushed);

	// Again, and the flushed rendering needs no synchronisation.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFlush();
	CHECK(IOSurfaceGetSeed(surface) != flushed);
	flushed = IOSurfaceGetSeed(surface);
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == BLUE);
	CHECK(IOSurfaceGetSeed(surface) == flushed);

	// Rendering that has not been flushed: an AvoidSync lock may succeed
	// (macOS's software renderer renders into the surface itself), or
	// fail with kIOReturnCannotLock without changing anything, where the
	// rendering is in a copy (Darling's copy path).
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	kern_return_t result = lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, NULL);
	printf("AvoidSync lock after unflushed rendering: 0x%x\n", result);
	CHECK(result == kIOReturnSuccess || result == kIOReturnCannotLock);
	if (result == kIOReturnCannotLock)
		CHECK(IOSurfaceGetSeed(surface) == flushed);
	glFlush();
	CHECK(lockAndRead(surface, kIOSurfaceLockAvoidSync | kIOSurfaceLockReadOnly, &pixel) == kIOReturnSuccess);
	CHECK(pixel == GREEN);
	CHECK(IOSurfaceGetSeed(surface) != flushed);

	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &texture);
	CFRelease(surface);
}

static void gl(void)
{
	CGLPixelFormatAttribute attrs[] = { kCGLPFAAllowOfflineRenderers, 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(attrs, &pixelFormat, &screens) != kCGLNoError || !pixelFormat
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError
		|| CGLSetCurrentContext(context) != kCGLNoError)
	{
		printf("FAIL: no offscreen OpenGL context\n");
		g_failures++;
		return;
	}
	printf("GL renderer: %s\n", (const char*) glGetString(GL_RENDERER));
	glCase(context, GL_TEXTURE_RECTANGLE_ARB, "rectangle");
	glCase(context, GL_TEXTURE_2D, "2D");
	CGLSetCurrentContext(NULL);
	CGLDestroyContext(context);
	CGLReleasePixelFormat(pixelFormat);
}

int main(void)
{
	signal(SIGALRM, onAlarm);
	alarm(120);

	p_didWrite = (void (*)(IOSurfaceRef)) dlsym(RTLD_DEFAULT, "_IOSurfaceGPUDidWrite");
	p_didWriteMemory = (void (*)(IOSurfaceRef)) dlsym(RTLD_DEFAULT, "_IOSurfaceGPUDidWriteMemory");

	// Fork before any framework is used; the child inherits the bootstrap
	// special port, which points to the parent's rendezvous port meanwhile.
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

	printf("on Darling: %s\n", onDarling() ? "yes" : "no");
	crossProcess(pid, rendezvous, bootstrap);
	seedRules();
	if (onDarling())
		versions();
	gl();

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
