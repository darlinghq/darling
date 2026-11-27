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
#import <AVFoundation/AVError.h>
#import <darling-stub-forwarding.h>

@implementation AVMutableComposition

+ (instancetype)composition
{
    return [[[self alloc] init] autorelease];
}

- (void)setNaturalSize:(CGSize)naturalSize
{
    [self _internal]->naturalSize = naturalSize;
}

// A snapshot of the composition: an immutable composition with copies of the tracks.
- (id)copyWithZone:(NSZone *)zone
{
    return [self _copyWithClass:[AVComposition class]];
}

- (id)mutableCopyWithZone:(NSZone *)zone
{
    return [self _copyWithClass:[AVMutableComposition class]];
}

- (AVMutableCompositionTrack *)addMutableTrackWithMediaType:(AVMediaType)mediaType
                                           preferredTrackID:(CMPersistentTrackID)preferredTrackID
{
    AVAssetInternal *internal = [self _internal];
    NSMutableArray *tracks = (NSMutableArray *)internal->tracks;
    CMPersistentTrackID trackID = preferredTrackID;
    BOOL taken = NO;
    CMPersistentTrackID maxID = 0;
    for (AVAssetTrack *t in tracks) {
        if ([t trackID] == trackID)
            taken = YES;
        if ([t trackID] > maxID)
            maxID = [t trackID];
    }
    // An unavailable or unspecified identifier is replaced by an unused one.
    if (trackID == kCMPersistentTrackID_Invalid || taken)
        trackID = maxID + 1;

    AVMutableCompositionTrack *track = [[AVMutableCompositionTrack alloc] _initWithAsset:self
                                                                                  trackID:trackID
                                                                                mediaType:mediaType
                                                                              naturalSize:CGSizeZero
                                                                         nominalFrameRate:0.0f
                                                                                timeRange:kCMTimeRangeInvalid
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
    [(NSMutableArray *)[self _internal]->tracks removeObject:track];
    [self _refreshDuration];
}

// A track the content of the given track can go into: one of the same media type.
- (AVMutableCompositionTrack *)mutableTrackCompatibleWithTrack:(AVAssetTrack *)track
{
    return [self _mutableTrackCompatibleWithTrack:track excluding:nil];
}

- (AVMutableCompositionTrack *)_mutableTrackCompatibleWithTrack:(AVAssetTrack *)track excluding:(NSArray *)excluded
{
    if (!track)
        return nil;
    NSString *mediaType = [track mediaType];
    for (AVMutableCompositionTrack *candidate in [self _internal]->tracks) {
        if ([candidate isKindOfClass:[AVMutableCompositionTrack class]] && ![excluded containsObject:candidate]
            && [[candidate mediaType] isEqualToString:mediaType])
            return candidate;
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
    if (!asset) {
        if (outError) {
            *outError = [NSError errorWithDomain:AVFoundationErrorDomain
                                            code:-11800
                                        userInfo:@{ NSLocalizedDescriptionKey: @"The operation could not be completed" }];
        }
        return NO;
    }

    // Each track of the asset goes into its own track of the composition.
    NSMutableArray *used = [NSMutableArray array];
    for (AVAssetTrack *source in [asset tracks]) {
        AVMutableCompositionTrack *destination = [self _mutableTrackCompatibleWithTrack:source excluding:used];
        if (!destination) {
            destination = [self addMutableTrackWithMediaType:[source mediaType]
                                            preferredTrackID:kCMPersistentTrackID_Invalid];
        }
        [used addObject:destination];
        if (![destination insertTimeRange:timeRange ofTrack:source atTime:startTime error:outError])
            return NO;
    }
    return YES;
}

- (void)insertEmptyTimeRange:(CMTimeRange)timeRange
{
    for (AVMutableCompositionTrack *track in [self tracks])
        [track insertEmptyTimeRange:timeRange];
}

- (void)removeTimeRange:(CMTimeRange)timeRange
{
    for (AVMutableCompositionTrack *track in [self tracks])
        [track removeTimeRange:timeRange];
}

- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration
{
    for (AVMutableCompositionTrack *track in [self tracks])
        [track scaleTimeRange:timeRange toDuration:duration];
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
