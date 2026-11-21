// Copyright (C) 2026 Lubos Dolezel
//
// An IOSurface that OpenGL renders into in one process and that other
// processes sample with OpenGL and read under CPU locks, shared through Mach
// ports (IOSurfaceCreateMachPort) and through global IDs (IOSurfaceLookup).
// Three processes: the parent forks a producer and a consumer before any of
// them uses a framework (GPU drivers cannot be used after a fork), then
// coordinates them over pipes and is a second consumer itself.
//
// Public-API checks, which hold on every renderer:
//  - port:     both consumers see the surface the producer rendered, with
//              its ID and seed; GL in each consumer samples the rendering;
//  - flush:    new rendering, once flushed, changes the seed every process
//              sees, GL in a consumer samples it, and a read-only lock with
//              kIOSurfaceLockAvoidSync in a consumer succeeds and sees it;
//  - write:    a CPU write in a consumer is what the producer's GL and CPU
//              see afterwards, with the seed the consumer's lock returned;
//  - global:   a surface made with kIOSurfaceIsGlobal is found by its ID in
//              the other processes; one made without it is not;
//  - exits:    after the consumer exits, and after the producer exits, the
//              remaining process still sees the last rendering (CPU and GL).
// Darling-only checks, through private interfaces where they exist:
//  - the IOSurface service in darlingserver counts the holders of a
//    surface, frees the surface with the last of them (also after process
//    exits), and how many processes read the memory;
//  - where the surface has a GPU buffer (NVIDIA), every process that
//    samples it uses that buffer: rendering reaches the consumers with no
//    copy between the buffer and the memory, a lock in a consumer copies
//    once, and a CPU write is one copy into the buffer;
//  - releasing a surface that was looked up, sampled and locked leaves no
//    descriptor open in the process (/proc/self/fd).
//
// With --measure, only a 1920x1080 surface is timed instead: the time from
// the producer's glFlush to the consumer's sample of the new rendering, and
// the copies made (medians of 20).
//
// Needs an offscreen OpenGL context. AppKit is set up first in each process
// so that the window system's display is the one used, unless the argument
// --no-window-system is given (an offscreen renderer of its own).
#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION 1
#import <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLIOSurface.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#if __has_include(<IOSurface/IOSurfacePriv.h>)
#include <IOSurface/IOSurfacePriv.h>
#define HAVE_PRIVATE_HEADER 1
#endif

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define W 64
#define H 32
#define BIG_W 1920
#define BIG_H 1080
#define TIMED_RUNS 20
#define TIMEOUT_MS 20000

#define GREY 0xff808080u
#define RED 0xffff0000u
#define GREEN 0xff00ff00u
#define BLUE 0xff0000ffu
#define YELLOW 0xffffff00u

static int g_failures;
static const char *g_who = "";
static bool g_windowSystem = true;
static CGLContextObj g_context;
static pid_t g_children[2];

#define CHECK(cond) do { if (!(cond)) { printf("%sFAIL %d: %s\n", g_who, __LINE__, #cond); g_failures++; } } while (0)
#define EXPECT_PIXEL(actual, expected) do { uint32_t a_ = (actual), e_ = (expected); \
	if (a_ != e_) { printf("%sFAIL %d: pixel %08x, expected %08x\n", g_who, __LINE__, a_, e_); g_failures++; } } while (0)

static void onAlarm(int sig) {
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	for (int i = 0; i < 2; i++)
		if (g_children[i] > 0)
			kill(g_children[i], SIGKILL);
	_exit(1);
}

static double nowMs(void) {
	static mach_timebase_info_data_t timebase;
	if (timebase.denom == 0)
		mach_timebase_info(&timebase);
	return (double) mach_absolute_time() * timebase.numer / timebase.denom / 1e6;
}

static int compareDoubles(const void *a, const void *b) {
	double x = *(const double *) a, y = *(const double *) b;
	return x < y ? -1 : x > y;
}

static double median(double *values, int count) {
	qsort(values, (size_t) count, sizeof(double), compareDoubles);
	return values[count / 2];
}

// Darling's private interfaces, where they exist.

struct copies {
	bool known, hasBuffer;
	uint64_t downloads, uploads;
};

static struct copies copiesOf(IOSurfaceRef surface) {
	struct copies result = { false, false, 0, 0 };
#ifdef HAVE_PRIVATE_HEADER
	typedef bool (*statistics_proc)(IOSurfaceRef, _IOSurfaceGPUBufferStatistics *);
	statistics_proc statistics = (statistics_proc) dlsym(RTLD_DEFAULT, "_IOSurfaceGetGPUBufferStatistics");
	_IOSurfaceGPUBufferStatistics s = { .version = _IOSURFACE_GPU_BUFFER_VERSION };
	if (statistics && statistics(surface, &s)) {
		result.known = true;
		result.hasBuffer = s.hasBuffer;
		result.downloads = s.downloads;
		result.uploads = s.uploads;
	}
#endif
	return result;
}

