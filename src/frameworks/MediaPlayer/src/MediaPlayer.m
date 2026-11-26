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
#include <objc/message.h>
#include <stdlib.h>
#include <string.h>
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

@interface MPSkipIntervalCommandEvent ()
- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp interval: (NSTimeInterval) interval;
@end

// The client side of the now-playing service, mediaremoted. The now-playing
// info and the remote command state of this process are sent to the service,
// which presents them to the desktop; remote commands come back over the same
// connection and are delivered to the targets registered with the command
// center. Without the service, MediaPlayer keeps working in-process.
@interface _MPMediaRemoteManager : NSObject {
    xpc_connection_t _connection;
    dispatch_queue_t _queue;
    NSTimeInterval _infoTimestamp;
    id _artwork;
    NSString *_artworkURL;
    NSString *_artworkPath;
    unsigned _artworkSerial;
}
+ (_MPMediaRemoteManager *) sharedManager;
- (void) nowPlayingInfoChanged;
- (void) playbackStateChanged;
- (void) commandsChanged;
- (void) removeArtworkFile;
@end

static void MPMediaRemoteAtExit(void);

@implementation _MPMediaRemoteManager

+ (_MPMediaRemoteManager *) sharedManager {
    static _MPMediaRemoteManager *mgr = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        mgr = [[_MPMediaRemoteManager alloc] init];
        atexit(MPMediaRemoteAtExit);
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

// Only called on _queue. Returns YES when a new connection was made, in which
// case the service knows nothing of this process yet.
- (BOOL) ensureConnection {
    if (_connection)
        return NO;

    const char *serviceName = getenv(MEDIAREMOTED_SERVICE_ENV);
    if (!serviceName)
        serviceName = MEDIAREMOTED_MACH_SERVICE;

    _connection = xpc_connection_create_mach_service(serviceName, _queue, 0);
    if (!_connection)
        return NO;

    xpc_connection_set_event_handler(_connection, ^(xpc_object_t event) {
        if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
            const char *op = xpc_dictionary_get_string(event, MR_KEY_OP);
            if (op && strcmp(op, MR_OP_INVOKE_COMMAND) == 0) {
                const char *cmd = xpc_dictionary_get_string(event, MR_KEY_COMMAND);
                double pos = xpc_dictionary_get_double(event, MR_KEY_POSITION_TIME);
                double interval = xpc_dictionary_get_double(event, MR_KEY_INTERVAL);
                if (cmd) {
                    [self handleRemoteCommand:[NSString stringWithUTF8String:cmd]
                                 positionTime:pos
                                     interval:interval];
                }
            }
        } else if (event == XPC_ERROR_CONNECTION_INVALID || event == XPC_ERROR_CONNECTION_INTERRUPTED) {
            // The service went away; the next change connects again and
            // resends everything.
            if (_connection) {
                xpc_connection_cancel(_connection);
                xpc_release(_connection);
                _connection = NULL;
            }
        }
    });

    xpc_connection_resume(_connection);

    xpc_object_t checkin = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(checkin, MR_KEY_OP, MR_OP_CHECKIN);
    [self addIdentityToDictionary:checkin];
    xpc_connection_send_message(_connection, checkin);
    xpc_release(checkin);
    return YES;
}

- (void) addIdentityToDictionary: (xpc_object_t) dict {
    xpc_dictionary_set_string(dict, MR_KEY_APP_NAME, [[[NSProcessInfo processInfo] processName] UTF8String]);
    NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
    if (bundleId)
        xpc_dictionary_set_string(dict, MR_KEY_BUNDLE_ID, [bundleId UTF8String]);
}

