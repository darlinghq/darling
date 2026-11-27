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
#import <CoreGraphics/CGAffineTransform.h>
#import <AVFoundation/AVAsynchronousKeyValueLoading.h>
#import <AVFoundation/AVMediaFormat.h>

@class AVAsset;
@class AVAssetTrackInternal;

@interface AVAssetTrack : NSObject <AVAsynchronousKeyValueLoading> {
@private
    AVAssetTrackInternal *_assetTrack;
}

@property (nonatomic, readonly, assign) AVAsset *asset;
@property (nonatomic, readonly) CMPersistentTrackID trackID;
@property (nonatomic, readonly) AVMediaType mediaType;
@property (nonatomic, readonly) CGSize naturalSize;
@property (nonatomic, readonly) float nominalFrameRate;
@property (nonatomic, readonly) CMTimeRange timeRange;
@property (nonatomic, readonly, getter=isEnabled) BOOL enabled;
@property (nonatomic, readonly, getter=isPlayable) BOOL playable;
@property (nonatomic, readonly) NSArray *formatDescriptions;
@property (nonatomic, readonly) CMTime minFrameDuration;
@property (nonatomic, readonly) CGAffineTransform preferredTransform;

@end