static void printCopies(const char *what, IOSurfaceRef surface) {
	struct copies c = copiesOf(surface);
	if (c.known)
		printf("%s%s: GPU buffer %s, downloads %llu, uploads %llu\n", g_who, what, c.hasBuffer ? "yes" : "no",
			(unsigned long long) c.downloads, (unsigned long long) c.uploads);
}

// The IOSurface service's view of a surface: holders, processes reading the
// memory, flags (1 global, 2 GPU buffer). Returns false where there is no
// service; *gone is set when the service does not know the surface.
struct serviceState {
	bool known, gone;
	uint32_t holders, memoryUsers, flags;
};

static struct serviceState serviceStateOf(uint32_t surfaceID) {
	struct serviceState state = { false, false, 0, 0, 0 };
	typedef int (*get_state_proc)(uint32_t, uint32_t *, uint32_t *, uint32_t *);
	get_state_proc getState = (get_state_proc) dlsym(RTLD_DEFAULT, "__darling_iosurface_get_state");
	if (!getState)
		return state;
	state.known = true;
	int error = getState(surfaceID, &state.holders, &state.memoryUsers, &state.flags);
	state.gone = error != 0;
	return state;
}

static void printState(const char *what, uint32_t surfaceID) {
	struct serviceState s = serviceStateOf(surfaceID);
	if (!s.known)
		printf("%s%s: no IOSurface service\n", g_who, what);
	else if (s.gone)
		printf("%s%s: surface %u not in the service\n", g_who, what, surfaceID);
	else
		printf("%s%s: surface %u: %u holders, %u reading the memory, flags %u\n", g_who, what, surfaceID,
			s.holders, s.memoryUsers, s.flags);
}

// Open descriptors of this process.
static int openDescriptors(void) {
	int count = 0;
	DIR *dir = opendir("/proc/self/fd");
	if (dir) {
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
			if (entry->d_name[0] != '.')
				count++;
		closedir(dir);
		return count - 1; // the directory's own descriptor
	}
	for (int fd = 0; fd < 4096; fd++)
		if (fcntl(fd, F_GETFD) != -1)
			count++;
	return count;
}

// Surfaces and pixels.

static void setNumber(CFMutableDictionaryRef dict, CFStringRef key, int value) {
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

static void fill(IOSurfaceRef surface, uint32_t value, uint32_t *seed) {
	IOSurfaceLock(surface, 0, seed);
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	size_t width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface);
	for (size_t y = 0; y < height; y++)
		for (size_t x = 0; x < width; x++)
			*(uint32_t *) (base + y * bytesPerRow + x * 4) = value;
	IOSurfaceUnlock(surface, 0, NULL);
}

static IOSurfaceRef createSurface(int width, int height, bool global) {
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setNumber(props, kIOSurfaceWidth, width);
	setNumber(props, kIOSurfaceHeight, height);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	setNumber(props, kIOSurfacePixelFormat, 'BGRA');
	if (global)
		CFDictionarySetValue(props, kIOSurfaceIsGlobal, kCFBooleanTrue);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	if (surface != NULL)
		fill(surface, GREY, NULL);
	return surface;
}

// The first and last pixel under a lock with options: their common value,
// 0xdeadbeef if they differ, or 0 if the lock fails (its result in *result).
static uint32_t lockedPixel(IOSurfaceRef surface, IOSurfaceLockOptions options, kern_return_t *result) {
	kern_return_t locked = IOSurfaceLock(surface, options, NULL);
	if (result)
		*result = locked;
	if (locked != kIOReturnSuccess)
		return 0;
	unsigned char *base = IOSurfaceGetBaseAddress(surface);
	size_t bytesPerRow = IOSurfaceGetBytesPerRow(surface);
	size_t width = IOSurfaceGetWidth(surface), height = IOSurfaceGetHeight(surface);
	uint32_t first = *(uint32_t *) base;
	uint32_t last = *(uint32_t *) (base + (height - 1) * bytesPerRow + (width - 1) * 4);
	IOSurfaceUnlock(surface, options, NULL);
	return first == last ? first : 0xdeadbeefu;
}

// OpenGL.

static bool startGL(void) {
	if (g_windowSystem) {
		@autoreleasepool {
			[NSApplication sharedApplication];
		}
	}
	CGLPixelFormatAttribute attrs[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
		kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
		(CGLPixelFormatAttribute) 0
	};
	CGLPixelFormatObj format = NULL;
	GLint screens = 0;
	if (CGLChoosePixelFormat(attrs, &format, &screens) != kCGLNoError || format == NULL)
		return false;
	CGLError error = CGLCreateContext(format, NULL, &g_context);
	CGLReleasePixelFormat(format);
	if (error != kCGLNoError)
		return false;
	CGLSetCurrentContext(g_context);
	return true;
}

struct producer {
	GLuint texture, framebuffer;
	int width, height;
};

