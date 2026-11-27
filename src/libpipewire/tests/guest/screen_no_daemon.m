// Copyright (C) 2026 Lubos Dolezel
// Without a ScreenCast portal or PipeWire server, starting a ScreenCaptureKit stream completes at once with
// an error, an AVCaptureSession with a screen input returns from startRunning at once and delivers no
// frames, and a CGDisplayStream fails to start. Run it without a session bus or compositor of the
// portal (no XDG_RUNTIME_DIR/DBUS_SESSION_BUS_ADDRESS pointing at one).
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#ifdef __LP64__
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#endif
#include <stdio.h>
#include <time.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

static volatile int frames;

@interface Counter : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@end
@implementation Counter
- (void)captureOutput:(AVCaptureOutput *)o didOutputSampleBuffer:(CMSampleBufferRef)sb fromConnection:(AVCaptureConnection *)c { frames++; }
@end

int main(void)
{
    @autoreleasepool {
        CGDirectDisplayID d = CGMainDisplayID();
        double t0 = now();
        CGDisplayStreamRef ds = CGDisplayStreamCreate(d, 64, 64, 'BGRA', NULL,
            ^(CGDisplayStreamFrameStatus s, uint64_t t, IOSurfaceRef surf, CGDisplayStreamUpdateRef u) {});
        CHECK(ds != NULL);
        if (ds) {
            CHECK(CGDisplayStreamStart(ds) != kCGErrorSuccess);
            CGDisplayStreamStop(ds);
            CFRelease(ds);
        }
        printf("display stream start: %.2fs\n", now() - t0);
        CHECK(now() - t0 < 5);

        t0 = now();
        AVCaptureSession *s = [[AVCaptureSession alloc] init];
        AVCaptureScreenInput *in = [[AVCaptureScreenInput alloc] initWithDisplayID:d];
        AVCaptureVideoDataOutput *out = [[AVCaptureVideoDataOutput alloc] init];
        Counter *counter = [[Counter alloc] init];
        [out setSampleBufferDelegate:counter queue:dispatch_queue_create("nd.q", NULL)];
        if ([s canAddInput:in]) [s addInput:in];
        if ([s canAddOutput:out]) [s addOutput:out];
        [s startRunning];
        printf("startRunning: %.2fs\n", now() - t0);
        CHECK(now() - t0 < 5);
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
        [s stopRunning];
        CHECK(frames == 0);

#ifdef __LP64__
        t0 = now();
        SCStreamConfiguration *cfg = [[SCStreamConfiguration alloc] init];
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        __block SCShareableContent *content = nil;
        [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *c, NSError *e) { content = c; dispatch_semaphore_signal(sem); }];
        dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC));
        SCDisplay *disp = [[content displays] firstObject];
        if (disp) {
            SCContentFilter *f = [[SCContentFilter alloc] initWithDisplay:disp excludingWindows:@[]];
            SCStream *st = [[SCStream alloc] initWithFilter:f configuration:cfg delegate:nil];
            __block NSError *err = nil;
            [st startCaptureWithCompletionHandler:^(NSError *e) { err = e; dispatch_semaphore_signal(sem); }];
            long to = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC));
            printf("SCStream start: %.2fs error %s\n", now() - t0, err ? [[err description] UTF8String] : "none");
            CHECK(to == 0);
            CHECK(err != nil);
            CHECK(now() - t0 < 5);
        } else {
            printf("SKIP: no SCDisplay\n");
        }
#endif
    }
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
