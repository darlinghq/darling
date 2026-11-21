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
#import <AVFoundation/AVMediaFormat.h>

@class AVAsset;

@interface AVAssetTrack : NSObject {
@protected
    __unsafe_unretained AVAsset *_asset;
    CMPersistentTrackID _trackID;
    AVMediaType _mediaType;
    CGSize _naturalSize;
    float _nominalFrameRate;
    CMTimeRange _timeRange;
    BOOL _enabled;
}

@property (readonly, assign) AVAsset *asset;
@property (readonly) CMPersistentTrackID trackID;
@property (readonly) AVMediaType mediaType;
@property (readonly) CGSize naturalSize;
@property (readonly) float nominalFrameRate;
@property (readonly) CMTimeRange timeRange;
@property (readonly, getter=isEnabled) BOOL enabled;

- (instancetype)_initWithAsset:(AVAsset *)asset
                       trackID:(CMPersistentTrackID)trackID
                     mediaType:(AVMediaType)mediaType
                   naturalSize:(CGSize)naturalSize
              nominalFrameRate:(float)nominalFrameRate
                     timeRange:(CMTimeRange)timeRange;

@end
