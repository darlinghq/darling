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
#import <QuartzCore/CATransaction.h>
#import <darling-stub-forwarding.h>

@implementation AVPlayerLayer

+ (AVPlayerLayer *)playerLayerWithPlayer:(AVPlayer *)player
{
    AVPlayerLayer *layer = [[[self alloc] init] autorelease];
    [layer setPlayer:player];
    return layer;
}

- (instancetype)init
{
    if ((self = [super init])) {
        _playerLayer = [[AVPlayerLayerInternal alloc] init];
        _playerLayer->videoGravity = [AVLayerVideoGravityResizeAspect copy];
        self.contentsGravity = kCAGravityResizeAspect;
    }
    return self;
}

// Core Animation makes layers of this class for its own use (the presentation layer).
- (instancetype)initWithLayer:(id)layer
{
    if ((self = [super initWithLayer:layer])) {
        _playerLayer = [[AVPlayerLayerInternal alloc] init];
        AVPlayerLayerInternal *source = [layer isKindOfClass:[AVPlayerLayer class]] ? ((AVPlayerLayer *)layer)->_playerLayer : nil;
        _playerLayer->videoGravity = [(source ? source->videoGravity : AVLayerVideoGravityResizeAspect) copy];
        _playerLayer->readyForDisplay = source ? source->readyForDisplay : NO;
    }
    return self;
}

- (void)dealloc
{
    if (_playerLayer->player) {
        // Waits for a frame delivery that is using the layer right now.
        [[_playerLayer->player _internal] unregisterLayer:self];
    }
    [_playerLayer release];
    [super dealloc];
}

- (AVPlayer *)player { return _playerLayer->player; }

- (void)setPlayer:(AVPlayer *)player
{
    if (_playerLayer->player == player)
        return;
    [self willChangeValueForKey:@"player"];
    AVPlayer *old = _playerLayer->player;
    if (old)
        [[old _internal] unregisterLayer:self];
    _playerLayer->player = [player retain];
    [old release];
    if (player) {
        [[player _internal] registerLayer:self];
        CGImageRef current = [[player _internal] copyLatestImage];
        if (current) {
            [self _showImage:current];
            CGImageRelease(current);
        }
    }
    [self didChangeValueForKey:@"player"];
}

- (AVLayerVideoGravity)videoGravity { return _playerLayer->videoGravity; }

- (void)setVideoGravity:(AVLayerVideoGravity)gravity
{
    if (!gravity || [_playerLayer->videoGravity isEqualToString:gravity])
        return;
    [self willChangeValueForKey:@"videoGravity"];
    [_playerLayer->videoGravity release];
    _playerLayer->videoGravity = [gravity copy];
    if ([gravity isEqualToString:AVLayerVideoGravityResize])
        self.contentsGravity = kCAGravityResize;
    else if ([gravity isEqualToString:AVLayerVideoGravityResizeAspectFill])
        self.contentsGravity = kCAGravityResizeAspectFill;
    else
        self.contentsGravity = kCAGravityResizeAspect;
    [self didChangeValueForKey:@"videoGravity"];
}

- (BOOL)isReadyForDisplay { return _playerLayer->readyForDisplay; }

// Main thread, so that the layer tree is only changed from there.
- (void)_showImage:(CGImageRef)image
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    self.contents = (id)image;
    [CATransaction commit];
    if (!_playerLayer->readyForDisplay) {
        [self willChangeValueForKey:@"readyForDisplay"];
        _playerLayer->readyForDisplay = YES;
        [self didChangeValueForKey:@"readyForDisplay"];
    }
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
