/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * IOMainPort/kIOMainPortDefault (macOS 12 names of IOMasterPort and
 * kIOMasterPortDefault), the NULL result of IOHIDManagerCopyDevices for an
 * empty device set.
 * The expected results were observed on macOS 27 (arm64 and x86_64).
 * Link with -framework IOKit -framework CoreFoundation.
 */
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDManager.h>

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	mach_port_t port = MACH_PORT_NULL;
	io_iterator_t it = 0;
	IOHIDManagerRef mgr;
	CFSetRef devs;
	CFMutableDictionaryRef nomatch;
	int vendor = 0x7ffffff0;
	CFNumberRef num;
	CFArrayRef empty;

	check("kIOMainPortDefault is MACH_PORT_NULL", kIOMainPortDefault == MACH_PORT_NULL);
	check("IOMainPort succeeds", IOMainPort(MACH_PORT_NULL, &port) == KERN_SUCCESS && port != MACH_PORT_NULL);
	check("IOServiceGetMatchingServices with kIOMainPortDefault",
	      IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOResources"), &it) == KERN_SUCCESS);
	if (it)
		IOObjectRelease(it);

	mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	devs = IOHIDManagerCopyDevices(mgr);
	check("no matching set: NULL", devs == NULL);
	if (devs)
		CFRelease(devs);

	nomatch = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	num = CFNumberCreate(NULL, kCFNumberIntType, &vendor);
	CFDictionarySetValue(nomatch, CFSTR(kIOHIDVendorIDKey), num);
	IOHIDManagerSetDeviceMatching(mgr, nomatch);
	devs = IOHIDManagerCopyDevices(mgr);
	check("dictionary that matches nothing: NULL", devs == NULL);
	if (devs)
		CFRelease(devs);

	empty = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	IOHIDManagerSetDeviceMatchingMultiple(mgr, empty);
	devs = IOHIDManagerCopyDevices(mgr);
	check("empty matching array: NULL", devs == NULL);
	if (devs)
		CFRelease(devs);

	IOHIDManagerSetDeviceMatching(mgr, nomatch);
	IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone);
	devs = IOHIDManagerCopyDevices(mgr);
	check("after IOHIDManagerOpen, nothing matching: NULL", devs == NULL);
	if (devs)
		CFRelease(devs);
	IOHIDManagerClose(mgr, kIOHIDOptionsTypeNone);

	CFRelease(empty);
	CFRelease(num);
	CFRelease(nomatch);
	CFRelease(mgr);

	return failures != 0;
}
