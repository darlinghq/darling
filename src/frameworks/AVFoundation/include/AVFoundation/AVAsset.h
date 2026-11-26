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
#import <CoreMedia/CMBase.h>
#import <CoreMedia/CMTime.h>
#import <CoreMedia/CMTimeRange.h>
#import <CoreGraphics/CGGeometry.h>
#import <AVFoundation/AVAsynchronousKeyValueLoading.h>
#import <AVFoundation/AVMediaFormat.h>

@class AVAssetTrack;
@class AVAssetInternal;

extern NSString *const AVURLAssetPreferPreciseDurationAndTimingKey;

@interface AVAsset : NSObject <AVAsynchronousKeyValueLoading> {
@private
    AVAssetInternal *_asset;
}

+ (instancetype)assetWithURL:(NSURL *)URL;

@property (nonatomic, readonly) CMTime duration;
@property (nonatomic, readonly) float preferredRate;
@property (nonatomic, readonly) float preferredVolume;
@property (nonatomic, readonly) CGSize naturalSize;
@property (nonatomic, readonly, getter=isPlayable) BOOL playable;
@property (nonatomic, readonly) NSArray<AVAssetTrack *> *tracks;

- (NSArray<AVAssetTrack *> *)tracksWithMediaType:(AVMediaType)mediaType;
- (NSArray<AVAssetTrack *> *)tracksWithMediaCharacteristic:(NSString *)mediaCharacteristic;
- (AVAssetTrack *)trackWithTrackID:(CMPersistentTrackID)trackID;

@end

// As in Apple's SDK, AVURLAsset comes with AVAsset.h.
#import <AVFoundation/AVURLAsset.h>
