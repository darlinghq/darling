// CFLAGS: -framework Foundation -framework AppKit -framework WebKit
// Copyright (C) 2026 Darling Developers
//
// Legacy WebKit classes that only need to exist and be linked: the class hierarchy of WebKitLegacy's
// public headers and the WebKit 2 find configuration class. Their unimplemented selectors are
// forwarded to the stub handler (WK_FORWARD_UNIMPLEMENTED_SELECTORS), which returns nil.
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>
#import <objc/runtime.h>
#include <stdio.h>

static int failures;

static void expectClass(const char *name, const char *superName)
{
	Class cls = NSClassFromString([NSString stringWithUTF8String:name]);
	if (!cls) {
		printf("FAIL: class %s is missing\n", name);
		failures++;
		return;
	}
	Class expected = NSClassFromString([NSString stringWithUTF8String:superName]);
	if (!expected || class_getSuperclass(cls) != expected) {
		Class actual = class_getSuperclass(cls);
		printf("FAIL: superclass of %s is %s, expected %s\n", name,
			actual ? class_getName(actual) : "(nil)", superName);
		failures++;
	}
}

// A selector the class does not implement must reach the stub handler and return nil, not raise.
static void expectUnimplementedSelectorIsStubbed(const char *name)
{
	Class cls = NSClassFromString([NSString stringWithUTF8String:name]);
	if (!cls)
		return;
	@try {
		id object = [[cls alloc] init];
		id result = [object performSelector:NSSelectorFromString(@"darlingPlaceholderUnimplementedSelector")];
		if (result != nil) {
			printf("FAIL: unimplemented selector on %s returned a value\n", name);
			failures++;
		}
	} @catch (id exception) {
		printf("FAIL: unimplemented selector on %s raised\n", name);
		failures++;
	}
}

int main(void)
{
	@autoreleasepool {
		expectClass("DOMCharacterData", "DOMNode");
		expectClass("DOMComment", "DOMCharacterData");
		expectClass("DOMHTMLElement", "DOMElement");
		expectClass("DOMRange", "DOMObject");

		expectUnimplementedSelectorIsStubbed("DOMCharacterData");
	}
	if (failures == 0)
		printf("OK\n");
	return failures != 0;
}
