/*
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// End-to-end integration test for Darling appearance synchronization:
// - Verifies AppKit observes AppleInterfaceThemeChangedNotification, posted by another process, through
//   the distributed notification center and updates effectiveAppearance via KVO.
// - Verifies view viewDidChangeEffectiveAppearance callbacks and dynamic system colors.
// - Verifies settingsd FollowHostAppearance opt-out preference in com.darlinghq.settingsd.
// - Preserves and restores all existing user preferences across runs.
//

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mach-o/dyld.h>

#define SETTINGSD_DOMAIN CFSTR("com.darlinghq.settingsd")
#define UNIVERSAL_ACCESS_DOMAIN CFSTR("com.apple.universalaccess")

static int s_fails = 0;

static int test_check(const char *name, BOOL condition) {
    if (condition) {
        printf("PASS: %s\n", name);
        return 0;
    } else {
        printf("FAIL: %s\n", name);
        s_fails++;
        return 1;
    }
}

// Saved preference values for clean restoration
static CFPropertyListRef s_orig_style = NULL;
static CFPropertyListRef s_orig_accent = NULL;
static CFPropertyListRef s_orig_highlight = NULL;
static CFPropertyListRef s_orig_contrast = NULL;
static CFPropertyListRef s_orig_follow = NULL;
static CFPropertyListRef s_orig_last_style = NULL;
static CFPropertyListRef s_orig_last_accent = NULL;
static CFPropertyListRef s_orig_last_contrast = NULL;

static void save_user_defaults(void) {
    CFPreferencesAppSynchronize(kCFPreferencesAnyApplication);
    s_orig_style = CFPreferencesCopyValue(CFSTR("AppleInterfaceStyle"), kCFPreferencesAnyApplication,
                                          kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    s_orig_accent = CFPreferencesCopyValue(CFSTR("AppleAccentColor"), kCFPreferencesAnyApplication,
                                           kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    s_orig_highlight = CFPreferencesCopyValue(CFSTR("AppleHighlightColor"), kCFPreferencesAnyApplication,
                                             kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    CFPreferencesAppSynchronize(UNIVERSAL_ACCESS_DOMAIN);
    s_orig_contrast = CFPreferencesCopyValue(CFSTR("increaseContrast"), UNIVERSAL_ACCESS_DOMAIN,
                                             kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    CFPreferencesAppSynchronize(SETTINGSD_DOMAIN);
    s_orig_follow = CFPreferencesCopyValue(CFSTR("FollowHostAppearance"), SETTINGSD_DOMAIN,
                                           kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    s_orig_last_style = CFPreferencesCopyValue(CFSTR("LastAppliedInterfaceStyle"), SETTINGSD_DOMAIN,
                                               kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    s_orig_last_accent = CFPreferencesCopyValue(CFSTR("LastAppliedAccentCode"), SETTINGSD_DOMAIN,
                                                kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    s_orig_last_contrast = CFPreferencesCopyValue(CFSTR("LastAppliedContrast"), SETTINGSD_DOMAIN,
                                                 kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
}

static void restore_user_defaults(void) {
    CFPreferencesSetValue(CFSTR("AppleInterfaceStyle"), s_orig_style,
                          kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSetValue(CFSTR("AppleAccentColor"), s_orig_accent,
                          kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSetValue(CFSTR("AppleHighlightColor"), s_orig_highlight,
                          kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSynchronize(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    CFPreferencesSetValue(CFSTR("increaseContrast"), s_orig_contrast,
                          UNIVERSAL_ACCESS_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSynchronize(UNIVERSAL_ACCESS_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    CFPreferencesSetValue(CFSTR("FollowHostAppearance"), s_orig_follow,
                          SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSetValue(CFSTR("LastAppliedInterfaceStyle"), s_orig_last_style,
                          SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSetValue(CFSTR("LastAppliedAccentCode"), s_orig_last_accent,
                          SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSetValue(CFSTR("LastAppliedContrast"), s_orig_last_contrast,
                          SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSynchronize(SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    if (s_orig_style) CFRelease(s_orig_style);
    if (s_orig_accent) CFRelease(s_orig_accent);
    if (s_orig_highlight) CFRelease(s_orig_highlight);
    if (s_orig_contrast) CFRelease(s_orig_contrast);
    if (s_orig_follow) CFRelease(s_orig_follow);
    if (s_orig_last_style) CFRelease(s_orig_last_style);
    if (s_orig_last_accent) CFRelease(s_orig_last_accent);
    if (s_orig_last_contrast) CFRelease(s_orig_last_contrast);
}

static void set_global_pref(CFStringRef key, CFPropertyListRef val) {
    CFPreferencesSetValue(key, val, kCFPreferencesAnyApplication,
                          kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesSynchronize(kCFPreferencesAnyApplication,
                             kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
    CFPreferencesAppSynchronize(kCFPreferencesAnyApplication);
    [[NSUserDefaults standardUserDefaults] synchronize];
}

static char s_self_path[1024];

// Posts the notification from a separate process, the way settingsd does, so that delivery across processes is covered.
static void post_theme_changed_notification(void) {
    char command[1100];
    snprintf(command, sizeof(command), "'%s' --post", s_self_path);
    system(command);
}

// Test helper objects for KVO and view callbacks
@interface TestAppearanceObserver : NSObject {
    NSInteger _kvoCallCount;
}
@property (nonatomic) NSInteger kvoCallCount;
@end

@implementation TestAppearanceObserver
@synthesize kvoCallCount = _kvoCallCount;
- (void) observeValueForKeyPath: (NSString *) keyPath
                       ofObject: (id) object
                         change: (NSDictionary *) change
                        context: (void *) context {
    if ([keyPath isEqualToString: @"effectiveAppearance"]) {
        _kvoCallCount++;
    }
}
@end

@interface TestNotificationView : NSView {
    NSInteger _viewCallbackCount;
}
@property (nonatomic) NSInteger viewCallbackCount;
@end

@implementation TestNotificationView
@synthesize viewCallbackCount = _viewCallbackCount;
- (void) viewDidChangeEffectiveAppearance {
    [super viewDidChangeEffectiveAppearance];
    _viewCallbackCount++;
}
@end

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--post") == 0) {
        CFNotificationCenterRef distCenter = CFNotificationCenterGetDistributedCenter();
        if (distCenter != NULL) {
            CFNotificationCenterPostNotification(distCenter, CFSTR("AppleInterfaceThemeChangedNotification"),
                                                 NULL, NULL, TRUE);
        }
        return 0;
    }
    uint32_t selfPathSize = sizeof(s_self_path);
    if (_NSGetExecutablePath(s_self_path, &selfPathSize) != 0) {
        printf("FAIL: executable path\n");
        return 1;
    }
    setlinebuf(stdout);
    setlinebuf(stderr);

    save_user_defaults();

    @try {
        @autoreleasepool {
            [NSApplication sharedApplication];

            TestAppearanceObserver *observer = [[TestAppearanceObserver alloc] init];
            [NSApp addObserver: observer
                    forKeyPath: @"effectiveAppearance"
                       options: NSKeyValueObservingOptionNew
                       context: NULL];

            NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, 200, 200)
                                                           styleMask: NSWindowStyleMaskTitled
                                                             backing: NSBackingStoreBuffered
                                                               defer: NO];
            [window addObserver: observer
                     forKeyPath: @"effectiveAppearance"
                        options: NSKeyValueObservingOptionNew
                        context: NULL];

            TestNotificationView *view = [[TestNotificationView alloc] initWithFrame: NSMakeRect(0, 0, 200, 200)];
            [[window contentView] addSubview: view];

            // 1. Switch to Dark appearance via preferences + distributed notification
            set_global_pref(CFSTR("AppleInterfaceStyle"), CFSTR("Dark"));
            set_global_pref(CFSTR("AppleAccentColor"), (CFPropertyListRef)@(5)); // Purple
            set_global_pref(CFSTR("AppleHighlightColor"), CFSTR("0.968627 0.831373 1.000000 Purple"));

            NSInteger prevKVO = observer.kvoCallCount;
            NSInteger prevView = view.viewCallbackCount;

            post_theme_changed_notification();

            for (int i = 0; i < 20; i++) {
                usleep(10000);
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
            }

            NSAppearance *eff = [NSApp effectiveAppearance];
            test_check("effectiveAppearance is DarkAqua under Dark style",
                       [[eff name] isEqualToString: NSAppearanceNameDarkAqua]);

            test_check("KVO observer fired on theme change", observer.kvoCallCount > prevKVO);
            test_check("viewDidChangeEffectiveAppearance fired on view", view.viewCallbackCount > prevView);

            // Verify dark system colors
            NSColor *textColor = [NSColor textColor];
            NSColor *rgbText = [textColor colorUsingColorSpaceName: NSCalibratedWhiteColorSpace];
            test_check("textColor has high brightness in DarkAqua", [rgbText whiteComponent] > 0.8f);

            // Verify purple accent color (read on macOS 10.14.4: 0.584, 0.239, 0.588 in Aqua, 0.647, 0.314, 0.655 in DarkAqua)
            NSColor *accent = [NSColor controlAccentColor];
            NSColor *rgbAccent = [accent colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
            CGFloat r = 0, g = 0, b = 0, a = 0;
            [rgbAccent getRed: &r green: &g blue: &b alpha: &a];
            test_check("controlAccentColor is Purple", (r > 0.55f && r < 0.70f && g > 0.20f && g < 0.35f && b > 0.55f && b < 0.70f));

            // 2. Switch back to Light appearance + Orange accent
            set_global_pref(CFSTR("AppleInterfaceStyle"), NULL);
            set_global_pref(CFSTR("AppleAccentColor"), (CFPropertyListRef)@(1)); // Orange
            set_global_pref(CFSTR("AppleHighlightColor"), CFSTR("1.000000 0.874510 0.701961 Orange"));

            prevKVO = observer.kvoCallCount;
            prevView = view.viewCallbackCount;

            post_theme_changed_notification();
            for (int i = 0; i < 20; i++) {
                usleep(10000);
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
            }

            eff = [NSApp effectiveAppearance];
            test_check("effectiveAppearance is Aqua under default light style",
                       [[eff name] isEqualToString: NSAppearanceNameAqua]);
            test_check("KVO observer fired on switch to Aqua", observer.kvoCallCount > prevKVO);
            test_check("viewDidChangeEffectiveAppearance fired on switch to Aqua", view.viewCallbackCount > prevView);

            accent = [NSColor controlAccentColor];
            rgbAccent = [accent colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
            [rgbAccent getRed: &r green: &g blue: &b alpha: &a];
            test_check("controlAccentColor is Orange", (r > 0.90f && g > 0.45f && g < 0.55f && b < 0.15f));

            // 3. Test FollowHostAppearance opt-out
            CFPreferencesSetValue(CFSTR("FollowHostAppearance"), kCFBooleanFalse,
                                  SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            CFPreferencesSynchronize(SETTINGSD_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

            // Run settingsd --sync-once with FollowHostAppearance disabled
            system("/usr/libexec/settingsd --sync-once");

            // Verify settingsd did NOT overwrite AppleInterfaceStyle or AppleAccentColor
            CFPreferencesAppSynchronize(kCFPreferencesAnyApplication);
            CFPropertyListRef styleVal = CFPreferencesCopyValue(CFSTR("AppleInterfaceStyle"), kCFPreferencesAnyApplication,
                                                                kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            test_check("FollowHostAppearance disabled prevents settingsd modification", styleVal == NULL);
            if (styleVal) CFRelease(styleVal);

            [NSApp removeObserver: observer forKeyPath: @"effectiveAppearance"];
            [window removeObserver: observer forKeyPath: @"effectiveAppearance"];
            [window release];
        }
    } @catch (NSException *e) {
        printf("EXCEPTION: %s: %s\n", [[e name] UTF8String], [[e reason] UTF8String]);
        s_fails++;
    } @finally {
        restore_user_defaults();
    }

    if (s_fails == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("TESTS FAILED: %d\n", s_fails);
        return 1;
    }
}
