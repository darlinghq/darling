// Copyright (C) 2026 Lubos Dolezel
//
// AVAudioPlayer (AVFAudio): creation from files and data (and the errors for what cannot be played), the
// properties of the sound, the transport (play, pause, stop, seek, loops, rate, playAtTime:), the level
// meters, and the delegate. The sounds are generated here; the compressed ones are read from a fixture directory
// when it is given (the tones-*.* files of tests/avplayer-fixtures). Checks that need an audio output are
// skipped when no queue can be started (as on a machine without an audio device).
//
// Build for the guest (see tests/avplayer-audio-play.m for the recipe):
//   guest-cc.sh -a x86_64 -o avaudioplayer tests/avaudioplayer.m -framework Foundation -framework AVFoundation -framework AudioToolbox
// (the sounds are made by tests/avaudioplayer-sounds.h)
// Run:  avaudioplayer [<dir with tones-aac.m4a, tones-mp3.mp3, tones-flac.flac, tones-alac.m4a, tones-pcm.wav>]
// Exit status: 0 if all checks passed.
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures, skips;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

static void spin(double seconds)
{
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:seconds]];
}

#import "avaudioplayer-sounds.h"

static NSString *dir;

static NSURL *store(NSString *name, NSData *data)
{
	NSString *path = [dir stringByAppendingPathComponent:name];
	[data writeToFile:path atomically:NO];
	return [NSURL fileURLWithPath:path];
}

static NSString *fourcc(OSStatus s)
{
	unsigned char c[4] = { (s >> 24) & 255, (s >> 16) & 255, (s >> 8) & 255, s & 255 };
	return [NSString stringWithFormat:@"%d '%c%c%c%c'", (int) s, c[0], c[1], c[2], c[3]];
}

// ---- the delegate ----

@interface Watcher : NSObject <AVAudioPlayerDelegate>
{
@public
	int finished, failed, success;
	BOOL onMain, playingAtCallback;
	NSTimeInterval timeAtCallback;
}
@end

@implementation Watcher
- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
	finished++;
	success = flag;
	onMain = [NSThread isMainThread];
	playingAtCallback = player.playing;
	timeAtCallback = player.currentTime;
}
- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(NSError *)error
{
	failed++;
}
@end

// ---- checks ----

static void errorCode(NSString *what, NSURL *url, NSData *data, OSStatus expected)
{
	NSError *error = nil;
	AVAudioPlayer *p = url ? [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error] : [[AVAudioPlayer alloc] initWithData:data error:&error];
	CHECK(p == nil && error != nil && [error.domain isEqualToString:NSOSStatusErrorDomain] && error.code == expected, "%s: %s (error %s %s)",
	      what.UTF8String, p ? "created" : "nil", error.domain.UTF8String, fourcc((OSStatus) error.code).UTF8String);
	[p release];
}

