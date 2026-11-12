// Copyright (C) 2026 Lubos Dolezel
//
// An IOSurface-backed CVPixelBuffer shared with another process through the Mach port of its
// surface: the child makes its own pixel buffer around the surface with
// CVPixelBufferCreateWithIOSurface, reads what the parent wrote under a read-only lock,
// writes under a lock of its own, and the parent sees that write, with the surface's seed
// moved. The parent reaches the child through the bootstrap special port, which the child
// inherits and which points to a rendezvous port of the parent for the child's inbox.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#include <IOSurface/IOSurface.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define W 40
#define H 24
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

static void onAlarm(int sig)
{
	(void) sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	if (g_child > 0)
		kill(g_child, SIGKILL);
	_exit(1);
}

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

static uint8_t pattern(size_t x, size_t y, size_t channel)
{
	return (uint8_t) (x * 5 + y * 3 + channel * 41);
}

// The child, which uses no framework before it has its message.
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
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreateWithIOSurface(NULL, surface, NULL, &buffer) == kCVReturnSuccess);
	if (!buffer)
		finish(1);
	CHECK(CVPixelBufferGetWidth(buffer) == W && CVPixelBufferGetHeight(buffer) == H);
	CHECK(CVPixelBufferGetPixelFormatType(buffer) == kCVPixelFormatType_32BGRA);
	CHECK(CVPixelBufferGetIOSurface(buffer) == surface);

	// What the parent wrote before it made the port.
	CHECK(CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	const uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);
	CHECK(base != NULL);
	for (size_t y = 0; base && y < H; y++)
		for (size_t x = 0; x < W; x++)
			for (size_t c = 0; c < 4; c++)
				if (base[y * bytesPerRow + x * 4 + c] != pattern(x, y, c))
				{
					CHECK(base[y * bytesPerRow + x * 4 + c] == pattern(x, y, c));
					y = H;
					x = W;
					break;
				}
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);

	// The child's own write.
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint8_t* writable = CVPixelBufferGetBaseAddress(buffer);
	if (writable)
		for (size_t x = 0; x < W; x++)
			writable[(H - 1) * bytesPerRow + x * 4] = (uint8_t) (255 - x);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);

	CVPixelBufferRelease(buffer);
	CFRelease(surface);
	mach_port_deallocate(task, message.port.name);
	finish(g_failures == 0 ? 0 : 1);
}

int main(void)
{
	signal(SIGALRM, onAlarm);
	alarm(120);

	// Fork before any framework is used; the child inherits the bootstrap special port.
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

	// A pixel buffer in a surface, filled under a lock.
	CFDictionaryRef empty = CFDictionaryCreate(NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFStringRef key = kCVPixelBufferIOSurfacePropertiesKey;
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, (const void**) &key, (const void**) &empty, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CVPixelBufferRef buffer = NULL;
	CHECK(CVPixelBufferCreate(NULL, W, H, kCVPixelFormatType_32BGRA, attributes, &buffer) == kCVReturnSuccess);
	IOSurfaceRef surface = buffer ? CVPixelBufferGetIOSurface(buffer) : NULL;
	CHECK(surface != NULL);
	if (!surface)
		finish(1);
	CHECK(CVPixelBufferLockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint8_t* base = CVPixelBufferGetBaseAddress(buffer);
	size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);
	CHECK(base != NULL);
	for (size_t y = 0; base && y < H; y++)
		for (size_t x = 0; x < W; x++)
			for (size_t c = 0; c < 4; c++)
				base[y * bytesPerRow + x * 4 + c] = pattern(x, y, c);
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, 0) == kCVReturnSuccess);
	uint32_t seedBefore = IOSurfaceGetSeed(surface);

	mach_port_t port = IOSurfaceCreateMachPort(surface);
	CHECK(MACH_PORT_VALID(port));
	if (connected && MACH_PORT_VALID(port))
		CHECK(sendPort(hello.port.name, port, MACH_MSG_TYPE_MOVE_SEND, bootstrap) == MACH_MSG_SUCCESS);

	int status = 0;
	pid_t reaped = waitpid(pid, &status, 0);
	g_child = 0;
	CHECK(reaped == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	// The child's write is in the parent's buffer, and the seed moved for it.
	CHECK(IOSurfaceGetSeed(surface) != seedBefore);
	CHECK(CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);
	base = CVPixelBufferGetBaseAddress(buffer);
	CHECK(base != NULL);
	for (size_t x = 0; base && x < W; x++)
		if (base[(H - 1) * bytesPerRow + x * 4] != (uint8_t) (255 - x))
		{
			CHECK(base[(H - 1) * bytesPerRow + x * 4] == (uint8_t) (255 - x));
			break;
		}
	// The rest of the pattern is untouched.
	CHECK(base && base[1 * bytesPerRow + 7 * 4 + 1] == pattern(7, 1, 1));
	CHECK(CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess);

	CVPixelBufferRelease(buffer);
	CFRelease(attributes);
	CFRelease(empty);
	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
