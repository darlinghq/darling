// Copyright (C) 2026 Darling Developers
//
// AVURLAsset / AVPlayerItem / AVPlayer / AVPlayerLayer playback through the documented AVFoundation
// interfaces only (no QTKit): the asset's metadata, the first frame, accurate seeks, playing to the end
// at the right speed, pausing, periodic time observers, the layer contents, and creating and releasing
// players while they play.
//
// The video fixtures are tests/avplayer-fixtures/redblue-h264.{mp4,mkv,ts}: 64x48, 10 fps, 1.2 s of H.264
// (High profile, B-frames, a keyframe every 6 frames): red for the first 0.6 s, blue for the rest.
// redblue-h264-aac.mp4 is the same video with an AAC track. The audio fixtures are
// tones-{aac.m4a,mp3.mp3,ac3.ac3,eac3.eac3,flac.flac,alac.m4a,opus.opus,pcm.wav}: 2 s of a 440 Hz tone
// for the first second and an 880 Hz tone for the second, and tones-vorbis.ogg, which has no Core Audio
// decoder. Audio goes to the default output device; this program checks the clock, the state and the
// notifications, and tests/avplayer-audio-sink.py checks what is heard.
// Pass the paths of one or more of them (the same file can be given from different places, for
// instance inside the prefix and under /Volumes/SystemRoot). Works for i386 and x86_64:
//   guest-cc.sh -a x86_64 -o avplayer-playback tests/avplayer-playback.m -framework Foundation -framework AVFoundation -framework QuartzCore -framework CoreMedia -framework CoreVideo
// Exits 0 if everything passed, 77 if the media cannot be opened at all (no host FFmpeg for that
// architecture), 1 on failure.
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static double nowSeconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Runs the main run loop until the condition holds or the time is up.
static BOOL spin(double timeout, BOOL (^condition)(void))
{
	double end = nowSeconds() + timeout;
	while (nowSeconds() < end) {
		if (condition && condition())
			return YES;
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
	}
	return condition ? condition() : YES;
}

// The center pixel of the newest frame the output has, as 0xRRGGBB; -1 if it has none.
static long centerPixel(AVPlayerItemVideoOutput *output, BOOL *gotNew)
{
	CVPixelBufferRef buffer = [output copyPixelBufferForItemTime:kCMTimeZero itemTimeForDisplay:NULL];
	if (gotNew)
		*gotNew = buffer != NULL;
	if (!buffer)
		return -1;
	__block long pixel = -1;
	if (CVPixelBufferGetPixelFormatType(buffer) == kCVPixelFormatType_32BGRA &&
	    CVPixelBufferGetWidth(buffer) == 64 && CVPixelBufferGetHeight(buffer) == 48 &&
	    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess) {
		const uint8_t *p = (const uint8_t *)CVPixelBufferGetBaseAddress(buffer) + 24 * CVPixelBufferGetBytesPerRow(buffer) + 32 * 4;
		pixel = (p[2] << 16) | (p[1] << 8) | p[0];
		CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
	}
	CVPixelBufferRelease(buffer);
	return pixel;
}

static BOOL isRed(long pixel) { return pixel >= 0 && ((pixel >> 16) & 0xff) > 180 && ((pixel >> 8) & 0xff) < 80 && (pixel & 0xff) < 80; }
static BOOL isBlue(long pixel) { return pixel >= 0 && ((pixel >> 16) & 0xff) < 80 && ((pixel >> 8) & 0xff) < 80 && (pixel & 0xff) > 180; }

