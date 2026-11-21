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

#import <Foundation/Foundation.h>
#import <QuartzCore/CALayer.h>
#import <QuartzCore/CATransaction.h>
#import <AVFoundation/AVAnimation.h>
#import <CoreVideo/CoreVideo.h>

@class AVPlayer;

@interface AVPlayerLayer : CALayer {
@protected
    AVPlayer *_player;
    AVLayerVideoGravity _videoGravity;
    BOOL _readyForDisplay;
}

+ (instancetype)playerLayerWithPlayer:(AVPlayer *)player;

@property (retain) AVPlayer *player;
@property (copy) AVLayerVideoGravity videoGravity;
@property (readonly, getter=isReadyForDisplay) BOOL readyForDisplay;

- (void)_displayCGImage:(CGImageRef)image;

@end
