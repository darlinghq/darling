// Copyright (C) 2026 Lubos Dolezel
//
// WebKit's legacy WebView API: the WebUndefined singleton class and the
// string values of the page-load progress notification names. The program
// passes unchanged on macOS 26.
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#import <objc/runtime.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		CHECK([WebViewProgressStartedNotification isEqualToString:@"WebProgressStartedNotification"]);
		CHECK([WebViewProgressEstimateChangedNotification isEqualToString:@"WebProgressEstimateChangedNotification"]);
		CHECK([WebViewProgressFinishedNotification isEqualToString:@"WebProgressFinishedNotification"]);

		Class cls = NSClassFromString(@"WebUndefined");
		CHECK(cls != Nil);
		CHECK(class_getSuperclass(cls) == [NSObject class]);

		WebUndefined *undef = [WebUndefined undefined];
		CHECK(undef != nil);
		CHECK([undef isKindOfClass:cls]);
		// There is only one undefined value.
		CHECK([WebUndefined undefined] == undef);
		CHECK([[WebUndefined alloc] init] == undef);
		CHECK([undef copy] == undef);
		CHECK([undef retain] == undef);
		[undef release];
		CHECK([undef autorelease] == undef);
		CHECK([undef conformsToProtocol:@protocol(NSCopying)]);
		CHECK([undef conformsToProtocol:@protocol(NSCoding)]);
		CHECK([[undef description] isEqualToString:@"undefined"]);

		// Notifications posted under the constants reach observers of the same name.
		__block int seen = 0;
		id token = [[NSNotificationCenter defaultCenter] addObserverForName:WebViewProgressStartedNotification
			object:nil queue:nil usingBlock:^(NSNotification *n) { seen++; }];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"WebProgressStartedNotification" object:nil];
		CHECK(seen == 1);
		[[NSNotificationCenter defaultCenter] removeObserver:token];
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
