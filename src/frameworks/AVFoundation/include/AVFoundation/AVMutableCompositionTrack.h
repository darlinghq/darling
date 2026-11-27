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
#import <CoreGraphics/CGAffineTransform.h>
#import <AVFoundation/AVCompositionTrack.h>

@interface AVMutableCompositionTrack : AVCompositionTrack {
@private
    CMTimeScale _naturalTimeScale;
    NSString *_languageCode;
    NSString *_extendedLanguageTag;
    CGAffineTransform _preferredTransform;
    float _preferredVolume;
}

@property (nonatomic) CMTimeScale naturalTimeScale;
@property (nonatomic, copy) NSString *languageCode;
@property (nonatomic, copy) NSString *extendedLanguageTag;
@property (nonatomic) CGAffineTransform preferredTransform;
@property (nonatomic) float preferredVolume;
@property (nonatomic, copy) NSArray<AVCompositionTrackSegment *> *segments;

- (BOOL)insertTimeRange:(CMTimeRange)timeRange
                ofTrack:(AVAssetTrack *)track
                 atTime:(CMTime)startTime
                  error:(NSError **)outError;
- (void)insertEmptyTimeRange:(CMTimeRange)timeRange;
- (void)removeTimeRange:(CMTimeRange)timeRange;
- (void)scaleTimeRange:(CMTimeRange)timeRange toDuration:(CMTime)duration;

@end
