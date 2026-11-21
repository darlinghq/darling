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
#import <darling-stub-forwarding.h>

@implementation AVPlayerItemVideoOutput

- (instancetype)initWithPixelBufferAttributes:(NSDictionary<NSString *, id> *)pixelBufferAttributes
{
    if ((self = [super init]))
        _videoOutputInternal = [[AVPlayerItemVideoOutputInternal alloc] init];
    return self;
}

- (instancetype)init
{
    return [self initWithPixelBufferAttributes:nil];
}

- (void)dealloc
{
    [_videoOutputInternal release];
    [super dealloc];
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