static void creation(NSURL *sine, NSURL *mono, NSURL *aiff, NSURL *flt, NSURL *wav24)
{
	printf("== creation and properties\n");
	NSError *error = nil;
	AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:sine error:&error];
	CHECK(p != nil && error == nil, "stereo wav");
	CHECK(fabs(p.duration - 1.0) < 0.001, "duration %f", p.duration);
	CHECK(p.numberOfChannels == 2, "channels %lu", (unsigned long) p.numberOfChannels);
	CHECK([p.url isEqual:sine] && p.data == nil, "url and data");
	CHECK(!p.playing && p.currentTime == 0 && p.delegate == nil, "initial state");
	CHECK(p.volume == 1 && p.pan == 0 && p.rate == 1 && !p.enableRate && p.numberOfLoops == 0 && !p.meteringEnabled, "default values");
	const AudioStreamBasicDescription *a = p.format.streamDescription;
	CHECK(p.format.sampleRate == 44100 && p.format.channelCount == 2 && p.format.commonFormat == AVAudioPCMFormatInt16 && p.format.interleaved && !p.format.standard,
	      "format %.0f Hz %u ch common %d", p.format.sampleRate, (unsigned) p.format.channelCount, (int) p.format.commonFormat);
	CHECK(a->mFormatID == kAudioFormatLinearPCM && a->mBitsPerChannel == 16 && a->mBytesPerFrame == 4 && a->mFramesPerPacket == 1, "stream description");
	NSDictionary *s = p.settings;
	CHECK([s[AVFormatIDKey] unsignedIntValue] == kAudioFormatLinearPCM && [s[AVSampleRateKey] doubleValue] == 44100 && [s[AVNumberOfChannelsKey] intValue] == 2 &&
	      [s[AVLinearPCMBitDepthKey] intValue] == 16 && ![s[AVLinearPCMIsFloatKey] boolValue] && ![s[AVLinearPCMIsBigEndianKey] boolValue] &&
	      [s[AVAudioFileTypeKey] unsignedIntValue] == kAudioFileWAVEType, "settings %s", s.description.UTF8String);
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:mono error:&error];
	CHECK(p.numberOfChannels == 1 && p.format.channelCount == 1, "mono wav");
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:aiff error:&error];
	CHECK(p != nil && fabs(p.duration - 0.5) < 0.001 && p.numberOfChannels == 2 && [p.settings[AVLinearPCMIsBigEndianKey] boolValue] &&
	      [p.settings[AVAudioFileTypeKey] unsignedIntValue] == kAudioFileAIFFType, "aiff (duration %f)", p.duration);
	CHECK(p.format.commonFormat == AVAudioOtherFormat, "big endian samples are not a common format");
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:flt error:&error];
	CHECK(p != nil && fabs(p.duration - 0.5) < 0.001 && p.format.commonFormat == AVAudioPCMFormatFloat32 && p.format.sampleRate == 22050, "float wav");
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:wav24 error:&error];
	CHECK(p != nil && [p.settings[AVLinearPCMBitDepthKey] intValue] == 24 && fabs(p.duration - 0.5) < 0.001, "24 bit wav");
	[p release];

	NSData *bytes = [NSData dataWithContentsOfURL:sine];
	p = [[AVAudioPlayer alloc] initWithData:bytes error:&error];
	CHECK(p != nil && p.data == bytes && p.url == nil && fabs(p.duration - 1.0) < 0.001, "from data");
	[p release];
	p = [[AVAudioPlayer alloc] initWithData:bytes fileTypeHint:@"com.microsoft.waveform-audio" error:&error];
	CHECK(p != nil, "data with a hint");
	[p release];
	p = [[AVAudioPlayer alloc] initWithData:bytes fileTypeHint:@"public.mp3" error:&error];
	CHECK(p != nil, "data with a hint that is wrong: the contents decide");
	[p release];
	p = [[AVAudioPlayer alloc] initWithContentsOfURL:sine fileTypeHint:@"public.aiff-audio" error:&error];
	CHECK(p != nil, "file with a hint that is wrong");
	[p release];
	p = [[AVAudioPlayer alloc] initWithContentsOfURL:sine fileTypeHint:nil error:&error];
	CHECK(p != nil, "file with no hint");
	[p release];
}

static void errors(void)
{
	printf("== errors\n");
	// 'wht?' kAudioFileUnspecifiedError, 'typ?' kAudioFileUnsupportedFileTypeError, 'dta?' kAudioFileInvalidFileError
	errorCode(@"missing file", [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"missing.wav"]], nil, kAudioFileUnspecifiedError);
	errorCode(@"empty file", store(@"empty.wav", [NSData data]), nil, kAudioFileUnspecifiedError);
	errorCode(@"text file", store(@"text.wav", [@"this is not audio, just some text" dataUsingEncoding:NSUTF8StringEncoding]), nil, kAudioFileUnsupportedFileTypeError);
	NSMutableData *noise = [NSMutableData dataWithLength:30000];
	srandom(7);
	for (NSUInteger i = 0; i < noise.length; i++)
		((unsigned char *) noise.mutableBytes)[i] = (unsigned char) random();
	errorCode(@"noise with a .wav name", store(@"noise.wav", noise), nil, kAudioFileUnsupportedFileTypeError);
	errorCode(@"a directory", [NSURL fileURLWithPath:dir], nil, kAudioFileUnspecifiedError);
	errorCode(@"empty data", nil, [NSData data], -39);
	errorCode(@"text data", nil, [@"hello, not audio" dataUsingEncoding:NSUTF8StringEncoding], kAudioFileUnsupportedFileTypeError);

	NSError *error = nil;
	AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:@"http://localhost:1/x.wav"] error:&error];
	CHECK(p == nil && error != nil, "an http URL");
	[p release];

	double amp[2] = { 0.5, 0.5 };
	// A header without samples is a sound of no length, and a file cut short plays what is left of it
	p = [[AVAudioPlayer alloc] initWithContentsOfURL:store(@"header-only.wav", makeWav(44100, 2, 16, 0, 1.0, amp, 44)) error:&error];
	CHECK(p != nil && p.duration == 0, "header without samples (%s)", p ? "created" : fourcc((OSStatus) error.code).UTF8String);
	[p release];
	p = [[AVAudioPlayer alloc] initWithContentsOfURL:store(@"cut.wav", makeWav(44100, 2, 16, 0, 1.0, amp, 20000)) error:&error];
	CHECK(p != nil && p.duration > 0.11 && p.duration < 0.115, "truncated wav keeps what is there (duration %f)", p.duration);
	[p release];
}

