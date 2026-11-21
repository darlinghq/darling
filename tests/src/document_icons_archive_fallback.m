/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/
// A type that conforms to public.archive and has no handler declaring an icon gets the generic archive
// icon, not the generic document icon. The type is declared by a made-up application without any icon.
#import <AppKit/AppKit.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static NSData* PixelsOf(NSImage* image)
{
	return [image TIFFRepresentation];
}

int main(void)
{
	@autoreleasepool {
		NSString* tag = [NSString stringWithFormat:@"%d", (int) getpid()];
		NSString* root = [NSHomeDirectory() stringByAppendingPathComponent:[@"docicons-arc-" stringByAppendingString:tag]];
		NSString* ext = [@"dicz" stringByAppendingString:tag];
		NSString* uti = [@"org.darlinghq.test.docicons.archive" stringByAppendingString:tag];
		NSString* app = [root stringByAppendingPathComponent:@"Arc.app"];
		NSString* macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
		[[NSFileManager defaultManager] createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
		[@"x" writeToFile:[macos stringByAppendingPathComponent:@"app"] atomically:YES encoding:NSUTF8StringEncoding error:NULL];
		NSDictionary* info = @{
			@"CFBundleIdentifier": [@"org.darlinghq.test.docicons.arc" stringByAppendingString:tag],
			@"CFBundleExecutable": @"app",
			@"CFBundlePackageType": @"APPL",
			@"UTExportedTypeDeclarations": @[ @{
				@"UTTypeIdentifier": uti,
				@"UTTypeConformsTo": @[ @"public.archive" ],
				@"UTTypeTagSpecification": @{ @"public.filename-extension": @[ ext ] },
			} ],
		};
		[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
		CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true) == noErr);

		NSWorkspace* ws = [NSWorkspace sharedWorkspace];
		NSImage* archive = [ws iconForFileType:uti];
		NSImage* generic = [ws iconForFileType:@"public.data"];
		CHECK(archive != nil && generic != nil);
		CHECK(![PixelsOf(archive) isEqualToData:PixelsOf(generic)]);
		CHECK([PixelsOf([ws iconForFileType:ext]) isEqualToData:PixelsOf(archive)]);

		NSString* file = [root stringByAppendingPathComponent:[@"a." stringByAppendingString:ext]];
		[@"x" writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:NULL];
		CHECK([PixelsOf([ws iconForFile:file]) isEqualToData:PixelsOf(archive)]);

		[[NSFileManager defaultManager] removeItemAtPath:root error:NULL];
	}
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