// The remote command handlers run on the main thread.
- (void) handleRemoteCommand: (NSString *) cmdName positionTime: (double) posTime interval: (double) interval {
    dispatch_async(dispatch_get_main_queue(), ^{
        MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
        MPNowPlayingPlaybackState state = [MPNowPlayingInfoCenter defaultCenter].playbackState;
        BOOL playing = (state == MPNowPlayingPlaybackStatePlaying);
        MPRemoteCommand *cmd = nil;
        MPRemoteCommandEvent *evt = nil;
        NSTimeInterval now = [[NSDate date] timeIntervalSince1970];

        if ([cmdName isEqualToString:@"play"]) {
            if ([self commandIsAvailable:center.playCommand])
                cmd = center.playCommand;
            else if (!playing && [self commandIsAvailable:center.togglePlayPauseCommand])
                cmd = center.togglePlayPauseCommand;
        } else if ([cmdName isEqualToString:@"pause"]) {
            if ([self commandIsAvailable:center.pauseCommand])
                cmd = center.pauseCommand;
            else if (playing && [self commandIsAvailable:center.togglePlayPauseCommand])
                cmd = center.togglePlayPauseCommand;
        } else if ([cmdName isEqualToString:@"togglePlayPause"]) {
            if ([self commandIsAvailable:center.togglePlayPauseCommand])
                cmd = center.togglePlayPauseCommand;
            else
                cmd = playing ? center.pauseCommand : center.playCommand;
        } else if ([cmdName isEqualToString:@"stop"]) {
            cmd = center.stopCommand;
        } else if ([cmdName isEqualToString:@"nextTrack"]) {
            cmd = center.nextTrackCommand;
        } else if ([cmdName isEqualToString:@"previousTrack"]) {
            cmd = center.previousTrackCommand;
        } else if ([cmdName isEqualToString:@"skipForward"] || [cmdName isEqualToString:@"skipBackward"]) {
            cmd = [cmdName isEqualToString:@"skipForward"] ? center.skipForwardCommand : center.skipBackwardCommand;
            evt = [[[MPSkipIntervalCommandEvent alloc] initWithCommand:cmd timestamp:now interval:interval] autorelease];
        } else if ([cmdName isEqualToString:@"changePlaybackPosition"]) {
            cmd = center.changePlaybackPositionCommand;
            evt = [[[MPChangePlaybackPositionCommandEvent alloc] initWithCommand:cmd timestamp:now positionTime:posTime] autorelease];
        }

        if (cmd != nil) {
            if (evt == nil)
                evt = [[[MPRemoteCommandEvent alloc] initWithCommand:cmd timestamp:now] autorelease];
            [cmd invokeWithEvent:evt];
        }
    });
}

- (BOOL) commandIsAvailable: (MPRemoteCommand *) cmd {
    return cmd.enabled && [cmd hasTargets];
}

// Called from any thread, right after nowPlayingInfo was set. The time is the
// one the elapsed playback time is relative to.
- (void) nowPlayingInfoChanged {
    @synchronized(self) {
        _infoTimestamp = [[NSDate date] timeIntervalSince1970];
    }
    [self playbackStateChanged];
}

- (void) playbackStateChanged {
    dispatch_async(_queue, ^{
        [self ensureConnection];
        if (_connection)
            [self sendSnapshot];
    });
}

- (void) commandsChanged {
    dispatch_async(_queue, ^{
        if ([self ensureConnection]) {
            [self sendSnapshot];
        } else if (_connection) {
            xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
            xpc_dictionary_set_string(dict, MR_KEY_OP, MR_OP_UPDATE_COMMANDS);
            xpc_dictionary_set_uint64(dict, MR_KEY_ENABLED_COMMANDS, [self enabledCommandsMask]);
            xpc_connection_send_message(_connection, dict);
            xpc_release(dict);
        }
    });
}

- (uint64_t) enabledCommandsMask {
    MPRemoteCommandCenter *center = [MPRemoteCommandCenter sharedCommandCenter];
    uint64_t mask = 0;
#define ADD(cmd, bit) if ([self commandIsAvailable:center.cmd]) mask |= bit
    ADD(playCommand, MR_CMD_PLAY);
    ADD(pauseCommand, MR_CMD_PAUSE);
    ADD(stopCommand, MR_CMD_STOP);
    ADD(togglePlayPauseCommand, MR_CMD_TOGGLE_PLAY_PAUSE);
    ADD(nextTrackCommand, MR_CMD_NEXT_TRACK);
    ADD(previousTrackCommand, MR_CMD_PREVIOUS_TRACK);
    ADD(changePlaybackRateCommand, MR_CMD_CHANGE_PLAYBACK_RATE);
    ADD(changeRepeatModeCommand, MR_CMD_CHANGE_REPEAT_MODE);
    ADD(changeShuffleModeCommand, MR_CMD_CHANGE_SHUFFLE_MODE);
    ADD(skipForwardCommand, MR_CMD_SKIP_FORWARD);
    ADD(skipBackwardCommand, MR_CMD_SKIP_BACKWARD);
    ADD(changePlaybackPositionCommand, MR_CMD_CHANGE_PLAYBACK_POSITION);
    ADD(ratingCommand, MR_CMD_RATING);
    ADD(likeCommand, MR_CMD_LIKE);
    ADD(dislikeCommand, MR_CMD_DISLIKE);
    ADD(bookmarkCommand, MR_CMD_BOOKMARK);
#undef ADD
    return mask;
}

