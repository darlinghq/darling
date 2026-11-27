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
#import <CoreVideo/CVBase.h>
#import <AVFoundation/AVPlayerItemOutput.h>

@class AVPlayerItemVideoOutputInternal;
@protocol AVPlayerItemOutputPullDelegate;

@interface AVPlayerItemVideoOutput : AVPlayerItemOutput {
@private
    AVPlayerItemVideoOutputInternal *_videoOutputInternal;
}

@property (nonatomic, readonly, weak) id<AVPlayerItemOutputPullDelegate> delegate;
@property (nonatomic, readonly) dispatch_queue_t delegateQueue;

- (instancetype)initWithPixelBufferAttributes:(NSDictionary<NSString *, id> *)pixelBufferAttributes;
- (instancetype)initWithOutputSettings:(NSDictionary<NSString *, id> *)outputSettings;
- (BOOL)hasNewPixelBufferForItemTime:(CMTime)itemTime;
- (CVPixelBufferRef)copyPixelBufferForItemTime:(CMTime)itemTime itemTimeForDisplay:(CMTime *)outItemTimeForDisplay CF_RETURNS_RETAINED;
- (CMTime)itemTimeForHostTime:(CFTimeInterval)hostTimeInSeconds;
- (CMTime)itemTimeForCVTimeStamp:(CVTimeStamp)timestamp;
- (void)setDelegate:(id<AVPlayerItemOutputPullDelegate>)delegate queue:(dispatch_queue_t)delegateQueue;
- (void)requestNotificationOfMediaDataChangeWithAdvanceInterval:(NSTimeInterval)interval;

@end

@protocol AVPlayerItemOutputPullDelegate <NSObject>
@optional
- (void)outputMediaDataWillChange:(AVPlayerItemOutput *)sender;
- (void)outputSequenceWasFlushed:(AVPlayerItemOutput *)output;
@end