static AVPlayerItemVideoOutput *attachOutput(AVPlayerItem *item)
{
	AVPlayerItemVideoOutput *output = [[[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:
		@{ (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) }] autorelease];
	[item addOutput:output];
	return output;
}

// Seeks and returns the color of the frame that is shown afterwards.
static long seekAndLook(AVPlayer *player, AVPlayerItemVideoOutput *output, double seconds, BOOL *finished)
{
	__block BOOL done = NO, ok = NO;
	[player seekToTime:CMTimeMakeWithSeconds(seconds, 600) completionHandler:^(BOOL f) { ok = f; done = YES; }];
	spin(3.0, ^BOOL { return done; });
	if (finished)
		*finished = ok;
	return centerPixel(output, NULL);
}

static void testAsset(NSString *path)
{
	NSURL *url = [NSURL fileURLWithPath:path];
	AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
	CHECK([asset isPlayable], "%s: the asset is playable", [path UTF8String]);
	CHECK([[asset URL] isEqual:url], "the asset keeps its URL");
	double duration = CMTimeGetSeconds([asset duration]);
	CHECK(duration > 1.1 && duration < 1.35, "duration is about 1.2 s (got %g)", duration);
	CHECK([asset naturalSize].width == 64 && [asset naturalSize].height == 48, "natural size is 64x48");

	NSArray *video = [asset tracksWithMediaType:AVMediaTypeVideo];
	CHECK([video count] == 1, "one video track (got %lu)", (unsigned long)[video count]);
	NSUInteger expectedAudio = [[path lastPathComponent] rangeOfString:@"-aac"].location != NSNotFound ? 1 : 0;
	CHECK([[asset tracksWithMediaType:AVMediaTypeAudio] count] == expectedAudio, "%lu audio track(s)", (unsigned long)expectedAudio);
	AVAssetTrack *track = [video firstObject];
	CHECK([track naturalSize].width == 64 && [track naturalSize].height == 48, "track natural size");
	CHECK([track nominalFrameRate] > 9.5 && [track nominalFrameRate] < 10.5, "track frame rate is 10 (got %g)", [track nominalFrameRate]);
	CHECK([track asset] == asset, "the track knows its asset");
	CHECK([asset trackWithTrackID:[track trackID]] == track, "trackWithTrackID: finds the track");
	CHECK([asset trackWithTrackID:12345] == nil, "trackWithTrackID: of an unknown ID is nil");

	// Class-cluster entry point.
	CHECK([[AVAsset assetWithURL:url] isKindOfClass:[AVURLAsset class]], "+[AVAsset assetWithURL:] makes a URL asset");

	// AVAsynchronousKeyValueLoading.
	__block BOOL loaded = NO;
	[asset loadValuesAsynchronouslyForKeys:@[@"duration", @"tracks", @"playable"] completionHandler:^{ loaded = YES; }];
	CHECK(spin(3.0, ^BOOL { return loaded; }), "the load completion handler runs");
	NSError *error = nil;
	CHECK([asset statusOfValueForKey:@"duration" error:&error] == AVKeyValueStatusLoaded, "duration is loaded");
	CHECK([asset statusOfValueForKey:@"noSuchProperty" error:&error] == AVKeyValueStatusFailed && error != nil, "an unknown key fails");
}

static void testBadAssets(NSString *directory)
{
	AVURLAsset *missing = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:@"/nonexistent/movie.mp4"] options:nil];
	CHECK(![missing isPlayable], "a missing file is not playable");
	CHECK(CMTimeGetSeconds([missing duration]) == 0, "a missing file has no duration");
	CHECK([[missing tracks] count] == 0, "a missing file has no tracks");

	NSString *junk = [directory stringByAppendingPathComponent:@"avplayer-junk.mp4"];
	[[NSData dataWithBytes:"this is not a movie, just some text that goes on for a while" length:58] writeToFile:junk atomically:NO];
	AVURLAsset *notMedia = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:junk] options:nil];
	CHECK(![notMedia isPlayable], "a text file is not playable");
	[[NSFileManager defaultManager] removeItemAtPath:junk error:NULL];

	CHECK(![[AVURLAsset URLAssetWithURL:[NSURL URLWithString:@"http://localhost:1/x.mp4"] options:nil] isPlayable],
	      "a network URL is not played (yet)");

	// A player for such an asset fails, a little later.
	AVPlayer *player = [AVPlayer playerWithURL:[NSURL fileURLWithPath:@"/nonexistent/movie.mp4"]];
	CHECK([player status] == AVPlayerStatusUnknown, "a new player is unknown until the item loaded");
	CHECK(spin(3.0, ^BOOL { return [player status] == AVPlayerStatusFailed; }), "a player of a missing file fails");
	CHECK([player error] != nil && [[player currentItem] status] == AVPlayerItemStatusFailed, "the player and the item report the error");
	[player play];
	[player seekToTime:kCMTimeZero];
	CHECK([player rate] == 0, "a failed player does not play");
	__block int result = -1;
	[player seekToTime:kCMTimeZero completionHandler:^(BOOL finished) { result = finished; }];
	CHECK(spin(2.0, ^BOOL { return result != -1; }) && result == 0, "a seek of a failed player completes unfinished");
}

