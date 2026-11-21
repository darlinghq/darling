// CFLAGS: -framework Foundation -framework AppKit -framework MediaPlayer
// Copyright (C) 2026 Lubos Dolezel
//
// MediaPlayer media item property keys (MPMediaItem.h) and the repeat and
// shuffle mode remote command events (MPRemoteCommandEvent.h). Applications
// weak-link these on older systems and read the key constants straight into
// a dictionary lookup, so a missing constant takes the process down.
// The values were observed on macOS 26 and are the documented constant names.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#import <objc/runtime.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

static BOOL hasReadOnlyProperty(Class cls, const char *name)
{
	objc_property_t property = class_getProperty(cls, name);
	return property && strstr(property_getAttributes(property), ",R") != NULL;
}

int main(void)
{
	@autoreleasepool {
		check(&MPMediaItemPropertyAlbumTrackCount != NULL && MPMediaItemPropertyAlbumTrackCount != nil);
		check([MPMediaItemPropertyAlbumTrackCount isEqualToString:@"albumTrackCount"]);
		check([MPMediaItemPropertyComposer isEqualToString:@"composer"]);
		check([MPMediaItemPropertyDiscCount isEqualToString:@"discCount"]);
		check([MPMediaItemPropertyDiscNumber isEqualToString:@"discNumber"]);
		check([MPMediaItemPropertyGenre isEqualToString:@"genre"]);

		// The keys are usable in a now playing dictionary, next to the existing ones.
		NSDictionary *info = @{
			MPMediaItemPropertyTitle: @"t",
			MPMediaItemPropertyAlbumTrackCount: @12,
			MPMediaItemPropertyAlbumTrackNumber: @3,
			MPMediaItemPropertyDiscCount: @2,
			MPMediaItemPropertyDiscNumber: @1,
			MPMediaItemPropertyComposer: @"c",
			MPMediaItemPropertyGenre: @"g",
		};
		[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = info;
		check([[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo isEqualToDictionary:info]);
		[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;

		// Repeat and shuffle mode events are remote command events with read-only properties.
		Class repeatEvent = [MPChangeRepeatModeCommandEvent class];
		Class shuffleEvent = [MPChangeShuffleModeCommandEvent class];
		check(repeatEvent != Nil && class_getSuperclass(repeatEvent) == [MPRemoteCommandEvent class]);
		check(shuffleEvent != Nil && class_getSuperclass(shuffleEvent) == [MPRemoteCommandEvent class]);
		check(hasReadOnlyProperty(repeatEvent, "repeatType") && hasReadOnlyProperty(repeatEvent, "preservesRepeatMode"));
		check(hasReadOnlyProperty(shuffleEvent, "shuffleType") && hasReadOnlyProperty(shuffleEvent, "preservesShuffleMode"));
		// The events are created by the system, not by init, so only the defaults of a bare allocation are read.
		MPChangeRepeatModeCommandEvent *event = [repeatEvent alloc];
		check(event.repeatType == MPRepeatTypeOff && !event.preservesRepeatMode);
		MPChangeShuffleModeCommandEvent *shuffle = [shuffleEvent alloc];
		check(shuffle.shuffleType == MPShuffleTypeOff && !shuffle.preservesShuffleMode);
		check(MPRepeatTypeAll == 2 && MPShuffleTypeCollections == 2);
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
