// Copyright (C) 2026 Lubos Dolezel
// Screen capture through ScreenCaptureKit and AVCaptureScreenInput of a display that
// shows the pattern drawn by pattern.py: four quadrants red, green, blue, white. A frame
// matches when each quadrant centre has the expected colour (tolerance 24 per channel).
// Usage: screen_pattern sck|avc|sck-crop|avc-crop
// The -crop modes capture one quadrant at half scale (sck-crop: the top-right one through the source rectangle,
// avc-crop: the bottom-left one through the crop rectangle, whose origin is at the bottom left) and expect an
// image of a quarter of the display in the colour of that quadrant, which checks the source rectangle and the
// output size of the display stream underneath.
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreGraphics/CoreGraphics.h>
#ifdef __LP64__
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#endif
#include <stdio.h>
#include <string.h>

static volatile int frames, matched;
static volatile size_t fw, fh;
static volatile OSType fmt;
static int tol = 24;
static int uniform;
static int uniformRgb[3];

static int near(int a, int b) { return abs(a - b) <= tol; }

static void checkImage(CVImageBufferRef img)
{
    CVPixelBufferLockBaseAddress(img, kCVPixelBufferLock_ReadOnly);
    size_t w = CVPixelBufferGetWidth(img), h = CVPixelBufferGetHeight(img), bpr = CVPixelBufferGetBytesPerRow(img);
    const unsigned char *b = CVPixelBufferGetBaseAddress(img);
    fw = w; fh = h; fmt = CVPixelBufferGetPixelFormatType(img);
    int ok = 0;
    if (b && fmt == kCVPixelFormatType_32BGRA) {
        static const int exp[4][3] = {{255,0,0},{0,255,0},{0,0,255},{255,255,255}};
        ok = 1;
        for (int q = 0; q < 4; q++) {
            if (uniform) {
                size_t ux = (q % 2 ? 3 * w / 4 : w / 4), uy = (q / 2 ? 3 * h / 4 : h / 4);
                const unsigned char *u = b + uy * bpr + ux * 4;
                if (!near(u[2], uniformRgb[0]) || !near(u[1], uniformRgb[1]) || !near(u[0], uniformRgb[2])) ok = 0;
                continue;
            }
            size_t x = (q % 2 ? 3 * w / 4 : w / 4), y = (q / 2 ? 3 * h / 4 : h / 4);
            const unsigned char *p = b + y * bpr + x * 4;
            int r = p[2], g = p[1], bl = p[0];
            if (frames == 1) printf("quadrant %d rgb %d %d %d\n", q, r, g, bl);
            if (!near(r, exp[q][0]) || !near(g, exp[q][1]) || !near(bl, exp[q][2])) ok = 0;
        }
    }
    CVPixelBufferUnlockBaseAddress(img, kCVPixelBufferLock_ReadOnly);
    if (ok) matched++;
}

@interface Sink : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate
#ifdef __LP64__
, SCStreamOutput
#endif
>
@end
@implementation Sink
- (void)captureOutput:(AVCaptureOutput *)o didOutputSampleBuffer:(CMSampleBufferRef)sb fromConnection:(AVCaptureConnection *)c
{
    CVImageBufferRef img = CMSampleBufferGetImageBuffer(sb);
    if (img) { frames++; checkImage(img); }
}
#ifdef __LP64__
- (void)stream:(SCStream *)s didOutputSampleBuffer:(CMSampleBufferRef)sb ofType:(SCStreamOutputType)t
{
    if (t != SCStreamOutputTypeScreen) return;
    CVImageBufferRef img = CMSampleBufferGetImageBuffer(sb);
    if (img) { frames++; checkImage(img); }
}
#endif
@end