static void testPlayerBasics(NSString *path)
{
	AVPlayerItem *item = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
	CHECK([item status] == AVPlayerItemStatusUnknown, "an item is unknown until a player took it");
	AVPlayerItemVideoOutput *output = attachOutput(item);
	CHECK([[item outputs] containsObject:output], "the output is added");
	AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
	CHECK([player currentItem] == item, "currentItem");
	CHECK([player rate] == 0, "a new player is paused");
	CHECK(spin(3.0, ^BOOL { return [player status] == AVPlayerStatusReadyToPlay; }), "the player becomes ready");
	CHECK([item status] == AVPlayerItemStatusReadyToPlay, "the item becomes ready");
	CHECK([player error] == nil, "no error");
	CHECK(CMTimeGetSeconds([item duration]) > 1.1, "item duration");
	CHECK([item presentationSize].width == 64, "item presentation size");

	// The first frame shows without playing.
	__block long pixel = -1;
	spin(3.0, ^BOOL { pixel = centerPixel(output, NULL); return pixel != -1; });
	CHECK(isRed(pixel), "the first frame is red (got %06lx)", pixel);
	BOOL gotNew = YES;
	centerPixel(output, &gotNew);
	CHECK(!gotNew, "the output has no new frame while nothing plays");
	CHECK(CMTimeGetSeconds([player currentTime]) == 0 && [player rate] == 0, "paused at 0");

	// Properties
	[player setVolume:0.5f];
	CHECK([player volume] == 0.5f, "volume");
	[player setMuted:YES];
	CHECK([player isMuted], "muted");

	// Accurate seeks: the frame shown is the one for the time, not the keyframe before it.
	BOOL finished = NO;
	CHECK(isBlue(seekAndLook(player, output, 0.9, &finished)) && finished, "seek to 0.9 s shows blue and finishes");
	double t = CMTimeGetSeconds([player currentTime]);
	CHECK(t > 0.89 && t < 0.91, "the time after seeking is the target (got %g)", t);
	CHECK(isRed(seekAndLook(player, output, 0.55, &finished)) && finished, "seek to 0.55 s shows red (the keyframe is at 0.0 s)");
	CHECK(isBlue(seekAndLook(player, output, 0.65, &finished)) && finished, "seek to 0.65 s shows blue");
	CHECK(isRed(seekAndLook(player, output, 0.0, &finished)) && finished, "seek to 0 shows red");
	CHECK(isBlue(seekAndLook(player, output, 5.0, &finished)) && finished, "seek past the end shows the last frame");
	t = CMTimeGetSeconds([player currentTime]);
	CHECK(t > 1.15 && t < 1.25, "seeking past the end stops at the duration (got %g)", t);
	CHECK(isRed(seekAndLook(player, output, -3.0, &finished)) && finished, "seek before the start shows the first frame");

	// Seeks that replace each other: the last one finishes, the replaced ones do not claim to have.
	__block int finishedCount = 0, unfinishedCount = 0, completions = 0;
	for (int i = 0; i < 6; i++) {
		double target = (i % 2) ? 0.9 : 0.1;
		[player seekToTime:CMTimeMakeWithSeconds(target, 600) completionHandler:^(BOOL f) {
			if (f) finishedCount++; else unfinishedCount++;
			completions++;
		}];
	}
	CHECK(spin(3.0, ^BOOL { return completions == 6; }), "every seek calls its completion handler (got %d)", completions);
	CHECK(finishedCount >= 1 && finishedCount + unfinishedCount == 6, "at least the last seek finished");
	__block long last = -1;
	spin(1.0, ^BOOL { last = centerPixel(output, NULL); return isBlue(last); });
	CHECK(isBlue(last), "after the last seek (0.9 s) the frame is blue (got %06lx)", last);
	// Item-level seeks go through the player too.
	__block BOOL itemSeek = NO;
	[item seekToTime:CMTimeMakeWithSeconds(0.2, 600) completionHandler:^(BOOL f) { itemSeek = f; }];
	CHECK(spin(3.0, ^BOOL { return itemSeek; }), "the item's seek finishes");
	CHECK(fabs(CMTimeGetSeconds([item currentTime]) - 0.2) < 0.01, "the item's time follows");
}

