// Probe 646: how a scroll-wheel event carries precise (pixel) deltas and gesture phases. Events are created with
// the public CGEventCreateScrollWheelEvent2 (pixel and line units), their public scroll fields are set, and the
// event is turned into an NSEvent with +eventWithCGEvent:. Prints hasPreciseScrollingDeltas, scrollingDeltaX/Y,
// deltaX/deltaY/deltaZ, phase and momentumPhase, and reads the CGEvent fields back from the NSEvent.
// Events are only created and inspected, never posted, so nothing reaches any application and no window is made.
// A connection to the window server is made only by NSApplication (no activation).
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <ApplicationServices/ApplicationServices.h>

static NSString *phs(NSEventPhase p) {
    if (p == NSEventPhaseNone) return @"None";
    NSMutableArray *a = [NSMutableArray array];
    if (p & NSEventPhaseBegan) [a addObject:@"Began"];
    if (p & NSEventPhaseStationary) [a addObject:@"Stationary"];
    if (p & NSEventPhaseChanged) [a addObject:@"Changed"];
    if (p & NSEventPhaseEnded) [a addObject:@"Ended"];
    if (p & NSEventPhaseCancelled) [a addObject:@"Cancelled"];
    if (p & NSEventPhaseMayBegin) [a addObject:@"MayBegin"];
    return PF(@"%@(0x%lx)", [a componentsJoinedByString:@"+"], (unsigned long)p);
}

static NSString *cgf(CGEventRef e) {
    return PF(@"CG[isContinuous=%lld deltaAxis1=%lld deltaAxis2=%lld fixedAxis1=%g fixedAxis2=%g pointAxis1=%lld pointAxis2=%lld scrollPhase=%lld momentumPhase=%lld scrollCount=%lld]",
              CGEventGetIntegerValueField(e, kCGScrollWheelEventIsContinuous),
              CGEventGetIntegerValueField(e, kCGScrollWheelEventDeltaAxis1), CGEventGetIntegerValueField(e, kCGScrollWheelEventDeltaAxis2),
              CGEventGetDoubleValueField(e, kCGScrollWheelEventFixedPtDeltaAxis1), CGEventGetDoubleValueField(e, kCGScrollWheelEventFixedPtDeltaAxis2),
              CGEventGetIntegerValueField(e, kCGScrollWheelEventPointDeltaAxis1), CGEventGetIntegerValueField(e, kCGScrollWheelEventPointDeltaAxis2),
              CGEventGetIntegerValueField(e, kCGScrollWheelEventScrollPhase), CGEventGetIntegerValueField(e, kCGScrollWheelEventMomentumPhase),
              CGEventGetIntegerValueField(e, kCGScrollWheelEventScrollCount));
}

static NSString *nsf(NSEvent *ev) {
    if (ev == nil) return @"NSEvent eventWithCGEvent: returned nil";
    NSString *base = PF(@"type=%ld precise=%d scrollingDeltaX=%g scrollingDeltaY=%g deltaX=%g deltaY=%g deltaZ=%g phase=%@ momentumPhase=%@ inverted=%d windowNumber=%ld",
                        (long)[ev type], (int)[ev hasPreciseScrollingDeltas], (double)[ev scrollingDeltaX], (double)[ev scrollingDeltaY],
                        (double)[ev deltaX], (double)[ev deltaY], (double)[ev deltaZ], phs([ev phase]), phs([ev momentumPhase]),
                        (int)[ev isDirectionInvertedFromDevice], (long)[ev windowNumber]);
    CGEventRef back = [ev CGEvent];
    return back ? PF(@"%@ ; roundtrip %@", base, cgf(back)) : [base stringByAppendingString:@" ; -CGEvent nil"];
}

static void emit(const char *id, NSString *input, CGEventRef e) {
    if (e == NULL) { probe_line(id, input, @"CGEventCreateScrollWheelEvent2 returned NULL"); return; }
    NSString *ex = nil;
    __block NSEvent *ev = nil;
    ex = probe_try(^{ ev = [NSEvent eventWithCGEvent:e]; });
    probe_line(id, input, PF(@"%@ ; NS[%@] [%@]", cgf(e), nsf(ev), ex));
    CFRelease(e);
}

