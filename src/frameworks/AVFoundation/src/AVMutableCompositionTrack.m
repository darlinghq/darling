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
#import <AVFoundation/AVMutableCompositionTrack.h>
#import <AVFoundation/AVError.h>
#import <darling-stub-forwarding.h>

// A track is a list of segments in track time, each with a source range of some file (or empty),
// kept in order, without gaps and without two neighbours that could be one.

static CMTimeRange TargetOf(AVCompositionTrackSegment *segment) { return [segment timeMapping].target; }

static CMTime EndOf(NSArray *segments)
{
    AVCompositionTrackSegment *last = [segments lastObject];
    return last ? CMTimeRangeGetEnd(TargetOf(last)) : kCMTimeZero;
}

static AVCompositionTrackSegment *SegmentLike(AVCompositionTrackSegment *like, CMTimeRange source, CMTimeRange target)
{
    if ([like isEmpty])
        return [AVCompositionTrackSegment compositionTrackSegmentWithTimeRange:target];
    return [AVCompositionTrackSegment compositionTrackSegmentWithURL:[like sourceURL] trackID:[like sourceTrackID]
                                                    sourceTimeRange:source targetTimeRange:target];
}

static AVCompositionTrackSegment *Shifted(AVCompositionTrackSegment *segment, CMTime delta)
{
    CMTimeMapping mapping = [segment timeMapping];
    mapping.target.start = CMTimeAdd(mapping.target.start, delta);
    return SegmentLike(segment, mapping.source, mapping.target);
}

// Source time that corresponds to an offset into the segment's target; a segment played faster or
// slower than the source maps in proportion.
static CMTime SourceOffset(AVCompositionTrackSegment *segment, CMTime targetOffset)
{
    CMTimeMapping mapping = [segment timeMapping];
    if (CMTimeCompare(mapping.source.duration, mapping.target.duration) == 0 || CMTimeGetSeconds(mapping.target.duration) <= 0)
        return targetOffset;
    double ratio = CMTimeGetSeconds(mapping.source.duration) / CMTimeGetSeconds(mapping.target.duration);
    return CMTimeMakeWithSeconds(CMTimeGetSeconds(targetOffset) * ratio, mapping.source.duration.timescale);
}

// Makes a segment boundary at the track time, splitting the segment that spans it.
static void SplitAt(NSMutableArray *segments, CMTime time)
{
    for (NSUInteger i = 0; i < [segments count]; i++) {
        AVCompositionTrackSegment *segment = [segments objectAtIndex:i];
        CMTimeRange target = TargetOf(segment);
        if (CMTimeCompare(time, target.start) <= 0 || CMTimeCompare(time, CMTimeRangeGetEnd(target)) >= 0)
            continue;
        CMTime offset = CMTimeSubtract(time, target.start);
        CMTimeRange target1 = CMTimeRangeMake(target.start, offset);
        CMTimeRange target2 = CMTimeRangeMake(time, CMTimeSubtract(CMTimeRangeGetEnd(target), time));
        CMTimeRange source = [segment timeMapping].source, source1 = source, source2 = source;
        if (![segment isEmpty]) {
            CMTime sourceOffset = SourceOffset(segment, offset);
            source1 = CMTimeRangeMake(source.start, sourceOffset);
            source2 = CMTimeRangeMake(CMTimeAdd(source.start, sourceOffset), CMTimeSubtract(source.duration, sourceOffset));
        }
        [segments replaceObjectAtIndex:i withObject:SegmentLike(segment, source1, target1)];
        [segments insertObject:SegmentLike(segment, source2, target2) atIndex:i + 1];
        return;
    }
}

static BOOL SameSpeed(CMTimeMapping a, CMTimeMapping b)
{
    double ta = CMTimeGetSeconds(a.target.duration), tb = CMTimeGetSeconds(b.target.duration);
    if (ta <= 0 || tb <= 0)
        return NO;
    double diff = CMTimeGetSeconds(a.source.duration) / ta - CMTimeGetSeconds(b.source.duration) / tb;
    return diff < 1e-9 && diff > -1e-9;
}

