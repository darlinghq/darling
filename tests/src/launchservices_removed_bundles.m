/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/
// Registrations of bundles that were removed stay in the LaunchServices database. A lookup must not
// return an application whose bundle is gone: it skips it and finds the next handler, or none.
// Two applications declare the same document extension and URL scheme; the preferred one is
// removed after both are registered.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static NSString* MakeApp(NSString* root, NSString* name, NSString* bundleID, NSString* extension, NSString* scheme, NSString* rank)
{
	NSString* app = [root stringByAppendingPathComponent:[name stringByAppendingString:@".app"]];
	NSString* macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
	[[NSFileManager defaultManager] createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
	[@"x" writeToFile:[macos stringByAppendingPathComponent:@"app"] atomically:YES encoding:NSUTF8StringEncoding error:NULL];
	NSDictionary* info = @{
		@"CFBundleIdentifier": bundleID,
		@"CFBundleExecutable": @"app",
		@"CFBundlePackageType": @"APPL",
		@"CFBundleDocumentTypes": @[ @{
			@"CFBundleTypeRole": @"Editor",
			@"LSHandlerRank": rank,
			@"CFBundleTypeExtensions": @[ extension ],
		} ],
		@"CFBundleURLTypes": @[ @{
			@"CFBundleURLName": bundleID,
			@"CFBundleURLSchemes": @[ scheme ],
		} ],
	};
	[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
	return app;
}

static NSString* PathOf(CFURLRef url)
{
	return url ? [(__bridge NSURL*) url path] : nil;
}

int main(void)
{
	@autoreleasepool {
		NSString* tag = [NSString stringWithFormat:@"%d", (int) getpid()];
		NSString* ext = [@"lsrb" stringByAppendingString:tag];
		NSString* scheme = [@"lsrb" stringByAppendingString:tag];
		NSString* idOwner = [@"org.darlinghq.test.lsrb.owner" stringByAppendingString:tag];
		NSString* idAlt = [@"org.darlinghq.test.lsrb.alt" stringByAppendingString:tag];
		// Not under /tmp: LaunchServices does not index applications there.
		NSString* root = [NSHomeDirectory() stringByAppendingPathComponent:[@"lsrb-" stringByAppendingString:tag]];
		NSString* owner = MakeApp(root, @"Owner", idOwner, ext, scheme, @"Owner");
		NSString* alt = MakeApp(root, @"Alt", idAlt, ext, scheme, @"Alternate");
		NSString* doc = [root stringByAppendingPathComponent:[@"a." stringByAppendingString:ext]];
		[@"x" writeToFile:doc atomically:YES encoding:NSUTF8StringEncoding error:NULL];
		NSURL* docURL = [NSURL fileURLWithPath:doc];

		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:owner], true) == noErr);
		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:alt], true) == noErr);

		CFURLRef h = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) docURL, kLSRolesAll, NULL);
		CHECK([PathOf(h) hasSuffix:@"Owner.app"]);
		if (h) CFRelease(h);

		// The preferred application goes away.
		[[NSFileManager defaultManager] removeItemAtPath:owner error:NULL];
		CHECK(![[NSFileManager defaultManager] fileExistsAtPath:owner]);

		h = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) docURL, kLSRolesAll, NULL);
		CHECK([PathOf(h) hasSuffix:@"Alt.app"]);
		if (h) CFRelease(h);

		FSRef ref;
		CFURLRef url = NULL;
		CHECK(LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, (__bridge CFStringRef) ext, kLSRolesAll, &ref, &url) == noErr);
		CHECK([PathOf(url) hasSuffix:@"Alt.app"]);
		if (url) CFRelease(url);

		// By bundle identifier the removed application has no location at all.
		CFArrayRef apps = LSCopyApplicationURLsForBundleIdentifier((__bridge CFStringRef) idOwner, NULL);
		CHECK(apps == NULL || CFArrayGetCount(apps) == 0);
		if (apps) CFRelease(apps);
		url = NULL;
		CHECK(LSFindApplicationForInfo(kLSUnknownCreator, (__bridge CFStringRef) idOwner, NULL, &ref, &url) == kLSApplicationNotFoundErr);
		if (url) CFRelease(url);
		url = NULL;
		CHECK(LSFindApplicationForInfo(kLSUnknownCreator, (__bridge CFStringRef) idAlt, NULL, &ref, &url) == noErr);
		if (url) CFRelease(url);

		// The scheme handler is the application that is still there.
		CFStringRef handler = LSCopyDefaultHandlerForURLScheme((__bridge CFStringRef) scheme);
		CHECK(handler != NULL && [(__bridge NSString*) handler isEqualToString:idAlt]);
		if (handler) CFRelease(handler);

		// With both gone there is no handler.
		[[NSFileManager defaultManager] removeItemAtPath:alt error:NULL];
		h = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) docURL, kLSRolesAll, NULL);
		CHECK(h == NULL);
		if (h) CFRelease(h);
		handler = LSCopyDefaultHandlerForURLScheme((__bridge CFStringRef) scheme);
		CHECK(handler == NULL);
		if (handler) CFRelease(handler);

		// Opening the document reports that there is no application instead of launching a missing one.
		LSLaunchURLSpec spec;
		memset(&spec, 0, sizeof(spec));
		spec.itemURLs = (__bridge CFArrayRef) @[ docURL ];
		CHECK(LSOpenFromURLSpec(&spec, NULL) == kLSApplicationNotFoundErr);

		[[NSFileManager defaultManager] removeItemAtPath:root error:NULL];
	}
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
