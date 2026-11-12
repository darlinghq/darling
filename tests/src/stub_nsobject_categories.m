// Copyright (C) 2026 Lubos Dolezel
//
// Stub frameworks that carry categories on NSObject (the bindings methods of
// AppKit) must not change how NSObject answers messages it does not implement.
// Loading the framework must leave -methodSignatureForSelector: of an unrelated
// object returning nil for an unknown selector (so the runtime raises
// "unrecognized selector" as usual), and the category methods themselves must
// exist.
//
// Usage: stub_nsobject_categories [path of the framework binary to load]
// The default is the AppKit framework of the system.
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

typedef id (*msg2)(id, SEL, id, id);

int main(int argc, char **argv)
{
	const char *path = argc > 1 ? argv[1] : "/System/Library/Frameworks/AppKit.framework/AppKit";

	@autoreleasepool {
		void *handle = dlopen(path, RTLD_NOW);
		if (handle == NULL) {
			printf("FAIL: cannot load %s: %s\n", path, dlerror());
			return 1;
		}

		SEL unknown = sel_registerName("darlingTestNoSuchSelector:withArgument:");
		NSObject *object = [[NSObject alloc] init];

		CHECK([object methodSignatureForSelector: unknown] == nil);
		CHECK([NSObject methodSignatureForSelector: unknown] == nil);
		CHECK([@"string" methodSignatureForSelector: unknown] == nil);
		CHECK([[NSDate date] methodSignatureForSelector: unknown] == nil);
		CHECK(![object respondsToSelector: unknown]);

		// The runtime still reports the message as unrecognized.
		BOOL raised = NO;
		@try {
			((msg2)objc_msgSend)(object, unknown, @"a", @"b");
		} @catch (NSException *e) {
			raised = [[e name] isEqual: NSInvalidArgumentException];
		}
		CHECK(raised);

		// Signatures of implemented methods are not replaced.
		NSMethodSignature *sig = [object methodSignatureForSelector: @selector(isEqual:)];
		CHECK(sig != nil && [sig numberOfArguments] == 3 && strcmp([sig methodReturnType], "c") == 0);

		// The bindings methods declared by the framework exist on every object.
		CHECK([object respondsToSelector: @selector(exposedBindings)]);
		CHECK([object respondsToSelector: @selector(unbind:)]);

		[object release];
	}

	if (failures == 0) {
		printf("PASS\n");
	}
	return failures != 0;
}
