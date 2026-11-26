/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/
// IOSurfaceSetOwnershipIdentity moves a surface's memory accounting to the
// task named by a task identity token (WebKit gives the surfaces it makes
// for a web content process to that process). Checked: the call succeeds for
// the calling task's own identity token under the graphics ledger tag, and
// the surface keeps its ID, size and contents afterwards.
// The function is SPI, so it is declared here; the graphics tag value is the
// one WebKit's IOSurfaceSPI.h lists.
#include <IOSurface/IOSurface.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern kern_return_t IOSurfaceSetOwnershipIdentity(IOSurfaceRef buffer, mach_port_t task_id_token, int newLedgerTag, uint32_t newLedgerOptions);

enum { kLedgerTagGraphics = 4 };

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL: %s\n", what); failures++; } else printf("PASS: %s\n", what); } while (0)

static void setInt(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	CFDictionarySetValue(dict, key, number);
	CFRelease(number);
}

int main(void)
{
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	setInt(props, kIOSurfaceWidth, 8);
	setInt(props, kIOSurfaceHeight, 4);
	setInt(props, kIOSurfaceBytesPerElement, 4);
	setInt(props, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef surface = IOSurfaceCreate(props);
	CFRelease(props);
	CHECK(surface != NULL, "a surface is created");
	if (!surface)
		return 1;

	IOSurfaceLock(surface, 0, NULL);
	memset(IOSurfaceGetBaseAddress(surface), 0xab, IOSurfaceGetAllocSize(surface));
	IOSurfaceUnlock(surface, 0, NULL);
	IOSurfaceID surfaceID = IOSurfaceGetID(surface);

	task_id_token_t token = MACH_PORT_NULL;
	kern_return_t kr = task_create_identity_token(mach_task_self(), &token);
	CHECK(kr == KERN_SUCCESS, "the task has an identity token");
	if (kr == KERN_SUCCESS) {
		CHECK(IOSurfaceSetOwnershipIdentity(surface, token, kLedgerTagGraphics, 0) == kIOReturnSuccess,
			"ownership moves to the task's identity under the graphics tag");
		mach_port_deallocate(mach_task_self(), token);
	}

	CHECK(IOSurfaceGetID(surface) == surfaceID, "the surface keeps its ID");
	CHECK(IOSurfaceGetWidth(surface) == 8 && IOSurfaceGetHeight(surface) == 4, "the surface keeps its size");
	IOSurfaceLock(surface, kIOSurfaceLockReadOnly, NULL);
	const uint8_t *bytes = IOSurfaceGetBaseAddress(surface);
	CHECK(bytes[0] == 0xab && bytes[IOSurfaceGetAllocSize(surface) - 1] == 0xab, "the surface keeps its contents");
	IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, NULL);

	CFRelease(surface);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
