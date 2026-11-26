// CFLAGS: -framework Foundation -framework AppKit -framework MediaPlayer
// Copyright (C) 2026 Lubos Dolezel
//
// MPNowPlayingInfoCenter, MPRemoteCommandCenter and the remote command events
// as documented for MediaPlayer, without the desktop: the now-playing service
// is pointed at a name that nobody serves, so everything runs in-process, as
// it does where the service or the host's session bus are not there.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>
#include <stdlib.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

// The private entry points that deliver a remote command and build events.
@interface MPRemoteCommand (TestAccess)
- (MPRemoteCommandHandlerStatus)invokeWithEvent:(MPRemoteCommandEvent *)event;
@end

@interface MPRemoteCommandEvent (TestAccess)
- (instancetype)initWithCommand:(MPRemoteCommand *)command timestamp:(NSTimeInterval)timestamp;
@end

@interface MPChangePlaybackPositionCommandEvent (TestAccess)
- (instancetype)initWithCommand:(MPRemoteCommand *)command timestamp:(NSTimeInterval)timestamp positionTime:(NSTimeInterval)positionTime;
@end

@interface MPSkipIntervalCommandEvent (TestAccess)
- (instancetype)initWithCommand:(MPRemoteCommand *)command timestamp:(NSTimeInterval)timestamp interval:(NSTimeInterval)interval;
@end

// A target whose action answers with a fixed status, as documented for
// addTarget:action: (the action returns an MPRemoteCommandHandlerStatus).
@interface StatusTarget : NSObject {
@public
	MPRemoteCommandHandlerStatus status;
	int calls;
}
- (MPRemoteCommandHandlerStatus)handle:(MPRemoteCommandEvent *)event;
@end

@implementation StatusTarget
- (MPRemoteCommandHandlerStatus)handle:(MPRemoteCommandEvent *)event { calls++; return status; }
@end

