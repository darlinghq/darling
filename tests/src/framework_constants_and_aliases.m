// Copyright (C) 2026 Lubos Dolezel
//
// Small framework surface: ASAuthorizationScopeEmail / FullName and CBCentralManagerOptionShowPowerAlertKey hold the
// strings macOS 26.7.3 holds in them; System.framework is loadable and exports the C library; WebView is an NSView
// and has -initWithFrame:frameName:groupName:.
#import <AppKit/AppKit.h>
#import <AuthenticationServices/AuthenticationServices.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <WebKit/WebKit.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	@autoreleasepool {
		CHECK([ASAuthorizationScopeEmail isEqualToString: @"email"]);
		CHECK([ASAuthorizationScopeFullName isEqualToString: @"full_name"]);
		CHECK([CBCentralManagerOptionShowPowerAlertKey isEqualToString: @"kCBInitOptionShowPowerAlert"]);

		void *h = dlopen("/System/Library/Frameworks/System.framework/System", RTLD_NOW);
		CHECK(h != NULL);
		CHECK(h && dlsym(h, "getpid") != NULL);

		Class cls = NSClassFromString(@"WebView");
		CHECK(cls != Nil && [cls isSubclassOfClass: [NSView class]]);
		id view = [[cls alloc] initWithFrame: NSMakeRect(0, 0, 10, 10) frameName: @"f" groupName: @"g"];
		CHECK(view != nil && [view isKindOfClass: [NSView class]]);
		[view release];
	}
	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
