/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel
 Copyright (C) 2026 Darling Developers

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

#import "AVPlaybackPrivate.h"
#import <AVFoundation/AVComposition.h>
#import "AVPlaybackEngine.h"
#import <AVFoundation/AVError.h>
#import <AVFoundation/AVURLAsset.h>
#import <CoreGraphics/CoreGraphics.h>
#import <darling-stub-forwarding.h>
#include <dispatch/dispatch.h>

static const NSInteger AVPlaybackErrorUnknown = -11800;
static const NSInteger AVPlaybackErrorFileFormatNotRecognized = -11828;

#pragma mark - Pixel buffer images

static void releasePixelBufferImageData(void *info, const void *data, size_t size)
{
    CVPixelBufferRef pixelBuffer = info;
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(pixelBuffer);
}

CGImageRef AVPlaybackCreateImageFromPixelBuffer(CVPixelBufferRef pixelBuffer)
{
    if (!pixelBuffer || CVPixelBufferGetPixelFormatType(pixelBuffer) != kCVPixelFormatType_32BGRA)
        return NULL;
    if (CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess)
        return NULL;

    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
    const void *base = CVPixelBufferGetBaseAddress(pixelBuffer);
    CGDataProviderRef provider = NULL;
    if (base) {
        CVPixelBufferRetain(pixelBuffer);
        provider = CGDataProviderCreateWithData(pixelBuffer, base, bytesPerRow * height, releasePixelBufferImageData);
    }
    if (!provider) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return NULL;
    }
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGImageRef image = CGImageCreate(width, height, 8, 32, bytesPerRow, space,
        kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst, provider, NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(space);
    CGDataProviderRelease(provider); // the image holds it, and through it the locked buffer
    return image;
}

#pragma mark - Periodic time observers

@interface AVPlayerPeriodicObserver : NSObject {
@public
    pthread_mutex_t lock;
    AVPlayerInternal *owner; // not retained; cleared by -detach
    void (^block)(CMTime);
    dispatch_source_t timer;
    CMTime lastReported;
}
@end

@implementation AVPlayerPeriodicObserver

- (void)detach
{
    pthread_mutex_lock(&lock);
    owner = nil;
    pthread_mutex_unlock(&lock);
    if (timer) {
        dispatch_source_set_event_handler(timer, NULL); // breaks the cycle through the handler
        dispatch_source_cancel(timer);
        dispatch_release(timer);
        timer = NULL;
    }
}

// On the observer's queue. Reports while playing, and once more after the time last changed.
- (void)fire
{
    pthread_mutex_lock(&lock);
    CMTime time = kCMTimeInvalid;
    BOOL playing = NO;
    if (owner) {
        time = [owner currentTime];
        playing = [owner rate] > 0;
    }
    BOOL report = owner && (playing || CMTimeCompare(time, lastReported) != 0 || CMTIME_IS_INVALID(lastReported));
    if (report)
        lastReported = time;
    void (^callback)(CMTime) = report ? [block retain] : nil;
    pthread_mutex_unlock(&lock);
    if (callback) {
        callback(time);
        [callback release];
    }
}

- (void)dealloc
{
    if (timer) {
        dispatch_source_set_event_handler(timer, NULL);
        dispatch_source_cancel(timer);
        dispatch_release(timer);
    }
    [block release];
    pthread_mutex_destroy(&lock);
    [super dealloc];
}

@end

#pragma mark - Player state

static void engineFrame(void *context, CVPixelBufferRef pixelBuffer)
{
    [(AVPlayerInternal *)context _engineFrame:pixelBuffer];
}

static void engineEnded(void *context)
{
    [(AVPlayerInternal *)context _engineEnded];
}

static void engineSeekDone(void *context, uint64_t token, bool finished)
{
    [(AVPlayerInternal *)context _engineSeekDone:token finished:finished];
}

static void engineStarted(void *context)
{
    [(AVPlayerInternal *)context _engineStarted];
}

@interface AVPlayerInternal ()
- (void)_engineFrame:(CVPixelBufferRef)pixelBuffer;
- (void)_engineEnded;
- (void)_engineSeekDone:(uint64_t)token finished:(BOOL)finished;
- (void)_engineStarted;
- (void)_deliverLatestFrame;
- (void)_onMain:(void (^)(AVPlayer *player))block;
- (void)_stopEngine;
@end

@implementation AVPlayerInternal

- (void)dealloc
{
    if (latestFrame)
        CVPixelBufferRelease(latestFrame);
    if (latestImage)
        CGImageRelease(latestImage);
    [item release];
    [error release];
    [seekHandlers release];
    [layers release];
    [observers release];
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&ownerLock);
    [super dealloc];
}


