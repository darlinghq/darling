/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/
// The first use of IOSurface looks up the IOSurfaceRoot service. IOServiceGetMatchingService consumes
// the matching dictionary it is given (see IOKitLib.h), so releasing it afterwards over-releases it and
// the next user of the freed memory, or a sending of a message to it, crashes (reliably on i386). The
// test re-executes itself with MallocScribble set, which fills freed blocks with a pattern, then
// creates a surface: an over-released dictionary makes that crash.
#include <IOSurface/IOSurface.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
	if (getenv("MallocScribble") == NULL) {
		setenv("MallocScribble", "1", 1);
		execv(argv[0], argv);
		perror("execv");
		return 1;
	}

	int value = 16;
	CFNumberRef number = CFNumberCreate(NULL, kCFNumberIntType, &value);
	const void *keys[] = { kIOSurfaceWidth, kIOSurfaceHeight, kIOSurfaceBytesPerElement };
	const void *values[] = { number, number, number };
	CFDictionaryRef properties = CFDictionaryCreate(NULL, keys, values, 3,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	IOSurfaceRef surface = IOSurfaceCreate(properties);
	if (surface == NULL) {
		printf("FAIL: IOSurfaceCreate returned NULL\n");
		return 1;
	}
	CFRelease(surface);
	CFRelease(properties);
	CFRelease(number);
	printf("PASS\n");
	return 0;
}
