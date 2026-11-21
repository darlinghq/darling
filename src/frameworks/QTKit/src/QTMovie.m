/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
 Copyright (C) 2026 Darling Developers

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

#import <QTKit/QTMovie.h>
#import <QTKit/QTKit.h>
#import <QTKit/QTTimeValue.h>
#import <AVFoundation/AVFoundation.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSBitmapImageRep.h>
#import <darling-stub-forwarding.h>

@implementation QTMovie

+ (BOOL)canInitWithFile:(NSString *)fileName
{
    if (!fileName) return NO;
    return [[NSFileManager defaultManager] fileExistsAtPath:fileName];
}

+ (BOOL)canInitWithURL:(NSURL *)url
{
    if (!url) return NO;
    if ([url isFileURL])
        return [self canInitWithFile:[url path]];
    return YES;
}

+ (NSArray *)movieFileTypes:(QTIncludeFlags)types
{
    return [AVURLAsset audiovisualTypes];
}

+ (NSArray *)movieUnregisteredFileTypes:(QTIncludeFlags)types
{
    return [NSArray array];
}

+ (instancetype)movieWithFile:(NSString *)fileName error:(NSError **)errorPtr
{
    return [[[self alloc] initWithFile:fileName error:errorPtr] autorelease];
}

+ (instancetype)movieWithURL:(NSURL *)url error:(NSError **)errorPtr
{
    return [[[self alloc] initWithURL:url error:errorPtr] autorelease];
}

+ (instancetype)movieWithData:(NSData *)data error:(NSError **)errorPtr
{
    return [[[self alloc] initWithData:data error:errorPtr] autorelease];
}

+ (instancetype)movieWithAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    return [[[self alloc] initWithAttributes:attributes error:errorPtr] autorelease];
}

+ (instancetype)movie
{
    return [[[self alloc] init] autorelease];
}

- (instancetype)init
{
    if ((self = [super init])) {
        _attributes = [[NSMutableDictionary alloc] init];
        _volume = 1.0f;
        _rate = 0.0f;
        _muted = NO;
        _loadState = QTMovieLoadStateLoading;
    }
    return self;
}

- (instancetype)initWithAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    NSString *file = [attributes objectForKey:QTMovieFileNameAttribute];
    if (file) {
        return [self initWithFile:file error:errorPtr];
    }
    NSURL *url = [attributes objectForKey:QTMovieURLAttribute];
    if (url) {
        return [self initWithURL:url error:errorPtr];
    }
    NSData *data = [attributes objectForKey:QTMovieDataAttribute];
    if (data) {
        return [self initWithData:data error:errorPtr];
    }
    if (errorPtr) {
        *errorPtr = [NSError errorWithDomain:QTKitErrorDomain code:-2000 userInfo:@{ NSLocalizedDescriptionKey: @"Missing movie path or URL attribute" }];
    }
    [self release];
    return nil;
}

- (instancetype)initWithFile:(NSString *)fileName error:(NSError **)errorPtr
{
    if (!fileName) {
        if (errorPtr) {
            *errorPtr = [NSError errorWithDomain:QTKitErrorDomain code:-2000 userInfo:@{ NSLocalizedDescriptionKey: @"Movie file path is nil" }];
        }
        [self release];
        return nil;
    }
    NSURL *url = [NSURL fileURLWithPath:fileName];
    return [self initWithURL:url error:errorPtr];
}

- (instancetype)initWithData:(NSData *)data error:(NSError **)errorPtr
{
    if (!data) {
        if (errorPtr) {
            *errorPtr = [NSError errorWithDomain:QTKitErrorDomain code:-2000 userInfo:@{ NSLocalizedDescriptionKey: @"Movie data is nil" }];
        }
        [self release];
        return nil;
    }
    NSString *tempPath = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"qtkit_temp_%p.mov", data]];
    if (![data writeToFile:tempPath atomically:YES]) {
        if (errorPtr) {
            *errorPtr = [NSError errorWithDomain:QTKitErrorDomain code:-2000 userInfo:@{ NSLocalizedDescriptionKey: @"Could not write temporary movie data" }];
        }
        [self release];
        return nil;
    }
    return [self initWithFile:tempPath error:errorPtr];
}

- (instancetype)initWithURL:(NSURL *)url error:(NSError **)errorPtr
{
    if ((self = [self init])) {
        _url = [url copy];
        _filePath = [[url path] copy];

        AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
        if (![asset isPlayable]) {
            _loadState = QTMovieLoadStateError;
            if (errorPtr) {
                *errorPtr = [NSError errorWithDomain:QTKitErrorDomain
                                                code:-2000
                                            userInfo:@{ NSLocalizedDescriptionKey: @"Could not open movie file" }];
            }
            [self release];
            return nil;
        }

        _playerItem = [[AVPlayerItem playerItemWithAsset:asset] retain];
        _player = [[AVPlayer playerWithPlayerItem:_playerItem] retain];
        _loadState = QTMovieLoadStateComplete;

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_playerItemDidPlayToEnd:)
                                                     name:AVPlayerItemDidPlayToEndTimeNotification
                                                   object:_playerItem];

        dispatch_async(dispatch_get_main_queue(), ^{
            [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieLoadStateDidChangeNotification object:self];
        });
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [_player release];
    [_playerItem release];
    [_url release];
    [_filePath release];
    [_attributes release];
    [super dealloc];
}

