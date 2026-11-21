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
#import <AVFoundation/AVAnimation.h>

@class AVPlayer;
@class AVPlayerLayerInternal;

@interface AVPlayerLayer : CALayer {
@private
    AVPlayerLayerInternal *_playerLayer;
}

+ (AVPlayerLayer *)playerLayerWithPlayer:(AVPlayer *)player;

@property (nonatomic, retain) AVPlayer *player;
@property (nonatomic, copy) AVLayerVideoGravity videoGravity;
@property (nonatomic, readonly, getter=isReadyForDisplay) BOOL readyForDisplay;

@end
