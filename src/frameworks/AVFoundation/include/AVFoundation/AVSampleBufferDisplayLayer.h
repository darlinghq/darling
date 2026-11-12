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
#import <QuartzCore/CALayer.h>
#import <QuartzCore/CATransaction.h>
#import <AVFoundation/AVAnimation.h>
#import <AVFoundation/AVQueuedSampleBufferRendering.h>

@interface AVSampleBufferDisplayLayer : CALayer <AVQueuedSampleBufferRendering>

@property (retain) __attribute__((NSObject)) CMTimebaseRef controlTimebase;
@property (copy) AVLayerVideoGravity videoGravity;
@property (readonly) AVQueuedSampleBufferRenderingStatus status;
@property (readonly) NSError *error;
@property (readonly) BOOL requiresFlushToResumeDecoding;
@property (nonatomic) BOOL preventsCapture;
@property (nonatomic) BOOL preventsDisplaySleepDuringVideoPlayback;
- (void)flushAndRemoveImage;

@end

extern NSNotificationName const AVSampleBufferDisplayLayerFailedToDecodeNotification;
extern NSString *const AVSampleBufferDisplayLayerFailedToDecodeNotificationErrorKey;
extern NSNotificationName const AVSampleBufferDisplayLayerRequiresFlushToResumeDecodingDidChangeNotification;