static void testPlayback(NSString *path)
{
	AVPlayerItem *item = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
	AVPlayerItemVideoOutput *output = attachOutput(item);
	AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
	spin(3.0, ^BOOL { return [player status] == AVPlayerStatusReadyToPlay; });

	__block BOOL ended = NO;
	__block double endedAt = 0;
	id token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; endedAt = nowSeconds(); }];
	__block int ticks = 0;
	__block double lastTick = -1;
	id observer = [player addPeriodicTimeObserverForInterval:CMTimeMake(1, 5) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
		ticks++;
		lastTick = CMTimeGetSeconds(time);
	}];

	double start = nowSeconds();
	[player play];
	CHECK([player rate] == 1.0f, "rate is 1 after play");

	// Colors while playing: red first, blue later.
	__block BOOL sawRed = NO, sawBlue = NO;
	__block double timeAtBlue = -1;
	spin(4.0, ^BOOL {
		long p = centerPixel(output, NULL);
		sawRed |= isRed(p);
		if (isBlue(p) && !sawBlue)
			timeAtBlue = CMTimeGetSeconds([player currentTime]);
		sawBlue |= isBlue(p);
		return ended;
	});
	// The frame follows the clock (which follows the audio, if there is any): blue starts at 0.6 s.
	CHECK(timeAtBlue > 0.55 && timeAtBlue < 0.75, "the first blue frame came when the clock read %g", timeAtBlue);
	double elapsed = nowSeconds() - start;
	CHECK(ended, "AVPlayerItemDidPlayToEndTimeNotification is posted");
	CHECK(elapsed > 1.0 && elapsed < 2.2, "playing takes about the duration (took %g s)", elapsed);
	CHECK(sawBlue, "blue frames were shown while playing");
	CHECK([player rate] == 0, "rate is 0 at the end");
	double t = CMTimeGetSeconds([player currentTime]);
	CHECK(t > 1.15 && t < 1.25, "the time at the end is the duration (got %g)", t);
	CHECK(ticks >= 3, "the periodic observer was called while playing (%d times)", ticks);
	CHECK(lastTick > 0.9, "the periodic observer saw the end (last %g)", lastTick);

	// Playing from the end does not start over by itself.
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
	CHECK([player rate] == 0, "play at the end stays paused");

	[player removeTimeObserver:observer];
	int before = ticks;
	BOOL finished;
	seekAndLook(player, output, 0.0, &finished);
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
	CHECK(ticks == before, "a removed time observer is not called again");
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// Pausing keeps the position.
	ended = NO;
	token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; }];
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.4]];
	[player pause];
	double paused = CMTimeGetSeconds([player currentTime]);
	CHECK(paused > 0.2 && paused < 0.7, "paused partway (at %g)", paused);
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.4]];
	CHECK(fabs(CMTimeGetSeconds([player currentTime]) - paused) < 0.02, "the time stands still while paused");
	CHECK(!ended, "no end while paused");
	[player play];
	CHECK(spin(3.0, ^BOOL { return ended; }), "resuming plays on to the end");
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// Twice the speed.
	seekAndLook(player, output, 0.0, &finished);
	ended = NO;
	token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; }];
	start = nowSeconds();
	[player setRate:2.0f];
	CHECK(spin(3.0, ^BOOL { return ended; }), "plays at rate 2 to the end");
	elapsed = nowSeconds() - start;
	CHECK(elapsed > 0.45 && elapsed < 1.1, "rate 2 takes half the time (took %g s)", elapsed);
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// A seek while playing continues to play from the new position.
	seekAndLook(player, output, 0.0, &finished);
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	seekAndLook(player, output, 0.8, &finished);
	CHECK([player rate] == 1.0f, "still playing after a seek");
	double after = CMTimeGetSeconds([player currentTime]);
	CHECK(after >= 0.8 && after < 1.2, "playing on from the seek target (at %g)", after);
	[player pause];
}

static void testLayer(NSString *path)
{
	AVPlayer *player = [AVPlayer playerWithURL:[NSURL fileURLWithPath:path]];
	AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:player];
	CHECK([layer isKindOfClass:[CALayer class]], "AVPlayerLayer is a CALayer");
	CHECK([layer player] == player, "the layer has the player");
	CHECK([[layer videoGravity] isEqualToString:AVLayerVideoGravityResizeAspect], "default gravity is aspect");
	CHECK(spin(3.0, ^BOOL { return [layer isReadyForDisplay] && [layer contents] != nil; }), "the layer gets the first frame");

	layer.videoGravity = AVLayerVideoGravityResize;
	CHECK([[layer videoGravity] isEqualToString:AVLayerVideoGravityResize], "gravity set");
	CHECK([[layer contentsGravity] isEqualToString:kCAGravityResize], "resize gravity maps to the contents gravity");
	layer.videoGravity = AVLayerVideoGravityResizeAspectFill;
	CHECK([[layer contentsGravity] isEqualToString:kCAGravityResizeAspectFill], "aspect fill maps to the contents gravity");

	// The frame follows seeks.
	id before = [layer contents];
	__block BOOL done = NO;
	[player seekToTime:CMTimeMakeWithSeconds(0.9, 600) completionHandler:^(BOOL f) { done = YES; }];
	spin(3.0, ^BOOL { return done && [layer contents] != before; });
	CHECK([layer contents] != before, "the layer's contents changed with the seek");

	// A layer added later gets the current frame at once.
	AVPlayerLayer *late = [[[AVPlayerLayer alloc] init] autorelease];
	late.player = player;
	CHECK([late contents] != nil, "a layer set to a playing player gets the current frame");
	late.player = nil;
	CHECK([late player] == nil, "player removed");

	// The presentation copy of a layer (made through -initWithLayer:) is an AVPlayerLayer too.
	AVPlayerLayer *copy = [[AVPlayerLayer alloc] initWithLayer:layer];
	CHECK([[copy videoGravity] isEqualToString:AVLayerVideoGravityResizeAspectFill], "a layer copy keeps the gravity");
	[copy release];

	// The layer can go while the player plays, and the player while the layer exists.
	[player play];
	layer = nil;
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
}

