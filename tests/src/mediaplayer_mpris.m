// CFLAGS: -framework Foundation -framework MediaPlayer -framework AppKit
//
// Tests for MediaPlayer MPRIS integration: now-playing info, playback state,
// remote command handling, artwork generation, and in-process fallback.

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>
#include <unistd.h>

static int failures = 0;
#define CHECK(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s %d: %s\n", ok_ ? "PASS" : "FAIL", __LINE__, #cond); if (!ok_) failures++; } while (0)

static int play_invoked = 0;
static int pause_invoked = 0;
static int toggle_invoked = 0;
static double seek_target_pos = -1.0;

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        MPNowPlayingInfoCenter *infoCenter = [MPNowPlayingInfoCenter defaultCenter];
        CHECK(infoCenter != nil);
        CHECK([MPNowPlayingInfoCenter defaultCenter] == infoCenter);

        MPRemoteCommandCenter *cmdCenter = [MPRemoteCommandCenter sharedCommandCenter];
        CHECK(cmdCenter != nil);
        CHECK([MPRemoteCommandCenter sharedCommandCenter] == cmdCenter);

        // 1. In-process fallback: Add targets and verify handlers execute
        id playToken = [cmdCenter.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            play_invoked++;
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        CHECK(playToken != nil);

        id pauseToken = [cmdCenter.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            pause_invoked++;
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        CHECK(pauseToken != nil);

        id toggleToken = [cmdCenter.togglePlayPauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            toggle_invoked++;
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        CHECK(toggleToken != nil);

        id posToken = [cmdCenter.changePlaybackPositionCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            if ([event isKindOfClass:[MPChangePlaybackPositionCommandEvent class]]) {
                seek_target_pos = [(MPChangePlaybackPositionCommandEvent *)event positionTime];
                return MPRemoteCommandHandlerStatusSuccess;
            }
            return MPRemoteCommandHandlerStatusCommandFailed;
        }];
        CHECK(posToken != nil);

        // Direct in-process invocation
        MPRemoteCommandEvent *evt = [[[MPRemoteCommandEvent alloc] initWithCommand:cmdCenter.playCommand
                                                                         timestamp:100.0] autorelease];
        [cmdCenter.playCommand performSelector:@selector(invokeWithEvent:) withObject:evt];
        CHECK(play_invoked == 1);

        MPChangePlaybackPositionCommandEvent *posEvt = [[[MPChangePlaybackPositionCommandEvent alloc] initWithCommand:cmdCenter.changePlaybackPositionCommand
                                                                                                             timestamp:100.0
                                                                                                          positionTime:42.5] autorelease];
        [cmdCenter.changePlaybackPositionCommand performSelector:@selector(invokeWithEvent:) withObject:posEvt];
        CHECK(seek_target_pos >= 42.49 && seek_target_pos <= 42.51);

        // 2. Artwork test
        NSImage *testImg = [[[NSImage alloc] initWithSize:NSMakeSize(32, 32)] autorelease];
        MPMediaItemArtwork *art = [[[MPMediaItemArtwork alloc] initWithImage:testImg] autorelease];
        CHECK(art != nil);
        CHECK([art bounds].size.width == 32);

        // 3. Now Playing metadata dictionary
        NSDictionary *dict = @{
            MPMediaItemPropertyTitle: @"Test Track",
            MPMediaItemPropertyArtist: @"Test Artist",
            MPMediaItemPropertyAlbumTitle: @"Test Album",
            MPMediaItemPropertyComposer: @"Test Composer",
            MPMediaItemPropertyGenre: @"Classical",
            MPMediaItemPropertyAlbumTrackNumber: @(2),
            MPMediaItemPropertyAlbumTrackCount: @(12),
            MPMediaItemPropertyDiscNumber: @(1),
            MPMediaItemPropertyDiscCount: @(1),
            MPMediaItemPropertyPlaybackDuration: @(300.0),
            MPNowPlayingInfoPropertyElapsedPlaybackTime: @(15.0),
            MPNowPlayingInfoPropertyPlaybackRate: @(1.0),
            MPMediaItemPropertyArtwork: art
        };

        infoCenter.nowPlayingInfo = dict;
        CHECK(infoCenter.nowPlayingInfo != nil);
        CHECK([infoCenter.nowPlayingInfo[MPMediaItemPropertyTitle] isEqualToString:@"Test Track"]);
        CHECK([infoCenter.nowPlayingInfo[MPMediaItemPropertyArtist] isEqualToString:@"Test Artist"]);
        CHECK([infoCenter.nowPlayingInfo[MPMediaItemPropertyPlaybackDuration] doubleValue] == 300.0);

        infoCenter.playbackState = MPNowPlayingPlaybackStatePlaying;
        CHECK(infoCenter.playbackState == MPNowPlayingPlaybackStatePlaying);

        // Run the runloop briefly so async XPC messages are flushed
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];

        infoCenter.playbackState = MPNowPlayingPlaybackStatePaused;
        CHECK(infoCenter.playbackState == MPNowPlayingPlaybackStatePaused);
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];

        // Cleanup targets
        [cmdCenter.playCommand removeTarget:playToken];
        [cmdCenter.pauseCommand removeTarget:pauseToken];
        [cmdCenter.togglePlayPauseCommand removeTarget:toggleToken];
        [cmdCenter.changePlaybackPositionCommand removeTarget:posToken];
        infoCenter.nowPlayingInfo = nil;
        infoCenter.playbackState = MPNowPlayingPlaybackStateStopped;
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
    }

    printf("In-guest MediaPlayer tests complete. Failures: %d\n", failures);
    return failures ? 1 : 0;
}
