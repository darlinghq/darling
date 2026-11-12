/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * IORegistry properties, IOServiceAddMatchingNotification() and IOServiceAddInterestNotification().
 * Must pass for x86_64 and i386 programs alike. Without arguments: a notification for a service
 * that exists is armed and its iterator yields that service, and an interest notification can be
 * registered. With the argument "pad" (Darling; a host game controller is created and removed while
 * this runs, for instance with a uinput device): the first-match callback runs when a service of
 * class IOHIDDevice appears, and the terminated callback when it goes away.
 */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <string.h>

static int failed;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failed = 1; } } while (0)

static int matched, terminated;

static void onMatched(void *refcon, io_iterator_t it)
{
	io_object_t o;
	while ((o = IOIteratorNext(it))) {
		matched++;
		IOObjectRelease(o);
	}
	CHECK(refcon == (void *) 0x1234);
}

static void onTerminated(void *refcon, io_iterator_t it)
{
	io_object_t o;
	while ((o = IOIteratorNext(it))) {
		terminated++;
		IOObjectRelease(o);
	}
	CHECK(refcon == (void *) 0x5678);
}

static void onInterest(void *refcon, io_service_t service, natural_t type, void *arg) {}

int main(int argc, char **argv)
{
	kern_return_t kr;
	IONotificationPortRef port = IONotificationPortCreate(kIOMasterPortDefault);
	CHECK(port != NULL);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(port), kCFRunLoopDefaultMode);

	/* registry properties are readable, one by one and all at once */
	io_service_t platform = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
	CHECK(platform != IO_OBJECT_NULL);
	CFTypeRef manufacturer = IORegistryEntryCreateCFProperty(platform, CFSTR("manufacturer"), kCFAllocatorDefault, 0);
	CHECK(manufacturer != NULL && CFGetTypeID(manufacturer) == CFDataGetTypeID() && CFDataGetLength(manufacturer) > 0);
	CFMutableDictionaryRef properties = NULL;
	kr = IORegistryEntryCreateCFProperties(platform, &properties, kCFAllocatorDefault, 0);
	CHECK(kr == KERN_SUCCESS && properties != NULL && CFDictionaryGetCount(properties) > 0);
	CHECK(properties != NULL && CFDictionaryContainsKey(properties, CFSTR("manufacturer")));
	IOObjectRelease(platform);

	/* a service that exists: the iterator holds it as soon as the notification is armed */
	io_iterator_t it = IO_OBJECT_NULL;
	kr = IOServiceAddMatchingNotification(port, kIOFirstMatchNotification,
		IOServiceMatching("IOPlatformExpertDevice"), onMatched, (void *) 0x1234, &it);
	CHECK(kr == KERN_SUCCESS);
	CHECK(it != IO_OBJECT_NULL);
	onMatched((void *) 0x1234, it);
	CHECK(matched >= 1);
	matched = 0;

	/* an interest notification on a service that exists */
	io_service_t pm = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPMrootDomain"));
	CHECK(pm != IO_OBJECT_NULL);
	io_object_t interest = IO_OBJECT_NULL;
	kr = IOServiceAddInterestNotification(port, pm, kIOGeneralInterest, onInterest, NULL, &interest);
	CHECK(kr == KERN_SUCCESS);
	CHECK(interest != IO_OBJECT_NULL);

	/* a matching dictionary that does not parse is refused */
	kr = IOServiceAddMatchingNotification(port, kIOFirstMatchNotification, NULL, onMatched, NULL, &it);
	CHECK(kr != KERN_SUCCESS);

	if (argc > 1 && !strcmp(argv[1], "pad")) {
		io_iterator_t mit = IO_OBJECT_NULL, tit = IO_OBJECT_NULL;
		kr = IOServiceAddMatchingNotification(port, kIOFirstMatchNotification,
			IOServiceMatching("IOHIDDevice"), onMatched, (void *) 0x1234, &mit);
		CHECK(kr == KERN_SUCCESS);
		kr = IOServiceAddMatchingNotification(port, kIOTerminatedNotification,
			IOServiceMatching("IOHIDDevice"), onTerminated, (void *) 0x5678, &tit);
		CHECK(kr == KERN_SUCCESS);
		onTerminated((void *) 0x5678, tit);
		onMatched((void *) 0x1234, mit);
		printf("waiting for a game controller to appear and disappear\n");
		fflush(stdout);
		CFAbsoluteTime end = CFAbsoluteTimeGetCurrent() + 30;
		while (CFAbsoluteTimeGetCurrent() < end && !(matched && terminated))
			CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.2, false);
		CHECK(matched >= 1);
		CHECK(terminated >= 1);
	}

	printf(failed ? "FAILED\n" : "PASSED\n");
	return failed;
}
