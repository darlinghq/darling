/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * IOKit exports the IOHID event-system SPI functions that command-line
 * monitoring tools bind to: IOHIDEventGetFloatValue,
 * IOHIDEventSystemClientCreate, IOHIDEventSystemClientSetMatching and
 * IOHIDServiceClientCopyEvent. Their presence in IOKit was observed on
 * macOS 27 (arm64 and x86_64). The functions are looked up with dlsym because
 * the SDK headers do not declare them.
 * Link with -framework CoreFoundation only; IOKit is opened with dlopen.
 */
#include <stdio.h>
#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

typedef CFTypeRef (*create_fn)(CFAllocatorRef);
typedef void (*set_matching_fn)(CFTypeRef, CFDictionaryRef);

int main(void)
{
	void *iokit = dlopen("/System/Library/Frameworks/IOKit.framework/IOKit", RTLD_NOW);
	create_fn create;
	set_matching_fn set_matching;
	CFTypeRef client;
	CFMutableDictionaryRef match;
	int usage_page = 1;
	CFNumberRef num;

	check("IOKit loads", iokit != NULL);
	if (!iokit)
		return 1;

	check("IOHIDEventGetFloatValue is exported", dlsym(iokit, "IOHIDEventGetFloatValue") != NULL);
	check("IOHIDServiceClientCopyEvent is exported", dlsym(iokit, "IOHIDServiceClientCopyEvent") != NULL);
	check("IOHIDEventSystemClientCreateWithType is exported",
	      dlsym(iokit, "IOHIDEventSystemClientCreateWithType") != NULL);

	create = (create_fn)dlsym(iokit, "IOHIDEventSystemClientCreate");
	set_matching = (set_matching_fn)dlsym(iokit, "IOHIDEventSystemClientSetMatching");
	check("IOHIDEventSystemClientCreate is exported", create != NULL);
	check("IOHIDEventSystemClientSetMatching is exported", set_matching != NULL);
	if (!create || !set_matching)
		return 1;

	client = create(kCFAllocatorDefault);
	check("IOHIDEventSystemClientCreate returns a client", client != NULL);
	if (client) {
		match = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
		                                  &kCFTypeDictionaryValueCallBacks);
		num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page);
		CFDictionarySetValue(match, CFSTR("PrimaryUsagePage"), num);
		set_matching(client, match);
		check("IOHIDEventSystemClientSetMatching accepts a matching dictionary", 1);
		set_matching(client, NULL);
		check("IOHIDEventSystemClientSetMatching accepts NULL", 1);
		CFRelease(num);
		CFRelease(match);
		CFRelease(client);
	}

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