// Only called on _queue, with a connection.
- (void) sendSnapshot {
    MPNowPlayingInfoCenter *center = [MPNowPlayingInfoCenter defaultCenter];
    NSDictionary *info = center.nowPlayingInfo;
    NSTimeInterval infoTimestamp;
    @synchronized(self) {
        infoTimestamp = _infoTimestamp;
    }

    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_string(dict, MR_KEY_OP, MR_OP_UPDATE_STATE);
    [self addIdentityToDictionary:dict];
    xpc_dictionary_set_uint64(dict, MR_KEY_PLAYBACK_STATE, center.playbackState);
    xpc_dictionary_set_bool(dict, MR_KEY_HAS_INFO, info != nil);
    xpc_dictionary_set_double(dict, MR_KEY_INFO_TIMESTAMP, infoTimestamp);

    id value;
    if ([(value = info[MPMediaItemPropertyTitle]) isKindOfClass:[NSString class]])
        xpc_dictionary_set_string(dict, MR_KEY_TITLE, [value UTF8String]);
    if ([(value = info[MPMediaItemPropertyArtist]) isKindOfClass:[NSString class]])
        xpc_dictionary_set_string(dict, MR_KEY_ARTIST, [value UTF8String]);
    if ([(value = info[MPMediaItemPropertyAlbumTitle]) isKindOfClass:[NSString class]])
        xpc_dictionary_set_string(dict, MR_KEY_ALBUM, [value UTF8String]);
    if ([(value = info[MPMediaItemPropertyComposer]) isKindOfClass:[NSString class]])
        xpc_dictionary_set_string(dict, MR_KEY_COMPOSER, [value UTF8String]);
    if ([(value = info[MPMediaItemPropertyGenre]) isKindOfClass:[NSString class]])
        xpc_dictionary_set_string(dict, MR_KEY_GENRE, [value UTF8String]);
    if ([(value = info[MPMediaItemPropertyAlbumTrackNumber]) isKindOfClass:[NSNumber class]])
        xpc_dictionary_set_int64(dict, MR_KEY_ALBUM_TRACK_NUMBER, [value integerValue]);
    if ([(value = info[MPMediaItemPropertyDiscNumber]) isKindOfClass:[NSNumber class]])
        xpc_dictionary_set_int64(dict, MR_KEY_DISC_NUMBER, [value integerValue]);
    if ([(value = info[MPMediaItemPropertyPlaybackDuration]) isKindOfClass:[NSNumber class]])
        xpc_dictionary_set_double(dict, MR_KEY_DURATION, [value doubleValue]);
    if ([(value = info[MPNowPlayingInfoPropertyElapsedPlaybackTime]) isKindOfClass:[NSNumber class]])
        xpc_dictionary_set_double(dict, MR_KEY_ELAPSED_TIME, [value doubleValue]);
    if ([(value = info[MPNowPlayingInfoPropertyPlaybackRate]) isKindOfClass:[NSNumber class]])
        xpc_dictionary_set_double(dict, MR_KEY_RATE, [value doubleValue]);

    NSString *artworkURL = [self artworkURLForArtwork:info[MPMediaItemPropertyArtwork]];
    if (artworkURL)
        xpc_dictionary_set_string(dict, MR_KEY_ARTWORK_URL, [artworkURL UTF8String]);

    xpc_dictionary_set_uint64(dict, MR_KEY_ENABLED_COMMANDS, [self enabledCommandsMask]);
    xpc_connection_send_message(_connection, dict);
    xpc_release(dict);
}