- (void)_playerItemDidPlayToEnd:(NSNotification *)note
{
    _rate = 0.0f;
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieRateDidChangeNotification
                                                            object:self
                                                          userInfo:@{ QTMovieRateDidChangeNotificationParameter: [NSNumber numberWithFloat:0.0f] }];
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieDidEndNotification object:self];
    });
}

#pragma mark - Attributes

- (NSDictionary *)movieAttributes
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithDictionary:_attributes];
    if (_filePath) dict[QTMovieFileNameAttribute] = _filePath;
    if (_url) dict[QTMovieURLAttribute] = _url;
    dict[QTMovieDurationAttribute] = [NSValue valueWithQTTime:[self duration]];
    dict[QTMovieCurrentTimeAttribute] = [NSValue valueWithQTTime:[self currentTime]];
    dict[QTMovieRateAttribute] = [NSNumber numberWithFloat:[self rate]];
    dict[QTMovieVolumeAttribute] = [NSNumber numberWithFloat:[self volume]];
    dict[QTMovieMutedAttribute] = [NSNumber numberWithBool:[self muted]];
    dict[QTMovieNaturalSizeAttribute] = [NSValue valueWithSize:[self currentSize]];
    dict[QTMovieCurrentSizeAttribute] = [NSValue valueWithSize:[self currentSize]];
    dict[QTMovieHasVideoAttribute] = [NSNumber numberWithBool:([[[_playerItem asset] tracksWithMediaType:AVMediaTypeVideo] count] > 0)];
    dict[QTMovieHasAudioAttribute] = [NSNumber numberWithBool:([[[_playerItem asset] tracksWithMediaType:AVMediaTypeAudio] count] > 0)];
    dict[QTMovieLoadStateAttribute] = [NSNumber numberWithInteger:_loadState];
    dict[QTMovieIsActiveAttribute] = [NSNumber numberWithBool:YES];
    dict[QTMovieIsLinearAttribute] = [NSNumber numberWithBool:YES];
    dict[QTMovieEditableAttribute] = [NSNumber numberWithBool:NO];
    return dict;
}

- (void)setMovieAttributes:(NSDictionary *)attributes
{
    for (NSString *key in attributes) {
        [self setAttribute:attributes[key] forKey:key];
    }
}

- (id)attributeForKey:(NSString *)attributeKey
{
    if ([attributeKey isEqualToString:QTMovieFileNameAttribute]) return _filePath;
    if ([attributeKey isEqualToString:QTMovieURLAttribute]) return _url;
    if ([attributeKey isEqualToString:QTMovieDurationAttribute]) return [NSValue valueWithQTTime:[self duration]];
    if ([attributeKey isEqualToString:QTMovieCurrentTimeAttribute]) return [NSValue valueWithQTTime:[self currentTime]];
    if ([attributeKey isEqualToString:QTMovieRateAttribute]) return [NSNumber numberWithFloat:[self rate]];
    if ([attributeKey isEqualToString:QTMovieVolumeAttribute]) return [NSNumber numberWithFloat:[self volume]];
    if ([attributeKey isEqualToString:QTMovieMutedAttribute]) return [NSNumber numberWithBool:[self muted]];
    if ([attributeKey isEqualToString:QTMovieNaturalSizeAttribute]) return [NSValue valueWithSize:[self currentSize]];
    if ([attributeKey isEqualToString:QTMovieCurrentSizeAttribute]) return [NSValue valueWithSize:[self currentSize]];
    if ([attributeKey isEqualToString:QTMovieHasVideoAttribute]) return [NSNumber numberWithBool:([[[_playerItem asset] tracksWithMediaType:AVMediaTypeVideo] count] > 0)];
    if ([attributeKey isEqualToString:QTMovieHasAudioAttribute]) return [NSNumber numberWithBool:([[[_playerItem asset] tracksWithMediaType:AVMediaTypeAudio] count] > 0)];
    if ([attributeKey isEqualToString:QTMovieLoadStateAttribute]) return [NSNumber numberWithInteger:_loadState];
    if ([attributeKey isEqualToString:QTMovieIsActiveAttribute]) return [NSNumber numberWithBool:YES];
    if ([attributeKey isEqualToString:QTMovieIsLinearAttribute]) return [NSNumber numberWithBool:YES];
    if ([attributeKey isEqualToString:QTMovieEditableAttribute]) return [NSNumber numberWithBool:NO];
    return [_attributes objectForKey:attributeKey];
}

