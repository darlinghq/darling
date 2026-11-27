/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef _VTFRAMEPROCESSOR_H_
#define _VTFRAMEPROCESSOR_H_

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreMedia/CMBase.h>
#import <VideoToolbox/VTFrameProcessorConfiguration.h>
#import <VideoToolbox/VTFrameProcessorParameters.h>
#import <VideoToolbox/VTFrameProcessorFrame.h>
#import <VideoToolbox/VTFrameProcessorErrors.h>
#import <VideoToolbox/VTFrameProcessor_TemporalNoiseFilter.h>

@protocol MTLCommandBuffer;

// There is no frame processing here: starting a session fails with
// VTFrameProcessorInitializationFailed and processing without a session fails with
// VTFrameProcessorSessionNotStarted.
@interface VTFrameProcessor : NSObject
- (instancetype)init;
- (BOOL)startSessionWithConfiguration:(id<VTFrameProcessorConfiguration>)configuration error:(NSError **)error;
- (BOOL)processWithParameters:(id<VTFrameProcessorParameters>)parameters error:(NSError **)error;
- (void)processWithParameters:(id<VTFrameProcessorParameters>)parameters
            completionHandler:(void (^)(id<VTFrameProcessorParameters> _Nullable, NSError * _Nullable))completionHandler;
- (void)processWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer parameters:(id<VTFrameProcessorParameters>)parameters;
- (void)endSession;
@end
#endif

#endif
