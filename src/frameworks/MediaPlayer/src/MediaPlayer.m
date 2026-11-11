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

#import <MediaPlayer/MediaPlayer.h>

// Values as macOS reports them.
NSString *const MPMediaItemPropertyAlbumTitle = @"albumTitle";
NSString *const MPMediaItemPropertyAlbumTrackNumber = @"albumTrackNumber";
NSString *const MPMediaItemPropertyArtist = @"artist";
NSString *const MPMediaItemPropertyArtwork = @"artwork";
NSString *const MPMediaItemPropertyPlaybackDuration = @"playbackDuration";
NSString *const MPMediaItemPropertyTitle = @"title";

NSString *const MPNowPlayingInfoPropertyElapsedPlaybackTime = @"MPNowPlayingInfoPropertyElapsedPlaybackTime";
NSString *const MPNowPlayingInfoPropertyMediaType = @"MPNowPlayingInfoPropertyMediaType";
NSString *const MPNowPlayingInfoPropertyPlaybackProgress = @"MPNowPlayingInfoPropertyPlaybackProgress";
NSString *const MPNowPlayingInfoPropertyPlaybackRate = @"MPNowPlayingInfoPropertyPlaybackRate";
NSString *const MPNowPlayingInfoPropertyDefaultPlaybackRate = @"MPNowPlayingInfoPropertyDefaultPlaybackRate";
NSString *const MPNowPlayingInfoPropertyPlaybackQueueIndex = @"MPNowPlayingInfoPropertyPlaybackQueueIndex";
NSString *const MPNowPlayingInfoPropertyPlaybackQueueCount = @"MPNowPlayingInfoPropertyPlaybackQueueCount";
NSString *const MPNowPlayingInfoPropertyChapterNumber = @"MPNowPlayingInfoPropertyChapterNumber";
NSString *const MPNowPlayingInfoPropertyChapterCount = @"MPNowPlayingInfoPropertyChapterCount";
NSString *const MPNowPlayingInfoPropertyIsLiveStream = @"MPNowPlayingInfoPropertyIsLiveStream";
NSString *const MPNowPlayingInfoPropertyAssetURL = @"MPNowPlayingInfoPropertyAssetURL";

@implementation MPMediaItemArtwork

@synthesize bounds = _bounds;

- (instancetype) initWithBoundsSize: (NSSize) boundsSize requestHandler: (NSImage * (^)(NSSize size)) requestHandler {
    self = [super init];
    if (self) {
        _bounds = NSMakeRect(0, 0, boundsSize.width, boundsSize.height);
        _requestHandler = [requestHandler copy];
    }
    return self;
}

- (instancetype) initWithImage: (NSImage *) image {
    self = [super init];
    if (self) {
        _image = [image retain];
        _bounds = NSMakeRect(0, 0, [image size].width, [image size].height);
    }
    return self;
}

- (void) dealloc {
    [_image release];
    [_requestHandler release];
    [super dealloc];
}

// The request handler is asked for an image each time one is wanted, for the
// size the caller asks about.
- (NSImage *) imageWithSize: (NSSize) size {
    if (_requestHandler != nil) {
        return _requestHandler(size);
    }
    return _image;
}

@end

@implementation MPNowPlayingInfoCenter

@synthesize nowPlayingInfo = _nowPlayingInfo;
@synthesize playbackState = _playbackState;

+ (MPNowPlayingInfoCenter *) defaultCenter {
    static MPNowPlayingInfoCenter *center = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        center = [[MPNowPlayingInfoCenter alloc] init];
    });
    return center;
}

- (void) dealloc {
    [_nowPlayingInfo release];
    [super dealloc];
}

@end

// One registered target: either an object and action (not retained, as the
// command does not own its targets) or a handler block. The entry itself is the
// opaque token returned for a handler.
@interface _MPRemoteCommandTarget : NSObject {
@public
    id target;
    SEL action;
    MPRemoteCommandHandlerStatus (^handler)(MPRemoteCommandEvent *);
}
@end

