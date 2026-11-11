// Copyright (C) 2026 Lubos Dolezel
//
// The private SystemAdministration stub framework loads, and messages sent to
// its classes that it does not implement answer with a zeroed result (nil, NO
// or 0) instead of whatever happened to be left in the return register.
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

typedef id (*object_msg)(id, SEL, id);
typedef BOOL (*bool_msg)(id, SEL, id);
typedef NSInteger (*integer_msg)(id, SEL);

int main(void)
{
	@autoreleasepool {
		void* handle = dlopen("/System/Library/PrivateFrameworks/SystemAdministration.framework/SystemAdministration", RTLD_NOW);
		if (!handle) {
			printf("FAIL: dlopen: %s\n", dlerror());
			return 1;
		}

		const char* names[] = { "ADMUser", "AdminAuthenticator", "UserUtilities" };
		for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
			Class cls = objc_getClass(names[i]);
			CHECK(cls != Nil);
			if (cls == Nil) {
				continue;
			}

			// Class-side forwarding.
			CHECK(((object_msg)objc_msgSend)(cls, sel_registerName("lookupUserWithName:"), @"nobody") == nil);
			CHECK(((bool_msg)objc_msgSend)(cls, sel_registerName("authenticateWithPassword:"), @"x") == NO);
			CHECK(((integer_msg)objc_msgSend)(cls, sel_registerName("userCount")) == 0);

			// Instance-side forwarding.
			id obj = [[cls alloc] init];
			CHECK(obj != nil);
			CHECK(((object_msg)objc_msgSend)(obj, sel_registerName("shortName:"), @"x") == nil);
			CHECK(((bool_msg)objc_msgSend)(obj, sel_registerName("isAdministrator:"), @"x") == NO);
			CHECK(((integer_msg)objc_msgSend)(obj, sel_registerName("userID")) == 0);
			[obj release];
		}
	}

	if (failures) {
		return 1;
	}
	puts("OK");
	return 0;
}