static void testLifetime(NSString *path)
{
	NSURL *url = [NSURL fileURLWithPath:path];
	for (int i = 0; i < 20; i++) {
		@autoreleasepool {
			AVPlayer *player = [[AVPlayer alloc] initWithURL:url];
			AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:player];
			id observer = [player addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:dispatch_get_main_queue() usingBlock:^(CMTime t) {}];
			(void)observer;
			[player play];
			if (i % 3 == 0)
				[player seekToTime:CMTimeMakeWithSeconds(0.5, 600)];
			if (i % 4 == 0)
				[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.02]];
			[player release]; // while it plays, with a layer, an observer and a seek
			(void)layer;
		}
	}
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];

	// Replacing the item of a playing player.
	AVPlayer *player = [AVPlayer playerWithURL:url];
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
	AVPlayerItem *other = [AVPlayerItem playerItemWithURL:url];
	[player replaceCurrentItemWithPlayerItem:other];
	CHECK([player currentItem] == other, "the item was replaced");
	CHECK([player rate] == 0, "the new item starts paused");
	CHECK(CMTimeGetSeconds([player currentTime]) == 0, "at the start");
	[player replaceCurrentItemWithPlayerItem:nil];
	CHECK([player currentItem] == nil, "the item was removed");
	[player play];
	CHECK([player rate] == 0, "a player without an item does not play");
	printf("lifetime ok\n");
}


#pragma mark - Audio

// Records the values of a key-value observed property.
@interface Recorder : NSObject {
@public
	NSMutableArray *values; // an explicit instance variable: the 32-bit runtime does not synthesize them
}
@end
@implementation Recorder
- (instancetype)init { if ((self = [super init])) values = [[NSMutableArray alloc] init]; return self; }
- (void)dealloc { [values release]; [super dealloc]; }
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	[values addObject:[object valueForKey:keyPath]];
}
@end

static void testAudioAsset(NSString *path)
{
	const char *name = [[path lastPathComponent] UTF8String];
	AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:path] options:nil];
	CHECK([asset isPlayable], "%s: an audio file is playable", name);
	double duration = CMTimeGetSeconds([asset duration]);
	CHECK(duration > 1.9 && duration < 2.1, "%s: duration is about 2 s (got %g)", name, duration);
	NSArray *audio = [asset tracksWithMediaType:AVMediaTypeAudio];
	CHECK([audio count] == 1, "%s: one audio track (got %lu)", name, (unsigned long)[audio count]);
	CHECK([[asset tracksWithMediaType:AVMediaTypeVideo] count] == 0, "%s: no video track", name);
	CHECK([asset naturalSize].width == 0 && [asset naturalSize].height == 0, "%s: no natural size", name);
	CHECK(CMTimeGetSeconds([[audio firstObject] timeRange].duration) > 1.9, "%s: the track lasts about 2 s", name);

	AVPlayerItem *item = [AVPlayerItem playerItemWithAsset:asset];
	AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
	CHECK(spin(3.0, ^BOOL { return [player status] == AVPlayerStatusReadyToPlay; }), "%s: the player becomes ready", name);
	CHECK([player rate] == 0 && [player timeControlStatus] == AVPlayerTimeControlStatusPaused, "%s: paused at the start", name);
	CHECK(CMTimeGetSeconds([player currentTime]) == 0, "%s: at 0", name);
}

