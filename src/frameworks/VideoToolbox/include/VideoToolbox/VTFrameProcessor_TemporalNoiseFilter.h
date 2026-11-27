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

#ifndef _VTFRAMEPROCESSOR_TEMPORALNOISEFILTER_H_
#define _VTFRAMEPROCESSOR_TEMPORALNOISEFILTER_H_

#ifdef __OBJC__
#import <VideoToolbox/VTFrameProcessorConfiguration.h>
#import <VideoToolbox/VTFrameProcessorParameters.h>
#import <VideoToolbox/VTFrameProcessorFrame.h>

// Not available here: the configuration reports isSupported as NO, no pixel format
// or dimension is supported, and both initialisers return nil.
@interface VTTemporalNoiseFilterConfiguration : NSObject <VTFrameProcessorConfiguration>
- (nullable instancetype)initWithFrameWidth:(NSInteger)frameWidth frameHeight:(NSInteger)frameHeight sourcePixelFormat:(OSType)sourcePixelFormat;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@property (nonatomic, readonly) NSInteger frameWidth;
@property (nonatomic, readonly) NSInteger frameHeight;
@property (nonatomic, readonly) NSArray<NSNumber *> *frameSupportedPixelFormats;
@property (nonatomic, readonly) NSDictionary<NSString *, id> *sourcePixelBufferAttributes;
@property (nonatomic, readonly) NSDictionary<NSString *, id> *destinationPixelBufferAttributes;
@property (nonatomic, readonly) NSInteger nextFrameCount;
@property (nonatomic, readonly) NSInteger previousFrameCount;
@property (class, nonatomic, readonly) NSArray<NSNumber *> *supportedSourcePixelFormats;
@property (class, nonatomic, readonly) CMVideoDimensions maximumDimensions;
@property (class, nonatomic, readonly) CMVideoDimensions minimumDimensions;
@property (class, nonatomic, readonly, getter=isSupported) BOOL supported;
@end

@interface VTTemporalNoiseFilterParameters : NSObject <VTFrameProcessorParameters>
- (nullable instancetype)initWithSourceFrame:(VTFrameProcessorFrame *)sourceFrame
                                  nextFrames:(NSArray<VTFrameProcessorFrame *> *)nextFrames
                              previousFrames:(NSArray<VTFrameProcessorFrame *> *)previousFrames
                            destinationFrame:(VTFrameProcessorFrame *)destinationFrame
                              filterStrength:(float)filterStrength
                            hasDiscontinuity:(Boolean)hasDiscontinuity;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@property (nonatomic, readonly) VTFrameProcessorFrame *sourceFrame;
@property (nonatomic, readonly) NSArray<VTFrameProcessorFrame *> *nextFrames;
@property (nonatomic, readonly) NSArray<VTFrameProcessorFrame *> *previousFrames;
@property (nonatomic) float filterStrength;
@property (nonatomic) BOOL hasDiscontinuity;
@property (nonatomic, readonly) VTFrameProcessorFrame *destinationFrame;
@end
#endif

#endif
