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
#import <AVFoundation/AVQueuedSampleBufferRendering.h>
#import <CoreMedia/CMSync.h>

@interface AVSampleBufferRenderSynchronizer : NSObject

@property (nonatomic, readonly) CMTimebaseRef timebase;
@property (nonatomic) float rate;
- (CMTime)currentTime;
- (void)setRate:(float)rate time:(CMTime)time;
- (void)setRate:(float)rate time:(CMTime)time atHostTime:(CMTime)hostTime;
@property (nonatomic) BOOL delaysRateChangeUntilHasSufficientMediaData;

@property (nonatomic, readonly) NSArray<id<AVQueuedSampleBufferRendering>> *renderers;
- (void)addRenderer:(id<AVQueuedSampleBufferRendering>)renderer;
- (void)removeRenderer:(id<AVQueuedSampleBufferRendering>)renderer atTime:(CMTime)time
     completionHandler:(void (^)(BOOL didRemoveRenderer))completionHandler;

- (id)addPeriodicTimeObserverForInterval:(CMTime)interval queue:(dispatch_queue_t)queue
                              usingBlock:(void (^)(CMTime time))block;
- (id)addBoundaryTimeObserverForTimes:(NSArray<NSValue *> *)times queue:(dispatch_queue_t)queue
                           usingBlock:(void (^)(void))block;
- (void)removeTimeObserver:(id)observer;

@end

extern NSNotificationName const AVSampleBufferRenderSynchronizerRateDidChangeNotification;
