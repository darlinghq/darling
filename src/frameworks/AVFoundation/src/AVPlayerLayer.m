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

#import <AVFoundation/AVPlayerLayer.h>
#import <AVFoundation/AVPlayer.h>
#import <darling-stub-forwarding.h>

@implementation AVPlayerLayer

+ (instancetype)playerLayerWithPlayer:(AVPlayer *)player
{
    AVPlayerLayer *layer = [[[self alloc] init] autorelease];
    layer.player = player;
    return layer;
}

- (instancetype)init
{
    if ((self = [super init])) {
        _videoGravity = [AVLayerVideoGravityResizeAspect copy];
        self.contentsGravity = kCAGravityResizeAspect;
        _readyForDisplay = NO;
    }
    return self;
}

- (void)dealloc
{
    if (_player) {
        [_player _unregisterPlayerLayer:self];
        [_player release];
    }
    [_videoGravity release];
    [super dealloc];
}

- (AVPlayer *)player { return _player; }

- (void)setPlayer:(AVPlayer *)player
{
    if (_player != player) {
        if (_player) {
            [_player _unregisterPlayerLayer:self];
            [_player release];
        }
        _player = [player retain];
        if (_player) {
            [_player _registerPlayerLayer:self];
            CGImageRef current = [_player _copyCurrentFrameImage];
            if (current) {
                [self _displayCGImage:current];
                CGImageRelease(current);
            }
        }
    }
}

- (AVLayerVideoGravity)videoGravity { return _videoGravity; }

- (void)setVideoGravity:(AVLayerVideoGravity)gravity
{
    if (_videoGravity != gravity) {
        [_videoGravity release];
        _videoGravity = [gravity copy];

        if ([_videoGravity isEqualToString:AVLayerVideoGravityResize]) {
            self.contentsGravity = kCAGravityResize;
        } else if ([_videoGravity isEqualToString:AVLayerVideoGravityResizeAspectFill]) {
            self.contentsGravity = kCAGravityResizeAspectFill;
        } else {
            self.contentsGravity = kCAGravityResizeAspect;
        }
    }
}

- (BOOL)isReadyForDisplay { return _readyForDisplay; }

- (void)_displayCGImage:(CGImageRef)image
{
    if (!image) return;
    CGImageRetain(image);
    dispatch_async(dispatch_get_main_queue(), ^{
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        self.contents = (id)image;
        [CATransaction commit];
        _readyForDisplay = YES;
        CGImageRelease(image);
    });
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
