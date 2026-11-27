// Copyright (C) 2026 Lubos Dolezel
//
// The shared WebPreferences object of the WebKit framework: +standardPreferences returns one object, whose
// font families and sizes, boolean settings and user style sheet location have values (the standard font is
// Times 16 and the fixed one Courier 13, as in Safari) and can be changed. Setting a nil font family or a
// size of zero or less restores the default.
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <objc/message.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

#define SEND(type, obj, sel, ...) ((type (*)(id, SEL, ...)) objc_msgSend)(obj, sel_registerName(sel), ##__VA_ARGS__)

int main(void) {
    @autoreleasepool {
        // the framework is loaded by name, as an application linking it would have it
        NSBundle *bundle = [NSBundle bundleWithPath: @"/System/Library/Frameworks/WebKit.framework"];
        CHECK([bundle load]);
        Class cls = NSClassFromString(@"WebPreferences");
        CHECK(cls != Nil);
        id prefs = SEND(id, cls, "standardPreferences");
        CHECK(prefs != nil);
        CHECK(SEND(id, cls, "standardPreferences") == prefs);

        NSString *standard = SEND(id, prefs, "standardFontFamily");
        NSString *fixed = SEND(id, prefs, "fixedFontFamily");
        CHECK([standard isEqualToString: @"Times"]);
        CHECK([fixed isEqualToString: @"Courier"]);
        CHECK(SEND(int, prefs, "defaultFontSize") == 16);
        CHECK(SEND(int, prefs, "defaultFixedFontSize") == 13);
        // the families are usable to make a font
        CHECK([NSFont fontWithName: standard size: 16] != nil);
        CHECK([NSFont fontWithName: fixed size: 13] != nil);

        CHECK(SEND(BOOL, prefs, "isJavaScriptEnabled") == YES);
        CHECK(SEND(BOOL, prefs, "javaScriptCanOpenWindowsAutomatically") == NO);
        CHECK(SEND(BOOL, prefs, "loadsImagesAutomatically") == YES);
        CHECK(SEND(BOOL, prefs, "userStyleSheetEnabled") == NO);
        CHECK(SEND(id, prefs, "userStyleSheetLocation") == nil);

        SEND(void, prefs, "setStandardFontFamily:", @"Helvetica");
        SEND(void, prefs, "setDefaultFontSize:", 20);
        SEND(void, prefs, "setJavaScriptEnabled:", NO);
        NSURL *url = [NSURL URLWithString: @"file:///tmp/style.css"];
        SEND(void, prefs, "setUserStyleSheetLocation:", url);
        CHECK([SEND(id, prefs, "standardFontFamily") isEqualToString: @"Helvetica"]);
        CHECK(SEND(int, prefs, "defaultFontSize") == 20);
        CHECK(SEND(BOOL, prefs, "isJavaScriptEnabled") == NO);
        CHECK([SEND(id, prefs, "userStyleSheetLocation") isEqual: url]);

        SEND(void, prefs, "setStandardFontFamily:", nil);
        SEND(void, prefs, "setDefaultFontSize:", 0);
        SEND(void, prefs, "setJavaScriptEnabled:", YES);
        CHECK([SEND(id, prefs, "standardFontFamily") isEqualToString: @"Times"]);
        CHECK(SEND(int, prefs, "defaultFontSize") == 16);
        SEND(void, prefs, "setUserStyleSheetLocation:", nil);
        CHECK(SEND(id, prefs, "userStyleSheetLocation") == nil);
    }
    if (failures)
        return 1;
    printf("PASS webkit_webpreferences\n");
    return 0;
}