int main(void)
{
	// Nobody serves this name: no connection to a now-playing service
	setenv("__MEDIAREMOTED_SERVICE_NAME", "org.darling.test.no-such-mediaremoted", 1);

	@autoreleasepool {
		MPNowPlayingInfoCenter *info = [MPNowPlayingInfoCenter defaultCenter];
		MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];

		// The centers are singletons
		check(info != nil && [MPNowPlayingInfoCenter defaultCenter] == info);
		check(center != nil && [MPRemoteCommandCenter sharedCommandCenter] == center);

		// Nothing is playing to begin with
		check([info nowPlayingInfo] == nil);
		check([info playbackState] == MPNowPlayingPlaybackStateUnknown);

		// nowPlayingInfo keeps what it was given, as a copy; nil clears it
		NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithObjectsAndKeys:
			@"Track", MPMediaItemPropertyTitle,
			@"Artist", MPMediaItemPropertyArtist,
			[NSNumber numberWithDouble:240.0], MPMediaItemPropertyPlaybackDuration,
			[NSNumber numberWithDouble:12.5], MPNowPlayingInfoPropertyElapsedPlaybackTime,
			[NSNumber numberWithDouble:1.0], MPNowPlayingInfoPropertyPlaybackRate,
			nil];
		[info setNowPlayingInfo:dict];
		[dict setObject:@"Changed later" forKey:MPMediaItemPropertyTitle];
		check([[[info nowPlayingInfo] objectForKey:MPMediaItemPropertyTitle] isEqualToString:@"Track"]);
		check([[[info nowPlayingInfo] objectForKey:MPNowPlayingInfoPropertyElapsedPlaybackTime] doubleValue] == 12.5);
		[info setNowPlayingInfo:nil];
		check([info nowPlayingInfo] == nil);

		// Every playback state can be set and read back
		MPNowPlayingPlaybackState states[] = {
			MPNowPlayingPlaybackStatePlaying, MPNowPlayingPlaybackStatePaused,
			MPNowPlayingPlaybackStateInterrupted, MPNowPlayingPlaybackStateStopped,
			MPNowPlayingPlaybackStateUnknown,
		};
		for (int i = 0; i < 5; i++) {
			[info setPlaybackState:states[i]];
			check([info playbackState] == states[i]);
		}

		// The info center can be used from other threads
		dispatch_sync(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[info setNowPlayingInfo:@{ MPMediaItemPropertyTitle: @"From a queue" }];
			[info setPlaybackState:MPNowPlayingPlaybackStatePlaying];
		});
		check([[[info nowPlayingInfo] objectForKey:MPMediaItemPropertyTitle] isEqualToString:@"From a queue"]);
		check([info playbackState] == MPNowPlayingPlaybackStatePlaying);
		[info setNowPlayingInfo:nil];
		[info setPlaybackState:MPNowPlayingPlaybackStateStopped];

		// A command without targets has nothing to do; one with a target
		// answers with the status of the target
		MPRemoteCommand *play = [center playCommand];
		MPRemoteCommandEvent *event = [[[MPRemoteCommandEvent alloc] initWithCommand:play timestamp:5.0] autorelease];
		check([event command] == play);
		check([event timestamp] == 5.0);
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusNoActionableNowPlayingItem);

		__block int handlerCalls = 0;
		__block MPRemoteCommandHandlerStatus handlerStatus = MPRemoteCommandHandlerStatusSuccess;
		id token = [play addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			handlerCalls++;
			return handlerStatus;
		}];
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusSuccess && handlerCalls == 1);
		handlerStatus = MPRemoteCommandHandlerStatusNoSuchContent;
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusNoSuchContent && handlerCalls == 2);

		// A disabled command does not run its targets
		[play setEnabled:NO];
		check(![play isEnabled]);
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusCommandFailed && handlerCalls == 2);
		[play setEnabled:YES];
		check([play isEnabled]);

		// The status returned by the action of a target is used, and a
		// success of any target makes the command a success
		StatusTarget *failing = [[[StatusTarget alloc] init] autorelease];
		failing->status = MPRemoteCommandHandlerStatusCommandFailed;
		[play removeTarget:token];
		[play addTarget:failing action:@selector(handle:)];
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusCommandFailed && failing->calls == 1);
		StatusTarget *succeeding = [[[StatusTarget alloc] init] autorelease];
		succeeding->status = MPRemoteCommandHandlerStatusSuccess;
		[play addTarget:succeeding action:@selector(handle:)];
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusSuccess && failing->calls == 2 && succeeding->calls == 1);
		[play removeTarget:nil];
		check([play invokeWithEvent:event] == MPRemoteCommandHandlerStatusNoActionableNowPlayingItem);

		// The events that carry a value
		MPChangePlaybackPositionCommand *positionCommand = [center changePlaybackPositionCommand];
		MPChangePlaybackPositionCommandEvent *positionEvent = [[[MPChangePlaybackPositionCommandEvent alloc]
			initWithCommand:positionCommand timestamp:1.0 positionTime:42.5] autorelease];
		check([positionEvent positionTime] == 42.5 && [positionEvent command] == positionCommand);
		check([positionEvent isKindOfClass:[MPRemoteCommandEvent class]]);

		MPSkipIntervalCommand *skipCommand = [center skipForwardCommand];
		MPSkipIntervalCommandEvent *skipEvent = [[[MPSkipIntervalCommandEvent alloc]
			initWithCommand:skipCommand timestamp:2.0 interval:15.0] autorelease];
		check([skipEvent interval] == 15.0 && [skipEvent command] == skipCommand);
		check([skipEvent isKindOfClass:[MPRemoteCommandEvent class]]);

		__block double receivedPosition = -1;
		__block double receivedInterval = -1;
		id positionToken = [positionCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			if (![e isKindOfClass:[MPChangePlaybackPositionCommandEvent class]])
				return MPRemoteCommandHandlerStatusCommandFailed;
			receivedPosition = [(MPChangePlaybackPositionCommandEvent *)e positionTime];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		id skipToken = [skipCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) {
			if (![e isKindOfClass:[MPSkipIntervalCommandEvent class]])
				return MPRemoteCommandHandlerStatusCommandFailed;
			receivedInterval = [(MPSkipIntervalCommandEvent *)e interval];
			return MPRemoteCommandHandlerStatusSuccess;
		}];
		check([positionCommand invokeWithEvent:positionEvent] == MPRemoteCommandHandlerStatusSuccess && receivedPosition == 42.5);
		check([skipCommand invokeWithEvent:skipEvent] == MPRemoteCommandHandlerStatusSuccess && receivedInterval == 15.0);
		[positionCommand removeTarget:positionToken];
		[skipCommand removeTarget:skipToken];

		// Artwork asks its request handler for the size that is wanted
		__block NSSize askedSize = NSZeroSize;
		MPMediaItemArtwork *artwork = [[[MPMediaItemArtwork alloc] initWithBoundsSize:NSMakeSize(64, 64)
			requestHandler:^NSImage *(NSSize size) {
				askedSize = size;
				return [[[NSImage alloc] initWithSize:size] autorelease];
			}] autorelease];
		check([artwork bounds].size.width == 64 && [artwork bounds].size.height == 64);
		NSImage *image = [artwork imageWithSize:NSMakeSize(16, 16)];
		check(image != nil && askedSize.width == 16 && askedSize.height == 16);

		// Artwork, state and command changes with no service around do not block or crash
		[info setNowPlayingInfo:@{ MPMediaItemPropertyTitle: @"With artwork", MPMediaItemPropertyArtwork: artwork }];
		[info setPlaybackState:MPNowPlayingPlaybackStatePlaying];
		[play addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *e) { return MPRemoteCommandHandlerStatusSuccess; }];
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
		check([[[info nowPlayingInfo] objectForKey:MPMediaItemPropertyTitle] isEqualToString:@"With artwork"]);
		[info setNowPlayingInfo:nil];
		[info setPlaybackState:MPNowPlayingPlaybackStateStopped];
	}

	printf("Failures: %d\n", failures);
	return failures ? 1 : 0;
}
