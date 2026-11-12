/*
 This file is part of Darling.

 Copyright (C) 2017-2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <AVFoundation/AVSampleBufferDisplayLayer.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <objc/message.h>

NSNotificationName const AVSampleBufferDisplayLayerFailedToDecodeNotification =
    @"AVSampleBufferDisplayLayerFailedToDecodeNotification";
NSString *const AVSampleBufferDisplayLayerFailedToDecodeNotificationErrorKey =
    @"AVSampleBufferDisplayLayerFailedToDecodeNotificationErrorKey";
NSNotificationName const AVSampleBufferDisplayLayerRequiresFlushToResumeDecodingDidChangeNotification =
    @"AVSampleBufferDisplayLayerRequiresFlushToResumeDecodingDidChangeNotification";

/* The 32-bit RGB layouts Core Graphics draws; 0 for the others. */
static CGBitmapInfo bitmapInfoForPixelFormat(OSType format)
{
    switch (format) {
    case kCVPixelFormatType_32BGRA: return kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst;
    case kCVPixelFormatType_32ARGB: return kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipFirst;
    case kCVPixelFormatType_32RGBA: return kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipLast;
    case kCVPixelFormatType_32ABGR: return kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipLast;
    default: return 0;
    }
}

static CGImageRef createImageFromPixelBuffer(CVPixelBufferRef buffer)
{
    CGBitmapInfo info = bitmapInfoForPixelFormat(CVPixelBufferGetPixelFormatType(buffer));
    if (info == 0 || CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
        return NULL;
    size_t width = CVPixelBufferGetWidth(buffer), height = CVPixelBufferGetHeight(buffer);
    size_t rowBytes = CVPixelBufferGetBytesPerRow(buffer);
    CFDataRef data = CFDataCreate(NULL, CVPixelBufferGetBaseAddress(buffer), rowBytes * height);
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    if (data == NULL)
        return NULL;
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CFRelease(data);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGImageRef image = CGImageCreate(width, height, 8, 32, rowBytes, space, info, provider, NULL, false,
                                     kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider);
    return image;
}

/*
 * Shows each enqueued video frame as the layer's contents when it is enqueued: the frames of a
 * client that paces its own output (VLC does) appear on time. Frames whose pixels Core Graphics
 * cannot draw (biplanar YCbCr) fail the layer with an error, as an undecodable frame does.
 */
@implementation AVSampleBufferDisplayLayer {
    CMTimebaseRef _controlTimebase;
    NSString *_videoGravity;
    AVQueuedSampleBufferRenderingStatus _status;
    NSError *_error;
    BOOL _preventsCapture;
    BOOL _preventsDisplaySleepDuringVideoPlayback;
    dispatch_queue_t _requestQueue;
    void (^_requestBlock)(void);
}

- (instancetype)init
{
    if ((self = [super init]) != nil) {
        _videoGravity = [AVLayerVideoGravityResizeAspect copy];
        _preventsDisplaySleepDuringVideoPlayback = YES;
    }
    return self;
}

- (void)dealloc
{
    [self stopRequestingMediaData];
    if (_controlTimebase != NULL)
        CFRelease(_controlTimebase);
    [_videoGravity release];
    [_error release];
    [super dealloc];
}

- (CMTimebaseRef)controlTimebase { return _controlTimebase; }

- (void)setControlTimebase:(CMTimebaseRef)timebase
{
    if (timebase != NULL)
        CFRetain(timebase);
    if (_controlTimebase != NULL)
        CFRelease(_controlTimebase);
    _controlTimebase = timebase;
}

- (CMTimebaseRef)timebase { return _controlTimebase; }

- (NSString *)videoGravity { return _videoGravity; }

- (void)setVideoGravity:(NSString *)gravity
{
    NSString *old = _videoGravity;
    _videoGravity = [gravity copy];
    [old release];
    if ([gravity isEqualToString:AVLayerVideoGravityResize])
        self.contentsGravity = kCAGravityResize;
    else if ([gravity isEqualToString:AVLayerVideoGravityResizeAspectFill])
        self.contentsGravity = kCAGravityResizeAspectFill;
    else
        self.contentsGravity = kCAGravityResizeAspect;
}

@synthesize status = _status, error = _error, preventsCapture = _preventsCapture,
    preventsDisplaySleepDuringVideoPlayback = _preventsDisplaySleepDuringVideoPlayback;

- (BOOL)requiresFlushToResumeDecoding { return NO; }
- (BOOL)isReadyForMoreMediaData { return _status != AVQueuedSampleBufferRenderingStatusFailed; }
- (BOOL)hasSufficientMediaDataForReliablePlaybackStart { return YES; }

- (void)failWithDescription:(NSString *)description
{
    NSLog(@"%@ failed: %@", [self class], description);
    [self willChangeValueForKey:@"status"];
    _status = AVQueuedSampleBufferRenderingStatusFailed;
    [_error release];
    _error = [[NSError alloc] initWithDomain:@"AVFoundationErrorDomain" code:-11821 /* AVErrorDecodeFailed */
                                    userInfo:@{ NSLocalizedDescriptionKey: description }];
    [self didChangeValueForKey:@"status"];
    [[NSNotificationCenter defaultCenter]
        postNotificationName:AVSampleBufferDisplayLayerFailedToDecodeNotification object:self
                    userInfo:@{ AVSampleBufferDisplayLayerFailedToDecodeNotificationErrorKey: _error }];
}

- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer
{
    if (sampleBuffer == NULL || _status == AVQueuedSampleBufferRenderingStatusFailed)
        return;
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (imageBuffer == NULL) {
        [self failWithDescription:@"The sample buffer has no image buffer to display"];
        return;
    }
    CGImageRef image = createImageFromPixelBuffer(imageBuffer);
    if (image == NULL) {
        [self failWithDescription:@"The image buffer's pixel format cannot be displayed"];
        return;
    }
    if (_status != AVQueuedSampleBufferRenderingStatusRendering) {
        [self willChangeValueForKey:@"status"];
        _status = AVQueuedSampleBufferRenderingStatusRendering;
        [self didChangeValueForKey:@"status"];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        self.contents = (id)image;
        [CATransaction commit];
        CGImageRelease(image);
        /* UIKit draws a layer's contents with its view: as with a presented GL frame, the view
           is redrawn and its window flushed. */
        id view = self.delegate;
        if ([view respondsToSelector:@selector(setNeedsDisplay:)])
            ((void (*)(id, SEL, BOOL))objc_msgSend)(view, @selector(setNeedsDisplay:), YES);
        id window = [view respondsToSelector:@selector(_appKitWindow)]
            ? ((id (*)(id, SEL))objc_msgSend)(view, @selector(_appKitWindow)) : nil;
        if ([window respondsToSelector:@selector(displayIfNeeded)])
            ((void (*)(id, SEL))objc_msgSend)(window, @selector(displayIfNeeded));
    });
}

- (void)flush
{
    if (_status == AVQueuedSampleBufferRenderingStatusFailed) {
        [self willChangeValueForKey:@"status"];
        _status = AVQueuedSampleBufferRenderingStatusUnknown;
        [_error release];
        _error = nil;
        [self didChangeValueForKey:@"status"];
    }
}

- (void)flushAndRemoveImage
{
    [self flush];
    dispatch_async(dispatch_get_main_queue(), ^{
        self.contents = nil;
    });
}

- (void)requestMediaDataWhenReadyOnQueue:(dispatch_queue_t)queue usingBlock:(void (^)(void))block
{
    [self stopRequestingMediaData];
    _requestQueue = queue;
    dispatch_retain(queue);
    _requestBlock = [block copy];
    void (^request)(void) = [_requestBlock retain];
    dispatch_async(queue, ^{
        request();
        [request release];
    });
}

- (void)stopRequestingMediaData
{
    if (_requestQueue != NULL) {
        dispatch_release(_requestQueue);
        _requestQueue = NULL;
    }
    [_requestBlock release];
    _requestBlock = nil;
}

@end
