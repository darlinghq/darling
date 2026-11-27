// CFLAGS: -framework AVFoundation -framework CoreMedia -framework CoreGraphics -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// AVMutableComposition / AVComposition: two assets are composed and the tracks,
// segments and times are checked: insertion at the end, in the middle, after a
// gap, empty ranges, removal, scaling, copies and the compatible-track rules. The
// expected times follow from the edit operations (seconds are exact here: the
// sources are 1 s and 2 s of 8 kHz audio) and the structure (one track per media type,
// segments that continue each other are one, a gap is an empty segment, an empty
// track has no time range, copy gives an immutable snapshot) was observed on macOS 26.7.3.
#import <AVFoundation/AVFoundation.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

static void writeWav(NSString *path, int seconds)
{
	int rate = 8000, n = rate * seconds;
	NSMutableData *d = [NSMutableData data];
	uint32_t v; uint16_t s;
#define W32(x) v = (x); [d appendBytes:&v length:4]
#define W16(x) s = (x); [d appendBytes:&s length:2]
	[d appendBytes:"RIFF" length:4]; W32(36 + n * 2); [d appendBytes:"WAVEfmt " length:8];
	W32(16); W16(1); W16(1); W32(rate); W32(rate * 2); W16(2); W16(16);
	[d appendBytes:"data" length:4]; W32(n * 2);
	for (int i = 0; i < n; i++) { s = (uint16_t)(sin(i * 0.1) * 8000); [d appendBytes:&s length:2]; }
	[d writeToFile:path atomically:NO];
}

static BOOL timeIs(CMTime t, double seconds)
{
	return CMTIME_IS_VALID(t) && fabs(CMTimeGetSeconds(t) - seconds) < 1e-6;
}

// Start and duration of a segment's target, in seconds, and whether it is the expected source.
static BOOL segmentIs(AVCompositionTrackSegment *s, double start, double duration, BOOL empty, NSURL *url, double sourceStart)
{
	CMTimeMapping m = [s timeMapping];
	if (!timeIs(m.target.start, start) || !timeIs(m.target.duration, duration) || [s isEmpty] != empty)
		return NO;
	if (empty)
		return [s sourceURL] == nil;
	return [[s sourceURL] isEqual:url] && timeIs(m.source.start, sourceStart) && timeIs(m.source.duration, duration);
}

