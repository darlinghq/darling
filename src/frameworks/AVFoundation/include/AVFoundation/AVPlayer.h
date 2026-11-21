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
#import <CoreMedia/CMTime.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreGraphics/CGImage.h>

@class AVPlayerItem;
@class AVPlayerLayer;

typedef NS_ENUM(NSInteger, AVPlayerStatus) {
    AVPlayerStatusUnknown = 0,
    AVPlayerStatusReadyToPlay = 1,
    AVPlayerStatusFailed = 2
};

typedef NS_ENUM(NSInteger, AVPlayerActionAtItemEnd) {
    AVPlayerActionAtItemEndAdvance = 0,
    AVPlayerActionAtItemEndPause = 1,
    AVPlayerActionAtItemEndNone = 2
};

@interface AVPlayer : NSObject {
@protected
    AVPlayerItem *_currentItem;
    AVPlayerStatus _status;
    NSError *_error;
    float _rate;
    float _volume;
    BOOL _muted;
    AVPlayerActionAtItemEnd _actionAtItemEnd;

    void *_playerEngine;
}

+ (instancetype)playerWithURL:(NSURL *)URL;
+ (instancetype)playerWithPlayerItem:(AVPlayerItem *)item;
- (instancetype)initWithURL:(NSURL *)URL;
- (instancetype)initWithPlayerItem:(AVPlayerItem *)item;

@property (readonly) AVPlayerStatus status;
@property (readonly) NSError *error;
@property (readonly) AVPlayerItem *currentItem;
@property float rate;
@property float volume;
@property (getter=isMuted) BOOL muted;
@property AVPlayerActionAtItemEnd actionAtItemEnd;

- (void)replaceCurrentItemWithPlayerItem:(AVPlayerItem *)item;
- (void)play;
- (void)pause;
- (CMTime)currentTime;
- (void)seekToTime:(CMTime)time;
- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler;

- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(dispatch_queue_t)queue usingBlock:(void (^)(CMTime time))block;
- (void)removeTimeObserver:(id)observer;

- (void)_registerPlayerLayer:(AVPlayerLayer *)layer;
- (void)_unregisterPlayerLayer:(AVPlayerLayer *)layer;
- (CGImageRef)_copyCurrentFrameImage;
- (void)_deliverFrameToLayers:(CGImageRef)image;
- (void)_didReachEOF;
- (void)_fireTimeObservers:(CMTime)time;

@end