// An audio-only file plays: the clock follows the audio, the end is announced, pausing, resuming, seeking
// and the rate work like they do with video.
static void testAudioPlayback(NSString *path)
{
	const char *name = [[path lastPathComponent] UTF8String];
	AVPlayerItem *item = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
	AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
	spin(3.0, ^BOOL { return [player status] == AVPlayerStatusReadyToPlay; });
	double duration = CMTimeGetSeconds([item duration]);

	Recorder *recorder = [[[Recorder alloc] init] autorelease];
	[player addObserver:recorder forKeyPath:@"timeControlStatus" options:0 context:NULL];
	__block BOOL ended = NO;
	__block double endedAt = 0;
	id token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; endedAt = nowSeconds(); }];

	double start = nowSeconds();
	[player play];
	CHECK([player rate] == 1.0f, "%s: rate is 1 after play", name);
	CHECK([player timeControlStatus] != AVPlayerTimeControlStatusPaused, "%s: playing or waiting to play", name);
	CHECK(spin(6.0, ^BOOL { return [player timeControlStatus] == AVPlayerTimeControlStatusPlaying; }), "%s: playback starts", name);
	CHECK([player reasonForWaitingToPlay] == nil, "%s: nothing to wait for while playing", name);
	CHECK(spin(duration + 6.0, ^BOOL { return ended; }), "%s: AVPlayerItemDidPlayToEndTimeNotification is posted", name);
	double elapsed = endedAt - start;
	// The clock follows what is heard and the audio takes a moment to start, so playing takes a little longer than
	// the media; the end can come a tenth of a second before the last of the audio is heard (the output latency).
	CHECK(elapsed > duration - 0.3 && elapsed < duration + 2.5, "%s: playing took %g s (duration %g)", name, elapsed, duration);
	CHECK([player rate] == 0 && [player timeControlStatus] == AVPlayerTimeControlStatusPaused, "%s: paused at the end", name);
	CHECK(fabs(CMTimeGetSeconds([player currentTime]) - duration) < 0.02, "%s: the time at the end is the duration (got %g)", name,
	      CMTimeGetSeconds([player currentTime]));
	CHECK([recorder->values count] >= 2 && [[recorder->values lastObject] integerValue] == AVPlayerTimeControlStatusPaused,
	      "%s: timeControlStatus is key-value observable (%lu changes)", name, (unsigned long)[recorder->values count]);
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
	CHECK([player rate] == 0, "%s: play at the end stays paused", name);
	[player removeObserver:recorder forKeyPath:@"timeControlStatus"];
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// Seeks finish, paused or playing, and past the end stop at the duration.
	__block int finished = -1;
	[player seekToTime:CMTimeMakeWithSeconds(1.3, 600) completionHandler:^(BOOL f) { finished = f; }];
	CHECK(spin(3.0, ^BOOL { return finished != -1; }) && finished == 1, "%s: a seek of a paused player finishes", name);
	CHECK(fabs(CMTimeGetSeconds([player currentTime]) - 1.3) < 0.02, "%s: the time is the seek target", name);
	finished = -1;
	[player seekToTime:CMTimeMakeWithSeconds(9.0, 600) completionHandler:^(BOOL f) { finished = f; }];
	CHECK(spin(3.0, ^BOOL { return finished != -1; }) && finished == 1, "%s: a seek past the end finishes", name);
	CHECK(fabs(CMTimeGetSeconds([player currentTime]) - duration) < 0.02, "%s: and stops at the duration", name);

	// Pausing keeps the position, resuming plays on to the end.
	finished = -1;
	[player seekToTime:kCMTimeZero completionHandler:^(BOOL f) { finished = f; }];
	spin(3.0, ^BOOL { return finished != -1; });
	ended = NO;
	token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; endedAt = nowSeconds(); }];
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.6]];
	[player pause];
	double paused = CMTimeGetSeconds([player currentTime]);
	CHECK(paused > 0.1 && paused < 0.7, "%s: paused partway (at %g)", name, paused);
	CHECK([player timeControlStatus] == AVPlayerTimeControlStatusPaused, "%s: paused", name);
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.4]];
	CHECK(fabs(CMTimeGetSeconds([player currentTime]) - paused) < 0.02, "%s: the time stands still while paused", name);
	CHECK(!ended, "%s: no end while paused", name);
	[player play];
	CHECK(spin(duration + 5.0, ^BOOL { return ended; }), "%s: resuming plays on to the end", name);
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// A seek while playing continues to play from the new position: 1.4 s is in the second tone.
	finished = -1;
	[player seekToTime:kCMTimeZero completionHandler:^(BOOL f) { finished = f; }];
	spin(3.0, ^BOOL { return finished != -1; });
	ended = NO;
	token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; endedAt = nowSeconds(); }];
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
	finished = -1;
	start = nowSeconds();
	[player seekToTime:CMTimeMakeWithSeconds(1.4, 600) completionHandler:^(BOOL f) { finished = f; }];
	CHECK(spin(3.0, ^BOOL { return finished != -1; }) && finished == 1, "%s: a seek of a playing player finishes", name);
	CHECK([player rate] == 1.0f, "%s: still playing after the seek", name);
	double after = CMTimeGetSeconds([player currentTime]);
	CHECK(after >= 1.39 && after < 1.9, "%s: playing on from the seek target (at %g)", name, after);
	CHECK(spin(4.0, ^BOOL { return ended; }), "%s: the end follows the seek", name);
	elapsed = endedAt - start;
	CHECK(elapsed > 0.4 && elapsed < 2.5, "%s: the rest takes about 0.6 s plus the start of the audio (took %g)", name, elapsed);
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// Twice the speed takes half the time.
	finished = -1;
	[player seekToTime:kCMTimeZero completionHandler:^(BOOL f) { finished = f; }];
	spin(3.0, ^BOOL { return finished != -1; });
	ended = NO;
	token = [[NSNotificationCenter defaultCenter] addObserverForName:AVPlayerItemDidPlayToEndTimeNotification object:item
		queue:nil usingBlock:^(NSNotification *n) { ended = YES; endedAt = nowSeconds(); }];
	start = nowSeconds();
	[player setRate:2.0f];
	CHECK([player rate] == 2.0f, "%s: the rate is 2", name);
	CHECK(spin(4.0, ^BOOL { return ended; }), "%s: plays at rate 2 to the end", name);
	elapsed = endedAt - start;
	CHECK(elapsed > duration / 2 - 0.1 && elapsed < duration / 2 + 2.0, "%s: rate 2 takes half the time (took %g s)", name, elapsed);
	[[NSNotificationCenter defaultCenter] removeObserver:token];

	// Volume and muting are properties of the player and take any value; changing them while playing is fine.
	[player setVolume:2.0f];
	CHECK([player volume] == 2.0f, "%s: volume above 1 is kept", name);
	[player setVolume:0.0f];
	CHECK([player volume] == 0.0f, "%s: volume 0", name);
	[player setVolume:0.25f];
	[player setMuted:YES];
	CHECK([player isMuted] && [player volume] == 0.25f, "%s: muting keeps the volume", name);
	finished = -1;
	[player seekToTime:kCMTimeZero completionHandler:^(BOOL f) { finished = f; }];
	spin(3.0, ^BOOL { return finished != -1; });
	[player setRate:1.0f];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	[player setMuted:NO];
	[player setVolume:1.0f];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	CHECK([player rate] == 1.0f, "%s: volume changes do not stop playback", name);
	[player pause];
}

