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
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include "mediaremoted_protocol.h"

// Values as macOS reports them.
NSString *const MPMediaItemPropertyAlbumArtist = @"albumArtist";
NSString *const MPMediaItemPropertyAlbumTitle = @"albumTitle";
NSString *const MPMediaItemPropertyAlbumTrackCount = @"albumTrackCount";
NSString *const MPMediaItemPropertyAlbumTrackNumber = @"albumTrackNumber";
NSString *const MPMediaItemPropertyArtist = @"artist";
NSString *const MPMediaItemPropertyArtwork = @"artwork";
NSString *const MPMediaItemPropertyComposer = @"composer";
NSString *const MPMediaItemPropertyDiscCount = @"discCount";
NSString *const MPMediaItemPropertyDiscNumber = @"discNumber";
NSString *const MPMediaItemPropertyGenre = @"genre";
NSString *const MPMediaItemPropertyPlaybackDuration = @"playbackDuration";
NSString *const MPMediaItemPropertyPlayCount = @"playCount";
NSString *const MPMediaItemPropertyRating = @"rating";
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

@interface MPRemoteCommand ()
- (MPRemoteCommandHandlerStatus) invokeWithEvent: (MPRemoteCommandEvent *) event;
- (BOOL) hasTargets;
@end

@interface MPRemoteCommandEvent ()
- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp;
@end

@interface MPChangePlaybackPositionCommandEvent ()
- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp positionTime: (NSTimeInterval) positionTime;
@end

@interface _MPMediaRemoteManager : NSObject {
    xpc_connection_t _connection;
    dispatch_queue_t _queue;
}
+ (_MPMediaRemoteManager *) sharedManager;
- (void) notifyStateChanged;
- (void) notifyCommandsChanged;
@end

@implementation _MPMediaRemoteManager

+ (_MPMediaRemoteManager *) sharedManager {
    static _MPMediaRemoteManager *mgr = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        mgr = [[_MPMediaRemoteManager alloc] init];
    });
    return mgr;
}

- (instancetype) init {
    self = [super init];
    if (self) {
        _queue = dispatch_queue_create("com.apple.mediaremote.client", DISPATCH_QUEUE_SERIAL);
    }
    return self;
}

- (void) dealloc {
    if (_connection) {
        xpc_connection_cancel(_connection);
        xpc_release(_connection);
    }
    dispatch_release(_queue);
    [super dealloc];
}

- (void) ensureConnection {
    if (_connection)
        return;

    const char *serviceName = getenv(MEDIAREMOTED_SERVICE_ENV);
    if (!serviceName)
        serviceName = MEDIAREMOTED_MACH_SERVICE;

    _connection = xpc_connection_create_mach_service(serviceName, _queue, 0);
    if (!_connection)
        return;

    xpc_connection_set_event_handler(_connection, ^(xpc_object_t event) {
        if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
            const char *op = xpc_dictionary_get_string(event, MR_KEY_OP);
            if (op && strcmp(op, MR_OP_INVOKE_COMMAND) == 0) {
                const char *cmd = xpc_dictionary_get_string(event, MR_KEY_COMMAND);
                double pos = xpc_dictionary_get_double(event, MR_KEY_POSITION_TIME);
                double off = xpc_dictionary_get_double(event, MR_KEY_OFFSET_TIME);
                [self handleRemoteCommand:cmd ? [NSString stringWithUTF8String:cmd] : @""
                             positionTime:pos
                               offsetTime:off];
            }
        } else if (event == XPC_ERROR_CONNECTION_INVALID || event == XPC_ERROR_CONNECTION_INTERRUPTED) {
            if (_connection) {
                xpc_release(_connection);
                _connection = NULL;
            }
        }
    });

    xpc_connection_resume(_connection);

    // Initial checkin
    xpc_object_t checkin = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(checkin, MR_KEY_OP, MR_OP_CHECKIN);
    xpc_dictionary_set_string(checkin, MR_KEY_APP_NAME, [[[NSProcessInfo processInfo] processName] UTF8String]);
    NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleId) {
        xpc_dictionary_set_string(checkin, MR_KEY_BUNDLE_ID, [bundleId UTF8String]);
    }
    xpc_connection_send_message(_connection, checkin);
    xpc_release(checkin);
}

