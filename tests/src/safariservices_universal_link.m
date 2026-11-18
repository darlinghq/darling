// CFLAGS: -framework Foundation -framework SafariServices
// Copyright (C) 2026 Lubos Dolezel
//
// SafariServices: SFUniversalLink exists and -initWithWebpageURL: answers nil for URLs
// that no installed app handles as a universal link (web, non-web and relative URLs).
// The program passes unchanged on macOS 26.7.3 (arm64).
#import <Foundation/Foundation.h>
#import <SafariServices/SafariServices.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wnonnull"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		Class cls = NSClassFromString(@"SFUniversalLink");
		CHECK(cls != Nil);
		CHECK(cls == [SFUniversalLink class]);
		CHECK([cls isSubclassOfClass:[NSObject class]]);

		NSArray *urls = @[ @"https://example.com/", @"http://example.com/x?y=1", @"ftp://example.com/",
			@"file:///tmp/x", @"foo:bar", @"/relative" ];
		for (NSString *string in urls) {
			SFUniversalLink *link = [[SFUniversalLink alloc] initWithWebpageURL:[NSURL URLWithString:string]];
			CHECK(link == nil);
			if (link != nil)
				printf("  unexpected link for %s\n", [string UTF8String]);
		}
		CHECK([[SFUniversalLink alloc] initWithWebpageURL:nil] == nil);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