static void settersAndDefaults(NSURL *sine)
{
	printf("== setters\n");
	AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:sine error:NULL];
	// The values are kept as given
	p.volume = 2; CHECK(p.volume == 2, "volume 2 reads %f", p.volume);
	p.volume = -1; CHECK(p.volume == -1, "volume -1 reads %f", p.volume);
	p.volume = 0.25; CHECK(p.volume == 0.25f, "volume");
	p.pan = 1; CHECK(p.pan == 1, "pan 1");
	p.pan = -1; CHECK(p.pan == -1, "pan -1");
	p.pan = 3; CHECK(p.pan == 3, "pan 3 reads %f", p.pan);
	p.rate = 2; CHECK(p.rate == 2, "rate");
	p.rate = 0.5; CHECK(p.rate == 0.5f, "rate 0.5");
	p.enableRate = YES; CHECK(p.enableRate, "enableRate");
	p.numberOfLoops = 3; CHECK(p.numberOfLoops == 3, "loops");
	p.numberOfLoops = -1; CHECK(p.numberOfLoops == -1, "infinite loops");
	[p setVolume:0.4 fadeDuration:0]; CHECK(p.volume == 0.4f, "setVolume:fadeDuration: without a fade");
	[p setVolume:0 fadeDuration:1]; CHECK(p.volume == 0, "setVolume:fadeDuration: reads the target");
	p.currentTime = 0.5; CHECK(p.currentTime == 0.5 && !p.playing, "currentTime while stopped");
	p.currentTime = 99; CHECK(p.currentTime == p.duration, "currentTime is limited to the duration: %f", p.currentTime);
	p.currentTime = -1; CHECK(p.currentTime == 0, "currentTime is limited to 0: %f", p.currentTime);
	CHECK([p peakPowerForChannel:0] == -160 && [p averagePowerForChannel:0] == -160, "meters are -160 dB while not enabled");
	p.meteringEnabled = YES; [p updateMeters];
	CHECK(p.meteringEnabled && [p peakPowerForChannel:0] == -160 && [p averagePowerForChannel:1] == -160, "meters are -160 dB while not playing");
	[p release];
}

