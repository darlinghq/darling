// Copyright (C) 2026 Lubos Dolezel
//
// Placeholder (stub) framework classes accept any message: the forwarding
// signature has one argument per colon in the selector, so messages with
// arguments reach -forwardInvocation: instead of failing with an argument-count
// error, class-side messages are accepted as well, and every unimplemented
// message answers a zero result (nil, NO or 0).
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

typedef id (*msg0)(id, SEL);
typedef void (*msg1)(id, SEL, id);
typedef id (*msg3)(id, SEL, id, id, id);
typedef BOOL (*bool_msg1)(id, SEL, id);
typedef NSInteger (*int_msg2)(id, SEL, id, id);

static void check_class(const char *name)
{
	Class cls = objc_getClass(name);
	CHECK(cls != Nil);
	if (cls == Nil) {
		return;
	}

	@try {
		// Instance side: one argument, three arguments, and a zero result.
		id instance = ((msg0)objc_msgSend)(((msg0)objc_msgSend)(cls, sel_registerName("alloc")), sel_registerName("init"));
		CHECK(instance != nil);
		((msg1)objc_msgSend)(instance, sel_registerName("setStubSettingEnabled:"), @"yes");
		CHECK(((msg3)objc_msgSend)(instance, sel_registerName("stubDoThing:withOther:andThird:"), @"a", @"b", @"c") == nil);
		CHECK(((bool_msg1)objc_msgSend)(instance, sel_registerName("stubIsValid:"), @"a") == NO);
		CHECK(((int_msg2)objc_msgSend)(instance, sel_registerName("stubCountFor:in:"), @"a", @"b") == 0);

		// The signature is real for implemented selectors and counts the colons for others.
		NSMethodSignature *known = [instance methodSignatureForSelector:@selector(description)];
		CHECK(known != nil && [known numberOfArguments] == 2 && strcmp([known methodReturnType], "@") == 0);
		NSMethodSignature *guessed = [instance methodSignatureForSelector:sel_registerName("a:b:c:d:")];
		CHECK(guessed != nil && [guessed numberOfArguments] == 6);
		NSMethodSignature *none = [instance methodSignatureForSelector:sel_registerName("noArgumentsStub")];
		CHECK(none != nil && [none numberOfArguments] == 2);

		// Class side.
		NSMethodSignature *classSignature = [cls methodSignatureForSelector:sel_registerName("stubClassThing:other:")];
		CHECK(classSignature != nil && [classSignature numberOfArguments] == 4);
		CHECK(((msg0)objc_msgSend)(cls, sel_registerName("stubSharedThing")) == nil);
		CHECK(((msg3)objc_msgSend)(cls, sel_registerName("stubClassDoThing:withOther:andThird:"), @"a", @"b", @"c") == nil);
		CHECK(((bool_msg1)objc_msgSend)(cls, sel_registerName("stubClassIsValid:"), @"a") == NO);
		[instance release];
	} @catch (NSException *exception) {
		printf("FAIL: %s raised %s: %s\n", name, [[exception name] UTF8String], [[exception reason] UTF8String]);
		failures++;
	}
}

int main(void)
{
	@autoreleasepool {
		void *webkit = dlopen("/System/Library/Frameworks/WebKit.framework/WebKit", RTLD_NOW);
		void *contacts = dlopen("/System/Library/Frameworks/Contacts.framework/Contacts", RTLD_NOW);
		CHECK(webkit != NULL);
		CHECK(contacts != NULL);

		check_class("WebPreferences");
		check_class("CNContactStore");
	}

	if (failures != 0) {
		return 1;
	}
	printf("PASS\n");
	return 0;
}