- (void) handleRemoteCommand: (NSString *) cmdName positionTime: (double) posTime offsetTime: (double) offTime {
    dispatch_async(dispatch_get_main_queue(), ^{
        MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
        MPRemoteCommand *cmd = nil;
        MPRemoteCommandEvent *evt = nil;

        if ([cmdName isEqualToString:@"play"]) {
            cmd = center.playCommand;
            if (!cmd.enabled || ![cmd hasTargets]) {
                cmd = center.togglePlayPauseCommand;
            }
        } else if ([cmdName isEqualToString:@"pause"]) {
            cmd = center.pauseCommand;
            if (!cmd.enabled || ![cmd hasTargets]) {
                cmd = center.togglePlayPauseCommand;
            }
        } else if ([cmdName isEqualToString:@"togglePlayPause"]) {
            cmd = center.togglePlayPauseCommand;
            if (!cmd.enabled || ![cmd hasTargets]) {
                if ([MPNowPlayingInfoCenter defaultCenter].playbackState == MPNowPlayingPlaybackStatePlaying)
                    cmd = center.pauseCommand;
                else
                    cmd = center.playCommand;
            }
        } else if ([cmdName isEqualToString:@"stop"]) {
            cmd = center.stopCommand;
        } else if ([cmdName isEqualToString:@"nextTrack"]) {
            cmd = center.nextTrackCommand;
        } else if ([cmdName isEqualToString:@"previousTrack"]) {
            cmd = center.previousTrackCommand;
        } else if ([cmdName isEqualToString:@"skipForward"]) {
            cmd = center.skipForwardCommand;
        } else if ([cmdName isEqualToString:@"skipBackward"]) {
            cmd = center.skipBackwardCommand;
        } else if ([cmdName isEqualToString:@"changePlaybackPosition"] || [cmdName isEqualToString:@"setPosition"]) {
            cmd = center.changePlaybackPositionCommand;
            evt = [[[MPChangePlaybackPositionCommandEvent alloc] initWithCommand:cmd
                                                                       timestamp:[[NSDate date] timeIntervalSince1970]
                                                                    positionTime:posTime] autorelease];
        } else if ([cmdName isEqualToString:@"seek"]) {
            if (offTime > 0 && center.skipForwardCommand.enabled && [center.skipForwardCommand hasTargets]) {
                cmd = center.skipForwardCommand;
            } else if (offTime < 0 && center.skipBackwardCommand.enabled && [center.skipBackwardCommand hasTargets]) {
                cmd = center.skipBackwardCommand;
            } else if (center.changePlaybackPositionCommand.enabled && [center.changePlaybackPositionCommand hasTargets]) {
                cmd = center.changePlaybackPositionCommand;
                double cur = [[MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo[MPNowPlayingInfoPropertyElapsedPlaybackTime] doubleValue];
                double targetPos = cur + offTime;
                if (targetPos < 0) targetPos = 0;
                evt = [[[MPChangePlaybackPositionCommandEvent alloc] initWithCommand:cmd
                                                                           timestamp:[[NSDate date] timeIntervalSince1970]
                                                                        positionTime:targetPos] autorelease];
            }
        }

        if (evt == nil && cmd != nil) {
            evt = [[[MPRemoteCommandEvent alloc] initWithCommand:cmd
                                                       timestamp:[[NSDate date] timeIntervalSince1970]] autorelease];
        }

        if (cmd != nil) {
            [cmd invokeWithEvent:evt];
        }
    });
}

- (void) notifyStateChanged {
    dispatch_async(_queue, ^{
        [self ensureConnection];
        if (!_connection)
            return;

        MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
        NSDictionary *info = center.nowPlayingInfo;

        xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
        xpc_dictionary_set_string(dict, MR_KEY_OP, MR_OP_UPDATE_STATE);
        xpc_dictionary_set_string(dict, MR_KEY_APP_NAME, [[[NSProcessInfo processInfo] processName] UTF8String]);
        NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
        if (bundleId) {
            xpc_dictionary_set_string(dict, MR_KEY_BUNDLE_ID, [bundleId UTF8String]);
        }
        xpc_dictionary_set_uint64(dict, MR_KEY_PLAYBACK_STATE, center.playbackState);

        if (info != nil) {
            NSString *title = info[MPMediaItemPropertyTitle];
            if (title) xpc_dictionary_set_string(dict, MR_KEY_TITLE, [title UTF8String]);

            id artistObj = info[MPMediaItemPropertyArtist];
            if ([artistObj isKindOfClass:[NSString class]]) {
                xpc_dictionary_set_string(dict, MR_KEY_ARTIST, [artistObj UTF8String]);
            } else if ([artistObj isKindOfClass:[NSArray class]] && [(NSArray *)artistObj count] > 0) {
                xpc_dictionary_set_string(dict, MR_KEY_ARTIST, [[(NSArray *)artistObj componentsJoinedByString:@", "] UTF8String]);
            }

            NSString *album = info[MPMediaItemPropertyAlbumTitle];
            if (album) xpc_dictionary_set_string(dict, MR_KEY_ALBUM, [album UTF8String]);

            NSString *composer = info[MPMediaItemPropertyComposer];
            if (composer) xpc_dictionary_set_string(dict, MR_KEY_COMPOSER, [composer UTF8String]);

            NSString *genre = info[MPMediaItemPropertyGenre];
            if (genre) xpc_dictionary_set_string(dict, MR_KEY_GENRE, [genre UTF8String]);

            NSNumber *trackNum = info[MPMediaItemPropertyAlbumTrackNumber];
            if (trackNum) xpc_dictionary_set_int64(dict, MR_KEY_ALBUM_TRACK_NUMBER, [trackNum intValue]);

            NSNumber *discNum = info[MPMediaItemPropertyDiscNumber];
            if (discNum) xpc_dictionary_set_int64(dict, MR_KEY_DISC_NUMBER, [discNum intValue]);

            NSNumber *duration = info[MPMediaItemPropertyPlaybackDuration];
            if (duration) xpc_dictionary_set_double(dict, MR_KEY_DURATION, [duration doubleValue]);

            NSNumber *elapsed = info[MPNowPlayingInfoPropertyElapsedPlaybackTime];
            if (elapsed) xpc_dictionary_set_double(dict, MR_KEY_ELAPSED_TIME, [elapsed doubleValue]);

            NSNumber *rate = info[MPNowPlayingInfoPropertyPlaybackRate];
            if (rate) xpc_dictionary_set_double(dict, MR_KEY_RATE, [rate doubleValue]);

            // Artwork
            id artObj = info[MPMediaItemPropertyArtwork];
            if ([artObj isKindOfClass:[MPMediaItemArtwork class]]) {
                MPMediaItemArtwork *art = (MPMediaItemArtwork *)artObj;
                NSImage *img = [art imageWithSize:art.bounds.size];
                if (img != nil) {
                    NSData *png = nil;
                    for (id rep in [img representations]) {
                        if ([rep respondsToSelector:@selector(representationUsingType:properties:)]) {
                            png = [rep representationUsingType:4 properties:@{}];
                            if (png) break;
                        }
                    }
                    if (!png && [img respondsToSelector:@selector(TIFFRepresentation)]) {
                        NSData *tiff = [img TIFFRepresentation];
                        Class repClass = NSClassFromString(@"NSBitmapImageRep");
                        if (tiff && repClass) {
                            id rep = [repClass imageRepWithData:tiff];
                            if ([rep respondsToSelector:@selector(representationUsingType:properties:)]) {
                                png = [rep representationUsingType:4 properties:@{}];
                            }
                        }
                    }
                    if (png) {
                        NSString *tmpDir = [[NSFileManager defaultManager] fileExistsAtPath:@"/Volumes/SystemRoot/tmp"] ? @"/Volumes/SystemRoot/tmp" : @"/tmp";
                        NSString *path = [NSString stringWithFormat:@"%@/darling_art_%d.png", tmpDir, getpid()];
                        if ([png writeToFile:path atomically:YES]) {
                            NSString *urlStr = [NSString stringWithFormat:@"file:///tmp/darling_art_%d.png", getpid()];
                            xpc_dictionary_set_string(dict, MR_KEY_ARTWORK_URL, [urlStr UTF8String]);
                        }
                    }
                }
            } else if ([artObj isKindOfClass:[NSString class]]) {
                xpc_dictionary_set_string(dict, MR_KEY_ARTWORK_URL, [(NSString *)artObj UTF8String]);
            }
        }

        [self appendCommandsStateToDict:dict];

        xpc_connection_send_message(_connection, dict);
        xpc_release(dict);
    });
}

- (void) appendCommandsStateToDict: (xpc_object_t) dict {
    MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
    uint64_t mask = 0;
    if (center.playCommand.enabled && [center.playCommand hasTargets]) mask |= MR_CMD_PLAY;
    if (center.pauseCommand.enabled && [center.pauseCommand hasTargets]) mask |= MR_CMD_PAUSE;
    if (center.stopCommand.enabled && [center.stopCommand hasTargets]) mask |= MR_CMD_STOP;
    if (center.togglePlayPauseCommand.enabled && [center.togglePlayPauseCommand hasTargets]) mask |= MR_CMD_TOGGLE_PLAY_PAUSE;
    if (center.nextTrackCommand.enabled && [center.nextTrackCommand hasTargets]) mask |= MR_CMD_NEXT_TRACK;
    if (center.previousTrackCommand.enabled && [center.previousTrackCommand hasTargets]) mask |= MR_CMD_PREVIOUS_TRACK;
    if (center.changePlaybackRateCommand.enabled && [center.changePlaybackRateCommand hasTargets]) mask |= MR_CMD_CHANGE_PLAYBACK_RATE;
    if (center.changeRepeatModeCommand.enabled && [center.changeRepeatModeCommand hasTargets]) mask |= MR_CMD_CHANGE_REPEAT_MODE;
    if (center.changeShuffleModeCommand.enabled && [center.changeShuffleModeCommand hasTargets]) mask |= MR_CMD_CHANGE_SHUFFLE_MODE;
    if (center.skipForwardCommand.enabled && [center.skipForwardCommand hasTargets]) mask |= MR_CMD_SKIP_FORWARD;
    if (center.skipBackwardCommand.enabled && [center.skipBackwardCommand hasTargets]) mask |= MR_CMD_SKIP_BACKWARD;
    if (center.changePlaybackPositionCommand.enabled && [center.changePlaybackPositionCommand hasTargets]) mask |= MR_CMD_CHANGE_PLAYBACK_POSITION;
    if (center.ratingCommand.enabled && [center.ratingCommand hasTargets]) mask |= MR_CMD_RATING;
    if (center.likeCommand.enabled && [center.likeCommand hasTargets]) mask |= MR_CMD_LIKE;
    if (center.dislikeCommand.enabled && [center.dislikeCommand hasTargets]) mask |= MR_CMD_DISLIKE;
    if (center.bookmarkCommand.enabled && [center.bookmarkCommand hasTargets]) mask |= MR_CMD_BOOKMARK;
    xpc_dictionary_set_uint64(dict, MR_KEY_ENABLED_COMMANDS, mask);
}

- (void) notifyCommandsChanged {
    dispatch_async(_queue, ^{
        [self ensureConnection];
        if (!_connection)
            return;

        xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
        xpc_dictionary_set_string(dict, MR_KEY_OP, MR_OP_UPDATE_STATE);
        [self appendCommandsStateToDict:dict];
        xpc_connection_send_message(_connection, dict);
        xpc_release(dict);
    });
}

@end

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

+ (MPNowPlayingInfoCenter *) defaultCenter {
    static MPNowPlayingInfoCenter *center = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        center = [[MPNowPlayingInfoCenter alloc] init];
    });
    return center;
}