int main(void) {
    @autoreleasepool {
        probe_env("646");
        probe_app();
        NSScreen *sc = [NSScreen mainScreen];
        probe_line("646.0", @"main screen backing scale factor (context for pixel versus point)", sc ? PF(@"%g", (double)[sc backingScaleFactor]) : @"no screen");

        // Units pixel and line, one or two axes.
        struct { int dy, dx; } pairs[] = {{10, 0}, {-10, 0}, {0, 25}, {7, -3}, {1, 0}, {0, 0}, {100, 100}, {1000, 0}, {-1, 1}};
        for (int u = 0; u < 2; u++)
            for (int i = 0; i < 9; i++) {
                CGScrollEventUnit unit = u ? kCGScrollEventUnitLine : kCGScrollEventUnitPixel;
                CGEventRef e = CGEventCreateScrollWheelEvent2(NULL, unit, 2, pairs[i].dy, pairs[i].dx, 0);
                emit("646.1", PF(@"CGEventCreateScrollWheelEvent2 unit=%s wheelCount=2 wheel1(dy)=%d wheel2(dx)=%d", u ? "Line" : "Pixel", pairs[i].dy, pairs[i].dx), e);
            }
        for (int u = 0; u < 2; u++) {
            CGEventRef e = CGEventCreateScrollWheelEvent2(NULL, u ? kCGScrollEventUnitLine : kCGScrollEventUnitPixel, 1, 10, 0, 0);
            emit("646.2", PF(@"unit=%s wheelCount=1 wheel1=10", u ? "Line" : "Pixel"), e);
        }

        // Flags set on the CGEvent after creation.
        CGEventRef e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitLine, 2, 3, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventIsContinuous, 1);
        emit("646.3", @"Line unit dy=3, then kCGScrollWheelEventIsContinuous set to 1", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 30, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventIsContinuous, 0);
        emit("646.4", @"Pixel unit dy=30, then kCGScrollWheelEventIsContinuous set to 0", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 0, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventPointDeltaAxis1, 12);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventPointDeltaAxis2, -4);
        emit("646.5", @"Pixel unit dy=0 dx=0, then kCGScrollWheelEventPointDeltaAxis1=12 Axis2=-4 set only", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 0, 0, 0);
        CGEventSetDoubleValueField(e, kCGScrollWheelEventFixedPtDeltaAxis1, 2.5);
        CGEventSetDoubleValueField(e, kCGScrollWheelEventFixedPtDeltaAxis2, -1.25);
        emit("646.6", @"Pixel unit dy=0 dx=0, then kCGScrollWheelEventFixedPtDeltaAxis1=2.5 Axis2=-1.25 set only", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
        CGEventSetDoubleValueField(e, kCGScrollWheelEventFixedPtDeltaAxis1, 2.5);
        emit("646.7", @"Pixel unit dy=10, then FixedPtDeltaAxis1 changed to 2.5 (fields disagree)", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventDeltaAxis1, 3);
        emit("646.8", @"Pixel unit dy=10, then kCGScrollWheelEventDeltaAxis1 changed to 3 (fields disagree)", e);

        // Phases.
        struct { int64_t v; const char *n; } sp[] = {{kCGScrollPhaseBegan, "Began"}, {kCGScrollPhaseChanged, "Changed"}, {kCGScrollPhaseEnded, "Ended"},
                                                     {kCGScrollPhaseCancelled, "Cancelled"}, {kCGScrollPhaseMayBegin, "MayBegin"}, {0, "0"}};
        for (int u = 0; u < 2; u++)
            for (int i = 0; i < 6; i++) {
                e = CGEventCreateScrollWheelEvent2(NULL, u ? kCGScrollEventUnitLine : kCGScrollEventUnitPixel, 2, 10, 0, 0);
                CGEventSetIntegerValueField(e, kCGScrollWheelEventScrollPhase, sp[i].v);
                emit("646.9", PF(@"unit=%s dy=10; scroll phase field set to %s (%lld)", u ? "Line" : "Pixel", sp[i].n, (long long)sp[i].v), e);
            }
        struct { int64_t v; const char *n; } mp[] = {{kCGMomentumScrollPhaseNone, "None"}, {kCGMomentumScrollPhaseBegin, "Begin"}, {kCGMomentumScrollPhaseContinue, "Continue"}, {kCGMomentumScrollPhaseEnd, "End"}};
        for (int i = 0; i < 4; i++) {
            e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
            CGEventSetIntegerValueField(e, kCGScrollWheelEventMomentumPhase, mp[i].v);
            emit("646.10", PF(@"unit=Pixel dy=10; momentum phase field set to %s (%lld)", mp[i].n, (long long)mp[i].v), e);
        }
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventScrollPhase, kCGScrollPhaseChanged);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventMomentumPhase, kCGMomentumScrollPhaseContinue);
        emit("646.11", @"unit=Pixel dy=10; scroll phase Changed and momentum phase Continue both set", e);
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
        CGEventSetIntegerValueField(e, kCGScrollWheelEventScrollCount, 5);
        emit("646.12", @"unit=Pixel dy=10; kCGScrollWheelEventScrollCount set to 5", e);

        // A flag that a gesture-less event keeps: nothing set, plain pixel event in a loop of three, order does not matter
        e = CGEventCreateScrollWheelEvent2(NULL, kCGScrollEventUnitPixel, 2, 10, 0, 0);
        NSEvent *ev = [NSEvent eventWithCGEvent:e];
        probe_line("646.13", @"Pixel dy=10: NSEvent methods called twice give the same answer", PF(@"%d %g %g | %d %g %g", (int)[ev hasPreciseScrollingDeltas], (double)[ev scrollingDeltaY], (double)[ev deltaY], (int)[ev hasPreciseScrollingDeltas], (double)[ev scrollingDeltaY], (double)[ev deltaY]));
        CFRelease(e);
    }
    return 0;
}