- (instancetype)initWithPlayer:(AVPlayer *)owner
{
    if ((self = [super init])) {
        player = owner;
        volume = 1.0f;
        actionAtItemEnd = AVPlayerActionAtItemEndPause;
        status = AVPlayerStatusUnknown;
        pthread_mutexattr_t attributes;
        pthread_mutexattr_init(&attributes);
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
        // Both are recursive: key-value observers and callbacks run with them held may call back in.
        pthread_mutex_init(&ownerLock, &attributes);
        pthread_mutex_init(&lock, &attributes);
        pthread_mutexattr_destroy(&attributes);
        seekHandlers = [[NSMutableDictionary alloc] init];
        layers = [[NSMutableArray alloc] init];
        observers = [[NSMutableArray alloc] init];
    }
    return self;
}

// Runs the block on the main queue with the owning player, if it is still there. The player can not
// go away while the block runs: its -dealloc waits for ownerLock.
- (void)_onMain:(void (^)(AVPlayer *))block
{
    void (^copy)(AVPlayer *) = [block copy];
    [self retain];
    dispatch_async(dispatch_get_main_queue(), ^{
        pthread_mutex_lock(&ownerLock);
        if (player && !invalidated)
            copy(player);
        pthread_mutex_unlock(&ownerLock);
        [copy release];
        [self release];
    });
}

- (void)_stopEngine
{
    struct AVPEEngine *old = engine;
    engine = NULL;
    // Joins the engine's thread: no frame or seek callback runs after this, and none of them may
    // be waiting for a lock the caller holds.
    AVPEEngineDestroy(old);

    pthread_mutex_lock(&lock);
    NSArray *handlers = [[seekHandlers allValues] retain];
    [seekHandlers removeAllObjects];
    if (latestFrame) {
        CVPixelBufferRelease(latestFrame);
        latestFrame = NULL;
    }
    if (latestImage) {
        CGImageRelease(latestImage);
        latestImage = NULL;
    }
    frameSerial++;
    pthread_mutex_unlock(&lock);

    for (void (^handler)(BOOL) in handlers) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{ handler(NO); });
    }
    [handlers release];
}

- (void)invalidate
{
    pthread_mutex_lock(&ownerLock);
    if (invalidated) {
        pthread_mutex_unlock(&ownerLock);
        return;
    }
    invalidated = YES;
    player = nil;
    pthread_mutex_unlock(&ownerLock);

    pthread_mutex_lock(&lock);
    NSArray *current = [observers copy];
    [observers removeAllObjects];
    pthread_mutex_unlock(&lock);
    for (AVPlayerPeriodicObserver *observer in current)
        [observer detach];
    [current release];

    [self _stopEngine];
    if (item) {
        [item _internal]->player = nil;
        [item release];
        item = nil;
    }
}

- (void)replaceItem:(AVPlayerItem *)newItem
{
    [newItem retain];
    [self _stopEngine];
    if (item) {
        [item _internal]->player = nil;
        [item release];
    }
    item = newItem;

    [error release];
    error = nil;
    status = AVPlayerStatusUnknown;
    if (!item)
        return;

    AVPlayer *owner = player;
    [item _internal]->player = owner;

    NSError *failure = nil;
    AVAsset *asset = [item asset];
    NSURL *assetURL = nil;
    if ([asset isKindOfClass:[AVURLAsset class]])
        assetURL = [(AVURLAsset *)asset URL];
    else if ([asset isKindOfClass:[AVComposition class]])
        assetURL = [(AVComposition *)asset _playbackURL];
    if (assetURL && [asset isPlayable] && [assetURL isFileURL]) {
        AVPEMedia *media = AVPEMediaOpen([[assetURL path] fileSystemRepresentation]);
        // A composition of one file plays that file: not when it holds only a part of it.
        if (media && [asset isKindOfClass:[AVComposition class]]
            && fabs(CMTimeGetSeconds([asset duration]) - AVPEMediaDurationUs(media) / 1e6) > 0.001) {
            AVPEMediaClose(media);
            media = NULL;
        }
        if (media) {
            AVPECallbacks callbacks = { self, engineFrame, engineEnded, engineSeekDone, engineStarted };
            engine = AVPEEngineCreate(media, callbacks); // takes the media, also when it fails
        }
    }
    [self updateVolume];
    [self updateAudioTimePitchAlgorithm];
    if (!engine) {
        failure = [NSError errorWithDomain:AVFoundationErrorDomain code:AVPlaybackErrorFileFormatNotRecognized
                                  userInfo:@{ NSLocalizedDescriptionKey: @"Cannot Open" }];
    }

    // Like on macOS the player and the item become ready (or fail) a little later, not within
    // the call that gave the item to the player.
    AVPlayerItem *current = item;
    NSError *result = [failure retain];
    [self _onMain:^(AVPlayer *owner) {
        if (item != current)
            return;
        [owner willChangeValueForKey:@"status"];
        [owner willChangeValueForKey:@"error"];
        status = result ? AVPlayerStatusFailed : AVPlayerStatusReadyToPlay;
        error = [result retain];
        [owner didChangeValueForKey:@"error"];
        [owner didChangeValueForKey:@"status"];
        [current _setStatus:result ? AVPlayerItemStatusFailed : AVPlayerItemStatusReadyToPlay error:result];
    }];
    [result release];
}

