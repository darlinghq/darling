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
#import <CoreVideo/CVPixelBuffer.h>
#import <AVFoundation/AVPlayerItemOutput.h>

@class AVPlayerItemVideoOutputInternal;

@interface AVPlayerItemVideoOutput : AVPlayerItemOutput {
@private
    AVPlayerItemVideoOutputInternal *_videoOutputInternal;
}

- (instancetype)initWithPixelBufferAttributes:(NSDictionary<NSString *, id> *)pixelBufferAttributes;
- (BOOL)hasNewPixelBufferForItemTime:(CMTime)itemTime;
- (CVPixelBufferRef)copyPixelBufferForItemTime:(CMTime)itemTime itemTimeForDisplay:(CMTime *)outItemTimeForDisplay CF_RETURNS_RETAINED;

@end