// Joins neighbours that continue each other: two empty stretches, or two stretches of the same
// source that follow on in the source at the same speed as they do in the track.
static void Coalesce(NSMutableArray *segments)
{
    for (NSUInteger i = 0; i + 1 < [segments count];) {
        AVCompositionTrackSegment *a = [segments objectAtIndex:i], *b = [segments objectAtIndex:i + 1];
        CMTimeMapping ma = [a timeMapping], mb = [b timeMapping];
        BOOL join = NO;
        if ([a isEmpty] && [b isEmpty]) {
            join = YES;
        } else if (![a isEmpty] && ![b isEmpty] && [a sourceTrackID] == [b sourceTrackID]
                   && ([a sourceURL] == [b sourceURL] || [[a sourceURL] isEqual:[b sourceURL]])
                   && CMTimeCompare(CMTimeRangeGetEnd(ma.source), mb.source.start) == 0 && SameSpeed(ma, mb)) {
            join = YES;
        }
        if (!join) {
            i++;
            continue;
        }
        CMTimeRange target = CMTimeRangeMake(ma.target.start, CMTimeAdd(ma.target.duration, mb.target.duration));
        CMTimeRange source = CMTimeRangeMake(ma.source.start, CMTimeAdd(ma.source.duration, mb.source.duration));
        [segments replaceObjectAtIndex:i withObject:SegmentLike(a, source, target)];
        [segments removeObjectAtIndex:i + 1];
    }
}

// The segments of the source track that cover the range, with the range start at zero. A track of a
// file is one stretch of it; a composition track is cut out of its own segments.
static NSArray *SegmentsOfRange(AVAssetTrack *track, CMTimeRange range)
{
    NSMutableArray *result = [NSMutableArray array];
    if ([track isKindOfClass:[AVCompositionTrack class]]) {
        for (AVCompositionTrackSegment *segment in [(AVCompositionTrack *)track _segmentStorage]) {
            CMTimeRange target = TargetOf(segment);
            CMTime start = CMTimeMaximum(target.start, range.start);
            CMTime end = CMTimeMinimum(CMTimeRangeGetEnd(target), CMTimeRangeGetEnd(range));
            if (CMTimeCompare(end, start) <= 0)
                continue;
            CMTimeRange clipped = CMTimeRangeMake(CMTimeSubtract(start, range.start), CMTimeSubtract(end, start));
            CMTimeRange source = [segment timeMapping].source;
            if (![segment isEmpty]) {
                CMTime from = SourceOffset(segment, CMTimeSubtract(start, target.start));
                CMTime to = SourceOffset(segment, CMTimeSubtract(end, target.start));
                source = CMTimeRangeMake(CMTimeAdd(source.start, from), CMTimeSubtract(to, from));
            }
            [result addObject:SegmentLike(segment, source, clipped)];
        }
        CMTime covered = EndOf(result);
        if (CMTimeCompare(covered, range.duration) < 0) {
            [result addObject:[AVCompositionTrackSegment compositionTrackSegmentWithTimeRange:
                CMTimeRangeMake(covered, CMTimeSubtract(range.duration, covered))]];
        }
        return result;
    }

    NSURL *URL = [[track asset] _internal]->URL;
    [result addObject:[AVCompositionTrackSegment compositionTrackSegmentWithURL:URL trackID:[track trackID]
                                                                sourceTimeRange:range
                                                                targetTimeRange:CMTimeRangeMake(kCMTimeZero, range.duration)]];
    return result;
}

@implementation AVMutableCompositionTrack

- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange
            formatDescriptions:(NSArray *)formatDescriptions
{
    if ((self = [super _initWithAsset:asset trackID:trackID mediaType:mediaType naturalSize:naturalSize
                     nominalFrameRate:nominalFrameRate timeRange:timeRange formatDescriptions:formatDescriptions])) {
        _naturalTimeScale = 600;
        _preferredTransform = CGAffineTransformIdentity;
        _preferredVolume = 1.0f;
    }
    return self;
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

// The segments of a mutable track can be replaced as a whole; they are then taken as they are, in order.
- (void)setSegments:(NSArray<AVCompositionTrackSegment *> *)segments
{
    NSMutableArray *storage = [self _segmentStorage];
    [storage setArray:segments ?: @[]];
    [self _didEditSegments];
}

- (void)_didEditSegments
{
    Coalesce([self _segmentStorage]);
    [self _refreshTimeRange];
    AVAsset *parent = [self asset];
    if ([parent isKindOfClass:[AVComposition class]])
        [(AVComposition *)parent _refreshDuration];
}

// Puts the segments (in time from zero, lasting `length`) into the track at `time`, pushing what
// follows later. Insertion after the end of the track leaves an empty stretch in between.
- (void)_insertSegments:(NSArray *)inserted length:(CMTime)length atTime:(CMTime)time
{
    NSMutableArray *segments = [self _segmentStorage];
    CMTime end = EndOf(segments);
    if (CMTimeCompare(time, end) > 0)
        [segments addObject:[AVCompositionTrackSegment compositionTrackSegmentWithTimeRange:
            CMTimeRangeMake(end, CMTimeSubtract(time, end))]];
    SplitAt(segments, time);
    NSUInteger index = 0;
    while (index < [segments count] && CMTimeCompare(TargetOf([segments objectAtIndex:index]).start, time) < 0)
        index++;
    for (NSUInteger i = index; i < [segments count]; i++)
        [segments replaceObjectAtIndex:i withObject:Shifted([segments objectAtIndex:i], length)];
    for (AVCompositionTrackSegment *segment in inserted)
        [segments insertObject:Shifted(segment, time) atIndex:index++];
    [self _didEditSegments];
}

- (BOOL)insertTimeRange:(CMTimeRange)timeRange
                ofTrack:(AVAssetTrack *)track
                 atTime:(CMTime)startTime
                  error:(NSError **)outError
{
    if (outError)
        *outError = nil;
    if (!track || !CMTIME_IS_VALID(timeRange.start) || !CMTIME_IS_VALID(timeRange.duration)
        || CMTimeCompare(timeRange.duration, kCMTimeZero) < 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:AVFoundationErrorDomain
                                            code:-11800
                                        userInfo:@{ NSLocalizedDescriptionKey: @"The operation could not be completed" }];
        }
        return NO;
    }

    AVAssetTrackInternal *dst = [self _internal];
    if (CGSizeEqualToSize(dst->naturalSize, CGSizeZero))
        dst->naturalSize = [track naturalSize];
    if (dst->nominalFrameRate <= 0.0f)
        dst->nominalFrameRate = [track nominalFrameRate];
    if (!dst->formatDescriptions || [dst->formatDescriptions count] == 0) {
        [dst->formatDescriptions release];
        dst->formatDescriptions = [[track formatDescriptions] copy];
    }

    [self _insertSegments:SegmentsOfRange(track, timeRange) length:timeRange.duration
                   atTime:CMTIME_IS_VALID(startTime) ? startTime : kCMTimeZero];

    AVAsset *parent = dst->asset;
    if ([parent isKindOfClass:[AVComposition class]]) {
        AVAssetInternal *composition = [parent _internal];
        if ([dst->mediaType isEqualToString:AVMediaTypeVideo] && !CGSizeEqualToSize(dst->naturalSize, CGSizeZero)
            && CGSizeEqualToSize(composition->naturalSize, CGSizeZero))
            composition->naturalSize = dst->naturalSize;
    }
    return YES;
}