// The algorithm of the item as the engine knows it: the deprecated LowQualityZeroLatency plays like
// TimeDomain, a name that is not an algorithm like the default.
- (void)updateAudioTimePitchAlgorithm
{
    if (!engine || !item)
        return;
    NSString *name = [item audioTimePitchAlgorithm];
    AVPEAudioAlgorithm algorithm = AVPEAudioAlgorithmSpectral;
    if ([name isEqualToString:AVAudioTimePitchAlgorithmTimeDomain] || [name isEqualToString:AVAudioTimePitchAlgorithmLowQualityZeroLatency])
        algorithm = AVPEAudioAlgorithmTimeDomain;
    else if ([name isEqualToString:AVAudioTimePitchAlgorithmVarispeed])
        algorithm = AVPEAudioAlgorithmVarispeed;
    AVPEEngineSetAudioAlgorithm(engine, algorithm);
}

// The level of the audio: the volume, or silence while muted.
- (void)updateVolume
{
    if (engine)
        AVPEEngineSetVolume(engine, muted ? 0.0f : volume);
}

- (NSInteger)timeControlStatus
{
    if (!engine || AVPEEngineRate(engine) <= 0)
        return 0; // AVPlayerTimeControlStatusPaused
    // Waiting while the clock stands still until the audio of the position is heard.
    return AVPEEngineWaiting(engine) ? 1 : 2;
}

- (float)rate
{
    return engine ? AVPEEngineRate(engine) : 0.0f;
}

- (void)setRate:(float)rate
{
    if (engine)
        AVPEEngineSetRate(engine, rate);
}

- (CMTime)currentTime
{
    if (!engine)
        return kCMTimeZero;
    return CMTimeMake(AVPEEngineTimeUs(engine), 1000000);
}

- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL))completionHandler
{
    if (!engine || !CMTIME_IS_NUMERIC(time)) {
        if (completionHandler) {
            void (^handler)(BOOL) = [completionHandler copy];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{ handler(NO); [handler release]; });
        }
        return;
    }

    uint64_t token;
    pthread_mutex_lock(&lock);
    token = ++lastToken;
    if (completionHandler)
        seekHandlers[@(token)] = [[completionHandler copy] autorelease];
    pthread_mutex_unlock(&lock);

    double seconds = CMTimeGetSeconds(time);
    AVPEEngineSeek(engine, (int64_t)(seconds * 1000000.0 + (seconds < 0 ? -0.5 : 0.5)), token);
}

- (void)registerLayer:(AVPlayerLayer *)layer
{
    NSValue *entry = [NSValue valueWithNonretainedObject:layer];
    pthread_mutex_lock(&lock);
    if (![layers containsObject:entry])
        [layers addObject:entry];
    pthread_mutex_unlock(&lock);
}

- (void)unregisterLayer:(AVPlayerLayer *)layer
{
    NSValue *entry = [NSValue valueWithNonretainedObject:layer];
    pthread_mutex_lock(&lock);
    [layers removeObject:entry];
    pthread_mutex_unlock(&lock);
}

- (CGImageRef)copyLatestImage
{
    pthread_mutex_lock(&lock);
    if (latestFrame && (!latestImage || latestImageSerial != frameSerial)) {
        CGImageRef image = AVPlaybackCreateImageFromPixelBuffer(latestFrame);
        if (latestImage)
            CGImageRelease(latestImage);
        latestImage = image;
        latestImageSerial = frameSerial;
    }
    CGImageRef result = latestImage ? CGImageRetain(latestImage) : NULL;
    pthread_mutex_unlock(&lock);
    return result;
}

