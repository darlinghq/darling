// CFLAGS: -framework Foundation -framework AppKit -framework MediaPlayer
// Copyright (C) 2026 Lubos Dolezel
//
// MediaPlayer media item property keys MPMediaItemPropertyAlbumArtist, PlayCount and Rating.
// Applications read the key constants straight into a dictionary lookup, and a process that
// imports one that is missing hangs when it reads it. The values were observed on macOS 26.7.3.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

int main(void)
{
	@autoreleasepool {
		check([MPMediaItemPropertyAlbumArtist isEqualToString:@"albumArtist"]);
		check([MPMediaItemPropertyPlayCount isEqualToString:@"playCount"]);
		check([MPMediaItemPropertyRating isEqualToString:@"rating"]);
		NSDictionary *info = @{
			MPMediaItemPropertyTitle: @"t",
			MPMediaItemPropertyAlbumArtist: @"a",
			MPMediaItemPropertyPlayCount: @4,
			MPMediaItemPropertyRating: @5,
		};
		[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = info;
		check([[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo isEqualToDictionary:info]);
		[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