@implementation _MPRemoteCommandTarget
- (void) dealloc {
    [handler release];
    [super dealloc];
}
@end

@interface MPRemoteCommand ()
- (void) invokeWithEvent: (MPRemoteCommandEvent *) event;
@end

@implementation MPRemoteCommand

@synthesize enabled = _enabled;

- (instancetype) init {
    self = [super init];
    if (self) {
        _enabled = YES;
        _targets = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void) dealloc {
    [_targets release];
    [super dealloc];
}

- (void) addTarget: (id) target action: (SEL) action {
    _MPRemoteCommandTarget *entry = [[_MPRemoteCommandTarget alloc] init];
    entry->target = target;
    entry->action = action;
    @synchronized(self) {
        [_targets addObject: entry];
    }
    [entry release];
}

- (id) addTargetWithHandler: (MPRemoteCommandHandlerStatus (^)(MPRemoteCommandEvent *event)) handler {
    _MPRemoteCommandTarget *entry = [[_MPRemoteCommandTarget alloc] init];
    entry->handler = [handler copy];
    @synchronized(self) {
        [_targets addObject: entry];
    }
    return [entry autorelease];
}

- (void) removeTarget: (id) target action: (SEL) action {
    @synchronized(self) {
        NSMutableIndexSet *remove = [NSMutableIndexSet indexSet];
        [_targets enumerateObjectsUsingBlock: ^(_MPRemoteCommandTarget *entry, NSUInteger idx, BOOL *stop) {
            BOOL sameTarget = (target == nil) || entry == target || entry->target == target;
            BOOL sameAction = (action == NULL) || entry->action == action;
            if (sameTarget && sameAction) {
                [remove addIndex: idx];
            }
        }];
        [_targets removeObjectsAtIndexes: remove];
    }
}

// A nil target removes every registered target; a handler token removes its
// handler.
- (void) removeTarget: (id) target {
    [self removeTarget: target action: NULL];
}

- (void) invokeWithEvent: (MPRemoteCommandEvent *) event {
    if (!_enabled) {
        return;
    }
    NSArray *targets;
    @synchronized(self) {
        targets = [[_targets copy] autorelease];
    }
    for (_MPRemoteCommandTarget *entry in targets) {
        if (entry->handler != nil) {
            entry->handler(event);
        } else if ([entry->target respondsToSelector: entry->action]) {
            [entry->target performSelector: entry->action withObject: event];
        }
    }
}

@end

@implementation MPSkipIntervalCommand

@synthesize preferredIntervals = _preferredIntervals;

- (instancetype) init {
    self = [super init];
    if (self) {
        _preferredIntervals = [@[ @10 ] retain];
    }
    return self;
}

- (void) dealloc {
    [_preferredIntervals release];
    [super dealloc];
}

@end

@implementation MPFeedbackCommand

@synthesize active = _active, localizedTitle = _localizedTitle, localizedShortTitle = _localizedShortTitle;

- (void) dealloc {
    [_localizedTitle release];
    [_localizedShortTitle release];
    [super dealloc];
}

@end

@implementation MPRatingCommand
@synthesize minimumRating = _minimumRating, maximumRating = _maximumRating;
@end

@implementation MPChangePlaybackRateCommand

@synthesize supportedPlaybackRates = _supportedPlaybackRates;

- (void) dealloc {
    [_supportedPlaybackRates release];
    [super dealloc];
}

@end

@implementation MPChangePlaybackPositionCommand
@end

@implementation MPChangeShuffleModeCommand
@synthesize currentShuffleType = _currentShuffleType;
@end

@implementation MPChangeRepeatModeCommand
@synthesize currentRepeatType = _currentRepeatType;
@end

@implementation MPRemoteCommandEvent

@synthesize command = _command;
@synthesize timestamp = _timestamp;

- (void) dealloc {
    [_command release];
    [super dealloc];
}

@end

@implementation MPChangePlaybackPositionCommandEvent

@synthesize positionTime = _positionTime;

@end

@implementation MPRemoteCommandCenter

@synthesize pauseCommand = _pauseCommand;
@synthesize playCommand = _playCommand;
@synthesize stopCommand = _stopCommand;
@synthesize togglePlayPauseCommand = _togglePlayPauseCommand;
@synthesize enableLanguageOptionCommand = _enableLanguageOptionCommand;
@synthesize disableLanguageOptionCommand = _disableLanguageOptionCommand;
@synthesize changePlaybackRateCommand = _changePlaybackRateCommand;
@synthesize changeRepeatModeCommand = _changeRepeatModeCommand;
@synthesize changeShuffleModeCommand = _changeShuffleModeCommand;
@synthesize nextTrackCommand = _nextTrackCommand;
@synthesize previousTrackCommand = _previousTrackCommand;
@synthesize skipForwardCommand = _skipForwardCommand;
@synthesize skipBackwardCommand = _skipBackwardCommand;
@synthesize seekForwardCommand = _seekForwardCommand;
@synthesize seekBackwardCommand = _seekBackwardCommand;
@synthesize changePlaybackPositionCommand = _changePlaybackPositionCommand;
@synthesize ratingCommand = _ratingCommand;
@synthesize likeCommand = _likeCommand;
@synthesize dislikeCommand = _dislikeCommand;
@synthesize bookmarkCommand = _bookmarkCommand;

+ (MPRemoteCommandCenter *) sharedCommandCenter {
    static MPRemoteCommandCenter *center = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        center = [[MPRemoteCommandCenter alloc] init];
    });
    return center;
}