// A 2D texture specified from the surface and a framebuffer with it.
static bool startProducer(struct producer *p, IOSurfaceRef surface) {
	p->width = (int) IOSurfaceGetWidth(surface);
	p->height = (int) IOSurfaceGetHeight(surface);
	glGenTextures(1, &p->texture);
	glBindTexture(GL_TEXTURE_2D, p->texture);
	CGLError error = CGLTexImageIOSurface2D(g_context, GL_TEXTURE_2D, GL_RGBA8, p->width, p->height,
		GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (error != kCGLNoError)
		return false;
	glGenFramebuffers(1, &p->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p->texture, 0);
	bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return complete;
}

static void stopProducer(struct producer *p) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &p->framebuffer);
	glDeleteTextures(1, &p->texture);
	p->framebuffer = p->texture = 0;
}

static void renderAndFlush(struct producer *p, uint32_t color) {
	glBindFramebuffer(GL_FRAMEBUFFER, p->framebuffer);
	glViewport(0, 0, p->width, p->height);
	glClearColor(((color >> 16) & 0xff) / 255.0f, ((color >> 8) & 0xff) / 255.0f, (color & 0xff) / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFlush();
}

// The common value of the first and last texel of a 2D texture, read
// through a read framebuffer only (Darling counts a texture attached to the
// draw framebuffer as rendered into).
static uint32_t readTexture(GLuint texture, int width, int height) {
	GLuint framebuffer = 0;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	uint32_t first = 0, last = 1;
	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		glReadPixels(0, 0, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &first);
		glReadPixels(width - 1, height - 1, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &last);
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	return first == last ? first : 0xdeadbeefu;
}

// What GL samples from the surface through a new 2D texture.
static uint32_t sampledPixel(IOSurfaceRef surface) {
	int width = (int) IOSurfaceGetWidth(surface), height = (int) IOSurfaceGetHeight(surface);
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	uint32_t result = 0xdeadbeefu;
	if (CGLTexImageIOSurface2D(g_context, GL_TEXTURE_2D, GL_RGBA8, width, height, GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV, surface, 0) == kCGLNoError)
		result = readTexture(texture, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);
	return result;
}

// Coordination: commands over pipes, Mach ports over the parent's
// rendezvous port, which the children inherit as their bootstrap port.

enum command {
	CMD_HELLO = 1,
	CMD_READY,
	CMD_SAMPLE,
	CMD_RENDER,
	CMD_WRITE,
	CMD_CHECK,
	CMD_GLOBAL,
	CMD_LOOKUP,
	CMD_EXIT,
	CMD_FAILED,
	CMD_MEASURE,
	CMD_FRAME,
};

struct command_message {
	int32_t command;
	uint32_t a, b, c;
	double time;
};

struct pipes {
	int toChild, fromChild;   // in the parent
	int toParent, fromParent; // in the child
};

static void sendCommand(int fd, int32_t command, uint32_t a, uint32_t b, uint32_t c) {
	struct command_message m = { command, a, b, c, nowMs() };
	if (write(fd, &m, sizeof(m)) != (ssize_t) sizeof(m)) {
		printf("%sFAIL: pipe write\n", g_who);
		_exit(1);
	}
}

static struct command_message receiveCommand(int fd) {
	struct command_message m;
	size_t got = 0;
	while (got < sizeof(m)) {
		ssize_t n = read(fd, (char *) &m + got, sizeof(m) - got);
		if (n <= 0) {
			printf("%sFAIL: pipe closed\n", g_who);
			_exit(1);
		}
		got += (size_t) n;
	}
	return m;
}

struct port_message {
	mach_msg_header_t header;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t ports[2];
};

struct received_port_message {
	struct port_message message;
	mach_msg_trailer_t trailer;
};

static kern_return_t sendPorts(mach_port_t destination, mach_port_t first, mach_msg_type_name_t firstDisposition,
	mach_port_t second, mach_msg_type_name_t secondDisposition) {
	struct port_message message;
	memset(&message, 0, sizeof(message));
	message.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX;
	message.header.msgh_size = sizeof(message);
	message.header.msgh_remote_port = destination;
	message.body.msgh_descriptor_count = 2;
	message.ports[0].name = first;
	message.ports[0].disposition = firstDisposition;
	message.ports[0].type = MACH_MSG_PORT_DESCRIPTOR;
	message.ports[1].name = second;
	message.ports[1].disposition = secondDisposition;
	message.ports[1].type = MACH_MSG_PORT_DESCRIPTOR;
	return mach_msg(&message.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(message), 0, MACH_PORT_NULL,
		TIMEOUT_MS, MACH_PORT_NULL);
}

static bool receivePorts(mach_port_t inbox, mach_port_t *first, mach_port_t *second) {
	struct received_port_message buffer;
	memset(&buffer, 0, sizeof(buffer));
	if (mach_msg(&buffer.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(buffer), inbox, TIMEOUT_MS,
			MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return false;
	if (!(buffer.message.header.msgh_bits & MACH_MSGH_BITS_COMPLEX) || buffer.message.body.msgh_descriptor_count != 2)
		return false;
	*first = buffer.message.ports[0].name;
	*second = buffer.message.ports[1].name;
	return true;
}

// A child's connection: its inbox, announced to the parent through the
// inherited bootstrap port, which the parent replaces with the real one.
static mach_port_t connectChild(void) {
	mach_port_t task = mach_task_self();
	mach_port_t parent = MACH_PORT_NULL, inbox = MACH_PORT_NULL;
	if (task_get_special_port(task, TASK_BOOTSTRAP_PORT, &parent) != KERN_SUCCESS
		|| mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &inbox) != KERN_SUCCESS
		|| sendPorts(parent, inbox, MACH_MSG_TYPE_MAKE_SEND, MACH_PORT_NULL, MACH_MSG_TYPE_COPY_SEND) != MACH_MSG_SUCCESS) {
		printf("%sFAIL: no connection to the parent\n", g_who);
		_exit(1);
	}
	mach_port_t bootstrap = MACH_PORT_NULL, unused = MACH_PORT_NULL;
	if (!receivePorts(inbox, &bootstrap, &unused)) {
		printf("%sFAIL: no reply from the parent\n", g_who);
		_exit(1);
	}
	if (MACH_PORT_VALID(bootstrap)) {
		task_set_special_port(task, TASK_BOOTSTRAP_PORT, bootstrap);
		bootstrap_port = bootstrap;
	}
	return inbox;
}

static void finishChild(struct pipes *p) {
	(void) p;
	fflush(stdout);
	_exit(g_failures == 0 ? 0 : 1);
}

// The producer.
static void producerMain(struct pipes *pp, bool measure) {
	g_who = "producer: ";
	alarm(55);
	mach_port_t inbox = connectChild();
	if (!startGL()) {
		printf("%sFAIL: no offscreen context\n", g_who);
		sendCommand(pp->toParent, CMD_FAILED, 0, 0, 0);
		finishChild(pp);
	}
	printf("%srenderer: %s\n", g_who, (const char *) glGetString(GL_RENDERER));

	IOSurfaceRef s = createSurface(measure ? BIG_W : W, measure ? BIG_H : H, false);
	struct producer p;
	CHECK(s != NULL);
	if (!s || !startProducer(&p, s)) {
		printf("%sFAIL: no producer texture\n", g_who);
		sendCommand(pp->toParent, CMD_FAILED, 0, 0, 0);
		finishChild(pp);
	}
	renderAndFlush(&p, RED);
	printCopies("after the first flush", s);

	// Two ports, one for each consumer.
	mach_port_t ports[2] = { IOSurfaceCreateMachPort(s), IOSurfaceCreateMachPort(s) };
	CHECK(MACH_PORT_VALID(ports[0]) && MACH_PORT_VALID(ports[1]));
	sendCommand(pp->toParent, CMD_READY, IOSurfaceGetID(s), IOSurfaceGetSeed(s), 0);
	// The parent tells which inbox to send to by sending its own first.
	mach_port_t rendezvous = MACH_PORT_NULL, unused = MACH_PORT_NULL;
	if (!receivePorts(inbox, &rendezvous, &unused) || sendPorts(rendezvous, ports[0], MACH_MSG_TYPE_MOVE_SEND,
			ports[1], MACH_MSG_TYPE_MOVE_SEND) != MACH_MSG_SUCCESS) {
		printf("%sFAIL: ports not sent\n", g_who);
		finishChild(pp);
	}

	IOSurfaceRef global = NULL, local = NULL;
	struct producer g = { 0 };
	for (;;) {
		struct command_message m = receiveCommand(pp->fromParent);
		switch (m.command) {
		case CMD_RENDER: {
			struct copies before = copiesOf(s);
			renderAndFlush(&p, m.a);
			struct copies after = copiesOf(s);
			printCopies("after a flush", s);
			sendCommand(pp->toParent, CMD_READY, IOSurfaceGetSeed(s),
				(uint32_t) (after.downloads - before.downloads), after.hasBuffer);
			break;
		}
		case CMD_CHECK: {
			// What the producer's GL and CPU see now, after a write elsewhere:
			// the texture is specified again, as an application does when the
			// seed changed.
			glBindTexture(GL_TEXTURE_2D, p.texture);
			CHECK(CGLTexImageIOSurface2D(g_context, GL_TEXTURE_2D, GL_RGBA8, p.width, p.height, GL_BGRA,
				GL_UNSIGNED_INT_8_8_8_8_REV, s, 0) == kCGLNoError);
			glBindTexture(GL_TEXTURE_2D, 0);
			EXPECT_PIXEL(readTexture(p.texture, p.width, p.height), m.a);
			uint32_t seed = 0;
			IOSurfaceLock(s, kIOSurfaceLockReadOnly, &seed);
			IOSurfaceUnlock(s, kIOSurfaceLockReadOnly, NULL);
			CHECK(seed == m.b);
			EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), m.a);
			printCopies("after the consumer's write", s);
			sendCommand(pp->toParent, CMD_READY, 0, 0, 0);
			break;
		}
		case CMD_GLOBAL: {
			global = createSurface(W, H, true);
			local = createSurface(W, H, false);
			CHECK(global != NULL && local != NULL);
			if (global && startProducer(&g, global))
				renderAndFlush(&g, RED);
			sendCommand(pp->toParent, CMD_READY, global ? IOSurfaceGetID(global) : 0,
				local ? IOSurfaceGetID(local) : 0, 0);
			break;
		}
		case CMD_MEASURE: {
			// Each frame: a new colour, flushed, then the time is sent and
			// the consumer samples it.
			static const uint32_t colors[] = { RED, GREEN, BLUE, YELLOW };
			struct copies before = copiesOf(s);
			for (int i = 0; i < TIMED_RUNS + 2; i++) {
				// The first two frames set up the consumer's texture.
				if (i == 2)
					before = copiesOf(s);
				uint32_t color = colors[i % 4];
				double t0 = nowMs();
				renderAndFlush(&p, color);
				struct command_message frame = { CMD_FRAME, color, 0, 0, t0 };
				write(pp->toParent, &frame, sizeof(frame));
				receiveCommand(pp->fromParent);
			}
			struct copies after = copiesOf(s);
			sendCommand(pp->toParent, CMD_READY, (uint32_t) (after.downloads - before.downloads),
				(uint32_t) (after.uploads - before.uploads), after.hasBuffer);
			break;
		}
		case CMD_EXIT:
			// The producer exits holding the surfaces; nothing is released.
			fflush(stdout);
			_exit(g_failures == 0 ? 0 : 1);
		default:
			printf("%sFAIL: unexpected command %d\n", g_who, m.command);
			finishChild(pp);
		}
	}
}

