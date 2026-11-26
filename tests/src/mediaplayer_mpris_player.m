// CFLAGS: -framework Foundation -framework AppKit -framework MediaPlayer
// Copyright (C) 2026 Lubos Dolezel
//
// A player for the MPRIS session bus test (tests/mediaplayer-mpris-session-bus.py):
// it publishes now-playing info and registers remote command targets like a
// media player does, writes every remote command it receives to a log file,
// and runs until a stop file appears.
//
// usage: mediaplayer_mpris_player <dir> <title> <artist> [--commands-only] [--artwork]
//   <dir>/ready   is created once the info is published
//   <dir>/stop    ends the player
//   <dir>/log     gets one line per remote command, "<name>" or "<name> <value>"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static NSString *g_dir;
static int g_track = 1;
static NSString *g_artist;
static NSString *g_baseTitle;

static void logLine(NSString *line)
{
	NSString *path = [g_dir stringByAppendingPathComponent:@"log"];
	FILE *f = fopen([path fileSystemRepresentation], "a");
	if (f) {
		fprintf(f, "%s\n", [line UTF8String]);
		fclose(f);
	}
}

static void publish(double elapsed, MPMediaItemArtwork *artwork)
{
	NSMutableDictionary *info = [NSMutableDictionary dictionary];
	[info setObject:[NSString stringWithFormat:@"%@ %d", g_baseTitle, g_track] forKey:MPMediaItemPropertyTitle];
	[info setObject:g_artist forKey:MPMediaItemPropertyArtist];
	[info setObject:@"The Album" forKey:MPMediaItemPropertyAlbumTitle];
	[info setObject:[NSNumber numberWithDouble:240.0] forKey:MPMediaItemPropertyPlaybackDuration];
	[info setObject:[NSNumber numberWithDouble:elapsed] forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];
	[info setObject:[NSNumber numberWithDouble:1.0] forKey:MPNowPlayingInfoPropertyPlaybackRate];
	if (artwork)
		[info setObject:artwork forKey:MPMediaItemPropertyArtwork];
	[[MPNowPlayingInfoCenter defaultCenter] setNowPlayingInfo:info];
}

int main(int argc, const char *argv[])
{
	@autoreleasepool {
		if (argc < 4)
			return 2;
		g_dir = [NSString stringWithUTF8String:argv[1]];
		g_baseTitle = [NSString stringWithUTF8String:argv[2]];
		g_artist = [NSString stringWithUTF8String:argv[3]];
		BOOL commandsOnly = NO, withArtwork = NO;
		for (int i = 4; i < argc; i++) {
			if (strcmp(argv[i], "--commands-only") == 0) commandsOnly = YES;
			if (strcmp(argv[i], "--artwork") == 0) withArtwork = YES;
		}

		MPNowPlayingInfoCenter *info = [MPNowPlayingInfoCenter defaultCenter];
		MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];

		[center.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"play");
			[info setPlaybackState:MPNowPlayingPlaybackStatePlaying];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"pause");
			[info setPlaybackState:MPNowPlayingPlaybackStatePaused];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.togglePlayPauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"togglePlayPause");
			BOOL playing = [info playbackState] == MPNowPlayingPlaybackStatePlaying;
			[info setPlaybackState:playing ? MPNowPlayingPlaybackStatePaused : MPNowPlayingPlaybackStatePlaying];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.stopCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"stop");
			[info setPlaybackState:MPNowPlayingPlaybackStateStopped];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"nextTrack");
			g_track++;
			publish(0.0, nil);
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			logLine(@"previousTrack");
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		[center.changePlaybackPositionCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			if (![e isKindOfClass:[MPChangePlaybackPositionCommandEvent class]])
				return MPRemoteCommandHandlerStatusCommandFailed;
			double position = [(MPChangePlaybackPositionCommandEvent *)e positionTime];
			logLine([NSString stringWithFormat:@"changePlaybackPosition %.3f", position]);
			publish(position, nil);
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		center.skipForwardCommand.enabled = NO;
		center.skipBackwardCommand.enabled = NO;

		if (!commandsOnly) {
			MPMediaItemArtwork *artwork = nil;
			if (withArtwork) {
				NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
					pixelsWide:8 pixelsHigh:8 bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO
					colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:0 bitsPerPixel:0] autorelease];
				memset([rep bitmapData], 0x80, [rep bytesPerRow] * 8);
				NSImage *image = [[[NSImage alloc] initWithSize:NSMakeSize(8, 8)] autorelease];
				[image addRepresentation:rep];
				artwork = [[[MPMediaItemArtwork alloc] initWithImage:image] autorelease];
			}
			publish(15.0, artwork);
			[info setPlaybackState:MPNowPlayingPlaybackStatePlaying];
		}

		FILE *f = fopen([[g_dir stringByAppendingPathComponent:@"ready"] fileSystemRepresentation], "w");
		if (f) {
			fprintf(f, "ready\n");
			fclose(f);
		}

		NSString *stopPath = [g_dir stringByAppendingPathComponent:@"stop"];
		NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:120.0];
		while ([deadline timeIntervalSinceNow] > 0 && access([stopPath fileSystemRepresentation], F_OK) != 0)
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
	}
	return 0;
}