- (void)insertEmptyTimeRange:(CMTimeRange)timeRange
{
    // Nothing is inserted into a track that has no content.
    if ([[self _segmentStorage] count] == 0 || !CMTIME_IS_VALID(timeRange.start) || !CMTIME_IS_VALID(timeRange.duration)
        || CMTimeCompare(timeRange.duration, kCMTimeZero) <= 0)
        return;
    [self _insertSegments:@[ [AVCompositionTrackSegment compositionTrackSegmentWithTimeRange:
                                CMTimeRangeMake(kCMTimeZero, timeRange.duration)] ]
                   length:timeRange.duration atTime:timeRange.start];
}

- (void)removeTimeRange:(CMTimeRange)timeRange
{
    NSMutableArray *segments = [self _segmentStorage];
    if (!CMTIME_IS_VALID(timeRange.start) || !CMTIME_IS_VALID(timeRange.duration))
        return;
    CMTime start = CMTimeMaximum(timeRange.start, kCMTimeZero);
    CMTime end = CMTimeMinimum(CMTimeRangeGetEnd(timeRange), EndOf(segments));
    if (CMTimeCompare(end, start) <= 0)
        return;
    SplitAt(segments, start);
    SplitAt(segments, end);
    CMTime length = CMTimeSubtract(end, start);
    NSMutableArray *kept = [NSMutableArray array];
    for (AVCompositionTrackSegment *segment in segments) {
        CMTimeRange target = TargetOf(segment);
        if (CMTimeCompare(CMTimeRangeGetEnd(target), start) <= 0)
            [kept addObject:segment];
        else if (CMTimeCompare(target.start, end) >= 0)
            [kept addObject:Shifted(segment, CMTimeMultiply(length, -1))];
    }
    [segments setArray:kept];
    [self _didEditSegments];
}

- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration
{
    NSMutableArray *segments = [self _segmentStorage];
    if (!CMTIME_IS_VALID(timeRange.start) || !CMTIME_IS_VALID(timeRange.duration) || !CMTIME_IS_VALID(duration)
        || CMTimeCompare(duration, kCMTimeZero) < 0)
        return;
    CMTime start = CMTimeMaximum(timeRange.start, kCMTimeZero);
    CMTime end = CMTimeMinimum(CMTimeRangeGetEnd(timeRange), EndOf(segments));
    if (CMTimeCompare(end, start) <= 0)
        return;
    SplitAt(segments, start);
    SplitAt(segments, end);

    CMTime oldLength = CMTimeSubtract(end, start);
    double factor = CMTimeGetSeconds(duration) / CMTimeGetSeconds(oldLength);
    CMTime delta = CMTimeSubtract(duration, oldLength);
    NSMutableArray *result = [NSMutableArray array];
    for (AVCompositionTrackSegment *segment in segments) {
        CMTimeMapping mapping = [segment timeMapping];
        if (CMTimeCompare(CMTimeRangeGetEnd(mapping.target), start) <= 0) {
            [result addObject:segment];
        } else if (CMTimeCompare(mapping.target.start, end) >= 0) {
            [result addObject:Shifted(segment, delta)];
        } else {
            // Inside the range: the boundaries move in proportion, the last one onto the new end.
            CMTime newStart = CMTimeAdd(start, CMTimeMakeWithSeconds(CMTimeGetSeconds(CMTimeSubtract(mapping.target.start, start)) * factor, duration.timescale));
            CMTime newEnd = CMTimeCompare(CMTimeRangeGetEnd(mapping.target), end) == 0
                ? CMTimeAdd(start, duration)
                : CMTimeAdd(start, CMTimeMakeWithSeconds(CMTimeGetSeconds(CMTimeSubtract(CMTimeRangeGetEnd(mapping.target), start)) * factor, duration.timescale));
            if (CMTimeCompare(mapping.target.start, start) == 0)
                newStart = start;
            [result addObject:SegmentLike(segment, mapping.source, CMTimeRangeFromTimeToTime(newStart, newEnd))];
        }
    }
    [segments setArray:result];
    [self _didEditSegments];
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
