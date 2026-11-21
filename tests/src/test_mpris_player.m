#import <Foundation/Foundation.h>
#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>
#include <unistd.h>

@interface PlayerApp : NSObject
@property (nonatomic, assign) BOOL playReceived;
@property (nonatomic, assign) BOOL pauseReceived;
@property (nonatomic, assign) NSTimeInterval seekPositionReceived;
@end

@implementation PlayerApp

- (instancetype)init
{
    self = [super init];
    if (self) {
        _seekPositionReceived = -1.0;
        
        MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
        [center.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            NSLog(@"[PlayerApp] Play command received!");
            self.playReceived = YES;
            [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePlaying;
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        
        [center.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            NSLog(@"[PlayerApp] Pause command received!");
            self.pauseReceived = YES;
            [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePaused;
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        
        [center.togglePlayPauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            NSLog(@"[PlayerApp] TogglePlayPause command received!");
            if ([MPNowPlayingInfoCenter defaultCenter].playbackState == MPNowPlayingPlaybackStatePlaying) {
                self.pauseReceived = YES;
                [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePaused;
            } else {
                self.playReceived = YES;
                [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePlaying;
            }
            return MPRemoteCommandHandlerStatusSuccess;
        }];
        
        [center.changePlaybackPositionCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent *event) {
            if ([event isKindOfClass:[MPChangePlaybackPositionCommandEvent class]]) {
                MPChangePlaybackPositionCommandEvent *posEvent = (MPChangePlaybackPositionCommandEvent *)event;
                NSLog(@"[PlayerApp] ChangePlaybackPosition to %f received!", posEvent.positionTime);
                self.seekPositionReceived = posEvent.positionTime;
                
                NSMutableDictionary *info = [[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo mutableCopy];
                if (!info) info = [NSMutableDictionary dictionary];
                info[MPNowPlayingInfoPropertyElapsedPlaybackTime] = @(posEvent.positionTime);
                [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = info;
                return MPRemoteCommandHandlerStatusSuccess;
            }
            return MPRemoteCommandHandlerStatusCommandFailed;
        }];
    }
    return self;
}

- (void)startPlayingWithTitle:(NSString *)title artist:(NSString *)artist
{
    MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
    NSDictionary *info = @{
        MPMediaItemPropertyTitle: title ? title : @"Test Track",
        MPMediaItemPropertyArtist: artist ? artist : @"Test Artist",
        MPMediaItemPropertyAlbumTitle: @"Test Album",
        MPMediaItemPropertyPlaybackDuration: @(240.0),
        MPNowPlayingInfoPropertyElapsedPlaybackTime: @(15.0),
        MPNowPlayingInfoPropertyPlaybackRate: @(1.0)
    };
    center.nowPlayingInfo = info;
    center.playbackState = MPNowPlayingPlaybackStatePlaying;
}

@end

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        const char *readyFile = argc > 1 ? argv[1] : "/tmp/player_ready.txt";
        const char *stopFile = argc > 2 ? argv[2] : "/tmp/player_stop.txt";
        const char *titleStr = argc > 3 ? argv[3] : "Test Track";
        const char *artistStr = argc > 4 ? argv[4] : "Test Artist";
        const char *pauseTrigger = argc > 5 ? argv[5] : NULL;
        const char *playTrigger = argc > 6 ? argv[6] : NULL;

        PlayerApp *app = [[PlayerApp alloc] init];
        [app startPlayingWithTitle:[NSString stringWithUTF8String:titleStr]
                            artist:[NSString stringWithUTF8String:artistStr]];
        
        // Signal ready by creating file
        FILE *f = fopen(readyFile, "w");
        if (f) {
            fprintf(f, "READY pid=%d\n", getpid());
            fclose(f);
        }
        
        // Run loop for up to 30 seconds, or until signal file created
        NSDate *deadline = [NSDate dateWithTimeIntervalSinceNow:30.0];
        while ([deadline timeIntervalSinceNow] > 0) {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            if (access(stopFile, F_OK) == 0) {
                break;
            }
            if (pauseTrigger && access(pauseTrigger, F_OK) == 0) {
                unlink(pauseTrigger);
                [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePaused;
            }
            if (playTrigger && access(playTrigger, F_OK) == 0) {
                unlink(playTrigger);
                [MPNowPlayingInfoCenter defaultCenter].playbackState = MPNowPlayingPlaybackStatePlaying;
            }
        }
        
        unlink(readyFile);
    }
    return 0;
}