// Players and their audio come and go while they play, are seeked and change rate.
static void testAudioLifetime(NSString *path)
{
	NSURL *url = [NSURL fileURLWithPath:path];
	for (int i = 0; i < 20; i++) {
		@autoreleasepool {
			AVPlayer *player = [[AVPlayer alloc] initWithURL:url];
			id observer = [player addPeriodicTimeObserverForInterval:CMTimeMake(1, 50) queue:dispatch_get_main_queue() usingBlock:^(CMTime t) {}];
			(void)observer;
			[player play];
			if (i % 2 == 0)
				[player setRate:2.0f];
			if (i % 3 == 0)
				[player seekToTime:CMTimeMakeWithSeconds(1.0, 600)];
			if (i % 4 == 0)
				[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
			if (i % 5 == 0) {
				[player setVolume:0.5f];
				[player pause];
				[player play];
			}
			[player release]; // while it plays
		}
	}
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];

	AVPlayer *player = [AVPlayer playerWithURL:url];
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	for (int i = 0; i < 10; i++) {
		[player setRate:(i % 2) ? 1.0f : 1.5f];
		[player seekToTime:CMTimeMakeWithSeconds(0.1 * i, 600)];
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.02]];
	}
	AVPlayerItem *other = [AVPlayerItem playerItemWithURL:url];
	[player replaceCurrentItemWithPlayerItem:other];
	CHECK([player rate] == 0, "the new item starts paused");
	[player play];
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	[player replaceCurrentItemWithPlayerItem:nil];
	CHECK([player currentItem] == nil, "the audio item was removed");
	printf("audio lifetime ok\n");
}

