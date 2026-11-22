// Probe 677: CGAcquireDisplayFadeReservation / CGReleaseDisplayFadeReservation / CGDisplayFade token semantics.
// Needs a window-server session (CoreGraphics display services). No window, no activation, no visible fade:
// the only CGDisplayFade calls go from blend 0 to blend 0 over 0 seconds.
// Risky calls (NULL token pointer) and the cross-process checks run in child copies of this program,
// so a crash is reported as a line instead of ending the probe.
#import "probe_common.h"
#import <ApplicationServices/ApplicationServices.h>
#include <math.h>
#include <unistd.h>

#ifndef kCGDisplayFadeReservationInvalidToken
#define PROBE_MACRO_MISSING 1
#endif

static NSString *code(int e) { return PF(@"%d", e); }

static NSString *tryAcquire(float interval, CGDisplayFadeReservationToken *tok) {
    CGDisplayFadeReservationToken t = 0xDEADBEEF; // sentinel: is it overwritten on failure?
    CGError e = CGAcquireDisplayFadeReservation(interval, &t);
    if (tok) *tok = t;
    return PF(@"result=%@ token=%u%@", code(e), (unsigned)t, t == 0xDEADBEEF ? @" (untouched)" : @"");
}

static void spawnSelf(NSString *mode, NSString *label) {
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = [[NSBundle mainBundle] executablePath] ?: @"./probe";
    task.arguments = @[mode];
    NSPipe *p = [NSPipe pipe];
    task.standardOutput = p;
    @try {
        [task launch];
    } @catch (NSException *e) {
        probe_line("677.child", label, PF(@"could not launch: %@", [e name]));
        return;
    }
    [task waitUntilExit];
    NSData *d = [[p fileHandleForReading] readDataToEndOfFile];
    NSString *out = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
    for (NSString *line in [out componentsSeparatedByString:@"\n"])
        if ([line hasPrefix:@"PROBE "]) printf("%s\n", [line UTF8String]);
    int reason = (int)task.terminationReason;
    if (reason != NSTaskTerminationReasonExit || task.terminationStatus != 0)
        probe_line("677.child", label, PF(@"child ended by %s %d", reason == NSTaskTerminationReasonExit ? "exit status" : "signal",
                                           (int)task.terminationStatus));
    fflush(stdout);
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (argc > 1) {
            const char *m = argv[1];
            if (!strcmp(m, "nulltoken")) {
                CGError e = CGAcquireDisplayFadeReservation(1.0f, NULL);
                probe_line("677.D.null-token", @"Acquire(1.0, NULL)", PF(@"returned %@ (no crash)", code(e)));
                return 0;
            }
            if (!strcmp(m, "try")) { // a second process tries to acquire while the parent holds one
                probe_line("677.C.other-process", @"child Acquire(1.0) while parent holds a reservation",
                           tryAcquire(1.0f, NULL));
                return 0;
            }
            if (!strcmp(m, "hold")) { // this process holds a reservation for 4 s; the parent tries meanwhile
                CGDisplayFadeReservationToken t;
                printf("PROBE 677.C.child-hold child Acquire(3.5) -> %s\n", [tryAcquire(3.5f, &t) UTF8String]);
                fflush(stdout);
                sleep(4);
                return 0;
            }
            return 2;
        }

        probe_env("677");
        probe_line("677", @"kCGMaxDisplayReservationInterval", PF(@"%g", (double)kCGMaxDisplayReservationInterval));
#ifdef PROBE_MACRO_MISSING
        probe_line("677", @"kCGDisplayFadeReservationInvalidToken", @"macro not defined by this SDK");
#else
        probe_line("677", @"kCGDisplayFadeReservationInvalidToken", PF(@"%u", (unsigned)kCGDisplayFadeReservationInvalidToken));
#endif
        probe_line("677", @"sizeof(CGDisplayFadeReservationToken)", PF(@"%zu", sizeof(CGDisplayFadeReservationToken)));
        probe_line("677", @"CGDisplayReservationInterval type", PF(@"sizeof=%zu", sizeof(CGDisplayReservationInterval)));

        // A. Interval range: acquire, then release at once. Inside, at and outside both documented ranges.
        float intervals[] = {0.0f, -1.0f, -0.0f, 1e-6f, 0.0001f, 0.5f, 0.999f, 1.0f, 2.0f, 14.999f,
                             kCGMaxDisplayReservationInterval, kCGMaxDisplayReservationInterval + 0.001f,
                             kCGMaxDisplayReservationInterval + 1.0f, 100.0f, INFINITY, -INFINITY, NAN, 1e30f};
        for (size_t i = 0; i < sizeof intervals / sizeof intervals[0]; i++) {
            CGDisplayFadeReservationToken t = 0;
            NSString *a = tryAcquire(intervals[i], &t);
            CGError r = (t != 0xDEADBEEF) ? CGReleaseDisplayFadeReservation(t) : -999;
            probe_line("677.A.interval", PF(@"Acquire(%g)", (double)intervals[i]),
                       PF(@"%@ ; release of that token: %@", a, r == -999 ? @"skipped" : code(r)));
        }

        // B. Tokens: values, repeat, second acquire while one is held, release then acquire, expiry.
        CGDisplayFadeReservationToken t1 = 0, t2 = 0, t3 = 0;
        probe_line("677.B.first", @"Acquire(5.0)", tryAcquire(5.0f, &t1));
        probe_line("677.B.second-same-process", @"Acquire(5.0) while t1 is held", tryAcquire(5.0f, &t2));
        probe_line("677.B.release-second", @"Release(token of the refused second acquire)",
                   code(CGReleaseDisplayFadeReservation(t2)));
        probe_line("677.B.still-held", @"Acquire(5.0) again after releasing the refused token (t1 still held?)",
                   tryAcquire(5.0f, &t3));

        // C. Is a reservation per process? A second process tries while this one holds t1.
        spawnSelf(@"try", @"try");
        probe_line("677.B.release-first", @"Release(t1)", code(CGReleaseDisplayFadeReservation(t1)));
        probe_line("677.B.release-first-again", @"Release(t1) a second time", code(CGReleaseDisplayFadeReservation(t1)));
        {
            CGDisplayFadeReservationToken a = 0;
            probe_line("677.B.reacquire", @"Acquire(5.0) after releasing t1", tryAcquire(5.0f, &a));
            probe_line("677.B.new-token-differs", @"token of the new reservation vs t1", a == t1 ? @"same value" : @"different value");
            probe_line("677.B.stale-release", @"Release(t1, the old token) while the new reservation is held",
                       code(CGReleaseDisplayFadeReservation(t1)));
            probe_line("677.B.new-still-held", @"Acquire(5.0) after the stale release (is the new one still held?)",
                       tryAcquire(5.0f, NULL));
            CGReleaseDisplayFadeReservation(a);
        }
        // Parent tries while a child holds one.
        {
            NSTask *task = [[NSTask alloc] init];
            task.launchPath = [[NSBundle mainBundle] executablePath] ?: @"./probe";
            task.arguments = @[@"hold"];
            NSPipe *p = [NSPipe pipe];
            task.standardOutput = p;
            [task launch];
            usleep(1000 * 1500); // let the child acquire
            CGDisplayFadeReservationToken pt = 0;
            probe_line("677.C.parent-try", @"parent Acquire(1.0) while a child process holds one", tryAcquire(1.0f, &pt));
            if (pt != 0xDEADBEEF) CGReleaseDisplayFadeReservation(pt);
            [task waitUntilExit];
            NSString *out = [[NSString alloc] initWithData:[[p fileHandleForReading] readDataToEndOfFile]
                                                  encoding:NSUTF8StringEncoding];
            for (NSString *line in [out componentsSeparatedByString:@"\n"])
                if ([line hasPrefix:@"PROBE "]) printf("%s\n", [line UTF8String]);
            probe_line("677.C.after-child-exit", @"parent Acquire(1.0) after that child exited", ({
                CGDisplayFadeReservationToken x = 0;
                NSString *s = tryAcquire(1.0f, &x);
                if (x != 0xDEADBEEF) CGReleaseDisplayFadeReservation(x);
                s;
            }));
        }

        // Expiry: a reservation lasts its interval; Release of an expired token.
        {
            CGDisplayFadeReservationToken e = 0, e2 = 0;
            probe_line("677.E.expiry", @"Acquire(1.0)", tryAcquire(1.0f, &e));
            probe_line("677.E.expiry", @"Acquire(1.0) after 0.5 s (held)", ({ usleep(500000); tryAcquire(1.0f, NULL); }));
            usleep(1000 * 1300);
            probe_line("677.E.expiry", @"Acquire(5.0) after 1.8 s total (expired?)", tryAcquire(5.0f, &e2));
            probe_line("677.E.expiry", @"Release(e) the expired token while e2 is held", code(CGReleaseDisplayFadeReservation(e)));
            probe_line("677.E.expiry", @"Acquire(1.0) (is e2 still held after the expired release?)", tryAcquire(1.0f, NULL));
            CGReleaseDisplayFadeReservation(e2);
        }

        // D. Release of tokens never issued, NULL token pointer (child), CGDisplayFade with various tokens.
        uint32_t bads[] = {0, 1, 2, 12345, 0xFFFFFFFFu};
        for (size_t bi = 0; bi < sizeof bads / sizeof bads[0]; bi++) {
            uint32_t bad = bads[bi];
            probe_line("677.D.release-unissued", PF(@"Release(%u), no reservation held", (unsigned)bad),
                       code(CGReleaseDisplayFadeReservation(bad)));
        }
        spawnSelf(@"nulltoken", @"nulltoken");

        // CGDisplayFade from blend 0 to blend 0 in 0 seconds: no visible change.
        {
            CGDisplayFadeReservationToken f = 0;
            tryAcquire(5.0f, &f);
            probe_line("677.F.fade", @"CGDisplayFade(valid token, 0 s, 0 -> 0)",
                       code(CGDisplayFade(f, 0.0f, 0.0f, 0.0f, 0, 0, 0, false)));
            CGReleaseDisplayFadeReservation(f);
            probe_line("677.F.fade", @"CGDisplayFade(released token)", code(CGDisplayFade(f, 0.0f, 0.0f, 0.0f, 0, 0, 0, false)));
            probe_line("677.F.fade", @"CGDisplayFade(token 0)", code(CGDisplayFade(0, 0.0f, 0.0f, 0.0f, 0, 0, 0, false)));
            probe_line("677.F.fade", @"CGDisplayFade(token 12345, never issued)",
                       code(CGDisplayFade(12345, 0.0f, 0.0f, 0.0f, 0, 0, 0, false)));
            CGDisplayFadeReservationToken g = 0;
            tryAcquire(0.2f, &g);
            usleep(400000);
            probe_line("677.F.fade", @"CGDisplayFade(token of a reservation that expired)",
                       code(CGDisplayFade(g, 0.0f, 0.0f, 0.0f, 0, 0, 0, false)));
        }
    }
    return 0;
}