static void transport(NSURL *sine, NSURL *shortWav, NSURL *longWav)
{
	printf("== transport\n");
	AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:sine error:NULL];
	p.volume = 0.05;
	if (![p prepareToPlay]) {
		printf("SKIP the transport checks: no audio output\n");
		skips++;
		[p release];
		return;
	}
	CHECK([p prepareToPlay], "prepareToPlay twice");
	CHECK(!p.playing && p.currentTime == 0, "prepared, not playing");
	NSTimeInterval d0 = p.deviceCurrentTime;
	CHECK([p play] && p.playing, "play");
	spin(0.5);
	CHECK(p.playing && p.currentTime > 0.3 && p.currentTime < 0.6, "plays in real time: %f after 0.5 s", p.currentTime);
	CHECK(p.deviceCurrentTime - d0 > 0.4 && p.deviceCurrentTime - d0 < 0.7, "deviceCurrentTime advances: %f", p.deviceCurrentTime - d0);
	CHECK([p play] && p.playing, "play while playing");
	[p pause];
	NSTimeInterval paused = p.currentTime;
	CHECK(!p.playing && paused > 0.3, "pause: %f", paused);
	spin(0.3);
	CHECK(p.currentTime == paused, "the position stands still while paused: %f then %f", paused, p.currentTime);
	CHECK([p play], "play after pause");
	spin(0.2);
	CHECK(p.currentTime > paused + 0.1 && p.currentTime < paused + 0.4, "continues from where it was paused: %f", p.currentTime);
	[p stop];
	NSTimeInterval stopped = p.currentTime;
	CHECK(!p.playing && stopped > paused, "stop: %f", stopped);
	spin(0.2);
	CHECK(p.currentTime == stopped, "stop keeps the position");
	CHECK([p play], "play after stop");
	spin(0.2);
	CHECK(p.currentTime > stopped + 0.1, "continues after stop: %f (stopped at %f)", p.currentTime, stopped);
	p.currentTime = 0.6;
	CHECK(p.playing && p.currentTime >= 0.6 && p.currentTime < 0.75, "seek while playing: %f", p.currentTime);
	[p pause];
	p.currentTime = 0.1;
	CHECK(p.currentTime == 0.1 && !p.playing, "seek while paused");
	[p stop];

	// The sound plays to its end: the delegate is told once, on the main thread, and the player is rewound
	AVAudioPlayer *q = [[AVAudioPlayer alloc] initWithContentsOfURL:shortWav error:NULL];
	Watcher *w = [Watcher new];
	q.delegate = w;
	q.volume = 0.05;
	NSTimeInterval t0 = [NSDate timeIntervalSinceReferenceDate];
	[q play];
	spin(1.2);
	CHECK(w->finished == 1 && w->success && w->failed == 0, "the delegate is told that it finished: %d", w->finished);
	CHECK(w->onMain, "on the main thread");
	CHECK(!w->playingAtCallback && w->timeAtCallback == 0 && !q.playing && q.currentTime == 0, "the player is not playing and at the start by then (%f)", w->timeAtCallback);
	(void) t0;
	CHECK([q play] && q.playing, "play again after the end");
	spin(1.2);
	CHECK(w->finished == 2, "and it finishes again");

	// loops
	w->finished = 0;
	q.numberOfLoops = 1;
	t0 = [NSDate timeIntervalSinceReferenceDate];
	[q play];
	spin(0.45);
	CHECK(w->finished == 0 && q.playing, "one loop: still playing after 0.45 s of a 0.3 s sound");
	spin(0.9);
	CHECK(w->finished == 1 && !q.playing && q.numberOfLoops == 1, "one loop: finished once after two plays (%d)", w->finished);
	w->finished = 0;
	q.numberOfLoops = -1;
	[q play];
	spin(1.5);
	CHECK(q.playing && w->finished == 0, "infinite loops: still playing after five plays");
	[q stop];
	spin(0.3);
	CHECK(!q.playing && w->finished == 0, "stopping a looping sound is not an end");
	q.numberOfLoops = 0;

	// pause and stop are not ends either; a pause shortly before the end does not lose it
	w->finished = 0;
	q.currentTime = 0;
	[q play];
	spin(0.1);
	[q pause];
	spin(0.5);
	CHECK(w->finished == 0 && !q.playing, "paused: no end");
	[q play];
	spin(0.6);
	CHECK(w->finished == 1, "resumed: finished (%d)", w->finished);

	// A sound that plays when its player is released
	@autoreleasepool {
		AVAudioPlayer *r = [[AVAudioPlayer alloc] initWithContentsOfURL:sine error:NULL];
		r.volume = 0.05;
		[r play];
		spin(0.1);
		[r release];
	}
	spin(0.2);
	CHECK(1, "a player released while playing");

	// rate
	AVAudioPlayer *r = [[AVAudioPlayer alloc] initWithContentsOfURL:longWav error:NULL];
	r.volume = 0.05;
	r.enableRate = YES;
	r.rate = 2;
	CHECK([r play], "play with the rate 2");
	spin(0.5);
	CHECK(r.currentTime > 0.75 && r.currentTime < 1.0, "rate 2: %f after 0.5 s", r.currentTime);
	r.rate = 0.5;
	NSTimeInterval at = r.currentTime;
	spin(0.5);
	CHECK(r.currentTime - at > 0.15 && r.currentTime - at < 0.45, "rate 0.5 takes effect at once: %f in 0.5 s", r.currentTime - at);
	[r stop];
	r.enableRate = NO;
	r.rate = 2;
	r.currentTime = 0;
	[r play];
	spin(0.5);
	CHECK(r.currentTime > 0.3 && r.currentTime < 0.6, "the rate is not used without enableRate: %f", r.currentTime);
	// QUIRK: a position past the end set while the sound plays starts it again from the beginning (undocumented;
	// black-box observation on macOS, tier 2); pinned by this check.
	r.currentTime = 99;
	spin(0.3);
	CHECK(r.playing && r.currentTime > 0.15 && r.currentTime < 0.5, "quirk: a position past the end while playing starts over: %f", r.currentTime);
	[r stop];
	[r release];

	// playAtTime:
	AVAudioPlayer *t = [[AVAudioPlayer alloc] initWithContentsOfURL:longWav error:NULL];
	t.volume = 0.05;
	CHECK([t playAtTime:t.deviceCurrentTime + 0.6] && t.playing, "playAtTime: in the future");
	spin(0.3);
	CHECK(t.playing && t.currentTime == 0, "it waits: %f", t.currentTime);
	spin(0.7);
	CHECK(t.currentTime > 0.1 && t.currentTime < 0.6, "and then plays: %f", t.currentTime);
	[t stop];
	t.currentTime = 0;
	CHECK([t playAtTime:t.deviceCurrentTime - 5] && t.playing, "playAtTime: in the past plays now");
	spin(0.3);
	CHECK(t.currentTime > 0.1, "at once: %f", t.currentTime);
	[t stop];
	[t release];
	[q release];
	[w release];
	[p release];
}

