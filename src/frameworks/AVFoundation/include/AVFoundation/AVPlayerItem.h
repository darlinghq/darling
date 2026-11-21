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

@class AVAsset;

typedef NS_ENUM(NSInteger, AVPlayerItemStatus) {
    AVPlayerItemStatusUnknown = 0,
    AVPlayerItemStatusReadyToPlay = 1,
    AVPlayerItemStatusFailed = 2
};

extern NSString *const AVPlayerItemDidPlayToEndTimeNotification;
extern NSString *const AVPlayerItemFailedToPlayToEndTimeNotification;
extern NSString *const AVPlayerItemPlaybackStalledNotification;
extern NSString *const AVPlayerItemTimeJumpedNotification;

@interface AVPlayerItem : NSObject {
@protected
    AVAsset *_asset;
    AVPlayerItemStatus _status;
    NSError *_error;
    CMTime _duration;
    CGSize _presentationSize;
    CMTime _currentTime;
    NSMutableArray *_tracks;
}

+ (instancetype)playerItemWithURL:(NSURL *)URL;
+ (instancetype)playerItemWithAsset:(AVAsset *)asset;
- (instancetype)initWithURL:(NSURL *)URL;
- (instancetype)initWithAsset:(AVAsset *)asset;

@property (readonly) AVPlayerItemStatus status;
@property (readonly) NSError *error;
@property (nonatomic, readonly) AVAsset *asset;
@property (readonly) CMTime duration;
@property (readonly) CGSize presentationSize;
@property (readonly) NSArray *tracks;

- (CMTime)currentTime;
- (void)seekToTime:(CMTime)time;
- (void)seekToTime:(CMTime)time completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter;
- (void)seekToTime:(CMTime)time toleranceBefore:(CMTime)toleranceBefore toleranceAfter:(CMTime)toleranceAfter completionHandler:(void (^)(BOOL finished))completionHandler;
- (void)stepByCount:(NSInteger)stepCount;

- (void)_setCurrentTime:(CMTime)time;
- (void)_didPlayToEnd;

@end