// The consumer.
static void consumerMain(struct pipes *pp, bool measure) {
	(void) measure;
	g_who = "consumer: ";
	alarm(55);
	mach_port_t inbox = connectChild();
	if (!startGL()) {
		printf("%sFAIL: no offscreen context\n", g_who);
		sendCommand(pp->toParent, CMD_FAILED, 0, 0, 0);
		finishChild(pp);
	}
	sendCommand(pp->toParent, CMD_HELLO, 0, 0, 0);

	mach_port_t port = MACH_PORT_NULL, unused = MACH_PORT_NULL;
	if (!receivePorts(inbox, &port, &unused)) {
		printf("%sFAIL: no surface port\n", g_who);
		finishChild(pp);
	}
	IOSurfaceRef s = IOSurfaceLookupFromMachPort(port);
	mach_port_deallocate(mach_task_self(), port);
	CHECK(s != NULL);
	if (!s)
		finishChild(pp);

	IOSurfaceRef global = NULL;
	for (;;) {
		struct command_message m = receiveCommand(pp->fromParent);
		switch (m.command) {
		case CMD_SAMPLE: {
			// m.a: the colour, m.b: the seed the producer saw, m.c: whether
			// the lock comes first.
			CHECK(IOSurfaceGetID(s) != 0);
			CHECK(IOSurfaceGetSeed(s) == m.b);
			struct copies before = copiesOf(s);
			EXPECT_PIXEL(sampledPixel(s), m.a);
			struct copies sampled = copiesOf(s);
			kern_return_t result = 0;
			EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly | kIOSurfaceLockAvoidSync, &result), m.a);
			CHECK(result == kIOReturnSuccess);
			struct copies locked = copiesOf(s);
			EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), m.a);
			struct copies again = copiesOf(s);
			CHECK(IOSurfaceGetSeed(s) == m.b);
			printCopies("after sampling and locks", s);
			if (sampled.hasBuffer) {
				// GL samples the buffer: nothing is copied for it.
				CHECK(sampled.downloads == before.downloads && sampled.uploads == before.uploads);
				// The first lock copies at most once, the second not at all.
				CHECK(locked.downloads <= sampled.downloads + 1);
				CHECK(again.downloads == locked.downloads);
			}
			sendCommand(pp->toParent, CMD_READY, sampled.hasBuffer, (uint32_t) (sampled.downloads - before.downloads),
				(uint32_t) (locked.downloads - sampled.downloads));
			break;
		}
		case CMD_WRITE: {
			uint32_t seed = 0;
			struct copies before = copiesOf(s);
			fill(s, m.a, &seed);
			struct copies after = copiesOf(s);
			if (after.hasBuffer)
				CHECK(after.uploads == before.uploads + 1);
			sendCommand(pp->toParent, CMD_READY, seed, 0, 0);
			break;
		}
		case CMD_LOOKUP: {
			// m.a: a global surface's ID, m.b: one that is not global.
			global = IOSurfaceLookup(m.a);
			CHECK(global != NULL);
			if (global) {
				CHECK(IOSurfaceGetID(global) == m.a);
				EXPECT_PIXEL(sampledPixel(global), RED);
				EXPECT_PIXEL(lockedPixel(global, kIOSurfaceLockReadOnly, NULL), RED);
			}
			IOSurfaceRef local = IOSurfaceLookup(m.b);
			CHECK(local == NULL);
			if (local)
				CFRelease(local);
			if (global)
				CFRelease(global);
			global = NULL;
			sendCommand(pp->toParent, CMD_READY, 0, 0, 0);
			break;
		}
		case CMD_MEASURE: {
			// One sample per frame, through the same texture specified again
			// each time, as a compositor does when the seed changes.
			double latency[TIMED_RUNS];
			int width = (int) IOSurfaceGetWidth(s), height = (int) IOSurfaceGetHeight(s);
			GLuint texture = 0;
			glGenTextures(1, &texture);
			struct copies before = copiesOf(s);
			int failures = g_failures;
			for (int i = 0; i < TIMED_RUNS + 2; i++) {
				if (i == 2)
					before = copiesOf(s);
				struct command_message frame = receiveCommand(pp->fromParent);
				glBindTexture(GL_TEXTURE_2D, texture);
				CGLTexImageIOSurface2D(g_context, GL_TEXTURE_2D, GL_RGBA8, width, height, GL_BGRA,
					GL_UNSIGNED_INT_8_8_8_8_REV, s, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
				uint32_t pixel = readTexture(texture, width, height);
				double t1 = nowMs();
				if (g_failures == failures)
					EXPECT_PIXEL(pixel, frame.a);
				// The first two frames set up the texture and the buffer.
				if (i >= 2)
					latency[i - 2] = t1 - frame.time;
				sendCommand(pp->toParent, CMD_READY, 0, 0, 0);
			}
			glDeleteTextures(1, &texture);
			struct copies after = copiesOf(s);
			printf("%s%dx%d, median of %d: producer flush to consumer sample %.2f ms; consumer copies: "
				"downloads %llu, uploads %llu, GPU buffer %s\n", g_who, width, height, TIMED_RUNS,
				median(latency, TIMED_RUNS), (unsigned long long) (after.downloads - before.downloads),
				(unsigned long long) (after.uploads - before.uploads), after.hasBuffer ? "yes" : "no");
			sendCommand(pp->toParent, CMD_READY, 0, 0, 0);
			break;
		}
		case CMD_EXIT:
			// The consumer exits holding the surface; nothing is released.
			fflush(stdout);
			_exit(g_failures == 0 ? 0 : 1);
		default:
			printf("%sFAIL: unexpected command %d\n", g_who, m.command);
			finishChild(pp);
		}
	}
}

