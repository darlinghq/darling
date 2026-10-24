#ifndef _MEDIAPLAYER_MEDIAPLAYER_H_
#define _MEDIAPLAYER_MEDIAPLAYER_H_

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>

#ifndef MP_EXTERN
#define MP_EXTERN FOUNDATION_EXPORT
#endif

typedef NS_ENUM(NSUInteger, MPNowPlayingPlaybackState) {
    MPNowPlayingPlaybackStateUnknown = 0,
    MPNowPlayingPlaybackStatePlaying,
    MPNowPlayingPlaybackStatePaused,
    MPNowPlayingPlaybackStateStopped,
    MPNowPlayingPlaybackStateInterrupted
};

MP_EXTERN NSString *const MPMediaItemPropertyAlbumTitle;
MP_EXTERN NSString *const MPMediaItemPropertyAlbumTrackNumber;
MP_EXTERN NSString *const MPMediaItemPropertyArtist;
MP_EXTERN NSString *const MPMediaItemPropertyArtwork;
MP_EXTERN NSString *const MPMediaItemPropertyPlaybackDuration;
MP_EXTERN NSString *const MPMediaItemPropertyTitle;

MP_EXTERN NSString *const MPNowPlayingInfoPropertyElapsedPlaybackTime;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyMediaType;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyPlaybackProgress;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyPlaybackRate;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyIsLiveStream;

@interface MPMediaItemArtwork : NSObject {
    NSImage *_image;
}
- (instancetype) initWithBoundsSize: (NSSize) boundsSize requestHandler: (NSImage * (^)(NSSize size)) requestHandler;
- (instancetype) initWithImage: (NSImage *) image;
- (NSImage *) imageWithSize: (NSSize) size;
@end

@interface MPNowPlayingInfoCenter : NSObject {
    NSDictionary<NSString *, id> *_nowPlayingInfo;
    MPNowPlayingPlaybackState _playbackState;
}

+ (MPNowPlayingInfoCenter *) defaultCenter;

@property(copy, nullable) NSDictionary<NSString *, id> *nowPlayingInfo;
@property MPNowPlayingPlaybackState playbackState;

@end

@interface MPRemoteCommand : NSObject {
    BOOL _enabled;
    id _target; /* weak, not retained per Apple API design */
    SEL _action;
}
@property(getter=isEnabled) BOOL enabled;
- (void) addTarget: (id) target action: (SEL) action;
- (void) removeTarget: (id) target action: (SEL) action;
- (void) removeTarget: (id) target;
- (id) addTargetWithHandler: (id) handler;
@end

@interface MPSkipIntervalCommand : MPRemoteCommand {
    NSArray *_preferredIntervals;
}
@property(copy) NSArray *preferredIntervals;
@end

@interface MPRemoteCommandEvent : NSObject {
    MPRemoteCommand *_command;
    NSTimeInterval _timestamp;
}
@property(readonly) MPRemoteCommand *command;
@property(readonly) NSTimeInterval timestamp;
@end

@interface MPChangePlaybackPositionCommandEvent : MPRemoteCommandEvent {
    NSTimeInterval _positionTime;
}
@property(readonly) NSTimeInterval positionTime;
@end

@interface MPRemoteCommandCenter : NSObject {
    NSMutableDictionary<NSString *, MPRemoteCommand *> *_commands;
    MPRemoteCommand *_pauseCommand;
    MPRemoteCommand *_playCommand;
    MPRemoteCommand *_stopCommand;
    MPRemoteCommand *_togglePlayPauseCommand;
    MPRemoteCommand *_nextTrackCommand;
    MPRemoteCommand *_previousTrackCommand;
    MPRemoteCommand *_changePlaybackPositionCommand;
    MPSkipIntervalCommand *_skipForwardCommand;
    MPSkipIntervalCommand *_skipBackwardCommand;
    MPRemoteCommand *_seekForwardCommand;
    MPRemoteCommand *_seekBackwardCommand;
}

+ (MPRemoteCommandCenter *) sharedCommandCenter;

@property(readonly) MPRemoteCommand *pauseCommand;
@property(readonly) MPRemoteCommand *playCommand;
@property(readonly) MPRemoteCommand *stopCommand;
@property(readonly) MPRemoteCommand *togglePlayPauseCommand;
@property(readonly) MPRemoteCommand *nextTrackCommand;
@property(readonly) MPRemoteCommand *previousTrackCommand;
@property(readonly) MPRemoteCommand *changePlaybackPositionCommand;
@property(readonly) MPSkipIntervalCommand *skipForwardCommand;
@property(readonly) MPSkipIntervalCommand *skipBackwardCommand;
@property(readonly) MPRemoteCommand *seekForwardCommand;
@property(readonly) MPRemoteCommand *seekBackwardCommand;

@end

#endif
