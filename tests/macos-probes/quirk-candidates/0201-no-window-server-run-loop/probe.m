// Probe 0201: what a Cocoa application does when asked for its next event while the process has no connection
// to a window server. Run it twice and keep both outputs: once in a normal login session (baseline) and once
// where no window server is reachable (for example over ssh to a Mac with nobody logged in, or as a launchd
// daemon). Every step is printed BEFORE it runs, so a crash or an abort still shows how far it got.
//   ./probe            runs the steps and prints the result of each
// Public API only: CGSessionCopyCurrentDictionary, CGGetActiveDisplayList, NSApplication, NSRunLoop, GCD.
#import "probe_common.h"
#import <AppKit/AppKit.h>
#if __has_include(<CoreGraphics/CGSession.h>)
#import <CoreGraphics/CGSession.h>
#define PROBE_HAVE_CGSESSION 1
#endif
#import <CoreGraphics/CoreGraphics.h>
#include <sys/resource.h>

static double cpuSeconds(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_utime.tv_sec + ru.ru_stime.tv_sec + (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1e6;
}

static int gTimerFires, gMainQueueRuns;

int main(void) {
    @autoreleasepool {
        probe_env("0201");
#ifdef PROBE_HAVE_CGSESSION
        CFDictionaryRef session = CGSessionCopyCurrentDictionary();
        probe_line("0201.session", @"CGSessionCopyCurrentDictionary", session ? @"non-NULL (a window server session exists)" : @"NULL (no window server session)");
        if (session) CFRelease(session);
#else
        probe_line("0201.session", @"CGSessionCopyCurrentDictionary", @"not declared by this SDK; skipped");
#endif
        CGDirectDisplayID ids[8];
        uint32_t n = 0;
        CGError e = CGGetActiveDisplayList(8, ids, &n);
        probe_line("0201.session", @"CGGetActiveDisplayList", PF(@"error %d ; %u active displays", (int)e, n));

        probe_line("0201.step", @"about to call +[NSApplication sharedApplication]", @"(started)");
        [NSApplication sharedApplication];
        probe_line("0201.step", @"+[NSApplication sharedApplication]", @"returned");
        probe_line("0201.step", @"about to call -setActivationPolicy:Accessory", @"(started)");
        BOOL ok = [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        probe_line("0201.step", @"-setActivationPolicy:Accessory", PF(@"returned %d", ok));

        NSTimer *timer = [NSTimer timerWithTimeInterval:0.1 repeats:YES block:^(NSTimer *t) { gTimerFires++; }];
        [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
        dispatch_async(dispatch_get_main_queue(), ^{ gMainQueueRuns++; });

        probe_line("0201.step", @"about to call -nextEventMatchingMask:NSEventMaskAny untilDate:+1s inMode:Default dequeue:YES", @"(started)");
        double c0 = cpuSeconds();
        CFAbsoluteTime t0 = CFAbsoluteTimeGetCurrent();
        NSEvent *ev = [NSApp nextEventMatchingMask:NSUIntegerMax  /* NSEventMaskAny */
                                         untilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
        double wall = CFAbsoluteTimeGetCurrent() - t0, cpu = cpuSeconds() - c0;
        probe_line("0201.nextEvent", @"untilDate +1s",
                   PF(@"returned %@ ; wall %.0f ms ; cpu %.0f ms ; timer (0.1 s, common modes) fired %d times ; main-queue block ran %d times",
                      ev ? PF(@"an event of type %ld", (long)[ev type]) : @"nil", wall * 1000, cpu * 1000, gTimerFires, gMainQueueRuns));
        gTimerFires = 0;
        t0 = CFAbsoluteTimeGetCurrent();
        ev = [NSApp nextEventMatchingMask:NSUIntegerMax untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        probe_line("0201.nextEvent", @"untilDate distantPast",
                   PF(@"returned %@ ; wall %.1f ms", ev ? @"an event" : @"nil", (CFAbsoluteTimeGetCurrent() - t0) * 1000));
        probe_line("0201.step", @"done", @"all steps finished");
    }
    return 0;
}
