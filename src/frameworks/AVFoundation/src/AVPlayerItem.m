/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel
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

#import "AVPlaybackPrivate.h"
#import <AVFoundation/AVURLAsset.h>
#import <darling-stub-forwarding.h>

@implementation AVPlayerItem

+ (instancetype)playerItemWithURL:(NSURL *)URL
{
    return [[[self alloc] initWithURL:URL] autorelease];
}

+ (instancetype)playerItemWithAsset:(AVAsset *)asset
{
    return [[[self alloc] initWithAsset:asset] autorelease];
}

- (instancetype)initWithURL:(NSURL *)URL
{
    return [self initWithAsset:[AVURLAsset URLAssetWithURL:URL options:nil]];
}

- (instancetype)initWithAsset:(AVAsset *)asset
{
    if ((self = [super init])) {
        _playerItem = [[AVPlayerItemInternal alloc] init];
        _playerItem->asset = [asset retain];
        _playerItem->status = AVPlayerItemStatusUnknown;
        _playerItem->outputs = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    for (AVPlayerItemOutput *output in [[_playerItem->outputs copy] autorelease])
        [self removeOutput:output];
    [_playerItem->audioTimePitchAlgorithm release];
    [_playerItem release];
    [super dealloc];
}

- (AVPlayerItemInternal *)_internal { return _playerItem; }

- (void)_setStatus:(AVPlayerItemStatus)status error:(NSError *)error
{
    [self willChangeValueForKey:@"status"];
    [self willChangeValueForKey:@"error"];
    _playerItem->status = status;
    [_playerItem->error release];
    _playerItem->error = [error retain];
    [self didChangeValueForKey:@"error"];
    [self didChangeValueForKey:@"status"];
}

- (AVPlayerItemStatus)status { return (AVPlayerItemStatus)_playerItem->status; }
- (NSError *)error { return _playerItem->error; }
- (AVAsset *)asset { return _playerItem->asset; }
- (CMTime)duration { return [_playerItem->asset duration]; }
- (CGSize)presentationSize { return [_playerItem->asset naturalSize]; }
- (NSArray *)tracks { return [NSArray array]; }
- (NSArray<AVPlayerItemOutput *> *)outputs { return [[_playerItem->outputs copy] autorelease]; }

// QUIRK: any string is accepted and read back as given, also one that is not an algorithm; nil restores the
// default, Spectral. The audio is processed with the algorithm the string names (an unknown one plays like
// Spectral, the deprecated LowQualityZeroLatency like TimeDomain).
// Not documented. Evidence: black-box observation on macOS 26.7.3 x86_64 and 10.14.4 (tier 2).
// Test: tests/avplayer-playback.m, "audioTimePitchAlgorithm".
- (AVAudioTimePitchAlgorithm)audioTimePitchAlgorithm
{
    NSString *value;
    @synchronized (self) {
        value = [[_playerItem->audioTimePitchAlgorithm retain] autorelease];
    }
    return value ?: AVAudioTimePitchAlgorithmSpectral;
}

- (void)setAudioTimePitchAlgorithm:(AVAudioTimePitchAlgorithm)algorithm
{
    @synchronized (self) {
        if (_playerItem->audioTimePitchAlgorithm != algorithm && ![_playerItem->audioTimePitchAlgorithm isEqualToString:algorithm]) {
            [_playerItem->audioTimePitchAlgorithm release];
            _playerItem->audioTimePitchAlgorithm = [algorithm copy];
        }
    }
    AVPlayer *player = _playerItem->player;
    if (player)
        [[player _internal] updateAudioTimePitchAlgorithm];
}

- (CMTime)currentTime
{
    AVPlayer *player = _playerItem->player;
    return player ? [player currentTime] : kCMTimeZero;
}

- (void)seekToTime:(CMTime)time
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler
{
    AVPlayer *player = _playerItem->player;
    if (player) {
        [player seekToTime:time completionHandler:completionHandler];
    } else if (completionHandler) {
        void (^handler)(BOOL) = [completionHandler copy];
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{ handler(NO); [handler release]; });
    }
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler
{
    [self seekToTime:time completionHandler:completionHandler];
}

- (void)stepByCount:(NSInteger)stepCount
{
    float rate = 30.0f;
    for (AVAssetTrack *track in [_playerItem->asset tracksWithMediaType:AVMediaTypeVideo]) {
        if ([track nominalFrameRate] > 0) {
            rate = [track nominalFrameRate];
            break;
        }
    }
    double seconds = CMTimeGetSeconds([self currentTime]) + (double)stepCount / rate;
    double duration = CMTimeGetSeconds([self duration]);
    if (seconds < 0)
        seconds = 0;
    if (duration > 0 && seconds > duration)
        seconds = duration;
    [self seekToTime:CMTimeMakeWithSeconds(seconds, 1000000)];
}

- (void)addOutput:(AVPlayerItemOutput *)output
{
    if ([output isKindOfClass:[AVPlayerItemVideoOutput class]] && ![_playerItem->outputs containsObject:output]) {
        [_playerItem->outputs addObject:output];
        [(AVPlayerItemVideoOutput *)output _setItem:self];
    }
}

- (void)removeOutput:(AVPlayerItemOutput *)output
{
    if ([_playerItem->outputs containsObject:output]) {
        [(AVPlayerItemVideoOutput *)output _setItem:nil];
        [_playerItem->outputs removeObject:output];
    }
}

- (AVKeyValueStatus)statusOfValueForKey:(NSString *)key error:(NSError **)outError
{
    return AVPlaybackStatusOfKey(self, key, outError);
}

- (void)loadValuesAsynchronouslyForKeys:(NSArray<NSString *> *)keys completionHandler:(void (^)(void))handler
{
    AVPlaybackLoadKeys(keys, handler);
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