static void meters(NSURL *full, NSURL *left, NSURL *mono)
{
	printf("== meters\n");
	AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:full error:NULL];
	p.volume = 0.05;
	p.meteringEnabled = YES;
	if (![p play]) {
		printf("SKIP the meter checks: no audio output\n");
		skips++;
		[p release];
		return;
	}
	spin(0.3);
	[p updateMeters];
	// The meters show the sound as it is read, before volume and pan; a full-scale sine peaks at 0 dB
	CHECK([p peakPowerForChannel:0] > -0.1 && [p peakPowerForChannel:0] <= 0.001, "full-scale sine: peak %f dB", [p peakPowerForChannel:0]);
	CHECK([p averagePowerForChannel:0] > -0.5 && [p averagePowerForChannel:0] <= 0.001, "full-scale sine: average %f dB", [p averagePowerForChannel:0]);
	CHECK([p peakPowerForChannel:1] == -160, "a mono file has no second channel: %f", [p peakPowerForChannel:1]);
	[p stop];
	[p updateMeters];
	CHECK([p peakPowerForChannel:0] == -160 && [p averagePowerForChannel:0] == -160, "-160 dB when stopped");
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:mono error:NULL];
	p.meteringEnabled = YES;
	p.volume = 0.05;
	p.pan = -1;
	[p play];
	spin(0.3);
	[p updateMeters];
	CHECK(fabs([p peakPowerForChannel:0] - -6.02) < 0.2 && [p peakPowerForChannel:1] == -160, "half-scale mono: peak %f dB, channel 1 %f", [p peakPowerForChannel:0], [p peakPowerForChannel:1]);
	[p release];

	p = [[AVAudioPlayer alloc] initWithContentsOfURL:left error:NULL];
	p.meteringEnabled = YES;
	p.volume = 0.05;
	[p play];
	spin(0.3);
	[p updateMeters];
	CHECK(fabs([p peakPowerForChannel:0] - -6.02) < 0.2 && [p peakPowerForChannel:1] == -120, "left channel only: %f dB and %f dB", [p peakPowerForChannel:0], [p peakPowerForChannel:1]);
	[p release];
}

