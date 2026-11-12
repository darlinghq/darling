// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices: LSRegisterURL registers an application bundle together with its CFBundleURLTypes, and
// the URL-scheme lookups then find it. Error results are those macOS reports for bad input.
// Run with no arguments.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(int argc, char **argv)
{
	@autoreleasepool {
		NSFileManager *fm = [NSFileManager defaultManager];
		NSString *apps = [NSHomeDirectory() stringByAppendingPathComponent:@"Applications"];
		[fm createDirectoryAtPath:apps withIntermediateDirectories:YES attributes:nil error:NULL];
		// Registrations of removed bundles stay in the database, so each run uses its own names.
		NSString *tag = [NSString stringWithFormat:@"%d", (int) getpid()];
		NSString *app = [apps stringByAppendingPathComponent:[NSString stringWithFormat:@"darlingregisterurl%@.app", tag]];
		NSString *macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
		[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
		[fm copyItemAtPath:[[NSBundle mainBundle] executablePath] toPath:[macos stringByAppendingPathComponent:@"x"] error:NULL];
		NSString *bid = [@"org.darlinghq.test.registerurl" stringByAppendingString:tag];
		NSString *schemeName = [@"darlingregisterscheme" stringByAppendingString:tag];
		[@{
			@"CFBundleIdentifier": bid, @"CFBundleExecutable": @"x", @"CFBundlePackageType": @"APPL",
			@"CFBundleVersion": @"1", @"CFBundleName": @"x",
			@"CFBundleURLTypes": @[ @{ @"CFBundleURLName": bid, @"CFBundleURLSchemes": @[ schemeName ] } ],
		} writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];

		// Bad input.
		CHECK(LSRegisterURL(NULL, true) == paramErr);
		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL URLWithString:@"http://example.com/a"], true) == paramErr);
		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:@"/nonexistent/x.app"], true) == fnfErr);
		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:@"/etc/hosts"], true) == kLSNotAnApplicationErr);

		// Not known before.
		CFStringRef scheme = (__bridge CFStringRef) schemeName;
		CHECK(LSCopyAllHandlersForURLScheme(scheme) == NULL);

		OSStatus st = LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true);
		printf("LSRegisterURL=%d\n", (int) st);
		CHECK(st == noErr);

		CFStringRef handler = LSCopyDefaultHandlerForURLScheme(scheme);
		CHECK(handler && [(__bridge NSString *) handler caseInsensitiveCompare:bid] == NSOrderedSame);
		if (handler) CFRelease(handler);

		NSArray *all = CFBridgingRelease(LSCopyAllHandlersForURLScheme(scheme));
		CHECK([all count] == 1 && [all[0] caseInsensitiveCompare:bid] == NSOrderedSame);

		NSURL *urlWithScheme = [NSURL URLWithString:[schemeName stringByAppendingString:@"://x/y"]];
		NSArray *appURLs = CFBridgingRelease(LSCopyApplicationURLsForURL((__bridge CFURLRef) urlWithScheme, kLSRolesAll));
		CHECK([appURLs count] == 1);
		CHECK([appURLs count] == 1 && [[[appURLs[0] path] stringByResolvingSymlinksInPath] isEqualToString:[app stringByResolvingSymlinksInPath]]);

		CFErrorRef error = NULL;
		CFURLRef def = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) urlWithScheme, kLSRolesAll, &error);
		CHECK(def != NULL);
		if (def) CFRelease(def);

		// An unregistered scheme has no handler and no application.
		NSURL *unknown = [NSURL URLWithString:@"darlingnosuchscheme://x"];
		CHECK(LSCopyApplicationURLsForURL((__bridge CFURLRef) unknown, kLSRolesAll) == NULL);
		CFURLRef none = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) unknown, kLSRolesAll, &error);
		CHECK(none == NULL && error && CFErrorGetCode(error) == kLSApplicationNotFoundErr);
		LSLaunchURLSpec spec = { 0 };
		spec.itemURLs = (__bridge CFArrayRef) @[ unknown ];
		CHECK(LSOpenFromURLSpec(&spec, NULL) == kLSApplicationNotFoundErr);

		[fm removeItemAtPath:app error:NULL];
		printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	}
	return g_failures != 0;
}
