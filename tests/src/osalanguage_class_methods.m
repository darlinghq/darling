// Copyright (C) 2026 Lubos Dolezel
//
// OSALanguage class methods and accessors for the AppleScript language:
// availableLanguages, defaultLanguage and languageForName:, with the language's
// name, version, component type, subtype and manufacturer. Unknown names give nil.
// Link with -framework OSAKit. The program passes unchanged on macOS 27.0.1
// (arm64 and x86_64), where JavaScript is available as well.
#import <OSAKit/OSAKit.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		OSALanguage *appleScript = [OSALanguage languageForName:@"AppleScript"];
		CHECK(appleScript != nil);
		CHECK([[appleScript name] isEqualToString:@"AppleScript"]);
		CHECK([[appleScript version] isEqualToString:@"2.8"]);
		CHECK([appleScript type] == 'osa ');
		CHECK([appleScript subType] == 'ascr');
		CHECK([appleScript manufacturer] == 'appl');
		CHECK([appleScript isThreadSafe]);

		CHECK([[OSALanguage availableLanguages] containsObject:appleScript]);
		CHECK([OSALanguage defaultLanguage] == appleScript);
		CHECK([OSALanguage languageForName:@"NoSuchLanguage"] == nil);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