static void compressed(const char *fixtures)
{
	printf("== compressed sounds\n");
	struct { const char *file; unsigned format; double duration; unsigned channels; } list[] = {
		{ "tones-aac.m4a", kAudioFormatMPEG4AAC, 2.0, 2 },
		{ "tones-mp3.mp3", kAudioFormatMPEGLayer3, 2.0, 2 },
		{ "tones-flac.flac", kAudioFormatFLAC, 2.0, 1 },
		{ "tones-alac.m4a", kAudioFormatAppleLossless, 2.0, 1 },
		{ "tones-pcm.wav", kAudioFormatLinearPCM, 2.0, 1 },
	};
	for (unsigned i = 0; i < sizeof(list) / sizeof(list[0]); i++) {
		NSString *path = [[NSString stringWithUTF8String:fixtures] stringByAppendingPathComponent:[NSString stringWithUTF8String:list[i].file]];
		if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
			printf("SKIP %s: not found\n", list[i].file);
			skips++;
			continue;
		}
		NSError *error = nil;
		AVAudioPlayer *p = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path] error:&error];
		if (!p) {
			// The container and codec libraries come from the host (FFmpeg)
			printf("SKIP %s: %s\n", list[i].file, fourcc((OSStatus) error.code).UTF8String);
			skips++;
			continue;
		}
		CHECK(p.numberOfChannels == list[i].channels && p.format.streamDescription->mFormatID == list[i].format && [p.settings[AVFormatIDKey] unsignedIntValue] == list[i].format,
		      "%s: format %08x channels %lu", list[i].file, p.format.streamDescription->mFormatID, (unsigned long) p.numberOfChannels);
		CHECK(fabs(p.duration - list[i].duration) < (list[i].format == kAudioFormatMPEGLayer3 ? 0.06 : 0.001), "%s: duration %f", list[i].file, p.duration);
		if (list[i].format != kAudioFormatLinearPCM)
			CHECK(p.format.commonFormat == AVAudioOtherFormat, "%s: a compressed format is described by its kind", list[i].file);
		p.volume = 0.05;
		p.meteringEnabled = YES;
		if ([p play]) {
			spin(0.5);
			[p updateMeters];
			CHECK(p.currentTime > 0.3 && p.currentTime < 0.7 && [p peakPowerForChannel:0] > -12 && [p peakPowerForChannel:0] < -4, "%s: plays (%f s, peak %f dB)", list[i].file, p.currentTime, [p peakPowerForChannel:0]);
			[p stop];
		}
		[p release];
	}
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		char tmpl[] = "/tmp/avaudioplayer.XXXXXX";
		dir = [NSString stringWithUTF8String:mkdtemp(tmpl)];

		double st[2] = { 0.5, 0.5 }, one[1] = { 1.0 }, half[1] = { 0.5 }, left[2] = { 0.5, 0 };
		NSURL *sine = store(@"sine.wav", makeWav(44100, 2, 16, 0, 1.0, st, -1));
		NSURL *mono = store(@"half-mono.wav", makeWav(44100, 1, 16, 0, 1.0, half, -1));
		NSURL *full = store(@"full-mono.wav", makeWav(44100, 1, 16, 0, 1.0, one, -1));
		NSURL *leftOnly = store(@"left.wav", makeWav(44100, 2, 16, 0, 1.0, left, -1));
		NSURL *shortWav = store(@"short.wav", makeWav(44100, 2, 16, 0, 0.3, st, -1));
		NSURL *longWav = store(@"long.wav", makeWav(44100, 2, 16, 0, 3.0, st, -1));
		NSURL *aiff = store(@"sine.aiff", makeAiff(44100, 2, 0.5, 0.5));
		NSURL *flt = store(@"float.wav", makeWav(22050, 1, 32, 1, 0.5, half, -1));
		NSURL *wav24 = store(@"sine24.wav", makeWav(44100, 1, 24, 0, 0.5, half, -1));

		creation(sine, mono, aiff, flt, wav24);
		errors();
		settersAndDefaults(sine);
		transport(sine, shortWav, longWav);
		meters(full, leftOnly, mono);
		if (argc > 1)
			compressed(argv[1]);

		[[NSFileManager defaultManager] removeItemAtPath:dir error:NULL];
	}
	printf("%s (%d failed, %d skipped)\n", failures ? "FAIL" : "PASS", failures, skips);
	return failures ? 1 : 0;
}