- (CVPixelBufferRef)copyLatestPixelBuffer:(uint64_t *)serial
{
    pthread_mutex_lock(&lock);
    CVPixelBufferRef result = latestFrame ? CVPixelBufferRetain(latestFrame) : NULL;
    if (serial)
        *serial = result ? frameSerial : 0;
    pthread_mutex_unlock(&lock);
    return result;
}

- (id)addPeriodicObserverWithInterval:(CMTime)interval queue:(dispatch_queue_t)queue block:(void (^)(CMTime))block
{
    double seconds = CMTIME_IS_NUMERIC(interval) ? CMTimeGetSeconds(interval) : 0;
    if (seconds < 0.001)
        seconds = 0.001;
    uint64_t nanoseconds = (uint64_t)(seconds * 1e9);

    AVPlayerPeriodicObserver *observer = [[AVPlayerPeriodicObserver alloc] init];
    pthread_mutex_init(&observer->lock, NULL);
    observer->owner = self;
    observer->block = [block copy];
    observer->lastReported = kCMTimeInvalid;
    observer->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue ?: dispatch_get_main_queue());
    dispatch_source_set_timer(observer->timer, dispatch_time(DISPATCH_TIME_NOW, nanoseconds), nanoseconds, nanoseconds / 10);
    dispatch_source_set_event_handler(observer->timer, ^{ [observer fire]; });

    pthread_mutex_lock(&lock);
    [observers addObject:observer];
    pthread_mutex_unlock(&lock);
    dispatch_resume(observer->timer);
    return [observer autorelease];
}

- (void)removePeriodicObserver:(id)observer
{
    if (![observer isKindOfClass:[AVPlayerPeriodicObserver class]])
        return;
    [observer retain];
    pthread_mutex_lock(&lock);
    [observers removeObject:observer];
    pthread_mutex_unlock(&lock);
    [observer detach];
    [observer release];
}

#pragma mark - Engine callbacks

// On the engine's thread.
- (void)_engineFrame:(CVPixelBufferRef)pixelBuffer
{
    pthread_mutex_lock(&lock);
    CVPixelBufferRef old = latestFrame;
    latestFrame = CVPixelBufferRetain(pixelBuffer);
    frameSerial++;
    BOOL schedule = !deliveryPending;
    deliveryPending = YES;
    pthread_mutex_unlock(&lock);
    if (old)
        CVPixelBufferRelease(old);

    // One delivery in flight at most: if the main thread is slow, the frames in between are skipped.
    if (schedule) {
        [self retain];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self _deliverLatestFrame];
            [self release];
        });
    }
}

// On the main thread.
- (void)_deliverLatestFrame
{
    pthread_mutex_lock(&lock);
    deliveryPending = NO;
    pthread_mutex_unlock(&lock);

    CGImageRef image = [self copyLatestImage];
    if (!image)
        return;

    pthread_mutex_lock(&lock);
    NSArray *current = [layers copy];
    for (NSValue *entry in current) {
        if ([layers containsObject:entry]) // an earlier layer's observer may have removed it
            [(AVPlayerLayer *)[entry nonretainedObjectValue] _showImage:image];
    }
    pthread_mutex_unlock(&lock);
    [current release];
    CGImageRelease(image);
}

- (void)_engineStarted
{
    [self _onMain:^(AVPlayer *owner) {
        [owner willChangeValueForKey:@"timeControlStatus"];
        [owner didChangeValueForKey:@"timeControlStatus"];
    }];
}

- (void)_engineEnded
{
    [self _onMain:^(AVPlayer *owner) {
        [owner willChangeValueForKey:@"rate"];
        [owner willChangeValueForKey:@"timeControlStatus"];
        [owner didChangeValueForKey:@"timeControlStatus"];
        [owner didChangeValueForKey:@"rate"];
        AVPlayerItem *current = item;
        if (current) {
            [[NSNotificationCenter defaultCenter] postNotificationName:AVPlayerItemDidPlayToEndTimeNotification object:current];
        }
    }];
}

- (void)_engineSeekDone:(uint64_t)token finished:(BOOL)finished
{
    pthread_mutex_lock(&lock);
    void (^handler)(BOOL) = [seekHandlers[@(token)] retain];
    [seekHandlers removeObjectForKey:@(token)];
    pthread_mutex_unlock(&lock);

    if (token != 0 && finished) {
        [self _onMain:^(AVPlayer *owner) {
            if (item)
                [[NSNotificationCenter defaultCenter] postNotificationName:AVPlayerItemTimeJumpedNotification object:item];
        }];
    }
    if (handler) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            handler(finished);
            [handler release];
        });
    }
}

#pragma mark - Unimplemented API

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
