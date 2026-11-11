// Copyright (C) 2026 Lubos Dolezel
//
// The CPAnalytics stub answers any message with nil/0 and does not log: selectors with
// arguments are accepted as called, and the result is zeroed.
#import <Foundation/Foundation.h>
#import <objc/message.h>
#import <objc/runtime.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		void *handle = dlopen("/System/Library/PrivateFrameworks/CPAnalytics.framework/CPAnalytics", RTLD_NOW);
		if (!handle) {
			printf("FAIL: dlopen: %s\n", dlerror());
			return 1;
		}

		Class cls = objc_getClass("CPAnalytics");
		CHECK(cls != Nil);
		if (cls == Nil)
			return 1;

		id shared = [cls sharedInstance];
		CHECK(shared != nil);

		id (*objectMsg)(id, SEL, id) = (id (*)(id, SEL, id)) objc_msgSend;
		BOOL (*boolMsg)(id, SEL, id, id) = (BOOL (*)(id, SEL, id, id)) objc_msgSend;
		NSInteger (*integerMsg)(id, SEL) = (NSInteger (*)(id, SEL)) objc_msgSend;

		CHECK(objectMsg(cls, sel_registerName("someUnknownClassMessage:"), @"x") == nil);
		CHECK(objectMsg(shared, sel_registerName("someUnknownMessage:"), @"x") == nil);
		CHECK(boolMsg(shared, sel_registerName("unknownFlag:withObject:"), @"x", @"y") == NO);
		CHECK(integerMsg(shared, sel_registerName("unknownCount")) == 0);
	}

	if (failures) {
		printf("FAILED: %d\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
