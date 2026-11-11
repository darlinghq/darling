// Copyright (C) 2026 Lubos Dolezel
//
// OSAKit error dictionary keys and storage type names, and the OpenScripting
// applet Info.plist keys.
// Link with -framework OSAKit -framework Carbon. The program passes unchanged on macOS 27.0.1.
#import <Carbon/Carbon.h>
#import <OSAKit/OSAKit.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK_STRING(constant, expected) do { \
	if (![(constant) isEqualToString:(expected)]) { \
		printf("FAIL %d: %s is %s\n", __LINE__, #constant, [(constant) UTF8String]); \
		g_failures++; \
	} \
} while (0)

extern NSString* const OSAAppletShowStartupScreenKey;
extern NSString* const OSAAppletStayOpenKey;

int main(void)
{
	@autoreleasepool {
		CHECK_STRING(OSAScriptErrorMessageKey, @"OSAScriptErrorMessageKey");
		CHECK_STRING(OSAScriptErrorBriefMessageKey, @"OSAScriptErrorBriefMessageKey");
		CHECK_STRING(OSAScriptErrorNumberKey, @"OSAScriptErrorNumberKey");
		CHECK_STRING(OSAScriptErrorPartialResultKey, @"OSAScriptErrorPartialResultKey");
		CHECK_STRING(OSAScriptErrorOffendingObjectKey, @"OSAScriptErrorOffendingObjectKey");
		CHECK_STRING(OSAScriptErrorExpectedTypeKey, @"OSAScriptErrorExpectedTypeKey");
		CHECK_STRING(OSAScriptErrorAppAddressKey, @"OSAScriptErrorAppAddressKey");
		CHECK_STRING(OSAScriptErrorRangeKey, @"OSAScriptErrorRangeKey");

		CHECK_STRING(OSAStorageScriptType, @"script");
		CHECK_STRING(OSAStorageScriptBundleType, @"script bundle");
		CHECK_STRING(OSAStorageApplicationType, @"application");
		CHECK_STRING(OSAStorageApplicationBundleType, @"application bundle");
		CHECK_STRING(OSAStorageTextType, @"text");

		CHECK_STRING(OSAAppletShowStartupScreenKey, @"OSAAppletShowStartupScreen");
		CHECK_STRING(OSAAppletStayOpenKey, @"OSAAppletStayOpen");
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