// The artwork is handed to the desktop as a PNG file on the host, which is
// written once per artwork object and removed when the artwork goes away.
// Only called on _queue.
- (NSString *) artworkURLForArtwork: (id) artwork {
    if (![artwork isKindOfClass:[MPMediaItemArtwork class]]) {
        [self removeArtworkFile];
        return nil;
    }
    if (artwork == _artwork)
        return _artworkURL;

    [self removeArtworkFile];
    _artwork = [artwork retain];

    MPMediaItemArtwork *art = (MPMediaItemArtwork *)artwork;
    NSImage *img = [art imageWithSize:art.bounds.size];
    if (img == nil)
        return nil;

    NSData *png = nil;
    for (id rep in [img representations]) {
        if ([rep respondsToSelector:@selector(representationUsingType:properties:)]) {
            png = [rep representationUsingType:NSPNGFileType properties:@{}];
            if (png)
                break;
        }
    }
    if (png == nil) {
        NSData *tiff = [img TIFFRepresentation];
        NSBitmapImageRep *rep = tiff ? [NSBitmapImageRep imageRepWithData:tiff] : nil;
        png = [rep representationUsingType:NSPNGFileType properties:@{}];
    }
    if (png == nil)
        return nil;

    // The prefix sees the host's /tmp as /Volumes/SystemRoot/tmp; the desktop
    // wants the host path.
    NSString *hostTmp = @"/Volumes/SystemRoot/tmp";
    BOOL isDirectory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath:hostTmp isDirectory:&isDirectory] || !isDirectory)
        return nil;
    NSString *name = [NSString stringWithFormat:@"darling_art_%d_%u.png", getpid(), ++_artworkSerial];
    NSString *path = [hostTmp stringByAppendingPathComponent:name];
    if (![png writeToFile:path atomically:YES])
        return nil;

    _artworkPath = [path retain];
    _artworkURL = [[NSString stringWithFormat:@"file:///tmp/%@", name] retain];
    return _artworkURL;
}

- (void) removeArtworkFile {
    if (_artworkPath)
        [[NSFileManager defaultManager] removeItemAtPath:_artworkPath error:NULL];
    [_artworkPath release];
    _artworkPath = nil;
    [_artworkURL release];
    _artworkURL = nil;
    [_artwork release];
    _artwork = nil;
}

@end

// Removes the artwork file of this process when it exits; nothing else is
// using the manager by then.
static void MPMediaRemoteAtExit(void) {
    [[_MPMediaRemoteManager sharedManager] removeArtworkFile];
}

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
    [[_MPMediaRemoteManager sharedManager] nowPlayingInfoChanged];
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
    [[_MPMediaRemoteManager sharedManager] playbackStateChanged];
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
    [[_MPMediaRemoteManager sharedManager] commandsChanged];
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
    [[_MPMediaRemoteManager sharedManager] commandsChanged];
}

- (id) addTargetWithHandler: (MPRemoteCommandHandlerStatus (^)(MPRemoteCommandEvent *event)) handler {
    _MPRemoteCommandTarget *entry = [[_MPRemoteCommandTarget alloc] init];
    entry->handler = [handler copy];
    @synchronized(self) {
        [_targets addObject: entry];
    }
    id token = [entry autorelease];
    [[_MPMediaRemoteManager sharedManager] commandsChanged];
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
    [[_MPMediaRemoteManager sharedManager] commandsChanged];
}

// A nil target removes every registered target; a handler token removes its
// handler.
- (void) removeTarget: (id) target {
    [self removeTarget: target action: NULL];
}

- (MPRemoteCommandHandlerStatus) invokeWithEvent: (MPRemoteCommandEvent *) event {
    if (![self isEnabled]) {
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
            // The action returns an MPRemoteCommandHandlerStatus.
            MPRemoteCommandHandlerStatus s = ((MPRemoteCommandHandlerStatus (*)(id, SEL, id)) objc_msgSend)(entry->target, entry->action, event);
            if (s == MPRemoteCommandHandlerStatusSuccess)
                result = MPRemoteCommandHandlerStatusSuccess;
            else if (result != MPRemoteCommandHandlerStatusSuccess)
                result = s;
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

@implementation MPSkipIntervalCommandEvent

@synthesize interval = _interval;

- (instancetype) initWithCommand: (MPRemoteCommand *) command timestamp: (NSTimeInterval) timestamp interval: (NSTimeInterval) interval {
    self = [super initWithCommand: command timestamp: timestamp];
    if (self) {
        _interval = interval;
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
