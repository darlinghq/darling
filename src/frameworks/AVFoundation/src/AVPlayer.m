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
#import <darling-stub-forwarding.h>

@implementation AVPlayer

+ (instancetype)playerWithURL:(NSURL *)URL
{
    return [[[self alloc] initWithURL:URL] autorelease];
}

+ (instancetype)playerWithPlayerItem:(AVPlayerItem *)item
{
    return [[[self alloc] initWithPlayerItem:item] autorelease];
}

- (instancetype)init
{
    return [self initWithPlayerItem:nil];
}

- (instancetype)initWithURL:(NSURL *)URL
{
    return [self initWithPlayerItem:[AVPlayerItem playerItemWithURL:URL]];
}

- (instancetype)initWithPlayerItem:(AVPlayerItem *)item
{
    if ((self = [super init])) {
        _player = [[AVPlayerInternal alloc] initWithPlayer:self];
        _player->automaticallyWaitsToMinimizeStalling = YES;
        [_player replaceItem:item];
    }
    return self;
}

- (void)dealloc
{
    [_player invalidate];
    [_player release];
    [super dealloc];
}

- (AVPlayerInternal *)_internal { return _player; }

- (AVPlayerStatus)status { return (AVPlayerStatus)_player->status; }
- (NSError *)error { return _player->error; }
- (AVPlayerItem *)currentItem { return _player->item; }
- (float)rate { return [_player rate]; }
- (void)setRate:(float)rate
{
    [self willChangeValueForKey:@"rate"];
    [self willChangeValueForKey:@"timeControlStatus"];
    [_player setRate:rate];
    [self didChangeValueForKey:@"timeControlStatus"];
    [self didChangeValueForKey:@"rate"];
}
- (AVPlayerTimeControlStatus)timeControlStatus { return (AVPlayerTimeControlStatus)[_player timeControlStatus]; }
// The wait for the audio to start is what AVFoundation calls evaluating the buffering rate.
- (AVPlayerWaitingReason)reasonForWaitingToPlay
{
    return [_player timeControlStatus] == AVPlayerTimeControlStatusWaitingToPlayAtSpecifiedRate
        ? AVPlayerWaitingWhileEvaluatingBufferingRateReason : nil;
}
- (void)play { [self setRate:1.0f]; }
- (void)pause { [self setRate:0.0f]; }
- (float)volume { return _player->volume; }
- (void)setVolume:(float)volume { _player->volume = volume; [_player updateVolume]; }
- (BOOL)isMuted { return _player->muted; }
- (void)setMuted:(BOOL)muted { _player->muted = muted; [_player updateVolume]; }
- (AVPlayerActionAtItemEnd)actionAtItemEnd { return (AVPlayerActionAtItemEnd)_player->actionAtItemEnd; }
- (void)setActionAtItemEnd:(AVPlayerActionAtItemEnd)action { _player->actionAtItemEnd = action; }
- (uint64_t)preferredVideoDecoderGPURegistryID { return _player->preferredVideoDecoderGPURegistryID; }
- (void)setPreferredVideoDecoderGPURegistryID:(uint64_t)registryID { _player->preferredVideoDecoderGPURegistryID = registryID; }
- (BOOL)automaticallyWaitsToMinimizeStalling { return _player->automaticallyWaitsToMinimizeStalling; }
- (void)setAutomaticallyWaitsToMinimizeStalling:(BOOL)waits { _player->automaticallyWaitsToMinimizeStalling = waits; }

- (void)replaceCurrentItemWithPlayerItem:(AVPlayerItem *)item
{
    [self willChangeValueForKey:@"currentItem"];
    [_player replaceItem:item];
    [self didChangeValueForKey:@"currentItem"];
}

- (CMTime)currentTime { return [_player currentTime]; }

- (void)seekToTime:(CMTime)time
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler
{
    [_player seekToTime:time completionHandler:completionHandler];
}

// Seeks are always exact: the frame shown is the one for the requested time, whatever tolerance is allowed.
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter
{
    [self seekToTime:time completionHandler:nil];
}

- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler
{
    [self seekToTime:time completionHandler:completionHandler];
}

- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(dispatch_queue_t)queue usingBlock:(void (^)(CMTime time))block
{
    return [_player addPeriodicObserverWithInterval:interval queue:queue block:block];
}

- (void)removeTimeObserver:(id)observer
{
    [_player removePeriodicObserver:observer];
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
