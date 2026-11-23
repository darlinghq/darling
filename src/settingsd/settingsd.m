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

#import <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include "settingsd_dbus.h"

#define SETTINGSD_DOMAIN CFSTR("com.darlinghq.settingsd")
#define UNIVERSAL_ACCESS_DOMAIN CFSTR("com.apple.universalaccess")

static BOOL s_verbose = NO;

// QUIRK: Predefined macOS accent presets (-1 Graphite to 6 Pink) in the light appearance, and their
// corresponding highlight float strings.
// Evidence: black-box reads of +[NSColor controlAccentColor] in sRGB for each AppleAccentColor value on macOS
// 10.14.4 (tier 2); the numbers are listed in https://alexwlchan.net/2022/changing-the-macos-accent-colour/. The
// highlight strings are the format that https://pypi.org/project/osx-colors/ shows ("0.752941 0.964706 0.678431
// Green"); on macOS 10.14.4, running with each of these strings and its accent reproduced the selection colours
// that darling-cocotron pins, so they are consistent with AppKit but were not read back from a Mac's preferences.
// Test: tests/settingsd-desktop-appearance.py (host portal mapping) and tests/src/appearance_theme_end_to_end.m
static const struct {
    int code;
    double r, g, b;
    const char *name;
    const char *highlight;
} s_accent_presets[] = {
    { -1, 0.59600, 0.59600, 0.59600, "Graphite", "0.847059 0.847059 0.847059 Graphite" },
    {  0, 0.87800, 0.22000, 0.24300, "Red",      "1.000000 0.733333 0.721569 Red" },
    {  1, 0.96900, 0.51000, 0.10600, "Orange",   "1.000000 0.874510 0.701961 Orange" },
    {  2, 0.98800, 0.72200, 0.15300, "Yellow",   "1.000000 0.937255 0.690196 Yellow" },
    {  3, 0.38400, 0.72900, 0.27500, "Green",    "0.752941 0.964706 0.678431 Green" },
    {  4, 0.00000, 0.47843, 1.00000, "Blue",     "0.698039 0.843137 1.000000 Blue" },
    {  5, 0.58400, 0.23900, 0.58800, "Purple",   "0.968627 0.831373 1.000000 Purple" },
    {  6, 0.96900, 0.31000, 0.62000, "Pink",     "1.000000 0.749020 0.823529 Pink" },
};

static int find_closest_accent(double r, double g, double b, const char **out_highlight, const char **out_name) {
    int best_code = 4;
    double best_dist = 1e9;
    const char *best_hl = s_accent_presets[5].highlight;
    const char *best_n = s_accent_presets[5].name;

    for (size_t i = 0; i < sizeof(s_accent_presets) / sizeof(s_accent_presets[0]); i++) {
        double dr = r - s_accent_presets[i].r;
        double dg = g - s_accent_presets[i].g;
        double db = b - s_accent_presets[i].b;
        double dist = dr * dr + dg * dg + db * db;
        if (dist < best_dist) {
            best_dist = dist;
            best_code = s_accent_presets[i].code;
            best_hl = s_accent_presets[i].highlight;
            best_n = s_accent_presets[i].name;
        }
    }
    if (out_highlight) *out_highlight = best_hl;
    if (out_name) *out_name = best_n;
    return best_code;
}

static BOOL check_follow_host_appearance(void) {
    CFPreferencesAppSynchronize(SETTINGSD_DOMAIN);
    CFPropertyListRef val = CFPreferencesCopyAppValue(CFSTR("FollowHostAppearance"), SETTINGSD_DOMAIN);
    BOOL shouldFollow = YES;
    if (val != NULL) {
        if (CFGetTypeID(val) == CFBooleanGetTypeID()) {
            shouldFollow = CFBooleanGetValue((CFBooleanRef)val);
        } else if (CFGetTypeID(val) == CFNumberGetTypeID()) {
            int n = 0;
            CFNumberGetValue((CFNumberRef)val, kCFNumberIntType, &n);
            shouldFollow = (n != 0);
        }
        CFRelease(val);
    }
    return shouldFollow;
}

static void trim_whitespace(char *str) {
    if (!str) return;
    char *start = str;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
        start++;
    if (start != str)
        memmove(str, start, strlen(start) + 1);
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}

