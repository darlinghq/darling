#import <MediaPlayer/MediaPlayer.h>
#include <stdio.h>
int main(void) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
    MPRemoteCommand *command = [center changePlaybackPositionCommand];
    Class expected = NSClassFromString(@"MPChangePlaybackPositionCommand");
    if (expected == Nil || ![command isKindOfClass:expected]) {
        fprintf(stderr, "Playback position command has the wrong runtime class\n");
        return 1;
    }
    if (![command isKindOfClass:[MPRemoteCommand class]]) return 2;
    if ([center changePlaybackPositionCommand] != command) return 3;
    [command setEnabled:NO];
    if ([[center changePlaybackPositionCommand] isEnabled]) return 4;
    [command setEnabled:YES];
    if (![[center changePlaybackPositionCommand] isEnabled]) return 5;
    if ([center playCommand] == command || ![[center playCommand] isEnabled]) return 6;
    [pool drain];
    puts("Playback position command class, identity and enabled state passed");
    return 0;
}
