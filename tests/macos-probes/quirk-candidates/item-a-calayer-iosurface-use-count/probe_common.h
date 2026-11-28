// probe_common.h - helpers shared by the probes. Public API only.
// Output format, one fact per line:   PROBE <id> <input> -> <output>
// This file is copied next to every probe so each directory builds on its own.
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

#define PF(...) [NSString stringWithFormat:__VA_ARGS__]
#define PROBE_SDK_AT_LEAST(v) (__MAC_OS_X_VERSION_MAX_ALLOWED >= (v))

static inline void probe_line(const char *probe, NSString *input, NSString *output) {
    printf("PROBE %s %s -> %s\n", probe, [input UTF8String], [output UTF8String]);
    fflush(stdout);
}

#if defined(__arm64__)
#define PROBE_PROCESS_ARCH "arm64"
#elif defined(__x86_64__)
#define PROBE_PROCESS_ARCH "x86_64"
#else
#define PROBE_PROCESS_ARCH "other"
#endif

// The first line of every probe: the OS version and the CPU architecture it ran on.
static inline void probe_env(const char *probe) {
    struct utsname u;
    uname(&u);
    probe_line(probe, @"env", PF(@"%@ ; machine %s ; process-arch %s",
                                 [[NSProcessInfo processInfo] operatingSystemVersionString], u.machine,
                                 PROBE_PROCESS_ARCH));
}

static inline NSString *probe_rect(CGRect r) {
    return PF(@"{{%g,%g},{%g,%g}}", (double)r.origin.x, (double)r.origin.y, (double)r.size.width,
              (double)r.size.height);
}
static inline NSString *probe_size(CGSize s) { return PF(@"{%g,%g}", (double)s.width, (double)s.height); }

// Run a block, report the exception it raised (name and a reason of at most 80 characters) or "no exception".
static inline NSString *probe_try(void (^block)(void)) {
    @try {
        block();
    } @catch (NSException *e) {
        NSString *reason = [e reason] ?: @"";
        if ([reason length] > 80)
            reason = [reason substringToIndex:80];
        return PF(@"RAISED %@ (%@)", [e name], reason);
    }
    return @"no exception";
}

#ifdef PROBE_APPKIT
#import <AppKit/AppKit.h>

// -fittingSize, -contentLayoutRect and a few other properties are read through key-value coding so the probe
// also builds against an SDK whose headers predate them. "n/a" means the OS does not answer the property.
static inline NSString *probe_kvc_size(id obj, NSString *key) {
    if (![obj respondsToSelector:NSSelectorFromString(key)]) return @"n/a";
    return probe_size([[obj valueForKey:key] sizeValue]);
}
static inline NSString *probe_fitting(id v) { return probe_kvc_size(v, @"fittingSize"); }

// A connection to the window server is needed for views, cells and windows. No Dock icon, no activation.
static inline void probe_app(void) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

static inline void probe_spin(NSTimeInterval seconds) {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

// NSAppearanceNameAqua / NSAppearanceNameDarkAqua by their documented string values, so the probe also builds
// against an SDK that predates the dark appearance. nil means the OS has no such appearance.
static inline NSAppearance *probe_appearance(BOOL dark) {
    return [NSAppearance appearanceNamed:dark ? @"NSAppearanceNameDarkAqua" : @"NSAppearanceNameAqua"];
}

// A borderless window that is never shown, to host a view for drawing.
static inline NSWindow *probe_window(NSRect contentRect, BOOL dark) {
    NSWindow *w = [[NSWindow alloc] initWithContentRect:contentRect
                                              styleMask:NSWindowStyleMaskBorderless
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [w setReleasedWhenClosed:NO];
    NSAppearance *a = probe_appearance(dark);
    if (a != nil && [w respondsToSelector:@selector(setAppearance:)])
        [w setAppearance:a];
    return w;
}

// Non-premultiplied sRGBA of one pixel as rrggbbaa. nil rep or out of range: "n/a".
static inline NSString *probe_pixel(NSBitmapImageRep *rep, NSInteger x, NSInteger y) {
    if (rep == nil || x < 0 || y < 0 || x >= [rep pixelsWide] || y >= [rep pixelsHigh])
        return @"n/a";
    NSColor *c = [[rep colorAtX:x y:y] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
    if (c == nil)
        return @"n/a";
    CGFloat r, g, b, a;
    [c getRed:&r green:&g blue:&b alpha:&a];
    return PF(@"%02x%02x%02x%02x", (int)lround(r * 255), (int)lround(g * 255), (int)lround(b * 255),
              (int)lround(a * 255));
}

// Draw a view (already in a window, or not) into a fresh bitmap at the backing scale of the main screen.
static inline NSBitmapImageRep *probe_snapshot(NSView *view) {
    NSRect b = [view bounds];
    NSBitmapImageRep *rep = [view bitmapImageRepForCachingDisplayInRect:b];
    if (rep == nil)
        return nil;
    [view cacheDisplayInRect:b toBitmapImageRep:rep];
    return rep;
}

static inline NSString *probe_font(NSFont *f) {
    return f == nil ? @"nil" : PF(@"%@ %g", [f fontName], (double)[f pointSize]);
}
#endif
