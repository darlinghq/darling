// Copyright (C) 2026 Lubos Dolezel
//
// The 360 and HDR compression property constants have the system's strings (read
// from the exported constants on macOS 26.7.3), and the frame processor classes
// report unavailability the documented way: VTFrameProcessorErrorDomain and its
// codes, the temporal noise filter configuration's isSupported NO with no supported
// format or dimension and a nil initialiser result (the unsupported-machine
// behaviour observed on macOS 26.7.3), and sessions that cannot start.
#import <VideoToolbox/VideoToolbox.h>
#import <CoreVideo/CoreVideo.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

extern const CFStringRef kVTCompressionPropertyKey_CameraCalibrationDataLensCollection;
extern const CFStringRef kVTCompressionPropertyKey_HDRMetadataInsertionMode;
extern const CFStringRef kVTCompressionPropertyKey_HorizontalFieldOfView;
extern const CFStringRef kVTCompressionPropertyKey_ProjectionKind;
extern const CFStringRef kVTHDRMetadataInsertionMode_None;
extern const CFStringRef kVTProjectionKind_Equirectangular;

int main(void)
{
	@autoreleasepool {
		CHECK(CFEqual(kVTCompressionPropertyKey_CameraCalibrationDataLensCollection, CFSTR("CameraCalibrationDataLensCollection")));
		CHECK(CFEqual(kVTCompressionPropertyKey_HDRMetadataInsertionMode, CFSTR("HDRMetadataInsertionMode")));
		CHECK(CFEqual(kVTCompressionPropertyKey_HorizontalFieldOfView, CFSTR("HorizontalFieldOfView")));
		CHECK(CFEqual(kVTCompressionPropertyKey_ProjectionKind, CFSTR("ProjectionKind")));
		CHECK(CFEqual(kVTHDRMetadataInsertionMode_None, CFSTR("HDRMetadataInsertionMode_None")));
		CHECK(CFEqual(kVTProjectionKind_Equirectangular, CFSTR("Equirectangular")));

		CHECK([VTFrameProcessorErrorDomain isEqualToString:@"VTFrameProcessorErrorDomain"]);
		CHECK(VTFrameProcessorSessionNotStarted == -19732 && VTFrameProcessorInitializationFailed == -19736);

		CHECK([VTTemporalNoiseFilterConfiguration isSupported] == NO);
		CMVideoDimensions lo = [VTTemporalNoiseFilterConfiguration minimumDimensions];
		CMVideoDimensions hi = [VTTemporalNoiseFilterConfiguration maximumDimensions];
		CHECK(lo.width == 0 && lo.height == 0 && hi.width == 0 && hi.height == 0);
		CHECK([[VTTemporalNoiseFilterConfiguration supportedSourcePixelFormats] count] == 0);
		VTTemporalNoiseFilterConfiguration *config = [[VTTemporalNoiseFilterConfiguration alloc]
			initWithFrameWidth:640 frameHeight:480 sourcePixelFormat:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange];
		CHECK(config == nil);

		CVPixelBufferRef buffer = NULL;
		CHECK(CVPixelBufferCreate(NULL, 64, 48, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnSuccess);
		VTFrameProcessorFrame *frame = [[VTFrameProcessorFrame alloc] initWithBuffer:buffer presentationTimeStamp:CMTimeMake(1, 30)];
		CHECK(frame != nil && frame.buffer == buffer);
		CHECK(CMTimeCompare(frame.presentationTimeStamp, CMTimeMake(1, 30)) == 0);
		[frame release];
		CHECK([[VTFrameProcessorFrame alloc] initWithBuffer:NULL presentationTimeStamp:kCMTimeZero] == nil);
		CHECK(CFGetRetainCount(buffer) == 1);

		VTTemporalNoiseFilterParameters *params = [[VTTemporalNoiseFilterParameters alloc]
			initWithSourceFrame:nil nextFrames:@[] previousFrames:@[] destinationFrame:nil filterStrength:0.5f hasDiscontinuity:NO];
		CHECK(params == nil);

		VTFrameProcessor *processor = [[VTFrameProcessor alloc] init];
		CHECK(processor != nil);
		NSError *error = nil;
		CHECK([processor startSessionWithConfiguration:(id)[NSNull null] error:&error] == NO);
		CHECK(error != nil && [[error domain] isEqualToString:VTFrameProcessorErrorDomain]
			&& [error code] == VTFrameProcessorInitializationFailed);
		error = nil;
		CHECK([processor processWithParameters:(id)[NSNull null] error:&error] == NO);
		CHECK(error != nil && [[error domain] isEqualToString:VTFrameProcessorErrorDomain]
			&& [error code] == VTFrameProcessorSessionNotStarted);
		CHECK([processor startSessionWithConfiguration:(id)[NSNull null] error:NULL] == NO);
		[processor endSession];

		__block NSError *handlerError = nil;
		__block id handlerResult = (id)1;
		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		[processor processWithParameters:(id)[NSNull null] completionHandler:^(id result, NSError *e) {
			handlerResult = result;
			handlerError = [e retain];
			dispatch_semaphore_signal(sem);
		}];
		CHECK(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0);
		CHECK(handlerResult == nil);
		CHECK(handlerError != nil && [handlerError code] == VTFrameProcessorSessionNotStarted);
		CFRelease(buffer);
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
