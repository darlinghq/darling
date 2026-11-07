// Copyright (C) 2026 Lubos Dolezel
//
// The private GPU callbacks of IOSurface.framework (IOSurfacePriv.h): the
// writeback callback runs before every lock and before a Mach port is made
// for a surface, not again for locks taken from inside it, and the forget
// callback runs when a surface is freed. Only the first registration with
// the known version takes effect.
//
// The program does not load OpenGL.framework, so its own registration is
// the first one in the process.
#include <CoreFoundation/CoreFoundation.h>
#include <IOSurface/IOSurface.h>
#include <IOSurface/IOSurfacePriv.h>
#include <mach/mach.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static int g_writebacks, g_forgets, g_ignoredCalls;
static IOSurfaceRef g_lastWriteback, g_lastForget;
static int g_lockInsideWriteback;

static void writeback(IOSurfaceRef surface)
{
	g_writebacks++;
	g_lastWriteback = surface;
	if (g_lockInsideWriteback)
	{
		// A registrant copies its contents in under a lock of its own.
		CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
		CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
	}
}

static void forget(IOSurfaceRef surface)
{
	g_forgets++;
	g_lastForget = surface;
}

static void ignored(IOSurfaceRef surface)
{
	(void) surface;
	g_ignoredCalls++;
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
	setNumber(props, kIOSurfaceWidth, 16);
	setNumber(props, kIOSurfaceHeight, 8);
	setNumber(props, kIOSurfaceBytesPerElement, 4);
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	return surface;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(20);

	// An unknown version does not claim the registration.
	_IOSurfaceGPUCallbacks future = {
		.version = _IOSURFACE_GPU_CALLBACKS_VERSION + 1,
		.writeback = ignored,
		.forget = ignored,
	};
	_IOSurfaceSetGPUCallbacks(&future);
	_IOSurfaceSetGPUCallbacks(NULL);

	_IOSurfaceGPUCallbacks callbacks = {
		.version = _IOSURFACE_GPU_CALLBACKS_VERSION,
		.writeback = writeback,
		.forget = forget,
	};
	_IOSurfaceSetGPUCallbacks(&callbacks);

	// A second registration is ignored, and the first one does not depend
	// on the caller's structure staying alive.
	_IOSurfaceGPUCallbacks second = {
		.version = _IOSURFACE_GPU_CALLBACKS_VERSION,
		.writeback = ignored,
		.forget = ignored,
	};
	_IOSurfaceSetGPUCallbacks(&second);
	callbacks.writeback = ignored;
	callbacks.forget = ignored;

	IOSurfaceRef surface = createSurface();
	CHECK(surface != NULL);
	if (surface == NULL)
		finish(1);
	CHECK(g_writebacks == 0);

	// Each lock, read-only or not, is preceded by a writeback.
	uint32_t seed = 0;
	CHECK(IOSurfaceLock(surface, 0, &seed) == kIOReturnSuccess);
	CHECK(g_writebacks == 1 && g_lastWriteback == surface);
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
	CHECK(IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);
	CHECK(g_writebacks == 2);
	CHECK(IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL) == kIOReturnSuccess);

	// Unlocking does not call it.
	CHECK(g_writebacks == 2);

	// Locks from inside the callback do not call it again.
	g_lockInsideWriteback = 1;
	CHECK(IOSurfaceLock(surface, 0, NULL) == kIOReturnSuccess);
	CHECK(g_writebacks == 3);
	CHECK(IOSurfaceUnlock(surface, 0, NULL) == kIOReturnSuccess);
	g_lockInsideWriteback = 0;

	// Creating a Mach port, which hands the memory to other processes, is
	// preceded by a writeback.
	mach_port_t port = IOSurfaceCreateMachPort(surface);
	if (MACH_PORT_VALID(port))
	{
		CHECK(g_writebacks == 4 && g_lastWriteback == surface);
		mach_port_deallocate(mach_task_self(), port);
	}
	else
	{
		printf("FAIL %d: IOSurfaceCreateMachPort returned no port\n", __LINE__);
		g_failures++;
	}

	// Freeing the surface calls forget with it.
	CHECK(g_forgets == 0);
	IOSurfaceRef freed = surface;
	CFRelease(surface);
	CHECK(g_forgets == 1 && g_lastForget == freed);

	CHECK(g_ignoredCalls == 0);

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
