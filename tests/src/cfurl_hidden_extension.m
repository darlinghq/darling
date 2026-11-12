// Copyright (C) 2026 Lubos Dolezel
//
// NSURLHasHiddenExtensionKey (kCFURLHasHiddenExtensionKey) reads and writes the
// extension-hidden bit (0x0010 of the big-endian flags word at byte 8) of the item's
// com.apple.FinderInfo attribute, keeping the other bytes; LaunchServices' extension-hidden
// attribute, item-info flag and LSSetExtensionHiddenForURL follow it. As observed on macOS
// 27.0.1 (arm64 and x86_64). Passes unchanged on macOS.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static BOOL hidden(NSURL* url)
{
	// A fresh URL object, so that no cached value is read.
	NSURL* fresh = [NSURL fileURLWithPath:[url path]];
	NSNumber* value = nil;
	NSError* error = nil;
	BOOL ok = [fresh getResourceValue:&value forKey:NSURLHasHiddenExtensionKey error:&error];
	CHECK(ok && value != nil);
	return [value boolValue];
}

static ssize_t finderInfo(NSString* path, unsigned char* info)
{
	memset(info, 0, 32);
	return getxattr([path fileSystemRepresentation], "com.apple.FinderInfo", info, 32, 0, XATTR_NOFOLLOW);
}

static BOOL lsAttribute(NSURL* url)
{
	FSRef ref;
	CHECK(CFURLGetFSRef((__bridge CFURLRef) url, &ref));
	CFTypeRef value = NULL;
	OSStatus status = LSCopyItemAttribute(&ref, kLSRolesAll, kLSItemExtensionIsHidden, &value);
	CHECK(status == noErr && value != NULL);
	BOOL result = value != NULL && CFBooleanGetValue(value);
	if (value)
		CFRelease(value);
	return result;
}

static BOOL lsFlag(NSURL* url, LSRequestedInfo which)
{
	LSItemInfoRecord record;
	memset(&record, 0, sizeof(record));
	OSStatus status = LSCopyItemInfoForURL((__bridge CFURLRef) url, which, &record);
	CHECK(status == noErr);
	return (record.flags & kLSItemInfoExtensionIsHidden) != 0;
}

int main(void)
{
	@autoreleasepool {
		char tmpl[] = "/tmp/hidden-extension-XXXXXX";
		NSString* dir = [NSString stringWithUTF8String:mkdtemp(tmpl)];
		NSString* file = [dir stringByAppendingPathComponent:@"doc.txt"];
		[@"x" writeToFile:file atomically:NO encoding:NSUTF8StringEncoding error:nil];
		NSURL* url = [NSURL fileURLWithPath:file];
		unsigned char info[32];

		// A new file does not have the flag, and no FinderInfo.
		CHECK(!hidden(url));
		CHECK(!lsAttribute(url));
		CHECK(!lsFlag(url, kLSRequestAllFlags));
		CHECK(finderInfo(file, info) < 0);

		// Setting it creates a 32-byte FinderInfo with only the flag set.
		NSError* error = nil;
		CHECK([url setResourceValue:@YES forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(hidden(url));
		CHECK(finderInfo(file, info) == 32);
		CHECK(info[8] == 0x00 && info[9] == 0x10);
		for (int i = 0; i < 32; i++)
			if (i != 9)
				CHECK(info[i] == 0);
		CHECK(lsAttribute([NSURL fileURLWithPath:file]));
		CHECK(lsFlag([NSURL fileURLWithPath:file], kLSRequestAllFlags));
		CHECK(lsFlag([NSURL fileURLWithPath:file], kLSRequestBasicFlagsOnly));
		CHECK(lsFlag([NSURL fileURLWithPath:file], kLSRequestExtensionFlagsOnly));

		// Other FinderInfo bytes and flags are kept when the flag changes.
		unsigned char other[32] = { 'T', 'E', 'X', 'T' };
		other[8] = 0x40; // kIsInvisible
		CHECK(setxattr([file fileSystemRepresentation], "com.apple.FinderInfo", other, 32, 0, 0) == 0);
		CHECK(!hidden(url));
		CHECK([[NSURL fileURLWithPath:file] setResourceValue:@YES forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(finderInfo(file, info) == 32);
		CHECK(info[0] == 'T' && info[3] == 'T' && info[8] == 0x40 && info[9] == 0x10);
		CHECK([[NSURL fileURLWithPath:file] setResourceValue:@NO forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(finderInfo(file, info) == 32);
		CHECK(info[0] == 'T' && info[8] == 0x40 && info[9] == 0x00);
		CHECK(!hidden(url));

		// LSSetExtensionHiddenForURL and ...ForRef write the same flag.
		CHECK(LSSetExtensionHiddenForURL((__bridge CFURLRef) [NSURL fileURLWithPath:file], true) == noErr);
		CHECK(hidden(url));
		CHECK(LSSetExtensionHiddenForURL((__bridge CFURLRef) [NSURL fileURLWithPath:file], false) == noErr);
		CHECK(!hidden(url));
		FSRef ref;
		CHECK(CFURLGetFSRef((__bridge CFURLRef) url, &ref));
		CHECK(LSSetExtensionHiddenForRef(&ref, true) == noErr);
		CHECK(hidden(url));

		// Clearing the flag of a file without FinderInfo succeeds and creates nothing.
		NSString* plain = [dir stringByAppendingPathComponent:@"plain.txt"];
		[@"x" writeToFile:plain atomically:NO encoding:NSUTF8StringEncoding error:nil];
		CHECK([[NSURL fileURLWithPath:plain] setResourceValue:@NO forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(finderInfo(plain, info) < 0);

		// Directories carry the flag as well.
		NSString* sub = [dir stringByAppendingPathComponent:@"folder.dir"];
		CHECK(mkdir([sub fileSystemRepresentation], 0755) == 0);
		NSURL* subURL = [NSURL fileURLWithPath:sub isDirectory:YES];
		CHECK([subURL setResourceValue:@YES forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(hidden(subURL));
		CHECK(finderInfo(sub, info) == 32 && info[9] == 0x10);

		// A symbolic link has its own (empty) FinderInfo: it does not read the target's flag.
		NSString* link = [dir stringByAppendingPathComponent:@"link.txt"];
		CHECK(symlink([file fileSystemRepresentation], [link fileSystemRepresentation]) == 0);
		CHECK(hidden(url));
		CHECK(!hidden([NSURL fileURLWithPath:link]));

		// A missing item is an error on read and write.
		NSURL* missing = [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"missing.txt"]];
		NSNumber* value = nil;
		error = nil;
		CHECK(![missing getResourceValue:&value forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(value == nil && error != nil);
		error = nil;
		CHECK(![missing setResourceValue:@YES forKey:NSURLHasHiddenExtensionKey error:&error]);
		CHECK(error != nil);
		CHECK(LSSetExtensionHiddenForURL((__bridge CFURLRef) missing, true) == fnfErr);
		CHECK(LSSetExtensionHiddenForURL(NULL, true) == paramErr);
		LSItemInfoRecord record;
		CHECK(LSCopyItemInfoForURL((__bridge CFURLRef) missing, kLSRequestAllFlags, &record) == fnfErr);

		[[NSFileManager defaultManager] removeItemAtPath:dir error:nil];
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
