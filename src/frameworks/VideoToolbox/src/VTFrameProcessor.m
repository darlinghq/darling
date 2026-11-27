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

#import <VideoToolbox/VTFrameProcessor.h>

NSErrorDomain const VTFrameProcessorErrorDomain = @"VTFrameProcessorErrorDomain";

static NSError *VTFPError(VTFrameProcessorError code)
{
    return [NSError errorWithDomain:VTFrameProcessorErrorDomain code:code userInfo:nil];
}

@implementation VTFrameProcessor

- (instancetype)init
{
    return [super init];
}

- (BOOL)startSessionWithConfiguration:(id<VTFrameProcessorConfiguration>)configuration error:(NSError **)error
{
    if (error)
        *error = VTFPError(VTFrameProcessorInitializationFailed);
    return NO;
}

- (BOOL)processWithParameters:(id<VTFrameProcessorParameters>)parameters error:(NSError **)error
{
    if (error)
        *error = VTFPError(VTFrameProcessorSessionNotStarted);
    return NO;
}

- (void)processWithParameters:(id<VTFrameProcessorParameters>)parameters
            completionHandler:(void (^)(id<VTFrameProcessorParameters>, NSError *))completionHandler
{
    void (^handler)(id<VTFrameProcessorParameters>, NSError *) = [completionHandler copy];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        handler(nil, VTFPError(VTFrameProcessorSessionNotStarted));
        [handler release];
    });
}

- (void)processWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer parameters:(id<VTFrameProcessorParameters>)parameters
{
}

- (void)endSession
{
}

@end

@implementation VTFrameProcessorFrame

- (instancetype)initWithBuffer:(CVPixelBufferRef)buffer presentationTimeStamp:(CMTime)presentationTimeStamp
{
    if (buffer == NULL) {
        [self release];
        return nil;
    }
    if ((self = [super init])) {
        _buffer = CVPixelBufferRetain(buffer);
        _presentationTimeStamp = presentationTimeStamp;
    }
    return self;
}

- (void)dealloc
{
    CVPixelBufferRelease(_buffer);
    [super dealloc];
}

- (CVPixelBufferRef)buffer { return _buffer; }
- (CMTime)presentationTimeStamp { return _presentationTimeStamp; }

@end

@implementation VTTemporalNoiseFilterConfiguration

- (instancetype)initWithFrameWidth:(NSInteger)frameWidth frameHeight:(NSInteger)frameHeight sourcePixelFormat:(OSType)sourcePixelFormat
{
    [self release];
    return nil;
}

+ (BOOL)isSupported { return NO; }
+ (NSArray<NSNumber *> *)supportedSourcePixelFormats { return @[]; }
+ (CMVideoDimensions)maximumDimensions { return (CMVideoDimensions){ 0, 0 }; }
+ (CMVideoDimensions)minimumDimensions { return (CMVideoDimensions){ 0, 0 }; }

- (NSInteger)frameWidth { return 0; }
- (NSInteger)frameHeight { return 0; }
- (NSArray<NSNumber *> *)frameSupportedPixelFormats { return @[]; }
- (NSDictionary<NSString *, id> *)sourcePixelBufferAttributes { return @{}; }
- (NSDictionary<NSString *, id> *)destinationPixelBufferAttributes { return @{}; }
- (NSInteger)nextFrameCount { return 0; }
- (NSInteger)previousFrameCount { return 0; }

@end

@implementation VTTemporalNoiseFilterParameters

- (instancetype)initWithSourceFrame:(VTFrameProcessorFrame *)sourceFrame
                         nextFrames:(NSArray<VTFrameProcessorFrame *> *)nextFrames
                     previousFrames:(NSArray<VTFrameProcessorFrame *> *)previousFrames
                   destinationFrame:(VTFrameProcessorFrame *)destinationFrame
                     filterStrength:(float)filterStrength
                   hasDiscontinuity:(Boolean)hasDiscontinuity
{
    [self release];
    return nil;
}

- (VTFrameProcessorFrame *)sourceFrame { return nil; }
- (NSArray<VTFrameProcessorFrame *> *)nextFrames { return @[]; }
- (NSArray<VTFrameProcessorFrame *> *)previousFrames { return @[]; }
- (VTFrameProcessorFrame *)destinationFrame { return nil; }

@end
