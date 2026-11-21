// Copyright (C) 2026 Lubos Dolezel
//
// LSCopyItemInfoForURL: kLSItemInfoIsPackage agrees with kCFURLIsPackageKey. Whether a directory
// is a package follows its type (an extension whose registered type conforms to com.apple.package),
// not a fixed list of extensions: .app, .bundle and .pages directories are packages, a .framework
// directory, a plain folder and a symbolic link to a package are not.
// Runs unchanged on macOS. Run with no arguments.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } else { printf("PASS %s\n", #cond); } } while (0)

static int urlIsPackage(NSURL *url)
{
	NSNumber *value = nil;
	if (![url getResourceValue:&value forKey:NSURLIsPackageKey error:NULL] || value == nil)
		return -1;
	return [value boolValue] ? 1 : 0;
}

// The package flag of LSCopyItemInfoForURL, or -1 when the call fails.
static int infoIsPackage(NSURL *url)
{
	LSItemInfoRecord info;
	if (LSCopyItemInfoForURL((CFURLRef) url, kLSRequestBasicFlagsOnly, &info) != noErr)
		return -1;
	return (info.flags & kLSItemInfoIsPackage) ? 1 : 0;
}

// Both answers for a name under dir, which must match each other and expected.
static void checkEntry(NSString *dir, NSString *name, int expected)
{
	NSURL *url = [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:name]];
	int viaInfo = infoIsPackage(url);
	int viaKey = urlIsPackage(url);
	printf("%s: LSCopyItemInfoForURL %d, IsPackageKey %d (expected %d)\n", [name UTF8String], viaInfo, viaKey, expected);
	if (viaInfo != viaKey || viaInfo != expected) {
		printf("FAIL %s\n", [name UTF8String]);
		g_failures++;
	} else {
		printf("PASS %s\n", [name UTF8String]);
	}
}

int main(int argc, char **argv)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSFileManager *fm = [NSFileManager defaultManager];

	char dirTemplate[] = "/tmp/lsitempackage.XXXXXX";
	NSString *dir = [NSString stringWithUTF8String:mkdtemp(dirTemplate)];
	for (NSString *name in [NSArray arrayWithObjects:@"plain", @"a.app", @"b.framework", @"c.bundle", @"d.pages", @"e.folder", @"f.noext", nil])
		[fm createDirectoryAtPath:[dir stringByAppendingPathComponent:name] withIntermediateDirectories:NO attributes:nil error:NULL];
	[fm createSymbolicLinkAtPath:[dir stringByAppendingPathComponent:@"link.app"] withDestinationPath:@"a.app" error:NULL];
	[fm createSymbolicLinkAtPath:[dir stringByAppendingPathComponent:@"link.framework"] withDestinationPath:@"b.framework" error:NULL];
	[fm createSymbolicLinkAtPath:[dir stringByAppendingPathComponent:@"link.plain"] withDestinationPath:@"plain" error:NULL];
	[@"x" writeToFile:[dir stringByAppendingPathComponent:@"file.app"] atomically:NO encoding:NSUTF8StringEncoding error:NULL];

	checkEntry(dir, @"plain", 0);
	checkEntry(dir, @"a.app", 1);
	checkEntry(dir, @"b.framework", 0);
	checkEntry(dir, @"c.bundle", 1);
	checkEntry(dir, @"d.pages", 1);
	checkEntry(dir, @"e.folder", 0);
	checkEntry(dir, @"f.noext", 0);
	checkEntry(dir, @"link.app", 0);
	checkEntry(dir, @"link.framework", 0);
	checkEntry(dir, @"link.plain", 0);
	checkEntry(dir, @"file.app", 0);

	[fm removeItemAtPath:dir error:NULL];
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	[pool drain];
	return g_failures != 0;
}
