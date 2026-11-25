// Copyright (C) 2026 Lubos Dolezel
//
// Plays a media file with AVPlayer for tests/avplayer-audio-sink.py, which records what reaches the
// audio output and compares it with the lines this program prints (wall-clock times, CLOCK_REALTIME, are
// shared with the recording host):
//   PLAY <time>             the moment the rate was set
//   FRAME <value> <time>    the video frame (the brightness of its center pixel) changed from dark to bright or back
//   ENDED <time>            AVPlayerItemDidPlayToEndTimeNotification
//   DONE <player time>
// Usage: avplayer-audio-play <file> [seconds=5 [rate=1 [volume=1 [muted=0 [seek-to=none [algorithm=default [rate-after=none [algorithm-after=none]]]]]]]]
//   algorithm: Spectral, TimeDomain, Varispeed or LowQualityZeroLatency (AVPlayerItem.audioTimePitchAlgorithm), set before the player plays;
//   rate-after, algorithm-after: a rate and an algorithm set 1.5 s after the start ("none" for no change).
//   guest-cc.sh -a x86_64 -o avplayer-audio-play tests/avplayer-audio-play.m -framework Foundation -framework AVFoundation -framework CoreMedia -framework CoreVideo
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double wallSeconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void run(double seconds, BOOL (^stop)(void))
{
	double end = wallSeconds() + seconds;
	while (wallSeconds() < end && !(stop && stop()))
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.002]];
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) {
		printf("usage: avplayer-audio-play <file> [seconds [rate [volume [muted [seek-to [algorithm [rate-after [algorithm-after]]]]]]]]\n");
		return 1;
	}
	@autoreleasepool {
		NSURL *url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:argv[1]]];
		double seconds = argc > 2 ? atof(argv[2]) : 5;
		float rate = argc > 3 ? (float)atof(argv[3]) : 1;
		float volume = argc > 4 ? (float)atof(argv[4]) : 1;
		BOOL muted = argc > 5 ? atoi(argv[5]) != 0 : NO;
		double seekTo = argc > 6 ? atof(argv[6]) : -1;
		NSString *algorithm = argc > 7 && strcmp(argv[7], "default") ? [NSString stringWithUTF8String:argv[7]] : nil;
		double rateAfter = argc > 8 && strcmp(argv[8], "none") ? atof(argv[8]) : -1;
		NSString *algorithmAfter = argc > 9 && strcmp(argv[9], "none") ? [NSString stringWithUTF8String:argv[9]] : nil;

		AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
		AVPlayerItem *item = [AVPlayerItem playerItemWithAsset:asset];
		AVPlayerItemVideoOutput *output = nil;
		if ([[asset tracksWithMediaType:AVMediaTypeVideo] count]) {
			output = [[[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:
				@{ (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) }] autorelease];
			[item addOutput:output];
		}
		if (algorithm)
			item.audioTimePitchAlgorithm = algorithm;
		AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
		player.volume = volume;
		player.muted = muted;
		__block double endedAt = 0;
		[[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item queue:nil
			usingBlock:^(NSNotification *n) { endedAt = wallSeconds(); printf("ENDED %.3f\n", endedAt); }];

		run(1.0, nil); // the player becomes ready, the first frame shows
		if (seekTo >= 0) {
			__block BOOL done = NO;
			[player seekToTime:CMTimeMakeWithSeconds(seekTo, 600) completionHandler:^(BOOL f) { done = YES; }];
			run(3.0, ^BOOL { return done; });
		}
		printf("PLAY %.3f\n", wallSeconds());
		player.rate = rate;
		const double playedAt = wallSeconds();
		__block BOOL changed = NO;
		__block int lastBright = -1;
		run(seconds, ^BOOL {
			if (!changed && wallSeconds() - playedAt >= 1.5) {
				changed = YES;
				if (algorithmAfter)
					item.audioTimePitchAlgorithm = algorithmAfter;
				if (rateAfter > 0)
					player.rate = (float)rateAfter;
				printf("CHANGED %.3f\n", wallSeconds());
			}
			if (output) {
				CVPixelBufferRef buffer = [output copyPixelBufferForItemTime:kCMTimeZero itemTimeForDisplay:NULL];
				if (buffer) {
					CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
					const uint8_t *p = (const uint8_t *)CVPixelBufferGetBaseAddress(buffer) + 24 * CVPixelBufferGetBytesPerRow(buffer) + 32 * 4;
					int value = p[0];
					CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
					CVPixelBufferRelease(buffer);
					if (lastBright >= 0 && (value > 128) != (lastBright > 128))
						printf("FRAME %d %.3f\n", value, wallSeconds());
					lastBright = value;
				}
			}
			return endedAt != 0;
		});
		run(1.0, nil); // what was queued plays out before the process exits
		printf("DONE %.3f\n", CMTimeGetSeconds([player currentTime]));
	}
	return 0;
}
