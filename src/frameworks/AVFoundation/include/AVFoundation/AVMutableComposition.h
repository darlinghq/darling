/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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
#import <AVFoundation/AVComposition.h>
#import <AVFoundation/AVMutableCompositionTrack.h>

@interface AVMutableComposition : AVComposition

@property (nonatomic) CGSize naturalSize;

+ (instancetype)composition;

- (AVMutableCompositionTrack *)addMutableTrackWithMediaType:(AVMediaType)mediaType
                                           preferredTrackID:(CMPersistentTrackID)preferredTrackID;
- (void)removeTrack:(AVCompositionTrack *)track;
- (AVMutableCompositionTrack *)mutableTrackCompatibleWithTrack:(AVAssetTrack *)track;
- (AVMutableCompositionTrack *)trackWithTrackID:(CMPersistentTrackID)trackID;
- (NSArray<AVMutableCompositionTrack *> *)tracksWithMediaType:(AVMediaType)mediaType;

- (BOOL)insertTimeRange:(CMTimeRange)timeRange
                ofAsset:(AVAsset *)asset
                 atTime:(CMTime)startTime
                  error:(NSError **)outError;
- (void)insertEmptyTimeRange:(CMTimeRange)timeRange;
- (void)removeTimeRange:(CMTimeRange)timeRange;
- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration;

@end