- (NSDictionary<NSString *,id> *) nowPlayingInfo {
    @synchronized(self) {
        return [[_nowPlayingInfo retain] autorelease];
    }
}

- (void) setNowPlayingInfo: (NSDictionary<NSString *,id> *) nowPlayingInfo {
    @synchronized(self) {
        if (_nowPlayingInfo != nowPlayingInfo) {
            [_nowPlayingInfo release];
            _nowPlayingInfo = [nowPlayingInfo copy];
        }
    }
    [[_MPMediaRemoteManager sharedManager] notifyStateChanged];
}

- (MPNowPlayingPlaybackState) playbackState {
    @synchronized(self) {
        return _playbackState;
    }
}

- (void) setPlaybackState: (MPNowPlayingPlaybackState) playbackState {
    @synchronized(self) {
        _playbackState = playbackState;
    }
    [[_MPMediaRemoteManager sharedManager] notifyStateChanged];
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

@implementation MPRemoteCommand

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

- (BOOL) isEnabled {
    @synchronized(self) {
        return _enabled;
    }
}

- (void) setEnabled: (BOOL) enabled {
    @synchronized(self) {
        if (_enabled != enabled) {
            _enabled = enabled;
        }
    }
    [[_MPMediaRemoteManager sharedManager] notifyCommandsChanged];
}

- (BOOL) hasTargets {
    @synchronized(self) {
        return [_targets count] > 0;
    }
}

- (void) addTarget: (id) target action: (SEL) action {
    _MPRemoteCommandTarget *entry = [[_MPRemoteCommandTarget alloc] init];
    entry->target = target;
    entry->action = action;
    @synchronized(self) {
        [_targets addObject: entry];
    }
    [entry release];
    [[_MPMediaRemoteManager sharedManager] notifyCommandsChanged];
}

- (id) addTargetWithHandler: (MPRemoteCommandHandlerStatus (^)(MPRemoteCommandEvent *event)) handler {
    _MPRemoteCommandTarget *entry = [[_MPRemoteCommandTarget alloc] init];
    entry->handler = [handler copy];
    @synchronized(self) {
        [_targets addObject: entry];
    }
    id token = [entry autorelease];
    [[_MPMediaRemoteManager sharedManager] notifyCommandsChanged];
    return token;
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
    [[_MPMediaRemoteManager sharedManager] notifyCommandsChanged];
}

// A nil target removes every registered target; a handler token removes its
// handler.
- (void) removeTarget: (id) target {
    [self removeTarget: target action: NULL];
}

- (MPRemoteCommandHandlerStatus) invokeWithEvent: (MPRemoteCommandEvent *) event {
    if (!_enabled) {
        return MPRemoteCommandHandlerStatusCommandFailed;
    }
    NSArray *targets;
    @synchronized(self) {
        targets = [[_targets copy] autorelease];
    }
    if ([targets count] == 0) {
        return MPRemoteCommandHandlerStatusNoActionableNowPlayingItem;
    }
    MPRemoteCommandHandlerStatus result = MPRemoteCommandHandlerStatusNoActionableNowPlayingItem;
    for (_MPRemoteCommandTarget *entry in targets) {
        if (entry->handler != nil) {
            MPRemoteCommandHandlerStatus s = entry->handler(event);
            if (s == MPRemoteCommandHandlerStatusSuccess)
                result = MPRemoteCommandHandlerStatusSuccess;
            else if (result != MPRemoteCommandHandlerStatusSuccess)
                result = s;
        } else if ([entry->target respondsToSelector: entry->action]) {
            [entry->target performSelector: entry->action withObject: event];
            result = MPRemoteCommandHandlerStatusSuccess;
        }
    }
    return result;
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

- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp {
    self = [super init];
    if (self) {
        _command = [command retain];
        _timestamp = timestamp;
    }
    return self;
}

- (void) dealloc {
    [_command release];
    [super dealloc];
}

@end

@implementation MPChangePlaybackPositionCommandEvent

@synthesize positionTime = _positionTime;

- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp positionTime: (NSTimeInterval) positionTime {
    self = [super initWithCommand: command timestamp: timestamp];
    if (self) {
        _positionTime = positionTime;
    }
    return self;
}

@end

@implementation MPChangeRepeatModeCommandEvent

@synthesize repeatType = _repeatType;
@synthesize preservesRepeatMode = _preservesRepeatMode;

@end

@implementation MPChangeShuffleModeCommandEvent

@synthesize shuffleType = _shuffleType;
@synthesize preservesShuffleMode = _preservesShuffleMode;

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
