// Copyright (C) 2026 Lubos Dolezel
//
// Plays a generated sound with AVAudioPlayer, following a script given as arguments, for
// tests/avaudioplayer-sink.py, which records what reaches the audio output and checks it.
//
//   avaudioplayer-play <sound> [<step>...]
//
// Sounds (generated, played from memory unless the name has the suffix "-file", which plays from a file):
//   sine      stereo 44.1 kHz, 440 Hz, amplitude 0.5, 1 s      long   the same, 3 s
//   left      the same with the right channel silent             right  the left channel silent
//   mono      one channel, 22.05 kHz, 440 Hz, amplitude 0.5, 1 s
//   tones     mono, 22.05 kHz, 440 Hz for one second and then 880 Hz for one second
//   short     stereo, 440 Hz, 0.4 s
// Steps:
//   play  pause  stop  prepare            the transport
//   wait:<seconds>                        runs the run loop (the delegate is called from it)
//   seek:<seconds>  vol:<v>  pan:<p>  loops:<n>  rate:<r> (also enables the rate)  fade:<v>:<seconds>
//   playat:<seconds from now>             playAtTime:
//   meter                                 updateMeters and prints the levels
// Printed: "PLAY <time>" when play (or playAtTime:) was called, "DONE <time> <success>" when the delegate was told that it
// finished, "TIME <currentTime>" after every wait, "METER ...", and the times are CLOCK_REALTIME in seconds.
//
//   guest-cc.sh -a x86_64 -o avaudioplayer-play tests/avaudioplayer-play.m -framework Foundation -framework AVFoundation
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#import "avaudioplayer-sounds.h"

static double wallSeconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

@interface Done : NSObject <AVAudioPlayerDelegate>
@end
@implementation Done
- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
	printf("DONE %.3f %d\n", wallSeconds(), (int) flag);
}
@end

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) {
		printf("usage: avaudioplayer-play <sound> [<step>...]\n");
		return 1;
	}
	@autoreleasepool {
		const double both[2] = { 0.5, 0.5 }, leftOnly[2] = { 0.5, 0 }, rightOnly[2] = { 0, 0.5 }, one[1] = { 0.5 };
		NSString *name = @(argv[1]);
		BOOL fromFile = [name hasSuffix:@"-file"];
		if (fromFile)
			name = [name substringToIndex:name.length - 5];

		NSData *sound = nil;
		if ([name isEqualToString:@"sine"]) sound = makeWav(44100, 2, 16, 0, 1.0, both, -1);
		else if ([name isEqualToString:@"long"]) sound = makeWav(44100, 2, 16, 0, 3.0, both, -1);
		else if ([name isEqualToString:@"left"]) sound = makeWav(44100, 2, 16, 0, 1.0, leftOnly, -1);
		else if ([name isEqualToString:@"right"]) sound = makeWav(44100, 2, 16, 0, 1.0, rightOnly, -1);
		else if ([name isEqualToString:@"mono"]) sound = makeWav(22050, 1, 16, 0, 1.0, one, -1);
		else if ([name isEqualToString:@"tones"]) sound = makeWavTones(22050, 1, 16, 0, 2.0, one, -1, 440, 880);
		else if ([name isEqualToString:@"short"]) sound = makeWav(44100, 2, 16, 0, 0.4, both, -1);
		else {
			printf("unknown sound %s\n", argv[1]);
			return 1;
		}

		NSError *error = nil;
		AVAudioPlayer *player;
		if (fromFile) {
			NSString *path = [NSString stringWithFormat:@"/tmp/avaudioplayer-play.%d.wav", (int) getpid()];
			[sound writeToFile:path atomically:NO];
			player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path] error:&error];
			unlink(path.UTF8String);
		} else
			player = [[AVAudioPlayer alloc] initWithData:sound error:&error];
		if (!player) {
			printf("cannot create the player: %ld\n", (long) error.code);
			return 1;
		}
		Done *done = [Done new];
		player.delegate = done;

		for (int i = 2; i < argc; i++) {
			const char *step = argv[i];
			if (!strcmp(step, "play")) {
				printf("PLAY %.3f\n", wallSeconds());
				if (![player play]) {
					printf("play failed\n");
					return 1;
				}
			} else if (!strcmp(step, "pause")) [player pause];
			else if (!strcmp(step, "stop")) [player stop];
			else if (!strcmp(step, "prepare")) [player prepareToPlay];
			else if (!strncmp(step, "wait:", 5)) {
				[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:atof(step + 5)]];
				printf("TIME %.3f\n", player.currentTime);
			} else if (!strncmp(step, "seek:", 5)) player.currentTime = atof(step + 5);
			else if (!strncmp(step, "vol:", 4)) player.volume = (float) atof(step + 4);
			else if (!strncmp(step, "pan:", 4)) player.pan = (float) atof(step + 4);
			else if (!strncmp(step, "loops:", 6)) player.numberOfLoops = atoi(step + 6);
			else if (!strncmp(step, "rate:", 5)) {
				player.enableRate = YES;
				player.rate = (float) atof(step + 5);
			} else if (!strncmp(step, "fade:", 5)) {
				float target = (float) atof(step + 5);
				const char *colon = strchr(step + 5, ':');
				[player setVolume:target fadeDuration:colon ? atof(colon + 1) : 0];
			} else if (!strncmp(step, "playat:", 7)) {
				printf("PLAY %.3f\n", wallSeconds());
				if (![player playAtTime:player.deviceCurrentTime + atof(step + 7)]) {
					printf("play failed\n");
					return 1;
				}
			} else if (!strcmp(step, "meter")) {
				player.meteringEnabled = YES;
				[player updateMeters];
				printf("METER %.2f %.2f\n", [player peakPowerForChannel:0], [player averagePowerForChannel:0]);
			} else {
				printf("unknown step %s\n", step);
				return 1;
			}
		}
		[player stop];
	}
	return 0;
}