// Audio without a Core Audio decoder is not playable, whatever the container.
static void testUnplayableAudio(NSString *path)
{
	AVURLAsset *asset = [AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:path] options:nil];
	CHECK(![asset isPlayable], "%s is not playable", [[path lastPathComponent] UTF8String]);
	AVPlayer *player = [AVPlayer playerWithURL:[NSURL fileURLWithPath:path]];
	CHECK(spin(3.0, ^BOOL { return [player status] == AVPlayerStatusFailed; }), "a player of audio that cannot be decoded fails");
	[player play];
	CHECK([player rate] == 0, "and does not play");
}

// AVPlayerItem.audioTimePitchAlgorithm: Spectral by default, any string is kept as given, nil restores the default, the item
// keeps its setting when a player takes it. (The sound at another rate is checked by tests/avplayer-audio-sink.py.)
// The behaviour of strings that are no algorithm is a QUIRK observed on macOS 26.7.3 and 10.14.4 (see AVPlayerItem.m).
static void testAudioTimePitchAlgorithm(NSString *path)
{
	AVPlayerItem *item = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
	CHECK([item.audioTimePitchAlgorithm isEqualToString:AVAudioTimePitchAlgorithmSpectral], "audioTimePitchAlgorithm is Spectral by default (%s)", [item.audioTimePitchAlgorithm UTF8String]);
	CHECK([AVAudioTimePitchAlgorithmSpectral isEqualToString:@"Spectral"] && [AVAudioTimePitchAlgorithmTimeDomain isEqualToString:@"TimeDomain"] &&
	      [AVAudioTimePitchAlgorithmVarispeed isEqualToString:@"Varispeed"], "the algorithm constants");
	NSArray *names = @[AVAudioTimePitchAlgorithmTimeDomain, AVAudioTimePitchAlgorithmVarispeed, AVAudioTimePitchAlgorithmSpectral, @"LowQualityZeroLatency"];
	for (NSString *name in names) {
		item.audioTimePitchAlgorithm = name;
		CHECK([item.audioTimePitchAlgorithm isEqualToString:name], "audioTimePitchAlgorithm reads back %s", [name UTF8String]);
	}
	item.audioTimePitchAlgorithm = @"bogus";
	CHECK([item.audioTimePitchAlgorithm isEqualToString:@"bogus"], "an unknown name is kept");
	item.audioTimePitchAlgorithm = nil;
	CHECK([item.audioTimePitchAlgorithm isEqualToString:AVAudioTimePitchAlgorithmSpectral], "nil restores Spectral (%s)", [item.audioTimePitchAlgorithm UTF8String]);
	item.audioTimePitchAlgorithm = AVAudioTimePitchAlgorithmTimeDomain;
	AVPlayer *player = [AVPlayer playerWithPlayerItem:item];
	CHECK([item.audioTimePitchAlgorithm isEqualToString:AVAudioTimePitchAlgorithmTimeDomain], "the setting stays when a player takes the item");
	item.audioTimePitchAlgorithm = AVAudioTimePitchAlgorithmSpectral;
	CHECK([item.audioTimePitchAlgorithm isEqualToString:AVAudioTimePitchAlgorithmSpectral], "it can be changed with a player");
	AVPlayerItem *other = [AVPlayerItem playerItemWithURL:[NSURL fileURLWithPath:path]];
	[player replaceCurrentItemWithPlayerItem:other];
	CHECK([other.audioTimePitchAlgorithm isEqualToString:AVAudioTimePitchAlgorithmSpectral], "a new item has the default");
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) {
		printf("usage: avplayer-playback fixture...\n");
		return 1;
	}
	@autoreleasepool {
		NSString *first = [NSString stringWithUTF8String:argv[1]];
		if (![[AVURLAsset URLAssetWithURL:[NSURL fileURLWithPath:first] options:nil] isPlayable]) {
			printf("SKIP: %s cannot be opened (no host FFmpeg for this architecture?)\n", argv[1]);
			return 77;
		}
		for (int i = 1; i < argc; i++) {
			NSString *path = [NSString stringWithUTF8String:argv[i]];
			NSString *file = [path lastPathComponent];
			printf("== %s\n", argv[i]);
			if ([file hasPrefix:@"tones-vorbis"]) {
				testUnplayableAudio(path);
			} else if ([file hasPrefix:@"tones-"]) {
				testAudioAsset(path);
				testAudioPlayback(path);
				testAudioLifetime(path);
			} else {
				testAsset(path);
				testPlayerBasics(path);
				testPlayback(path);
				testLayer(path);
			}
		}
		testBadAssets(NSTemporaryDirectory());
		testLifetime(first);
		testAudioTimePitchAlgorithm(first);
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS: AVPlayer playback\n");
	return 0;
}