int main(int argc, char **argv)
{
    @autoreleasepool {
        int failures = 0;
        int sck = argc > 1 && !strncmp(argv[1], "sck", 3);
        int crop = argc > 1 && strstr(argv[1], "-crop") != NULL;
        Sink *sink = [[Sink alloc] init];
        printf("preflight %d\n", CGPreflightScreenCaptureAccess());
        CGDirectDisplayID d = CGMainDisplayID();
        CGRect b = CGDisplayBounds(d);
        printf("display %u %gx%g\n", d, b.size.width, b.size.height);
#ifdef __LP64__
        if (sck) {
            dispatch_semaphore_t sem = dispatch_semaphore_create(0);
            __block SCShareableContent *content = nil; __block NSError *cerr = nil;
            [SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent *c, NSError *e) { content = c; cerr = e; dispatch_semaphore_signal(sem); }];
            dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 60 * NSEC_PER_SEC));
            printf("displays %lu error %s\n", (unsigned long)[[content displays] count], cerr ? [[cerr description] UTF8String] : "none");
            SCDisplay *disp = [[content displays] firstObject];
            if (!disp) { printf("FAIL: no display\nFAILED\n"); return 1; }
            printf("scdisplay %ldx%ld\n", (long)[disp width], (long)[disp height]);
            SCContentFilter *f = [[SCContentFilter alloc] initWithDisplay:disp excludingWindows:@[]];
            SCStreamConfiguration *cfg = [[SCStreamConfiguration alloc] init];
            cfg.width = [disp width]; cfg.height = [disp height];
            if (crop) {
                /* top-right quadrant (green), at half scale */
                uniform = 1; uniformRgb[0] = 0; uniformRgb[1] = 255; uniformRgb[2] = 0;
                cfg.sourceRect = CGRectMake(b.size.width / 2, 0, b.size.width / 2, b.size.height / 2);
                cfg.width = (size_t)(b.size.width / 4); cfg.height = (size_t)(b.size.height / 4);
            }
            cfg.pixelFormat = kCVPixelFormatType_32BGRA;
            cfg.minimumFrameInterval = CMTimeMake(1, 15);
            SCStream *st = [[SCStream alloc] initWithFilter:f configuration:cfg delegate:nil];
            NSError *err = nil;
            dispatch_queue_t q = dispatch_queue_create("sck.q", NULL);
            [st addStreamOutput:sink type:SCStreamOutputTypeScreen sampleHandlerQueue:q error:&err];
            if (err) printf("addStreamOutput error %s\n", [[err description] UTF8String]);
            dispatch_semaphore_t s2 = dispatch_semaphore_create(0);
            [st startCaptureWithCompletionHandler:^(NSError *e) { printf("start: %s\n", e ? [[e description] UTF8String] : "ok"); dispatch_semaphore_signal(s2); }];
            dispatch_semaphore_wait(s2, dispatch_time(DISPATCH_TIME_NOW, 60 * NSEC_PER_SEC));
            for (int i = 0; i < 150 && matched < 5; i++) [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            dispatch_semaphore_t s3 = dispatch_semaphore_create(0);
            [st stopCaptureWithCompletionHandler:^(NSError *e) { dispatch_semaphore_signal(s3); }];
            dispatch_semaphore_wait(s3, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC));
        } else
#endif
        {
            AVCaptureSession *s = [[AVCaptureSession alloc] init];
            AVCaptureScreenInput *in = [[AVCaptureScreenInput alloc] initWithDisplayID:d];
            AVCaptureVideoDataOutput *out = [[AVCaptureVideoDataOutput alloc] init];
            out.videoSettings = @{(id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)};
            dispatch_queue_t q = dispatch_queue_create("avc.q", NULL);
            [out setSampleBufferDelegate:sink queue:q];
            if (crop) {
                /* bottom-left quadrant (blue), at half scale; the crop rectangle has a bottom-left origin */
                uniform = 1; uniformRgb[0] = 0; uniformRgb[1] = 0; uniformRgb[2] = 255;
                in.cropRect = CGRectMake(0, 0, b.size.width / 2, b.size.height / 2);
                in.scaleFactor = 0.5;
            }
            if ([s canAddInput:in]) [s addInput:in]; else { printf("FAIL: cannot add input\n"); failures++; }
            if ([s canAddOutput:out]) [s addOutput:out]; else { printf("FAIL: cannot add output\n"); failures++; }
            [s startRunning];
            for (int i = 0; i < 150 && matched < 5; i++) [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            [s stopRunning];
        }
        printf("frames %d matched %d size %zux%zu fmt %.4s\n", frames, matched, fw, fh, (char[]){(char)(fmt>>24),(char)(fmt>>16),(char)(fmt>>8),(char)fmt,0});
        if (crop && frames >= 5 && (fw != (size_t)(b.size.width / 4) || fh != (size_t)(b.size.height / 4))) {
            printf("FAIL: wrong cropped size\n"); failures++;
        }
        if (frames < 5) { printf("FAIL: no frames\n"); failures++; }
        else if (matched < 5) { printf("FAIL: pixels do not match the pattern\n"); failures++; }
        printf(failures ? "FAILED\n" : "PASS\n");
        return failures != 0;
    }
}
