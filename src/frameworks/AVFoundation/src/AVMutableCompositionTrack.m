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

#import <AVFoundation/AVMutableCompositionTrack.h>
#import <AVFoundation/AVComposition.h>
#import <AVFoundation/AVError.h>
#import "AVPlaybackPrivate.h"
#import <darling-stub-forwarding.h>

@implementation AVMutableCompositionTrack {
    CMTimeScale _naturalTimeScale;
    NSString *_languageCode;
    NSString *_extendedLanguageTag;
    CGAffineTransform _preferredTransform;
    float _preferredVolume;
}

- (void)dealloc
{
    [_languageCode release];
    [_extendedLanguageTag release];
    [super dealloc];
}

- (CMTimeScale)naturalTimeScale { return _naturalTimeScale; }
- (void)setNaturalTimeScale:(CMTimeScale)naturalTimeScale { _naturalTimeScale = naturalTimeScale; }
- (NSString *)languageCode { return [[_languageCode retain] autorelease]; }
- (void)setLanguageCode:(NSString *)languageCode
{
    if (_languageCode != languageCode) {
        [_languageCode release];
        _languageCode = [languageCode copy];
    }
}
- (NSString *)extendedLanguageTag { return [[_extendedLanguageTag retain] autorelease]; }
- (void)setExtendedLanguageTag:(NSString *)extendedLanguageTag
{
    if (_extendedLanguageTag != extendedLanguageTag) {
        [_extendedLanguageTag release];
        _extendedLanguageTag = [extendedLanguageTag copy];
    }
}
- (CGAffineTransform)preferredTransform { return _preferredTransform; }
- (void)setPreferredTransform:(CGAffineTransform)preferredTransform { _preferredTransform = preferredTransform; }
- (float)preferredVolume { return _preferredVolume; }
- (void)setPreferredVolume:(float)preferredVolume { _preferredVolume = preferredVolume; }

- (BOOL)insertTimeRange:(CMTimeRange)timeRange
                ofTrack:(AVAssetTrack *)track
                 atTime:(CMTime)startTime
                  error:(NSError **)outError
{
    if (outError)
        *outError = nil;
    if (!track) {
        if (outError) {
            *outError = [NSError errorWithDomain:AVFoundationErrorDomain
                                            code:-11800
                                        userInfo:@{ NSLocalizedDescriptionKey: @"Invalid source track" }];
        }
        return NO;
    }

    AVAssetTrackInternal *dst = [self _internal];
    if (CGSizeEqualToSize(dst->naturalSize, CGSizeZero))
        dst->naturalSize = [track naturalSize];
    if (dst->nominalFrameRate <= 0.0f)
        dst->nominalFrameRate = [track nominalFrameRate];
    if (!dst->formatDescriptions || [dst->formatDescriptions count] == 0)
        dst->formatDescriptions = [[track formatDescriptions] copy];

    CMTime insertStart = CMTIME_IS_VALID(startTime) ? startTime : kCMTimeZero;
    CMTime insertDuration = CMTIME_IS_VALID(timeRange.duration) ? timeRange.duration : [track timeRange].duration;
    if (!CMTIME_IS_VALID(insertDuration))
        insertDuration = kCMTimeZero;

    CMTime newEnd = CMTimeAdd(insertStart, insertDuration);
    if (!CMTIME_IS_VALID(dst->timeRange.start) || !CMTIME_IS_VALID(dst->timeRange.duration) ||
        CMTimeCompare(dst->timeRange.duration, kCMTimeZero) <= 0) {
        dst->timeRange = CMTimeRangeMake(insertStart, insertDuration);
    } else {
        CMTime oldStart = dst->timeRange.start;
        CMTime oldEnd = CMTimeRangeGetEnd(dst->timeRange);
        CMTime start = CMTimeCompare(insertStart, oldStart) < 0 ? insertStart : oldStart;
        CMTime end = CMTimeCompare(newEnd, oldEnd) > 0 ? newEnd : oldEnd;
        dst->timeRange = CMTimeRangeMake(start, CMTimeSubtract(end, start));
    }

    AVAsset *parent = dst->asset;
    if (parent) {
        AVAssetInternal *compInternal = [parent _internal];
        CMTime trackEnd = CMTimeRangeGetEnd(dst->timeRange);
        if (CMTimeCompare(trackEnd, compInternal->duration) > 0)
            compInternal->duration = trackEnd;
        if ([dst->mediaType isEqualToString:AVMediaTypeVideo] &&
            !CGSizeEqualToSize(dst->naturalSize, CGSizeZero) &&
            CGSizeEqualToSize(compInternal->naturalSize, CGSizeZero)) {
            compInternal->naturalSize = dst->naturalSize;
        }
        if (!compInternal->URL && [[track asset] _internal]->URL)
            compInternal->URL = [[[track asset] _internal]->URL retain];
    }

    return YES;
}

- (void)insertEmptyTimeRange:(CMTimeRange)timeRange
{
    AVAssetTrackInternal *dst = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTimeCompare(timeRange.duration, kCMTimeZero) > 0) {
        dst->timeRange.duration = CMTimeAdd(dst->timeRange.duration, timeRange.duration);
        AVAsset *parent = dst->asset;
        if (parent) {
            AVAssetInternal *compInternal = [parent _internal];
            CMTime trackEnd = CMTimeRangeGetEnd(dst->timeRange);
            if (CMTimeCompare(trackEnd, compInternal->duration) > 0)
                compInternal->duration = trackEnd;
        }
    }
}

- (void)removeTimeRange:(CMTimeRange)timeRange
{
    AVAssetTrackInternal *dst = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTimeCompare(timeRange.duration, kCMTimeZero) > 0) {
        CMTime sub = CMTimeSubtract(dst->timeRange.duration, timeRange.duration);
        dst->timeRange.duration = CMTimeCompare(sub, kCMTimeZero) > 0 ? sub : kCMTimeZero;
    }
}

- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration
{
    AVAssetTrackInternal *dst = [self _internal];
    if (CMTIME_IS_VALID(timeRange.duration) && CMTIME_IS_VALID(duration)) {
        CMTime diff = CMTimeSubtract(duration, timeRange.duration);
        CMTime updated = CMTimeAdd(dst->timeRange.duration, diff);
        dst->timeRange.duration = CMTimeCompare(updated, kCMTimeZero) > 0 ? updated : kCMTimeZero;
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
