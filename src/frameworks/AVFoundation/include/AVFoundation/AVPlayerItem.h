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
#import <CoreGraphics/CGGeometry.h>
#import <AVFoundation/AVAsynchronousKeyValueLoading.h>
#import <AVFoundation/AVAudioProcessingSettings.h>

@class AVAsset;
@class AVPlayerItemOutput;
@class AVPlayerItemInternal;

typedef NS_ENUM(NSInteger, AVPlayerItemStatus) {
    AVPlayerItemStatusUnknown = 0,
    AVPlayerItemStatusReadyToPlay = 1,
    AVPlayerItemStatusFailed = 2
};

extern NSString *const AVPlayerItemDidPlayToEndTimeNotification;
extern NSString *const AVPlayerItemFailedToPlayToEndTimeNotification;
extern NSString *const AVPlayerItemPlaybackStalledNotification;
extern NSString *const AVPlayerItemTimeJumpedNotification;

@interface AVPlayerItem : NSObject <AVAsynchronousKeyValueLoading> {
@private
    AVPlayerItemInternal *_playerItem;
}

+ (instancetype)playerItemWithURL:(NSURL *)URL;
+ (instancetype)playerItemWithAsset:(AVAsset *)asset;
- (instancetype)initWithURL:(NSURL *)URL;
- (instancetype)initWithAsset:(AVAsset *)asset;

@property (nonatomic, readonly) AVPlayerItemStatus status;
@property (nonatomic, readonly) NSError *error;
@property (nonatomic, readonly) AVAsset *asset;
@property (nonatomic, readonly) CMTime duration;
@property (nonatomic, readonly) CGSize presentationSize;
@property (nonatomic, readonly) NSArray *tracks;
@property (nonatomic, readonly) NSArray<AVPlayerItemOutput *> *outputs;
// How the audio is processed when the player plays at a rate other than 1. Spectral (the default) and
// TimeDomain keep the pitch, Varispeed lets it follow the rate.
@property (nonatomic, copy, null_resettable) AVAudioTimePitchAlgorithm audioTimePitchAlgorithm;

- (CMTime)currentTime;
- (void)seekToTime:(CMTime)time;
- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)stepByCount:(NSInteger)stepCount;

- (void)addOutput:(AVPlayerItemOutput *)output;
- (void)removeOutput:(AVPlayerItemOutput *)output;

@end
