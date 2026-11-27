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
#import <AVFoundation/AVMutableComposition.h>
#import <AVFoundation/AVMutableCompositionTrack.h>
#import <darling-stub-forwarding.h>

@implementation AVComposition

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

// The tracks as they are now; a later edit of a mutable composition does not change the array.
- (NSArray<AVCompositionTrack *> *)tracks
{
    return [[[self _internal]->tracks copy] autorelease];
}

- (void)_refreshDuration
{
    AVAssetInternal *internal = [self _internal];
    CMTime end = kCMTimeZero;
    for (AVAssetTrack *track in internal->tracks) {
        CMTimeRange range = [track timeRange];
        if (CMTIME_IS_VALID(range.start) && CMTIME_IS_VALID(range.duration) && CMTimeCompare(CMTimeRangeGetEnd(range), end) > 0)
            end = CMTimeRangeGetEnd(range);
    }
    internal->duration = end;
}

// The composition plays through the engine that plays files, so only a composition that is one file
// from beginning to end, unchanged, can play: every stretch of it is the same file at the same
// time in the composition as in the file.
- (NSURL *)_playbackURL
{
    NSURL *url = nil;
    for (AVCompositionTrack *track in [self _internal]->tracks) {
        for (AVCompositionTrackSegment *segment in [track _segmentStorage]) {
            if ([segment isEmpty])
                continue;
            CMTimeMapping mapping = [segment timeMapping];
            if (CMTimeCompare(mapping.source.start, mapping.target.start) != 0
                || CMTimeCompare(mapping.source.duration, mapping.target.duration) != 0)
                return nil;
            if (url == nil)
                url = [segment sourceURL];
            else if (![url isEqual:[segment sourceURL]])
                return nil;
        }
    }
    return url;
}

// A copy of the composition of the given class (AVComposition or AVMutableComposition) with its own
// tracks; the segments are immutable and shared.
- (id)_copyWithClass:(Class)cls
{
    AVComposition *copy = [[cls alloc] init];
    AVAssetInternal *from = [self _internal], *to = [copy _internal];
    to->duration = from->duration;
    to->naturalSize = from->naturalSize;
    to->URL = [from->URL retain];
    Class trackClass = [cls isSubclassOfClass:[AVMutableComposition class]] ? [AVMutableCompositionTrack class] : [AVCompositionTrack class];
    for (AVCompositionTrack *track in from->tracks) {
        AVAssetTrackInternal *t = [track _internal];
        AVCompositionTrack *clone = [[trackClass alloc] _initWithAsset:copy trackID:t->trackID mediaType:t->mediaType
                                                           naturalSize:t->naturalSize nominalFrameRate:t->nominalFrameRate
                                                             timeRange:t->timeRange formatDescriptions:t->formatDescriptions];
        [[clone _segmentStorage] addObjectsFromArray:[track _segmentStorage]];
        if ([clone isKindOfClass:[AVMutableCompositionTrack class]] && [track isKindOfClass:[AVMutableCompositionTrack class]]) {
            AVMutableCompositionTrack *source = (AVMutableCompositionTrack *)track, *target = (AVMutableCompositionTrack *)clone;
            target.naturalTimeScale = source.naturalTimeScale;
            target.languageCode = source.languageCode;
            target.extendedLanguageTag = source.extendedLanguageTag;
            target.preferredTransform = source.preferredTransform;
            target.preferredVolume = source.preferredVolume;
        }
        [(NSMutableArray *)to->tracks addObject:clone];
        [clone release];
    }
    return copy;
}

// An immutable composition is its own copy.
- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (id)mutableCopyWithZone:(NSZone *)zone
{
    return [self _copyWithClass:[AVMutableComposition class]];
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
