/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The graphics services of the IORegistry. An IOAccelerator that has "VRAM,totalMB" has a
 * positive number, and "vendor-id" and "device-id" are 4 bytes. With the argument "require" (for
 * Darling, where the host's GPUs are published) there is at least one IOAccelerator with a memory
 * size, and no IOFramebuffer (an Apple silicon Mac has none either). Without it nothing has to be
 * there, as on an Apple silicon Mac, whose IOAccelerator has no memory size.
 */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failed;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failed = 1; } } while (0)

static int64_t numberProperty(io_registry_entry_t e, const char *key, int *present)
{
	CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
	CFTypeRef v = IORegistryEntryCreateCFProperty(e, k, NULL, 0);
	int64_t result = 0;
	*present = 0;
	if (v) {
		*present = 1;
		if (CFGetTypeID(v) != CFNumberGetTypeID() || !CFNumberGetValue(v, kCFNumberSInt64Type, &result)) {
			printf("FAIL: %s is not a number\n", key);
			failed = 1;
		}
		CFRelease(v);
	}
	CFRelease(k);
	return result;
}

static void checkID(io_registry_entry_t e, const char *key)
{
	CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
	CFTypeRef v = IORegistryEntryCreateCFProperty(e, k, NULL, 0);
	if (v) {
		CHECK(CFGetTypeID(v) == CFDataGetTypeID() && CFDataGetLength(v) == 4);
		CFRelease(v);
	}
	CFRelease(k);
}

int main(int argc, char **argv)
{
	int require = argc > 1 && strcmp(argv[1], "require") == 0;
	int accelerators = 0, framebuffers = 0;
	int64_t firstMB = -1;
	io_iterator_t it;
	io_registry_entry_t e;

	CHECK(IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOAccelerator"), &it) == KERN_SUCCESS);
	while ((e = IOIteratorNext(it))) {
		int present;
		int64_t mb = numberProperty(e, "VRAM,totalMB", &present);
		printf("IOAccelerator: VRAM,totalMB=%s%lld\n", present ? "" : "(absent) ", (long long) mb);
		if (present)
			CHECK(mb > 0);
		if (present && firstMB < 0)
			firstMB = mb;
		checkID(e, "vendor-id");
		checkID(e, "device-id");
		accelerators++;
		IOObjectRelease(e);
	}
	IOObjectRelease(it);

	CHECK(IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOFramebuffer"), &it) == KERN_SUCCESS);
	while ((e = IOIteratorNext(it))) {
		framebuffers++;
		IOObjectRelease(e);
	}
	IOObjectRelease(it);

	if (require) {
		CHECK(accelerators >= 1);
		CHECK(firstMB > 0);
		CHECK(framebuffers == 0);
	}
	puts(failed ? "FAILED" : "PASSED");
	return failed;
}
