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
#import <CoreMedia/CMTimeRange.h>
#import <CoreGraphics/CGGeometry.h>

@class AVAsset;

// Private state of the public class of the same name without "Internal".
@interface AVAssetTrackInternal : NSObject {
@public
    __unsafe_unretained AVAsset *asset;
    CMPersistentTrackID trackID;
    NSString *mediaType;
    CGSize naturalSize;
    float nominalFrameRate;
    CMTimeRange timeRange;
}

@end
