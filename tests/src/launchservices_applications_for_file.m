// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices: LSCopyApplicationURLsForURL for a file URL answers the registered applications that
// declare the file's name extension, and NULL when there are none. Run with no arguments.
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
		NSString *app = [apps stringByAppendingPathComponent:[NSString stringWithFormat:@"darlingfileapp%@.app", tag]];
		NSString *macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
		[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
		[fm copyItemAtPath:[[NSBundle mainBundle] executablePath] toPath:[macos stringByAppendingPathComponent:@"x"] error:NULL];
		NSString *extension = [@"dfa" stringByAppendingString:tag];
		[@{
			@"CFBundleIdentifier": [@"org.darlinghq.test.fileapp" stringByAppendingString:tag],
			@"CFBundleExecutable": @"x", @"CFBundlePackageType": @"APPL", @"CFBundleVersion": @"1", @"CFBundleName": @"x",
			@"CFBundleDocumentTypes": @[ @{ @"CFBundleTypeName": @"Test", @"CFBundleTypeRole": @"Editor",
				@"CFBundleTypeExtensions": @[ extension ] } ],
		} writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];

		NSURL *file = [NSURL fileURLWithPath:[@"/tmp/document." stringByAppendingString:extension]];
		NSURL *upper = [NSURL fileURLWithPath:[@"/tmp/document." stringByAppendingString:[extension uppercaseString]]];
		NSURL *noExtension = [NSURL fileURLWithPath:@"/tmp/document"];

		// Not known before the registration.
		CHECK(LSCopyApplicationURLsForURL((__bridge CFURLRef) file, kLSRolesAll) == NULL);

		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true) == noErr);

		NSArray *found = CFBridgingRelease(LSCopyApplicationURLsForURL((__bridge CFURLRef) file, kLSRolesAll));
		CHECK([found count] == 1);
		CHECK([found count] == 1 && [[[found[0] path] stringByResolvingSymlinksInPath] isEqualToString:[app stringByResolvingSymlinksInPath]]);
		// The extension is matched without regard to case; a file without one has no applications.
		NSArray *foundUpper = CFBridgingRelease(LSCopyApplicationURLsForURL((__bridge CFURLRef) upper, kLSRolesAll));
		CHECK([foundUpper count] == 1);
		CHECK(LSCopyApplicationURLsForURL((__bridge CFURLRef) noExtension, kLSRolesAll) == NULL);
		CHECK(LSCopyApplicationURLsForURL(NULL, kLSRolesAll) == NULL);

		[fm removeItemAtPath:app error:NULL];
		// A bundle that was removed is not an answer.
		CHECK(LSCopyApplicationURLsForURL((__bridge CFURLRef) file, kLSRolesAll) == NULL);
		printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	}
	return g_failures != 0;
}