- (void)setAttribute:(id)value forKey:(NSString *)attributeKey
{
    if ([attributeKey isEqualToString:QTMovieCurrentTimeAttribute] && [value isKindOfClass:[NSValue class]]) {
        [self setCurrentTime:[(NSValue *)value QTTimeValue]];
    } else if ([attributeKey isEqualToString:QTMovieRateAttribute] && [value respondsToSelector:@selector(floatValue)]) {
        [self setRate:[value floatValue]];
    } else if ([attributeKey isEqualToString:QTMovieVolumeAttribute] && [value respondsToSelector:@selector(floatValue)]) {
        [self setVolume:[value floatValue]];
    } else if ([attributeKey isEqualToString:QTMovieMutedAttribute] && [value respondsToSelector:@selector(boolValue)]) {
        [self setMuted:[value boolValue]];
    } else if (value && attributeKey) {
        [_attributes setObject:value forKey:attributeKey];
    }
}

#pragma mark - Playback Controls

- (void)play
{
    [self setRate:1.0f];
}

- (void)stop
{
    [self setRate:0.0f];
}

- (float)rate
{
    return _player ? [_player rate] : _rate;
}

- (void)setRate:(float)rate
{
    _rate = rate;
    if (_player) {
        [_player setRate:rate];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieRateDidChangeNotification
                                                            object:self
                                                          userInfo:@{ QTMovieRateDidChangeNotificationParameter: [NSNumber numberWithFloat:rate] }];
    });
}

- (float)volume
{
    return _player ? [_player volume] : _volume;
}

- (void)setVolume:(float)volume
{
    _volume = volume;
    if (_player) {
        [_player setVolume:volume];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieVolumeDidChangeNotification object:self];
    });
}

- (BOOL)muted
{
    return _player ? [_player isMuted] : _muted;
}

- (void)setMuted:(BOOL)muted
{
    _muted = muted;
    if (_player) {
        [_player setMuted:muted];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieMutedDidChangeNotification object:self];
    });
}

- (QTTime)duration
{
    return _playerItem ? QTTimeFromCMTime([_playerItem duration]) : QTZeroTime;
}

- (QTTime)currentTime
{
    return _player ? QTTimeFromCMTime([_player currentTime]) : QTZeroTime;
}

- (void)setCurrentTime:(QTTime)time
{
    if (_player) {
        [_player seekToTime:CMTimeFromQTTime(time)];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:QTMovieTimeDidChangeNotification object:self];
    });
}

- (void)gotoBeginning
{
    [self setCurrentTime:QTZeroTime];
}

- (void)gotoEnd
{
    [self setCurrentTime:[self duration]];
}

- (void)stepForward
{
    if (_playerItem) {
        [_playerItem stepByCount:1];
    }
}

- (void)stepBackward
{
    if (_playerItem) {
        [_playerItem stepByCount:-1];
    }
}

#pragma mark - Frame Access & Geometry

- (NSSize)currentSize
{
    if (_playerItem && [_playerItem asset]) {
        CGSize s = [[_playerItem asset] naturalSize];
        return NSMakeSize(s.width, s.height);
    }
    return NSZeroSize;
}

- (NSImage *)currentFrameImage
{
    if (!_player) return nil;
    CGImageRef cg = [_player _copyCurrentFrameImage];
    if (!cg) return nil;

    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc] initWithCGImage:cg] autorelease];
    CGImageRelease(cg);
    if (!rep) return nil;

    NSImage *image = [[[NSImage alloc] initWithSize:NSMakeSize(rep.pixelsWide, rep.pixelsHigh)] autorelease];
    [image addRepresentation:rep];
    return image;
}

- (NSImage *)posterImage
{
    return [self currentFrameImage];
}

- (NSImage *)frameImageAtTime:(QTTime)time
{
    return [self frameImageAtTime:time withAttributes:nil error:NULL];
}

- (NSImage *)frameImageAtTime:(QTTime)time withAttributes:(NSDictionary *)attributes error:(NSError **)errorPtr
{
    [self setCurrentTime:time];
    return [self currentFrameImage];
}

- (BOOL)isIdling { return YES; }
- (void)setIdling:(BOOL)idling {}
- (BOOL)autoIdles { return YES; }
- (void)setAutoIdles:(BOOL)autoIdles {}

- (NSInteger)loadState { return _loadState; }
- (QTTimeRange)availableRanges { return QTMakeTimeRange(QTZeroTime, [self duration]); }
- (QTTimeRange)loadedRanges { return QTMakeTimeRange(QTZeroTime, [self duration]); }

- (AVPlayer *)_player { return _player; }

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
