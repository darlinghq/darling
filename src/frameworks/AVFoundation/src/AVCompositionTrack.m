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

#import "AVPlaybackPrivate.h"
#import <darling-stub-forwarding.h>

@implementation AVCompositionTrack

- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange
            formatDescriptions:(NSArray *)formatDescriptions
{
    if ((self = [super _initWithAsset:asset trackID:trackID mediaType:mediaType naturalSize:naturalSize
                     nominalFrameRate:nominalFrameRate timeRange:timeRange formatDescriptions:formatDescriptions]))
        _segments = [[NSMutableArray alloc] init];
    return self;
}

- (void)dealloc
{
    [_segments release];
    [super dealloc];
}

- (NSMutableArray *)_segmentStorage { return _segments; }

- (NSArray *)segments { return [[_segments copy] autorelease]; }

- (void)_refreshTimeRange
{
    AVCompositionTrackSegment *last = [_segments lastObject];
    // A track without segments has no time range.
    [self _internal]->timeRange = last ? CMTimeRangeMake(kCMTimeZero, CMTimeRangeGetEnd([last timeMapping].target))
                                       : kCMTimeRangeInvalid;
}

- (AVCompositionTrackSegment *)segmentForTrackTime:(CMTime)trackTime
{
    for (AVCompositionTrackSegment *segment in _segments) {
        if (CMTimeRangeContainsTime([segment timeMapping].target, trackTime))
            return segment;
    }
    return nil;
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
