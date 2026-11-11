// CFLAGS: -framework Foundation -framework AppKit -framework FinderSync
// Copyright (C) 2026 Lubos Dolezel

#import <Foundation/Foundation.h>
#import <FinderSync/FinderSync.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

int main(void)
{
	@autoreleasepool {
		FIFinderSyncController *c = [FIFinderSyncController defaultController];
		check(c != nil);
		check([FIFinderSyncController defaultController] == c);

		// directoryURLs starts out empty and is copied on assignment.
		check([[c directoryURLs] count] == 0);
		NSURL *dir = [NSURL fileURLWithPath:@"/tmp"];
		NSMutableSet *set = [NSMutableSet setWithObject:dir];
		c.directoryURLs = set;
		[set removeAllObjects];
		check([[c directoryURLs] count] == 1);
		check([[c directoryURLs] containsObject:dir]);

		// Badge identifiers are kept per URL and can be cleared.
		NSURL *file = [NSURL fileURLWithPath:@"/tmp/file"];
		check([c badgeIdentifierForURL:file] == nil);
		[c setBadgeIdentifier:@"synced" forURL:file];
		check([[c badgeIdentifierForURL:file] isEqualToString:@"synced"]);
		[c setBadgeIdentifier:nil forURL:file];
		check([c badgeIdentifierForURL:file] == nil);
		[c setBadgeIdentifier:@"x" forURL:nil];
		check([c badgeIdentifierForURL:nil] == nil);

		// No extension is ever enabled.
		check(![FIFinderSyncController isExtensionEnabled]);
	}
	return failures ? 1 : 0;
}