static bool reap(pid_t pid, const char *who) {
	int status = 0;
	pid_t reaped = waitpid(pid, &status, 0);
	bool ok = reaped == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0;
	if (!ok)
		printf("FAIL: the %s ended with status %d\n", who, WIFEXITED(status) ? WEXITSTATUS(status) : -1);
	return ok;
}

static struct command_message expectReady(int fd, const char *who) {
	struct command_message m = receiveCommand(fd);
	if (m.command != CMD_READY && m.command != CMD_HELLO) {
		printf("FAIL: the %s failed (command %d)\n", who, m.command);
		for (int i = 0; i < 2; i++)
			if (g_children[i] > 0)
				kill(g_children[i], SIGKILL);
		_exit(1);
	}
	return m;
}

int main(int argc, char **argv) {
	signal(SIGALRM, onAlarm);
	alarm(58);
	setvbuf(stdout, NULL, _IOLBF, 0);
	bool measure = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--no-window-system") == 0)
			g_windowSystem = false;
		else if (strcmp(argv[i], "--measure") == 0)
			measure = true;
	}

	// Fork before any framework is used: the children inherit the
	// rendezvous port as their bootstrap port.
	mach_port_t task = mach_task_self();
	mach_port_t rendezvous = MACH_PORT_NULL, bootstrap = MACH_PORT_NULL;
	CHECK(mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &rendezvous) == KERN_SUCCESS);
	CHECK(mach_port_insert_right(task, rendezvous, rendezvous, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS);
	task_get_special_port(task, TASK_BOOTSTRAP_PORT, &bootstrap);
	CHECK(task_set_special_port(task, TASK_BOOTSTRAP_PORT, rendezvous) == KERN_SUCCESS);

	struct pipes pp[2];
	mach_port_t inboxes[2] = { MACH_PORT_NULL, MACH_PORT_NULL };
	for (int i = 0; i < 2; i++) {
		int down[2], up[2];
		if (pipe(down) != 0 || pipe(up) != 0) {
			printf("FAIL: pipes\n");
			return 1;
		}
		pp[i] = (struct pipes) { down[1], up[0], up[1], down[0] };
		fflush(stdout);
		pid_t pid = fork();
		if (pid == 0) {
			close(down[1]);
			close(up[0]);
			if (i == 0)
				producerMain(&pp[i], measure);
			else
				consumerMain(&pp[i], measure);
		}
		close(down[0]);
		close(up[1]);
		g_children[i] = pid;
		CHECK(pid > 0);
		if (pid < 0)
			return 1;
		// The child announces its inbox and gets the real bootstrap port
		// before the next child is forked, so that the inboxes are known
		// apart.
		mach_port_t unused = MACH_PORT_NULL;
		if (!receivePorts(rendezvous, &inboxes[i], &unused)) {
			printf("FAIL: no connection from child %d\n", i);
			onAlarm(0);
		}
		CHECK(sendPorts(inboxes[i], bootstrap, MACH_MSG_TYPE_COPY_SEND, MACH_PORT_NULL, MACH_MSG_TYPE_COPY_SEND) == MACH_MSG_SUCCESS);
	}
	task_set_special_port(task, TASK_BOOTSTRAP_PORT, bootstrap);
	struct pipes *producer = &pp[0], *consumer = &pp[1];

	// The producer has rendered and made two ports; it sends them to the
	// rendezvous, and the parent forwards one to the consumer.
	struct command_message ready = expectReady(producer->fromChild, "producer");
	uint32_t surfaceID = ready.a, seed = ready.b;
	expectReady(consumer->fromChild, "consumer");
	mach_port_t ports[2] = { MACH_PORT_NULL, MACH_PORT_NULL };
	CHECK(sendPorts(inboxes[0], rendezvous, MACH_MSG_TYPE_MAKE_SEND, MACH_PORT_NULL, MACH_MSG_TYPE_COPY_SEND)
		== MACH_MSG_SUCCESS);
	if (!receivePorts(rendezvous, &ports[0], &ports[1]) || !MACH_PORT_VALID(ports[0]) || !MACH_PORT_VALID(ports[1])) {
		printf("FAIL: no surface ports from the producer\n");
		onAlarm(0);
	}
	CHECK(sendPorts(inboxes[1], ports[1], MACH_MSG_TYPE_MOVE_SEND, MACH_PORT_NULL, MACH_MSG_TYPE_COPY_SEND)
		== MACH_MSG_SUCCESS);

	g_who = "parent: ";
	if (!startGL()) {
		printf("FAIL: no offscreen context\n");
		onAlarm(0);
	}
	printf("parent: renderer: %s\n", (const char *) glGetString(GL_RENDERER));

	if (measure) {
		// The parent only relays.
		mach_port_deallocate(task, ports[0]);
		sendCommand(producer->toChild, CMD_MEASURE, 0, 0, 0);
		sendCommand(consumer->toChild, CMD_MEASURE, 0, 0, 0);
		for (int i = 0; i < TIMED_RUNS + 2; i++) {
			struct command_message frame = receiveCommand(producer->fromChild);
			write(consumer->toChild, &frame, sizeof(frame));
			expectReady(consumer->fromChild, "consumer");
			sendCommand(producer->toChild, CMD_READY, 0, 0, 0);
		}
		struct command_message copies = expectReady(producer->fromChild, "producer");
		printf("parent: producer copies while measuring: downloads %u, uploads %u, GPU buffer %s\n",
			copies.a, copies.b, copies.c ? "yes" : "no");
		expectReady(consumer->fromChild, "consumer");
		sendCommand(producer->toChild, CMD_EXIT, 0, 0, 0);
		sendCommand(consumer->toChild, CMD_EXIT, 0, 0, 0);
		CHECK(reap(g_children[1], "consumer"));
		g_children[1] = 0;
		CHECK(reap(g_children[0], "producer"));
		g_children[0] = 0;
		printf(g_failures == 0 ? "PASS\n" : "FAIL: %d\n", g_failures);
		fflush(stdout);
		_exit(g_failures == 0 ? 0 : 1);
	}

	// port: the parent is the second consumer.
	IOSurfaceRef s = IOSurfaceLookupFromMachPort(ports[0]);
	mach_port_deallocate(task, ports[0]);
	CHECK(s != NULL);
	if (!s)
		onAlarm(0);
	CHECK(IOSurfaceGetID(s) == surfaceID);
	CHECK(IOSurfaceGetSeed(s) == seed);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), RED);
	EXPECT_PIXEL(sampledPixel(s), RED);
	printCopies("after the lookup", s);
	sendCommand(consumer->toChild, CMD_SAMPLE, RED, seed, 0);
	struct command_message sampled = expectReady(consumer->fromChild, "consumer");
	bool gpuBuffer = sampled.a != 0;
	printf("GPU buffer shared: %s\n", gpuBuffer ? "yes" : "no");
	struct serviceState state = serviceStateOf(surfaceID);
	printState("after both consumers sampled", surfaceID);
	if (state.known) {
		CHECK(!state.gone && state.holders == 3);
		if (gpuBuffer) {
			// Every process uses the buffer; none reads the memory.
			CHECK(state.flags & 2);
			CHECK(state.memoryUsers == 0);
		} else
			CHECK(state.memoryUsers == 3);
	}

	// flush: new rendering reaches both consumers.
	sendCommand(producer->toChild, CMD_RENDER, GREEN, 0, 0);
	ready = expectReady(producer->fromChild, "producer");
	uint32_t flushed = ready.a;
	CHECK(flushed != seed);
	CHECK(IOSurfaceGetSeed(s) == flushed);
	if (gpuBuffer)
		// No process reads the memory: the flush copies nothing.
		CHECK(ready.b == 0);
	printf("producer copies into the memory at the flush: %u\n", ready.b);
	struct copies before = copiesOf(s);
	EXPECT_PIXEL(sampledPixel(s), GREEN);
	struct copies after = copiesOf(s);
	if (gpuBuffer)
		CHECK(after.downloads == before.downloads && after.uploads == before.uploads);
	sendCommand(consumer->toChild, CMD_SAMPLE, GREEN, flushed, 0);
	sampled = expectReady(consumer->fromChild, "consumer");
	if (gpuBuffer) {
		CHECK(sampled.b == 0);
		// The consumer's lock made the one copy into the memory.
		CHECK(sampled.c == 1);
	}
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly | kIOSurfaceLockAvoidSync, NULL), GREEN);
	if (gpuBuffer)
		// Not again in this process.
		CHECK(copiesOf(s).downloads == after.downloads);

	// write: a CPU write in the consumer reaches the producer's GL and CPU.
	sendCommand(consumer->toChild, CMD_WRITE, BLUE, 0, 0);
	ready = expectReady(consumer->fromChild, "consumer");
	uint32_t written = ready.a;
	CHECK(written != flushed);
	CHECK(IOSurfaceGetSeed(s) == written);
	sendCommand(producer->toChild, CMD_CHECK, BLUE, written, 0);
	expectReady(producer->fromChild, "producer");
	EXPECT_PIXEL(sampledPixel(s), BLUE);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), BLUE);

	// global: found by ID in other processes, unless not global.
	sendCommand(producer->toChild, CMD_GLOBAL, 0, 0, 0);
	ready = expectReady(producer->fromChild, "producer");
	uint32_t globalID = ready.a, localID = ready.b;
	CHECK(globalID != 0 && localID != 0 && globalID != localID && globalID != surfaceID);
	int descriptors = openDescriptors();
	IOSurfaceRef global = IOSurfaceLookup(globalID);
	CHECK(global != NULL);
	if (global) {
		CHECK(IOSurfaceGetID(global) == globalID);
		EXPECT_PIXEL(lockedPixel(global, kIOSurfaceLockReadOnly, NULL), RED);
		EXPECT_PIXEL(sampledPixel(global), RED);
	}
	IOSurfaceRef local = IOSurfaceLookup(localID);
	CHECK(local == NULL);
	if (local)
		CFRelease(local);
	sendCommand(consumer->toChild, CMD_LOOKUP, globalID, localID, 0);
	expectReady(consumer->fromChild, "consumer");
	state = serviceStateOf(globalID);
	printState("global surface after the consumer released it", globalID);
	if (state.known) {
		CHECK(!state.gone && state.holders == 2 && (state.flags & 1));
	}
	if (global)
		CFRelease(global);
	state = serviceStateOf(globalID);
	if (state.known)
		CHECK(!state.gone && state.holders == 1);
	int descriptorsAfter = openDescriptors();
	printf("parent: open descriptors before the global lookup %d, after its release %d\n", descriptors,
		descriptorsAfter);
	CHECK(descriptorsAfter == descriptors);

	// consumer exit: its holds go with it.
	sendCommand(consumer->toChild, CMD_EXIT, 0, 0, 0);
	CHECK(reap(g_children[1], "consumer"));
	g_children[1] = 0;
	state = serviceStateOf(surfaceID);
	printState("after the consumer exited", surfaceID);
	if (state.known) {
		CHECK(!state.gone && state.holders == 2);
		if (gpuBuffer)
			CHECK(state.memoryUsers == 0);
	}
	sendCommand(producer->toChild, CMD_RENDER, RED, 0, 0);
	ready = expectReady(producer->fromChild, "producer");
	EXPECT_PIXEL(sampledPixel(s), RED);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly, NULL), RED);

	// producer exit: the last rendering stays with the surface, and so does
	// the GPU buffer.
	sendCommand(producer->toChild, CMD_RENDER, YELLOW, 0, 0);
	ready = expectReady(producer->fromChild, "producer");
	flushed = ready.a;
	sendCommand(producer->toChild, CMD_EXIT, 0, 0, 0);
	CHECK(reap(g_children[0], "producer"));
	g_children[0] = 0;
	state = serviceStateOf(surfaceID);
	printState("after the producer exited", surfaceID);
	if (state.known) {
		CHECK(!state.gone && state.holders == 1);
		if (gpuBuffer)
			CHECK(state.flags & 2);
	}
	// The producer's other surfaces went with it.
	state = serviceStateOf(globalID);
	if (state.known)
		CHECK(state.gone);
	state = serviceStateOf(localID);
	if (state.known)
		CHECK(state.gone);
	CHECK(IOSurfaceGetSeed(s) == flushed);
	EXPECT_PIXEL(sampledPixel(s), YELLOW);
	EXPECT_PIXEL(lockedPixel(s, kIOSurfaceLockReadOnly | kIOSurfaceLockAvoidSync, NULL), YELLOW);
	printCopies("at the end", s);

	// release: the surface leaves the service with the last holder.
	CFRelease(s);
	state = serviceStateOf(surfaceID);
	if (state.known)
		CHECK(state.gone);
	printState("after the last release", surfaceID);

	CGLSetCurrentContext(NULL);
	CGLReleaseContext(g_context);
	printf(g_failures == 0 ? "PASS\n" : "FAIL: %d\n", g_failures);
	fflush(stdout);
	_exit(g_failures == 0 ? 0 : 1);
}
