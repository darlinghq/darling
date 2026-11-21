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
#import <CoreMedia/CMSampleBuffer.h>
#import <CoreMedia/CMSync.h>
#import <dispatch/dispatch.h>

typedef NS_ENUM(NSInteger, AVQueuedSampleBufferRenderingStatus) {
    AVQueuedSampleBufferRenderingStatusUnknown,
    AVQueuedSampleBufferRenderingStatusRendering,
    AVQueuedSampleBufferRenderingStatusFailed,
};

@protocol AVQueuedSampleBufferRendering <NSObject>

@property (nonatomic, readonly) CMTimebaseRef timebase;
- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer;
- (void)flush;
@property (nonatomic, readonly, getter=isReadyForMoreMediaData) BOOL readyForMoreMediaData;
- (void)requestMediaDataWhenReadyOnQueue:(dispatch_queue_t)queue usingBlock:(void (^)(void))block;
- (void)stopRequestingMediaData;
@property (nonatomic, readonly) BOOL hasSufficientMediaDataForReliablePlaybackStart;

@end
