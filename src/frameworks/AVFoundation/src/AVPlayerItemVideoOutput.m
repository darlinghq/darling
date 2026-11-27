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
#include <mach/mach_time.h>
#import <darling-stub-forwarding.h>

@implementation AVPlayerItemVideoOutput

- (instancetype)initWithPixelBufferAttributes:(NSDictionary<NSString *, id> *)pixelBufferAttributes
{
    if ((self = [super init]))
        _videoOutputInternal = [[AVPlayerItemVideoOutputInternal alloc] init];
    return self;
}

- (instancetype)initWithOutputSettings:(NSDictionary<NSString *, id> *)outputSettings
{
    // A nil dictionary selects the defaults; a dictionary naming nothing at all is rejected.
    if (outputSettings != nil && [outputSettings count] == 0) {
        [self release];
        [NSException raise:NSInvalidArgumentException
                    format:@"*** -[AVPlayerItemVideoOutput initWithOutputSettings:] Output settings dictionary does not contain any recognized keys"];
    }
    return [self initWithPixelBufferAttributes:outputSettings];
}

- (instancetype)init
{
    return [self initWithPixelBufferAttributes:nil];
}

- (void)dealloc
{
    if (_videoOutputInternal && _videoOutputInternal->delegateQueue) {
        dispatch_release(_videoOutputInternal->delegateQueue);
        _videoOutputInternal->delegateQueue = NULL;
    }
    [_videoOutputInternal release];
    [super dealloc];
}

- (id<AVPlayerItemOutputPullDelegate>)delegate
{
    return _videoOutputInternal->delegate;
}

- (dispatch_queue_t)delegateQueue
{
    return _videoOutputInternal->delegateQueue;
}

- (void)setDelegate:(id<AVPlayerItemOutputPullDelegate>)delegate queue:(dispatch_queue_t)delegateQueue
{
    _videoOutputInternal->delegate = delegate;
    if (_videoOutputInternal->delegateQueue != delegateQueue) {
        if (_videoOutputInternal->delegateQueue)
            dispatch_release(_videoOutputInternal->delegateQueue);
        _videoOutputInternal->delegateQueue = delegateQueue;
        if (_videoOutputInternal->delegateQueue)
            dispatch_retain(_videoOutputInternal->delegateQueue);
    }
}

- (void)requestNotificationOfMediaDataChangeWithAdvanceInterval:(NSTimeInterval)interval
{
}

// The item time shown at a host time (seconds on the host clock, as CACurrentMediaTime counts):
// the player's current time advanced by the time left until then at the playback rate. Without
// an item to play the time is invalid.
- (CMTime)_itemTimeForHostSeconds:(double)hostSeconds
{
    AVPlayer *player = _videoOutputInternal->item ? [_videoOutputInternal->item _internal]->player : nil;
    if (!player)
        return kCMTimeInvalid;

    static mach_timebase_info_data_t timebase;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ mach_timebase_info(&timebase); });
    double now = (double)mach_absolute_time() * timebase.numer / timebase.denom / 1e9;

    CMTime current = [player currentTime];
    if (!CMTIME_IS_VALID(current))
        return kCMTimeInvalid;
    return CMTimeAdd(current, CMTimeMakeWithSeconds((hostSeconds - now) * [player rate], 1000000000));
}

- (CMTime)itemTimeForHostTime:(CFTimeInterval)hostTimeInSeconds
{
    return [self _itemTimeForHostSeconds:hostTimeInSeconds];
}

- (CMTime)itemTimeForCVTimeStamp:(CVTimeStamp)timestamp
{
    if (!(timestamp.flags & kCVTimeStampHostTimeValid))
        return kCMTimeInvalid;
    static mach_timebase_info_data_t timebase;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ mach_timebase_info(&timebase); });
    double seconds = (double)timestamp.hostTime * timebase.numer / timebase.denom / 1e9;
    return [self _itemTimeForHostSeconds:seconds];
}

- (void)_setItem:(AVPlayerItem *)item
{
    _videoOutputInternal->item = item;
    _videoOutputInternal->lastSerial = 0;
}

// The decoded frames are always 32BGRA, whatever pixel format the attributes ask for.
- (CVPixelBufferRef)_copyLatestAfterSerial:(uint64_t *)serial
{
    AVPlayer *player = _videoOutputInternal->item ? [_videoOutputInternal->item _internal]->player : nil;
    return player ? [[player _internal] copyLatestPixelBuffer:serial] : NULL;
}

- (BOOL)hasNewPixelBufferForItemTime:(CMTime)itemTime
{
    uint64_t serial = 0;
    CVPixelBufferRef buffer = [self _copyLatestAfterSerial:&serial];
    BOOL result = buffer != NULL && serial != _videoOutputInternal->lastSerial;
    if (buffer)
        CVPixelBufferRelease(buffer);
    return result;
}

- (CVPixelBufferRef)copyPixelBufferForItemTime:(CMTime)itemTime itemTimeForDisplay:(CMTime *)outItemTimeForDisplay
{
    uint64_t serial = 0;
    CVPixelBufferRef buffer = [self _copyLatestAfterSerial:&serial];
    if (buffer && serial == _videoOutputInternal->lastSerial) {
        CVPixelBufferRelease(buffer);
        buffer = NULL;
    }
    if (buffer) {
        _videoOutputInternal->lastSerial = serial;
        if (outItemTimeForDisplay)
            *outItemTimeForDisplay = [[_videoOutputInternal->item _internal]->player currentTime];
    }
    return buffer;
}

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