static void read_fallback_configs(int *out_has_dark, int *out_dark,
                                  int *out_has_contrast, int *out_contrast,
                                  int *out_has_accent, double *out_r, double *out_g, double *out_b) {
    const char *config_home = getenv("XDG_CONFIG_HOME");
    NSString *configDir = nil;
    if (config_home && config_home[0] != '\0') {
        configDir = [NSString stringWithUTF8String:config_home];
    } else {
        const char *home = getenv("HOME");
        if (home && home[0] != '\0') {
            configDir = [NSString stringWithFormat:@"%s/.config", home];
        }
    }

    if (!configDir)
        return;

    // 1. GTK configs: check gtk-4.0 then gtk-3.0
    NSArray<NSString *> *gtkFiles = @[
        [configDir stringByAppendingPathComponent:@"gtk-4.0/settings.ini"],
        [configDir stringByAppendingPathComponent:@"gtk-3.0/settings.ini"]
    ];

    for (NSString *path in gtkFiles) {
        FILE *f = fopen([path UTF8String], "r");
        if (!f) continue;

        char line[512];
        char section[128] = "";
        while (fgets(line, sizeof(line), f)) {
            trim_whitespace(line);
            if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
                continue;

            if (line[0] == '[' && line[strlen(line) - 1] == ']') {
                snprintf(section, sizeof(section), "%.*s", (int)(strlen(line) - 2), line + 1);
                trim_whitespace(section);
                continue;
            }

            if (strcasecmp(section, "Settings") == 0) {
                char *eq = strchr(line, '=');
                if (eq) {
                    *eq = '\0';
                    char *key = line;
                    char *val = eq + 1;
                    trim_whitespace(key);
                    trim_whitespace(val);

                    if (strcasecmp(key, "gtk-application-prefer-dark-theme") == 0) {
                        if (strcasecmp(val, "1") == 0 || strcasecmp(val, "true") == 0 || strcasecmp(val, "yes") == 0) {
                            *out_has_dark = 1;
                            *out_dark = 1;
                        } else if (strcasecmp(val, "0") == 0 || strcasecmp(val, "false") == 0 || strcasecmp(val, "no") == 0) {
                            *out_has_dark = 1;
                            *out_dark = 0;
                        }
                    } else if (strcasecmp(key, "gtk-theme-name") == 0) {
                        if (!*out_has_dark) {
                            if (strcasestr(val, "-dark") != NULL || strcasestr(val, "dark") != NULL) {
                                *out_has_dark = 1;
                                *out_dark = 1;
                            }
                        }
                    }
                }
            }
        }
        fclose(f);
        if (*out_has_dark) break;
    }

    // 2. KDE config: kdeglobals
    NSString *kdeFile = [configDir stringByAppendingPathComponent:@"kdeglobals"];
    FILE *kf = fopen([kdeFile UTF8String], "r");
    if (kf) {
        char line[512];
        char section[128] = "";
        while (fgets(line, sizeof(line), kf)) {
            trim_whitespace(line);
            if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
                continue;

            if (line[0] == '[' && line[strlen(line) - 1] == ']') {
                snprintf(section, sizeof(section), "%.*s", (int)(strlen(line) - 2), line + 1);
                trim_whitespace(section);
                continue;
            }

            char *eq = strchr(line, '=');
            if (!eq) continue;
            *eq = '\0';
            char *key = line;
            char *val = eq + 1;
            trim_whitespace(key);
            trim_whitespace(val);

            if (strcasecmp(section, "General") == 0) {
                if (strcasecmp(key, "ColorScheme") == 0) {
                    if (!*out_has_dark) {
                        if (strcasestr(val, "dark") != NULL) {
                            *out_has_dark = 1;
                            *out_dark = 1;
                        }
                    }
                } else if (strcasecmp(key, "AccentColor") == 0) {
                    // KDE AccentColor format: R,G,B (e.g. 61,174,233 or floats)
                    double kr = 0, kg = 0, kb = 0;
                    if (sscanf(val, "%lf,%lf,%lf", &kr, &kg, &kb) == 3) {
                        if (kr > 1.0 || kg > 1.0 || kb > 1.0) {
                            kr /= 255.0;
                            kg /= 255.0;
                            kb /= 255.0;
                        }
                        if (kr >= 0.0 && kr <= 1.0 && kg >= 0.0 && kg <= 1.0 && kb >= 0.0 && kb <= 1.0) {
                            *out_has_accent = 1;
                            *out_r = kr;
                            *out_g = kg;
                            *out_b = kb;
                        }
                    }
                }
            } else if (strcasecmp(section, "KDE") == 0) {
                if (strcasecmp(key, "contrast") == 0) {
                    double c = 0;
                    if (sscanf(val, "%lf", &c) == 1) {
                        *out_has_contrast = 1;
                        *out_contrast = (c >= 7.0) ? 1 : 0;
                    }
                }
            }
        }
        fclose(kf);
    }
}

