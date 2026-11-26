// The WebKit frameworks can be found by their bundle identifiers once WebKit
// is loaded, as on macOS 26.7.3 (x86_64), where the same lookups give the
// paths checked here. WebCore uses its own identifier to find its localized
// strings, and WebKit's errors are described with them; the English strings
// are in WebCore's bundle.
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#include <stdio.h>

static int failures;

static void expect(NSString *identifier, NSString *path)
{
    CFBundleRef bundle = CFBundleGetBundleWithIdentifier((__bridge CFStringRef)identifier);
    NSString *found = nil;
    if (bundle) {
        NSURL *url = (NSURL *)CFBundleCopyBundleURL(bundle);
        found = [url path];
        [url autorelease];
    }
    BOOL ok = [found isEqualToString: path] && [[[NSBundle bundleWithIdentifier: identifier] bundlePath] isEqualToString: path];
    printf("%s: %s -> %s\n", ok ? "PASS" : "FAIL", [identifier UTF8String], found ? [found UTF8String] : "NULL");
    if (!ok)
        failures++;
}

int main(void)
{
    @autoreleasepool {
        (void)[WKWebView class];
        expect(@"com.apple.WebKit", @"/System/Library/Frameworks/WebKit.framework");
        expect(@"com.apple.WebCore", @"/System/Library/Frameworks/WebKit.framework/Versions/A/Frameworks/WebCore.framework");
        expect(@"com.apple.WebKitLegacy", @"/System/Library/Frameworks/WebKit.framework/Versions/A/Frameworks/WebKitLegacy.framework");
        expect(@"com.apple.JavaScriptCore", @"/System/Library/Frameworks/JavaScriptCore.framework");

        NSBundle *webCore = [NSBundle bundleWithIdentifier: @"com.apple.WebCore"];
        NSString *string = [webCore localizedStringForKey: @"A JavaScript exception occurred" value: @"(missing)" table: nil];
        BOOL found = [string isEqualToString: @"A JavaScript exception occurred"];
        printf("%s: WebCore's localized strings are found\n", found ? "PASS" : "FAIL");
        if (!found)
            failures++;
    }
    return failures != 0;
}
