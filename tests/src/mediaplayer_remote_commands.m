// CFLAGS: -framework Foundation -framework AppKit -framework MediaPlayer
// Copyright (C) 2026 Lubos Dolezel
//
// MediaPlayer remote commands and now playing info, as documented for
// MPRemoteCommandCenter, MPRemoteCommand and MPNowPlayingInfoCenter.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

@interface MPRemoteCommand (TestAccess)
- (void)invokeWithEvent:(MPRemoteCommandEvent *)event;
@end

static int deallocs = 0;
static int calls = 0;

@interface Target : NSObject
- (MPRemoteCommandHandlerStatus)handle:(MPRemoteCommandEvent *)event;
@end
@implementation Target
- (MPRemoteCommandHandlerStatus)handle:(MPRemoteCommandEvent *)event { calls++; return MPRemoteCommandHandlerStatusSuccess; }
- (void)dealloc { deallocs++; [super dealloc]; }
@end

int main(void)
{
	@autoreleasepool {
		MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
		check(center != nil);
		check([MPRemoteCommandCenter sharedCommandCenter] == center);

		// The playback position command has its documented subclass, and the
		// command objects are stable and distinct.
		MPRemoteCommand *position = [center changePlaybackPositionCommand];
		check([position isKindOfClass:NSClassFromString(@"MPChangePlaybackPositionCommand")]);
		check([position isKindOfClass:[MPRemoteCommand class]]);
		check([center changePlaybackPositionCommand] == position);
		check([center playCommand] != [center pauseCommand]);
		check([[center ratingCommand] isKindOfClass:NSClassFromString(@"MPRatingCommand")]);
		check([[center changeRepeatModeCommand] isKindOfClass:NSClassFromString(@"MPChangeRepeatModeCommand")]);
		check([[center changeShuffleModeCommand] isKindOfClass:NSClassFromString(@"MPChangeShuffleModeCommand")]);
		check([[center changePlaybackRateCommand] isKindOfClass:NSClassFromString(@"MPChangePlaybackRateCommand")]);
		check([[center likeCommand] isKindOfClass:NSClassFromString(@"MPFeedbackCommand")]);

		// Commands start enabled and can be switched off.
		check([[center playCommand] isEnabled]);
		[position setEnabled:NO];
		check(![[center changePlaybackPositionCommand] isEnabled]);
		check([[center playCommand] isEnabled]);
		[position setEnabled:YES];
		check([[center changePlaybackPositionCommand] isEnabled]);

		// Skip interval commands default to a single 10 second interval.
		check([[[center skipForwardCommand] preferredIntervals] isEqual:@[ @10 ]]);
		check(![[center likeCommand] isActive]);

		// Targets are not retained; several targets and handlers can be added.
		MPRemoteCommand *play = [center playCommand];
		Target *target = [[Target alloc] init];
		[play addTarget:target action:@selector(handle:)];
		__block int handlerCalls = 0;
		id token = [play addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			handlerCalls++;
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		check(token != nil);
		[play invokeWithEvent:nil];
		check(calls == 1 && handlerCalls == 1);
		[play removeTarget:token];
		[play invokeWithEvent:nil];
		check(calls == 2 && handlerCalls == 1);
		[play removeTarget:target action:@selector(handle:)];
		[play invokeWithEvent:nil];
		check(calls == 2);

		[play addTarget:target action:@selector(handle:)];
		[target release];
		check(deallocs == 1);
		[play removeTarget:nil];

		// Now playing info and artwork.
		MPNowPlayingInfoCenter *info = [MPNowPlayingInfoCenter defaultCenter];
		check([MPNowPlayingInfoCenter defaultCenter] == info);
		check([info nowPlayingInfo] == nil);
		check([info playbackState] == MPNowPlayingPlaybackStateUnknown);
		info.nowPlayingInfo = @{ MPMediaItemPropertyTitle: @"Song" };
		check([[info nowPlayingInfo][MPMediaItemPropertyTitle] isEqual:@"Song"]);
		info.playbackState = MPNowPlayingPlaybackStatePlaying;
		check([info playbackState] == MPNowPlayingPlaybackStatePlaying);
		check([MPMediaItemPropertyTitle isEqualToString:@"title"]);
		check([MPMediaItemPropertyAlbumTitle isEqualToString:@"albumTitle"]);
		check([MPMediaItemPropertyPlaybackDuration isEqualToString:@"playbackDuration"]);

		__block NSSize requested = NSZeroSize;
		MPMediaItemArtwork *art = [[MPMediaItemArtwork alloc] initWithBoundsSize:NSMakeSize(64, 32) requestHandler:^NSImage *(NSSize size) {
			requested = size;
			return nil;
		}];
		check([art bounds].size.width == 64 && [art bounds].size.height == 32);
		check(requested.width == 0);
		check([art imageWithSize:NSMakeSize(10, 10)] == nil);
		check(requested.width == 10);
		[art release];
	}
	return failures ? 1 : 0;
}