static void apply_appearance_settings(int has_dark, int dark,
                                      int has_contrast, int contrast,
                                      int has_accent, double r, double g, double b) {
    if (!check_follow_host_appearance()) {
        if (s_verbose) NSLog(@"settingsd: FollowHostAppearance is disabled; skipping sync");
        return;
    }

    BOOL styleChanged = NO, contrastChanged = NO, colorsChanged = NO;

    // 1. Color scheme (AppleInterfaceStyle in global preferences)
    if (has_dark) {
        NSString *hostStyle = dark ? @"Dark" : @"Light";
        CFPropertyListRef lastStyleRef = CFPreferencesCopyAppValue(CFSTR("LastAppliedInterfaceStyle"), SETTINGSD_DOMAIN);
        NSString *lastStyle = lastStyleRef ? (__bridge NSString *)lastStyleRef : nil;

        if (![hostStyle isEqualToString:lastStyle]) {
            if (dark) {
                CFPreferencesSetValue(CFSTR("AppleInterfaceStyle"), CFSTR("Dark"),
                                      kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
                styleChanged = YES;
            } else {
                if ([lastStyle isEqualToString:@"Dark"]) {
                    CFPreferencesSetValue(CFSTR("AppleInterfaceStyle"), NULL,
                                          kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
                    styleChanged = YES;
                }
            }
            CFPreferencesSetAppValue(CFSTR("LastAppliedInterfaceStyle"), (__bridge CFStringRef)hostStyle, SETTINGSD_DOMAIN);
        }
        if (lastStyleRef) CFRelease(lastStyleRef);
    }

    // 2. High contrast (increaseContrast in com.apple.universalaccess)
    if (has_contrast) {
        CFPropertyListRef lastContrastRef = CFPreferencesCopyAppValue(CFSTR("LastAppliedContrast"), SETTINGSD_DOMAIN);
        int lastContrast = lastContrastRef ? [(__bridge id)lastContrastRef intValue] : -1;

        if (lastContrast != contrast) {
            CFPreferencesSetValue(CFSTR("increaseContrast"), contrast ? kCFBooleanTrue : kCFBooleanFalse,
                                  UNIVERSAL_ACCESS_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            CFPreferencesSynchronize(UNIVERSAL_ACCESS_DOMAIN, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            CFPreferencesSetAppValue(CFSTR("LastAppliedContrast"), (CFPropertyListRef)@(contrast), SETTINGSD_DOMAIN);
            contrastChanged = YES;
        }
        if (lastContrastRef) CFRelease(lastContrastRef);
    }

    // 3. Accent and Highlight color (AppleAccentColor and AppleHighlightColor in global preferences)
    if (has_accent) {
        const char *highlightStr = NULL;
        const char *accentName = NULL;
        int accentCode = find_closest_accent(r, g, b, &highlightStr, &accentName);

        CFPropertyListRef lastAccentRef = CFPreferencesCopyAppValue(CFSTR("LastAppliedAccentCode"), SETTINGSD_DOMAIN);
        int lastAccent = lastAccentRef ? [(__bridge id)lastAccentRef intValue] : -999;

        if (lastAccent != accentCode) {
            CFPreferencesSetValue(CFSTR("AppleAccentColor"), (CFPropertyListRef)@(accentCode),
                                  kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            CFPreferencesSetValue(CFSTR("AppleHighlightColor"), (CFPropertyListRef)[NSString stringWithUTF8String:highlightStr],
                                  kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
            CFPreferencesSetAppValue(CFSTR("LastAppliedAccentCode"), (CFPropertyListRef)@(accentCode), SETTINGSD_DOMAIN);
            colorsChanged = YES;
        }
        if (lastAccentRef) CFRelease(lastAccentRef);
    }

    if (styleChanged || contrastChanged || colorsChanged) {
        CFPreferencesSynchronize(kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
        CFPreferencesAppSynchronize(SETTINGSD_DOMAIN);

        // QUIRK: a change of the interface style is announced as AppleInterfaceThemeChangedNotification and a change
        // of the accent or highlight colour as AppleColorPreferencesChangedNotification, both through the
        // distributed notification center; Increase Contrast has no known notification of its own, so it is announced
        // with the first. Neither name is documented by Apple.
        // Evidence: widely reported behaviour (tier 4), see the references next to the observers in
        // darling-cocotron AppKit/NSApplication.m (AppleInterfaceThemeChangedNotification:
        // https://samasaur1.github.io/blog/terminal-apps-respecting-system-dark-mode; accent change:
        // https://alexwlchan.net/2022/changing-the-macos-accent-colour/).
        // Test: tests/src/appearance_theme_end_to_end.m
        CFNotificationCenterRef distCenter = CFNotificationCenterGetDistributedCenter();
        if (distCenter != NULL) {
            if (styleChanged || contrastChanged)
                CFNotificationCenterPostNotification(distCenter, CFSTR("AppleInterfaceThemeChangedNotification"), NULL, NULL, TRUE);
            if (colorsChanged)
                CFNotificationCenterPostNotification(distCenter, CFSTR("AppleColorPreferencesChangedNotification"), NULL, NULL, TRUE);
        }
    }
}

static void on_appearance_changed(int has_dark, int dark, int has_contrast, int contrast, int has_accent, double r, double g, double b) {
    apply_appearance_settings(has_dark, dark, has_contrast, contrast, has_accent, r, g, b);
}

static dispatch_source_t s_dbus_source = NULL;
static dispatch_source_t s_reconnect_timer = NULL;
static int s_reconnect_delay_sec = 2;

static void schedule_reconnect(void);

static void cleanup_connection(void) {
    if (s_dbus_source) {
        dispatch_source_cancel(s_dbus_source);
        dispatch_release(s_dbus_source);
        s_dbus_source = NULL;
    }
    settingsd_dbus_close();
    schedule_reconnect();
}

static void try_connect_and_setup(void) {
    int has_dark = 0, dark = 0;
    int has_contrast = 0, contrast = 0;
    int has_accent = 0;
    double r = 0.0, g = 0.48, b = 1.0;

    int dbus_ok = (settingsd_dbus_init() == 0);
    if (dbus_ok) {
        if (settingsd_dbus_read_appearance(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b) != 0) {
            // Bus open but portal did not answer; fall back to configs
            read_fallback_configs(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b);
        }
        apply_appearance_settings(has_dark, dark, has_contrast, contrast, has_accent, r, g, b);

        int fd = -1;
        if (settingsd_dbus_get_fd(&fd) == 0 && fd >= 0) {
            s_reconnect_delay_sec = 2;
            if (s_reconnect_timer) {
                dispatch_source_cancel(s_reconnect_timer);
                dispatch_release(s_reconnect_timer);
                s_reconnect_timer = NULL;
            }

            s_dbus_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue());
            dispatch_source_set_event_handler(s_dbus_source, ^{
                @autoreleasepool {
                    int rc = settingsd_dbus_process_messages(0, on_appearance_changed);
                    if (rc < 0) {
                        cleanup_connection();
                    }
                }
            });
            dispatch_resume(s_dbus_source);
            return;
        }
    }

    // Bus connection failed
    read_fallback_configs(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b);
    apply_appearance_settings(has_dark, dark, has_contrast, contrast, has_accent, r, g, b);
    schedule_reconnect();
}

static void schedule_reconnect(void) {
    if (s_reconnect_timer)
        return;

    s_reconnect_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
    dispatch_source_set_timer(s_reconnect_timer,
        dispatch_time(DISPATCH_TIME_NOW, (int64_t)s_reconnect_delay_sec * NSEC_PER_SEC),
        DISPATCH_TIME_FOREVER, 500 * NSEC_PER_MSEC);

    if (s_reconnect_delay_sec < 32)
        s_reconnect_delay_sec *= 2;

    dispatch_source_set_event_handler(s_reconnect_timer, ^{
        @autoreleasepool {
            if (s_reconnect_timer) {
                dispatch_source_cancel(s_reconnect_timer);
                dispatch_release(s_reconnect_timer);
                s_reconnect_timer = NULL;
            }
            try_connect_and_setup();
        }
    });
    dispatch_resume(s_reconnect_timer);
}

int main(int argc, char *argv[]) {
    @autoreleasepool {
        BOOL syncOnce = NO;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--sync-once") == 0 || strcmp(argv[i], "-1") == 0) {
                syncOnce = YES;
            } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
                s_verbose = YES;
            }
        }

        if (syncOnce) {
            int has_dark = 0, dark = 0;
            int has_contrast = 0, contrast = 0;
            int has_accent = 0;
            double r = 0.0, g = 0.48, b = 1.0;

            int dbus_ok = (settingsd_dbus_init() == 0);
            if (dbus_ok) {
                if (settingsd_dbus_read_appearance(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b) != 0) {
                    dbus_ok = 0;
                }
            }

            if (!dbus_ok) {
                read_fallback_configs(&has_dark, &dark, &has_contrast, &contrast, &has_accent, &r, &g, &b);
            }

            apply_appearance_settings(has_dark, dark, has_contrast, contrast, has_accent, r, g, b);

            if (dbus_ok)
                settingsd_dbus_close();
            return 0;
        }

        try_connect_and_setup();
        dispatch_main();
    }
    return 0;
}
