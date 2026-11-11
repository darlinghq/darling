/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MEDIAPLAYER_MEDIAPLAYER_H_
#define _MEDIAPLAYER_MEDIAPLAYER_H_

#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>

#ifndef MP_EXTERN
#define MP_EXTERN FOUNDATION_EXPORT
#endif

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MPNowPlayingPlaybackState) {
    MPNowPlayingPlaybackStateUnknown = 0,
    MPNowPlayingPlaybackStatePlaying,
    MPNowPlayingPlaybackStatePaused,
    MPNowPlayingPlaybackStateStopped,
    MPNowPlayingPlaybackStateInterrupted
};

typedef NS_ENUM(NSUInteger, MPNowPlayingInfoMediaType) {
    MPNowPlayingInfoMediaTypeNone = 0,
    MPNowPlayingInfoMediaTypeAudio,
    MPNowPlayingInfoMediaTypeVideo
};

typedef NS_ENUM(NSInteger, MPRemoteCommandHandlerStatus) {
    MPRemoteCommandHandlerStatusSuccess = 0,
    MPRemoteCommandHandlerStatusNoSuchContent = 100,
    MPRemoteCommandHandlerStatusNoActionableNowPlayingItem = 110,
    MPRemoteCommandHandlerStatusDeviceNotFound = 120,
    MPRemoteCommandHandlerStatusCommandFailed = 200
};

typedef NS_ENUM(NSInteger, MPShuffleType) {
    MPShuffleTypeOff,
    MPShuffleTypeItems,
    MPShuffleTypeCollections
};

typedef NS_ENUM(NSInteger, MPRepeatType) {
    MPRepeatTypeOff,
    MPRepeatTypeOne,
    MPRepeatTypeAll
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
MP_EXTERN NSString *const MPNowPlayingInfoPropertyDefaultPlaybackRate;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyPlaybackQueueIndex;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyPlaybackQueueCount;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyChapterNumber;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyChapterCount;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyIsLiveStream;
MP_EXTERN NSString *const MPNowPlayingInfoPropertyAssetURL;

@interface MPMediaItemArtwork : NSObject {
    NSImage *_image;
    NSRect _bounds;
    NSImage *_Nullable (^_requestHandler)(NSSize size);
}
- (instancetype) initWithBoundsSize: (NSSize) boundsSize requestHandler: (NSImage *_Nullable (^)(NSSize size)) requestHandler;
- (instancetype) initWithImage: (NSImage *) image;
- (nullable NSImage *) imageWithSize: (NSSize) size;
@property(readonly) NSRect bounds;
@end

@interface MPNowPlayingInfoCenter : NSObject {
    NSDictionary<NSString *, id> *_nowPlayingInfo;
    MPNowPlayingPlaybackState _playbackState;
}

+ (MPNowPlayingInfoCenter *) defaultCenter;

@property(copy, nullable) NSDictionary<NSString *, id> *nowPlayingInfo;
@property MPNowPlayingPlaybackState playbackState;

@end

@class MPRemoteCommandEvent;

@interface MPRemoteCommand : NSObject {
    BOOL _enabled;
    NSMutableArray *_targets;
}
@property(getter=isEnabled) BOOL enabled;
- (void) addTarget: (id) target action: (SEL) action;
- (void) removeTarget: (id) target action: (nullable SEL) action;
- (void) removeTarget: (nullable id) target;
- (id) addTargetWithHandler: (MPRemoteCommandHandlerStatus (^)(MPRemoteCommandEvent *event)) handler;
@end

@interface MPSkipIntervalCommand : MPRemoteCommand {
    NSArray<NSNumber *> *_preferredIntervals;
}
@property(copy) NSArray<NSNumber *> *preferredIntervals;
@end

@interface MPFeedbackCommand : MPRemoteCommand {
    BOOL _active;
    NSString *_localizedTitle;
    NSString *_localizedShortTitle;
}
@property(getter=isActive) BOOL active;
@property(copy) NSString *localizedTitle;
@property(copy) NSString *localizedShortTitle;
@end

@interface MPRatingCommand : MPRemoteCommand {
    float _minimumRating;
    float _maximumRating;
}
@property float minimumRating;
@property float maximumRating;
@end

@interface MPChangePlaybackRateCommand : MPRemoteCommand {
    NSArray<NSNumber *> *_supportedPlaybackRates;
}
@property(copy) NSArray<NSNumber *> *supportedPlaybackRates;
@end

@interface MPChangePlaybackPositionCommand : MPRemoteCommand
@end

@interface MPChangeShuffleModeCommand : MPRemoteCommand {
    MPShuffleType _currentShuffleType;
}
@property MPShuffleType currentShuffleType;
@end

@interface MPChangeRepeatModeCommand : MPRemoteCommand {
    MPRepeatType _currentRepeatType;
}
@property MPRepeatType currentRepeatType;
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
    MPRemoteCommand *_pauseCommand;
    MPRemoteCommand *_playCommand;
    MPRemoteCommand *_stopCommand;
    MPRemoteCommand *_togglePlayPauseCommand;
    MPRemoteCommand *_enableLanguageOptionCommand;
    MPRemoteCommand *_disableLanguageOptionCommand;
    MPChangePlaybackRateCommand *_changePlaybackRateCommand;
    MPChangeRepeatModeCommand *_changeRepeatModeCommand;
    MPChangeShuffleModeCommand *_changeShuffleModeCommand;
    MPRemoteCommand *_nextTrackCommand;
    MPRemoteCommand *_previousTrackCommand;
    MPSkipIntervalCommand *_skipForwardCommand;
    MPSkipIntervalCommand *_skipBackwardCommand;
    MPRemoteCommand *_seekForwardCommand;
    MPRemoteCommand *_seekBackwardCommand;
    MPChangePlaybackPositionCommand *_changePlaybackPositionCommand;
    MPRatingCommand *_ratingCommand;
    MPFeedbackCommand *_likeCommand;
    MPFeedbackCommand *_dislikeCommand;
    MPFeedbackCommand *_bookmarkCommand;
}

+ (MPRemoteCommandCenter *) sharedCommandCenter;

@property(readonly) MPRemoteCommand *pauseCommand;
@property(readonly) MPRemoteCommand *playCommand;
@property(readonly) MPRemoteCommand *stopCommand;
@property(readonly) MPRemoteCommand *togglePlayPauseCommand;
@property(readonly) MPRemoteCommand *enableLanguageOptionCommand;
@property(readonly) MPRemoteCommand *disableLanguageOptionCommand;
@property(readonly) MPChangePlaybackRateCommand *changePlaybackRateCommand;
@property(readonly) MPChangeRepeatModeCommand *changeRepeatModeCommand;
@property(readonly) MPChangeShuffleModeCommand *changeShuffleModeCommand;
@property(readonly) MPRemoteCommand *nextTrackCommand;
@property(readonly) MPRemoteCommand *previousTrackCommand;
@property(readonly) MPSkipIntervalCommand *skipForwardCommand;
@property(readonly) MPSkipIntervalCommand *skipBackwardCommand;
@property(readonly) MPRemoteCommand *seekForwardCommand;
@property(readonly) MPRemoteCommand *seekBackwardCommand;
@property(readonly) MPChangePlaybackPositionCommand *changePlaybackPositionCommand;
@property(readonly) MPRatingCommand *ratingCommand;
@property(readonly) MPFeedbackCommand *likeCommand;
@property(readonly) MPFeedbackCommand *dislikeCommand;
@property(readonly) MPFeedbackCommand *bookmarkCommand;

@end

NS_ASSUME_NONNULL_END

#endif
