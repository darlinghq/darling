// Copyright (C) 2026 Lubos Dolezel
//
// OSAScript keeps its source text and copies, a failed compile reports an error dictionary,
// OSALanguageInstance wraps a language, and OSAScriptView/OSATextStorage/OSAScriptController are AppKit
// classes. Only behaviour that does not need an AppleScript engine is checked, so the program
// also passes on macOS.
// Link with -framework OSAKit -framework AppKit.
#import <AppKit/AppKit.h>
#import <OSAKit/OSAKit.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL %d: %s\n", __LINE__, #cond); \
		g_failures++; \
	} \
} while (0)

int main(void)
{
	@autoreleasepool {
		OSALanguage *language = [OSALanguage languageForName: @"AppleScript"];
		CHECK(language != nil);

		// Source text round trip, with and without an explicit language
		OSAScript *script = [[[OSAScript alloc] initWithSource: @"display dialog \"hello\""] autorelease];
		CHECK([[script source] isEqualToString: @"display dialog \"hello\""]);
		CHECK([[script sourceAndReturnError: NULL] isEqualToString: @"display dialog \"hello\""]);
		CHECK([script language] != nil);

		OSAScript *withLanguage = [[[OSAScript alloc] initWithSource: @"1 + 1" language: language] autorelease];
		CHECK([[withLanguage source] isEqualToString: @"1 + 1"]);
		CHECK([[withLanguage language] isEqual: language]);

		// Language instances wrap the language they were made for
		OSALanguageInstance *instance = [OSALanguageInstance languageInstanceWithLanguage: language];
		CHECK(instance != nil);
		CHECK([[instance language] isEqual: language]);

		// Copying keeps the source
		OSAScript *copy = [[withLanguage copy] autorelease];
		CHECK(copy != nil && copy != withLanguage);
		CHECK([[copy source] isEqualToString: @"1 + 1"]);

		// A source that cannot compile reports an error dictionary with a message and a number
		OSAScript *bad = [[[OSAScript alloc] initWithSource: @"tell tell tell"] autorelease];
		NSDictionary *errorInfo = nil;
		CHECK(![bad compileAndReturnError: &errorInfo]);
		CHECK(errorInfo != nil);
		CHECK([errorInfo objectForKey: OSAScriptErrorMessageKey] != nil);
		CHECK([[errorInfo objectForKey: OSAScriptErrorNumberKey] isKindOfClass: [NSNumber class]]);
		CHECK(![bad isCompiled]);

		// The AppKit-based classes sit in the AppKit hierarchy
		CHECK([OSAScriptView isSubclassOfClass: [NSTextView class]]);
		CHECK([OSATextStorage isSubclassOfClass: [NSTextStorage class]]);
		CHECK([OSAScriptController isSubclassOfClass: [NSController class]]);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
