/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * CFGetAllocator() of an Objective-C object that is not a CF object (such as
 * the instances of NSObject subclasses; IOKit's IOHIDDevice is one) has no
 * allocator recorded in a CF header and answers the default system allocator.
 * Link with -framework Foundation -framework CoreFoundation.
 */
#import <Foundation/Foundation.h>
#include <stdio.h>

@interface PlainObject : NSObject
@end
@implementation PlainObject
@end

static int failures;

static void expect(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	@autoreleasepool {
		PlainObject *plain = [[PlainObject alloc] init];
		expect("NULL gives the system default allocator", CFGetAllocator(NULL) == kCFAllocatorSystemDefault);
		expect("plain ObjC object gives the system default allocator",
			CFGetAllocator((__bridge CFTypeRef) plain) == kCFAllocatorSystemDefault);
		expect("CF object created with the default allocator",
			CFGetAllocator((CFTypeRef) CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks)) == CFAllocatorGetDefault()
			|| CFGetAllocator((CFTypeRef) CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks)) == kCFAllocatorSystemDefault);
	}
	return failures != 0;
}
