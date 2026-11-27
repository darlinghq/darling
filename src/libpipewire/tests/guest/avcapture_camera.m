// Copyright (C) 2026 Lubos Dolezel
// AVCaptureDevice camera discovery and frame delivery. With no camera backend or no
// camera the device list is empty and the session cannot be built; with a camera
// the video data output delivers sample buffers with a pixel buffer of the
// negotiated size. Prints "CAMERAS n" so the harness can tell the two cases apart.
// Usage: avcapture_camera [expect-cameras|expect-none]
#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSAutoreleasePool.h>
#import <AVFoundation/AVCaptureDevice.h>
#import <AVFoundation/AVCaptureInput.h>
#import <AVFoundation/AVCaptureDeviceInput.h>
#import <AVFoundation/AVCaptureOutput.h>
#import <AVFoundation/AVCaptureSession.h>
#import <AVFoundation/AVCaptureVideoDataOutput.h>
#import <AVFoundation/AVMediaFormat.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <string.h>

static volatile int frames;
static volatile size_t fw, fh;

@interface Sink : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@end
@implementation Sink
- (void)captureOutput:(AVCaptureOutput *)o didOutputSampleBuffer:(CMSampleBufferRef)sb fromConnection:(AVCaptureConnection *)c
{
    CVImageBufferRef img = CMSampleBufferGetImageBuffer(sb);
    if (img) { fw = CVPixelBufferGetWidth(img); fh = CVPixelBufferGetHeight(img); frames++; }
}
@end

int main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int failures = 0;
    int wantCameras = argc > 1 && !strcmp(argv[1], "expect-cameras");
    int wantNone = argc > 1 && !strcmp(argv[1], "expect-none");
    NSArray *devs = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    printf("CAMERAS %lu\n", (unsigned long)[devs count]);
    if (wantNone && [devs count] != 0) { printf("FAIL: expected no cameras\n"); failures++; }
    if (wantCameras && [devs count] == 0) { printf("FAIL: expected a camera\n"); failures++; }
    if (wantNone && [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] != nil) { printf("FAIL: default device present\n"); failures++; }
    if ([devs count]) {
        AVCaptureDevice *dev = devs[0];
        printf("device: %s\n", [[dev localizedName] UTF8String]);
        printf("formats %lu active %s\n", (unsigned long)[[dev formats] count], [dev activeFormat] ? "yes" : "no");
        NSError *err = nil;
        AVCaptureDeviceInput *in = [AVCaptureDeviceInput deviceInputWithDevice:dev error:&err];
        if (!in) { printf("FAIL: input: %s\n", [[err description] UTF8String]); failures++; }
        else {
            AVCaptureSession *s = [[AVCaptureSession alloc] init];
            AVCaptureVideoDataOutput *out = [[AVCaptureVideoDataOutput alloc] init];
            Sink *sink = [[Sink alloc] init];
            dispatch_queue_t q = dispatch_queue_create("q2.camera", NULL);
            [out setSampleBufferDelegate:sink queue:q];
            if ([s canAddInput:in]) [s addInput:in]; else { printf("FAIL: cannot add input\n"); failures++; }
            if ([s canAddOutput:out]) [s addOutput:out]; else { printf("FAIL: cannot add output\n"); failures++; }
            [s startRunning];
            for (int i = 0; i < 100 && frames < 5; i++) [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            printf("frames %d size %zux%zu\n", frames, fw, fh);
            if (frames < 5 || fw == 0 || fh == 0) { printf("FAIL: no frames\n"); failures++; }
            [s stopRunning];
        }
    }
    [pool drain];
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