- (instancetype) init {
    self = [super init];
    if (self) {
        _pauseCommand = [[MPRemoteCommand alloc] init];
        _playCommand = [[MPRemoteCommand alloc] init];
        _stopCommand = [[MPRemoteCommand alloc] init];
        _togglePlayPauseCommand = [[MPRemoteCommand alloc] init];
        _enableLanguageOptionCommand = [[MPRemoteCommand alloc] init];
        _disableLanguageOptionCommand = [[MPRemoteCommand alloc] init];
        _changePlaybackRateCommand = [[MPChangePlaybackRateCommand alloc] init];
        _changeRepeatModeCommand = [[MPChangeRepeatModeCommand alloc] init];
        _changeShuffleModeCommand = [[MPChangeShuffleModeCommand alloc] init];
        _nextTrackCommand = [[MPRemoteCommand alloc] init];
        _previousTrackCommand = [[MPRemoteCommand alloc] init];
        _skipForwardCommand = [[MPSkipIntervalCommand alloc] init];
        _skipBackwardCommand = [[MPSkipIntervalCommand alloc] init];
        _seekForwardCommand = [[MPRemoteCommand alloc] init];
        _seekBackwardCommand = [[MPRemoteCommand alloc] init];
        _changePlaybackPositionCommand = [[MPChangePlaybackPositionCommand alloc] init];
        _ratingCommand = [[MPRatingCommand alloc] init];
        _likeCommand = [[MPFeedbackCommand alloc] init];
        _dislikeCommand = [[MPFeedbackCommand alloc] init];
        _bookmarkCommand = [[MPFeedbackCommand alloc] init];
    }
    return self;
}

- (void) dealloc {
    [_pauseCommand release];
    [_playCommand release];
    [_stopCommand release];
    [_togglePlayPauseCommand release];
    [_enableLanguageOptionCommand release];
    [_disableLanguageOptionCommand release];
    [_changePlaybackRateCommand release];
    [_changeRepeatModeCommand release];
    [_changeShuffleModeCommand release];
    [_nextTrackCommand release];
    [_previousTrackCommand release];
    [_skipForwardCommand release];
    [_skipBackwardCommand release];
    [_seekForwardCommand release];
    [_seekBackwardCommand release];
    [_changePlaybackPositionCommand release];
    [_ratingCommand release];
    [_likeCommand release];
    [_dislikeCommand release];
    [_bookmarkCommand release];
    [super dealloc];
}

@end