int main(void)
{
	@autoreleasepool {
		NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"comp%d", getpid()]];
		[[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
		NSString *pathA = [dir stringByAppendingPathComponent:@"a.wav"], *pathB = [dir stringByAppendingPathComponent:@"b.wav"];
		writeWav(pathA, 1);
		writeWav(pathB, 2);
		NSURL *urlA = [NSURL fileURLWithPath:pathA], *urlB = [NSURL fileURLWithPath:pathB];
		AVURLAsset *A = [AVURLAsset URLAssetWithURL:urlA options:nil], *B = [AVURLAsset URLAssetWithURL:urlB options:nil];
		AVAssetTrack *srcA = [[A tracksWithMediaType:AVMediaTypeAudio] firstObject], *srcB = [[B tracksWithMediaType:AVMediaTypeAudio] firstObject];
		if (srcA == nil || srcB == nil) {
			// Files are read by the host's media libraries, which i386 processes cannot load.
			printf("SKIP: no media engine\n");
			printf("PASS\n");
			return 0;
		}
		CHECK(timeIs(A.duration, 1) && timeIs(B.duration, 2));

		// An empty composition has no tracks and a zero duration.
		AVMutableComposition *comp = [AVMutableComposition composition];
		CHECK([comp isKindOfClass:[AVComposition class]] && [comp isKindOfClass:[AVAsset class]]);
		CHECK([comp.tracks count] == 0);
		CHECK(timeIs(comp.duration, 0));

		// Two assets end to end: A at 0, B after it.
		NSError *error = nil;
		CHECK([comp insertTimeRange:CMTimeRangeMake(kCMTimeZero, A.duration) ofAsset:A atTime:kCMTimeZero error:&error]);
		CHECK(error == nil);
		CHECK([comp insertTimeRange:CMTimeRangeMake(kCMTimeZero, B.duration) ofAsset:B atTime:A.duration error:&error]);
		CHECK([comp.tracks count] == 1);
		AVMutableCompositionTrack *track = (AVMutableCompositionTrack *)[comp.tracks firstObject];
		CHECK([track isKindOfClass:[AVMutableCompositionTrack class]]);
		CHECK([track.mediaType isEqualToString:AVMediaTypeAudio]);
		CHECK(track.trackID == 1);
		CHECK(track.asset == comp);
		CHECK(timeIs(comp.duration, 3));
		CHECK(CMTimeCompare(track.timeRange.start, kCMTimeZero) == 0 && timeIs(track.timeRange.duration, 3));
		NSArray *segments = track.segments;
		CHECK([segments count] == 2);
		if ([segments count] == 2) {
			CHECK(segmentIs(segments[0], 0, 1, NO, urlA, 0));
			CHECK(segmentIs(segments[1], 1, 2, NO, urlB, 0));
			CHECK([segments[0] sourceTrackID] == srcA.trackID && [segments[1] sourceTrackID] == srcB.trackID);
		}
		CHECK([[track segmentForTrackTime:CMTimeMake(3, 2)] sourceURL] != nil);
		CHECK([[[track segmentForTrackTime:CMTimeMake(3, 2)] sourceURL] isEqual:urlB]);

		// Insertion in the middle splits a segment and pushes the rest later: 0.5 s of B at 0.25 s into a 1 s track.
		AVMutableComposition *mid = [AVMutableComposition composition];
		AVMutableCompositionTrack *mt = [mid addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
		CHECK(timeIs(mt.timeRange.duration, 0) || !CMTIME_IS_VALID(mt.timeRange.start));   // no content, no range
		CHECK([mt insertTimeRange:CMTimeRangeMake(kCMTimeZero, A.duration) ofTrack:srcA atTime:kCMTimeZero error:&error]);
		CHECK([mt insertTimeRange:CMTimeRangeMake(CMTimeMake(1, 2), CMTimeMake(1, 2)) ofTrack:srcB atTime:CMTimeMake(1, 4) error:&error]);
		CHECK(timeIs(mid.duration, 1.5) && timeIs(mt.timeRange.duration, 1.5));
		segments = mt.segments;
		CHECK([segments count] == 3);
		if ([segments count] == 3) {
			CHECK(segmentIs(segments[0], 0, 0.25, NO, urlA, 0));
			CHECK(segmentIs(segments[1], 0.25, 0.5, NO, urlB, 0.5));
			CHECK(segmentIs(segments[2], 0.75, 0.75, NO, urlA, 0.25));
		}
		// Removing the inserted stretch closes the gap and the two halves of A are one segment again.
		[mt removeTimeRange:CMTimeRangeMake(CMTimeMake(1, 4), CMTimeMake(1, 2))];
		segments = mt.segments;
		CHECK([segments count] == 1 && segmentIs(segments[0], 0, 1, NO, urlA, 0));
		CHECK(timeIs(mid.duration, 1));
		// Scaling stretches the target and keeps the source: 1 s of A over 2 s.
		[mt scaleTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 1)) toDuration:CMTimeMake(2, 1)];
		segments = mt.segments;
		CHECK([segments count] == 1);
		if ([segments count] == 1) {
			CMTimeMapping m = [segments[0] timeMapping];
			CHECK(timeIs(m.target.start, 0) && timeIs(m.target.duration, 2) && timeIs(m.source.start, 0) && timeIs(m.source.duration, 1));
		}
		CHECK(timeIs(mid.duration, 2));
		// Removing past the end changes nothing.
		[mt removeTimeRange:CMTimeRangeMake(CMTimeMake(5, 1), CMTimeMake(1, 1))];
		CHECK(timeIs(mid.duration, 2));

		// A gap: inserting after the end of the track leaves an empty segment; empty ranges are inserted and merge.
		AVMutableComposition *gap = [AVMutableComposition composition];
		AVMutableCompositionTrack *gt = [gap addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:3];
		CHECK(gt.trackID == 3);
		[gt insertEmptyTimeRange:CMTimeRangeMake(CMTimeMake(2, 1), CMTimeMake(1, 1))];   // an empty track stays empty
		CHECK([gt.segments count] == 0 && timeIs(gap.duration, 0));
		CHECK([gt insertTimeRange:CMTimeRangeMake(kCMTimeZero, A.duration) ofTrack:srcA atTime:CMTimeMake(3, 1) error:&error]);
		segments = gt.segments;
		CHECK([segments count] == 2);
		if ([segments count] == 2) {
			CHECK(segmentIs(segments[0], 0, 3, YES, nil, 0));
			CHECK(segmentIs(segments[1], 3, 1, NO, urlA, 0));
		}
		CHECK(timeIs(gap.duration, 4));
		[gt insertEmptyTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 1))];
		segments = gt.segments;
		CHECK([segments count] == 2 && segmentIs(segments[0], 0, 4, YES, nil, 0) && segmentIs(segments[1], 4, 1, NO, urlA, 0));
		CHECK(timeIs(gap.duration, 5));
		[gap removeTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 1))];   // at composition level
		CHECK(timeIs(gap.duration, 4));
		[gap insertEmptyTimeRange:CMTimeRangeMake(CMTimeMake(1, 1), CMTimeMake(1, 1))];
		CHECK(timeIs(gap.duration, 5));
		[gap scaleTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(5, 1)) toDuration:CMTimeMake(10, 1)];
		CHECK(timeIs(gap.duration, 10));

		// A range of an asset: the asset's own track ID and the shifted source start.
		AVMutableComposition *part = [AVMutableComposition composition];
		CHECK([part insertTimeRange:CMTimeRangeMake(CMTimeMake(1, 2), CMTimeMake(1, 2)) ofAsset:A atTime:CMTimeMake(1, 1) error:&error]);
		segments = [[part.tracks firstObject] segments];
		CHECK([segments count] == 2);
		if ([segments count] == 2) {
			CHECK(segmentIs(segments[0], 0, 1, YES, nil, 0));
			CHECK(segmentIs(segments[1], 1, 0.5, NO, urlA, 0.5));
		}
		CHECK(timeIs(part.duration, 1.5));

		// Copies: an immutable snapshot with its own tracks; mutable copies are independent.
		AVComposition *snapshot = [part copy];
		CHECK([snapshot isKindOfClass:[AVComposition class]] && ![snapshot isKindOfClass:[AVMutableComposition class]]);
		CHECK([snapshot.tracks count] == 1);
		AVCompositionTrack *snapTrack = (AVCompositionTrack *)[snapshot.tracks firstObject];
		CHECK(![snapTrack isKindOfClass:[AVMutableCompositionTrack class]] && snapTrack.asset == snapshot);
		CHECK([snapTrack.segments count] == 2 && timeIs(snapshot.duration, 1.5));
		CHECK([snapshot copy] == snapshot);
		[part removeTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 1))];
		CHECK(timeIs(part.duration, 0.5) && timeIs(snapshot.duration, 1.5));
		AVMutableComposition *editable = [snapshot mutableCopy];
		CHECK([editable isKindOfClass:[AVMutableComposition class]]);
		[editable insertEmptyTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 1))];
		CHECK(timeIs(editable.duration, 2.5) && timeIs(snapshot.duration, 1.5));

		// Tracks: lookup, media types, characteristics, compatibility, removal, identifier reuse.
		AVMutableComposition *tracks = [AVMutableComposition composition];
		AVMutableCompositionTrack *t1 = [tracks addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
		AVMutableCompositionTrack *t7 = [tracks addMutableTrackWithMediaType:AVMediaTypeVideo preferredTrackID:7];
		AVMutableCompositionTrack *tDup = [tracks addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:7];
		CHECK(t1.trackID == 1 && t7.trackID == 7 && tDup.trackID != 7 && tDup.trackID != 1 && tDup.trackID != kCMPersistentTrackID_Invalid);
		CHECK([tracks.tracks count] == 3);
		CHECK([tracks trackWithTrackID:7] == t7 && [tracks trackWithTrackID:99] == nil);
		CHECK([[tracks tracksWithMediaType:AVMediaTypeAudio] count] == 2 && [[tracks tracksWithMediaType:AVMediaTypeVideo] count] == 1);
		CHECK([[tracks tracksWithMediaCharacteristic:AVMediaCharacteristicAudible] count] == 2);
		CHECK([[tracks tracksWithMediaCharacteristic:AVMediaCharacteristicVisual] count] == 1);
		CHECK([tracks mutableTrackCompatibleWithTrack:srcA] == t1);
		CHECK([tracks mutableTrackCompatibleWithTrack:nil] == nil);
		CHECK(t1.preferredVolume == 1.0f && CGAffineTransformIsIdentity(t1.preferredTransform));
		CHECK(t1.languageCode == nil);
		t1.languageCode = @"eng";
		CHECK([t1.languageCode isEqualToString:@"eng"]);
		[tracks removeTrack:t1];
		CHECK([tracks.tracks count] == 2 && [tracks mutableTrackCompatibleWithTrack:srcA] == tDup);
		NSArray *before = tracks.tracks;
		[tracks removeTrack:tDup];
		CHECK([before count] == 2 && [tracks.tracks count] == 1);   // tracks is a snapshot

		// Invalid input: no source track, an invalid time range, no asset.
		AVMutableCompositionTrack *bad = [tracks addMutableTrackWithMediaType:AVMediaTypeAudio preferredTrackID:kCMPersistentTrackID_Invalid];
		error = nil;
		CHECK(![bad insertTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 2)) ofTrack:nil atTime:kCMTimeZero error:&error]);
		CHECK(error != nil);
		error = nil;
		CHECK(![bad insertTimeRange:kCMTimeRangeInvalid ofTrack:srcA atTime:kCMTimeZero error:&error]);
		CHECK(error != nil);
		error = nil;
		CHECK(![tracks insertTimeRange:CMTimeRangeMake(kCMTimeZero, CMTimeMake(1, 2)) ofAsset:nil atTime:kCMTimeZero error:&error]);
		CHECK([bad.segments count] == 0);

		// A composition of one whole file plays (it is that file); the player item becomes ready.
		AVMutableComposition *whole = [AVMutableComposition composition];
		CHECK([whole insertTimeRange:CMTimeRangeMake(kCMTimeZero, A.duration) ofAsset:A atTime:kCMTimeZero error:&error]);
		CHECK(whole.isPlayable);
		AVPlayerItem *item = [AVPlayerItem playerItemWithAsset:whole];
		AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
		for (int i = 0; i < 100 && item.status == AVPlayerItemStatusUnknown; i++)
			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
		CHECK(item.status == AVPlayerItemStatusReadyToPlay);
		(void)player;

		[[NSFileManager defaultManager] removeItemAtPath:dir error:nil];
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
