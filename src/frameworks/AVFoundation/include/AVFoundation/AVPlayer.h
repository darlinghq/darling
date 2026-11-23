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

@class AVPlayerItem;
@class AVPlayerInternal;

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

typedef NS_ENUM(NSInteger, AVPlayerTimeControlStatus) {
    AVPlayerTimeControlStatusPaused = 0,
    AVPlayerTimeControlStatusWaitingToPlayAtSpecifiedRate = 1,
    AVPlayerTimeControlStatusPlaying = 2
};

typedef NSString *AVPlayerWaitingReason;
extern NSString *const AVPlayerWaitingToMinimizeStallsReason;
extern NSString *const AVPlayerWaitingWhileEvaluatingBufferingRateReason;
extern NSString *const AVPlayerWaitingWithNoItemToPlayReason;

@interface AVPlayer : NSObject {
@private
    AVPlayerInternal *_player;
}

+ (instancetype)playerWithURL:(NSURL *)URL;
+ (instancetype)playerWithPlayerItem:(AVPlayerItem *)item;
- (instancetype)initWithURL:(NSURL *)URL;
- (instancetype)initWithPlayerItem:(AVPlayerItem *)item;

@property (nonatomic, readonly) AVPlayerStatus status;
@property (nonatomic, readonly) NSError *error;
@property (nonatomic, readonly) AVPlayerItem *currentItem;
@property (nonatomic) float rate;
@property (nonatomic) float volume;
@property (nonatomic, getter=isMuted) BOOL muted;
@property (nonatomic) AVPlayerActionAtItemEnd actionAtItemEnd;
@property (nonatomic, readonly) AVPlayerTimeControlStatus timeControlStatus;
@property (nonatomic, readonly, copy) AVPlayerWaitingReason reasonForWaitingToPlay;

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

@end
