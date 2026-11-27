/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#import <AVFoundation/AVMutableComposition.h>
#import <AVFoundation/AVMutableCompositionTrack.h>
#import <AVFoundation/AVError.h>
#import "AVPlaybackPrivate.h"
#import <darling-stub-forwarding.h>

@implementation AVMutableComposition

@dynamic tracks;

+ (instancetype)composition
{
    return [[[self alloc] init] autorelease];
}

- (instancetype)init
{
    if ((self = [super init])) {
        AVAssetInternal *internal = [self _internal];
        [internal->tracks release];
        internal->tracks = [[NSMutableArray alloc] init];
        internal->playable = YES;
    }
    return self;
}

- (void)setNaturalSize:(CGSize)naturalSize
{
    [self _internal]->naturalSize = naturalSize;
}

- (AVMutableCompositionTrack *)addMutableTrackWithMediaType:(AVMediaType)mediaType
                                           preferredTrackID:(CMPersistentTrackID)preferredTrackID
{
    AVAssetInternal *internal = [self _internal];
    NSMutableArray *tracks = (NSMutableArray *)internal->tracks;
    CMPersistentTrackID trackID = preferredTrackID;
    if (trackID == kCMPersistentTrackID_Invalid) {
        CMPersistentTrackID maxID = 0;
        for (AVAssetTrack *t in tracks) {
            if ([t trackID] > maxID)
                maxID = [t trackID];
        }
        trackID = maxID + 1;
    }

    AVMutableCompositionTrack *track = [[AVMutableCompositionTrack alloc] _initWithAsset:self
                                                                                  trackID:trackID
                                                                                mediaType:mediaType
                                                                              naturalSize:CGSizeZero
                                                                         nominalFrameRate:0.0f
                                                                                timeRange:kCMTimeRangeZero
                                                                       formatDescriptions:nil];
    if (track) {
        [tracks addObject:track];
        [track release];
    }
    return track;
}

- (void)removeTrack:(AVCompositionTrack *)track
{
    if (!track)
        return;
    NSMutableArray *tracks = (NSMutableArray *)[self _internal]->tracks;
    [tracks removeObject:track];
}

- (AVMutableCompositionTrack *)mutableTrackCompatibleWithTrack:(AVAssetTrack *)track
{
    if (!track)
        return nil;
    NSString *mediaType = [track mediaType];
    for (AVMutableCompositionTrack *candidate in [self tracks]) {
        if ([candidate isKindOfClass:[AVMutableCompositionTrack class]] &&
            [[candidate mediaType] isEqualToString:mediaType]) {
            return candidate;
        }
    }
    return nil;
}

- (BOOL)insertTimeRange:(CMTimeRange)timeRange
                ofAsset:(AVAsset *)asset
                 atTime:(CMTime)startTime
                  error:(NSError **)outError
{
    if (outError)
        *outError = nil;
    if (!asset)
        return NO;

    for (AVAssetTrack *srcTrack in [asset tracks]) {
        AVMutableCompositionTrack *dstTrack = [self mutableTrackCompatibleWithTrack:srcTrack];
        if (!dstTrack) {
            dstTrack = [self addMutableTrackWithMediaType:[srcTrack mediaType]
                                         preferredTrackID:kCMPersistentTrackID_Invalid];
        }
        if (![dstTrack insertTimeRange:timeRange ofTrack:srcTrack atTime:startTime error:outError])
            return NO;
    }
    return YES;
}

- (void)insertEmptyTimeRange:(CMTimeRange)timeRange
{
    AVAssetInternal *internal = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTimeCompare(timeRange.duration, kCMTimeZero) > 0)
        internal->duration = CMTimeAdd(internal->duration, timeRange.duration);
}

- (void)removeTimeRange:(CMTimeRange)timeRange
{
    AVAssetInternal *internal = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTimeCompare(timeRange.duration, kCMTimeZero) > 0) {
        CMTime sub = CMTimeSubtract(internal->duration, timeRange.duration);
        internal->duration = CMTimeCompare(sub, kCMTimeZero) > 0 ? sub : kCMTimeZero;
    }
}

- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration
{
    AVAssetInternal *internal = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTIME_IS_VALID(duration)) {
        CMTime diff = CMTimeSubtract(duration, timeRange.duration);
        CMTime updated = CMTimeAdd(internal->duration, diff);
        internal->duration = CMTimeCompare(updated, kCMTimeZero) > 0 ? updated : kCMTimeZero;
    }
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
