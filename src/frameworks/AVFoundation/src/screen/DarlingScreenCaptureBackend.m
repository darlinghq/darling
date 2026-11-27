/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#import "DarlingScreenCaptureBackend.h"
#import "../camera/DarlingCameraFrame.h"

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <objc/runtime.h>

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
#include <screen_capture_backend.h>
#else
/* Only the method declarations need the type in a build without PipeWire. */
struct darling_screen_capture_frame;
#endif

@implementation AVCaptureInputPort (DarlingScreen)

+ (AVCaptureInputPort *)darling_videoPortForScreenInput:(AVCaptureScreenInput *)input
{
	AVCaptureInputPort *port = [[self alloc] init];

	(void) input;
	objc_setAssociatedObject(port, &kDarlingCameraMediaTypeKey, AVMediaTypeVideo,
		OBJC_ASSOCIATION_RETAIN_NONATOMIC);
	return [port autorelease];
}

@end

@interface DarlingScreenCaptureEngine ()
- (void)captureFrame:(const struct darling_screen_capture_frame *)frame;
- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped;
- (void)finishDelayedSample;
@end

static OSType DarlingScreenOutputFormat(NSDictionary *videoSettings)
{
	NSNumber *number = videoSettings[(NSString *) kCVPixelBufferPixelFormatTypeKey];
	OSType requested = number && [number isKindOfClass: [NSNumber class]]
		? (OSType) [number unsignedIntValue] : kCVPixelFormatType_32BGRA;

	switch (requested)
	{
		case kCVPixelFormatType_32BGRA:
		case kCVPixelFormatType_32RGBA:
		case kCVPixelFormatType_32ARGB:
		case kCVPixelFormatType_32ABGR:
		case kCVPixelFormatType_24RGB:
		case kCVPixelFormatType_24BGR:
		case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
		case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
		case kCVPixelFormatType_420YpCbCr8Planar:
			return requested;
		default:
			return kCVPixelFormatType_32BGRA;
	}
}

static enum darling_camera_output_layout DarlingScreenOutputLayout(OSType format)
{
	switch (format)
	{
		case kCVPixelFormatType_32RGBA: return DARLING_CAMERA_OUTPUT_RGBA;
		case kCVPixelFormatType_32ARGB: return DARLING_CAMERA_OUTPUT_ARGB;
		case kCVPixelFormatType_32ABGR: return DARLING_CAMERA_OUTPUT_ABGR;
		case kCVPixelFormatType_24RGB: return DARLING_CAMERA_OUTPUT_RGB24;
		case kCVPixelFormatType_24BGR: return DARLING_CAMERA_OUTPUT_BGR24;
		case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange: return DARLING_CAMERA_OUTPUT_NV12;
		case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange: return DARLING_CAMERA_OUTPUT_NV12_FULL;
		case kCVPixelFormatType_420YpCbCr8Planar: return DARLING_CAMERA_OUTPUT_I420;
		default: return DARLING_CAMERA_OUTPUT_BGRA;
	}
}

static uint64_t DarlingScreenFrameHash(const uint8_t *data, size_t size)
{
	uint64_t hash = UINT64_C(1469598103934665603);
	size_t i;

	for (i = 0; i < size; i++)
	{
		hash ^= data[i];
		hash *= UINT64_C(1099511628211);
	}
	return hash;
}

static int64_t DarlingScreenMinimumIntervalNs(CMTime duration)
{
	Float64 seconds = CMTimeGetSeconds(duration);

	if (!(seconds > 0))
		return 0;
	return (int64_t) (seconds * 1000000000.0);
}

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
/* cropRect uses the documented bottom-left origin; PipeWire frames use a
 * top-left origin. The crop is clipped to the frame before scaling. */
static BOOL DarlingScreenCrop(const struct darling_screen_capture_frame *frame,
	CGRect displayBounds, CGRect cropRect, int32_t *xOut, int32_t *yOut,
	uint32_t *widthOut, uint32_t *heightOut)
{
	int64_t left, top, right, bottom;

	if (CGRectIsEmpty(cropRect))
	{
		*xOut = 0;
		*yOut = 0;
		*widthOut = frame->width;
		*heightOut = frame->height;
		return *widthOut != 0 && *heightOut != 0;
	}

	left = (int64_t) (cropRect.origin.x - displayBounds.origin.x);
	top = (int64_t) frame->height
		- (int64_t) (cropRect.origin.y - displayBounds.origin.y + cropRect.size.height);
	right = left + (int64_t) cropRect.size.width;
	bottom = top + (int64_t) cropRect.size.height;

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > frame->width) right = frame->width;
	if (bottom > frame->height) bottom = frame->height;
	if (right <= left || bottom <= top)
		return NO;

	*xOut = (int32_t) left;
	*yOut = (int32_t) top;
	*widthOut = (uint32_t) (right - left);
	*heightOut = (uint32_t) (bottom - top);
	return YES;
}
#endif

static CMSampleBufferRef DarlingScreenCreateSampleBuffer(
	const uint8_t *bgra, uint32_t width, uint32_t height,
	int64_t ptsNs, int64_t durationNs, OSType pixelFormat)
{
	CVPixelBufferRef pixelBuffer = NULL;
	CMVideoFormatDescriptionRef description = NULL;
	CMSampleBufferRef sample = NULL;
	CMSampleTimingInfo timing;
	const uint8_t *srcPlanes[4] = { bgra, NULL, NULL, NULL };
	uint32_t srcStrides[4] = { width * 4, 0, 0, 0 };
	const uint8_t *dstPlanes[4] = { NULL, NULL, NULL, NULL };
	uint32_t dstStrides[4] = { 0, 0, 0, 0 };
	size_t planeCount, i;

	if (CVPixelBufferCreate(NULL, width, height, pixelFormat, NULL, &pixelBuffer)
			!= kCVReturnSuccess)
		return NULL;
	if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) != kCVReturnSuccess)
	{
		CVPixelBufferRelease(pixelBuffer);
		return NULL;
	}

	planeCount = CVPixelBufferGetPlaneCount(pixelBuffer);
	if (planeCount == 0)
	{
		dstPlanes[0] = CVPixelBufferGetBaseAddress(pixelBuffer);
		dstStrides[0] = (uint32_t) CVPixelBufferGetBytesPerRow(pixelBuffer);
	}
	else if (planeCount <= 4)
	{
		for (i = 0; i < planeCount; i++)
		{
			dstPlanes[i] = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, i);
			dstStrides[i] = (uint32_t) CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, i);
		}
	}
	if (dstPlanes[0])
		DarlingCameraConvertFrame(srcPlanes, srcStrides, DARLING_CAMERA_PIXEL_BGRX,
			dstPlanes, dstStrides, DarlingScreenOutputLayout(pixelFormat), width, height);
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

	if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) == noErr)
	{
		memset(&timing, 0, sizeof(timing));
		if (ptsNs >= 0)
			timing.presentationTimeStamp = CMTimeMake(ptsNs, 1000000000);
		else
			timing.presentationTimeStamp = CMTimeConvertScale(
				CMClockGetTime(CMClockGetHostTimeClock()), 1000000000,
				kCMTimeRoundingMethod_RoundHalfAwayFromZero);
		if (durationNs <= 0)
			durationNs = 33333333;
		timing.duration = CMTimeMake(durationNs, 1000000000);
		CMSampleBufferCreateReadyWithImageBuffer(NULL, pixelBuffer, description, &timing, &sample);
	}
	if (description)
		CFRelease(description);
	CVPixelBufferRelease(pixelBuffer);
	return sample;
}

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
static void DarlingScreenFrame(const struct darling_screen_capture_frame *frame, void *userData);
static void DarlingScreenError(int error, void *userData);
#endif

@implementation DarlingScreenCaptureEngine

- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames
	screenInput:(AVCaptureScreenInput *)screenInput
{
	self = [super init];
	if (!self)
		return nil;

	_output = [output retain];
	_connection = [connection retain];
	_videoSettings = [videoSettings copy];
	_alwaysDiscardsLateVideoFrames = alwaysDiscardsLateVideoFrames;
	_screenInput = [screenInput retain];
	atomic_store(&_running, false);
	atomic_store(&_inFlight, 0);
	_lastPtsNs = -1;
	return self;
}

- (void)dealloc
{
	[self stop];
	[_output release];
	[_connection release];
	[_videoSettings release];
	[_screenInput release];
	[super dealloc];
}

- (void)start
{
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	CGRect bounds = CGDisplayBounds([_screenInput darling_displayID]);
	struct darling_screen_capture_callbacks callbacks;
	int result;

	if (CGRectIsEmpty(bounds))
		return;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.frame = DarlingScreenFrame;
	callbacks.error = DarlingScreenError;
	result = darling_screen_capture_open((int32_t) bounds.origin.x, (int32_t) bounds.origin.y,
		(uint32_t) bounds.size.width, (uint32_t) bounds.size.height,
		[_screenInput capturesCursor], darling_screen_capture_supports_hdr() ? 1 : 0,
		&callbacks, self, &_capture);
	if (result != 0)
		return;
	result = darling_screen_capture_start(_capture, 1);
	if (result == 0)
		result = darling_screen_capture_wait(_capture, 5000);
	if (result != 0)
	{
		darling_screen_capture_close(_capture);
		_capture = NULL;
		return;
	}
	atomic_store(&_running, true);
#endif
}

- (void)stop
{
	atomic_store(&_running, false);
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	if (_capture)
	{
		darling_screen_capture_close(_capture);
		_capture = NULL;
	}
#endif
}

- (void)captureFrame:(const struct darling_screen_capture_frame *)frame
{
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	CGRect bounds = CGDisplayBounds([_screenInput darling_displayID]);
	CGRect crop = [_screenInput cropRect];
	CGFloat scale = [_screenInput scaleFactor];
	int32_t sourceX = 0, sourceY = 0;
	uint32_t sourceWidth = 0, sourceHeight = 0;
	uint32_t width, height;
	int64_t minIntervalNs, durationNs;
	uint8_t *bgra;
	uint64_t hash;
	CMSampleBufferRef sample;
	OSType pixelFormat;

	if (!atomic_load(&_running) || CGRectIsEmpty(bounds) || ![_connection isEnabled])
		return;
	if (!DarlingScreenCrop(frame, bounds, crop, &sourceX, &sourceY,
			&sourceWidth, &sourceHeight))
		return;
	if (!(scale > 0))
		scale = 1;
	width = (uint32_t) (sourceWidth * scale);
	height = (uint32_t) (sourceHeight * scale);
	if (width == 0 || height == 0 || width > 16384 || height > 16384)
		return;

	minIntervalNs = DarlingScreenMinimumIntervalNs([_screenInput minFrameDuration]);
	durationNs = minIntervalNs > 0 ? minIntervalNs : 33333333;
	if (minIntervalNs > 0 && _lastPtsNs >= 0 && frame->pts_ns >= _lastPtsNs
			&& frame->pts_ns - _lastPtsNs < minIntervalNs)
		return;

	bgra = (uint8_t *) malloc((size_t) width * height * 4);
	if (!bgra)
		return;
	if (darling_screen_capture_convert_bgra_rect(frame, bgra, width * 4, width, height,
			sourceX, sourceY, sourceWidth, sourceHeight, 0, 0, width, height) != 0)
	{
		free(bgra);
		return;
	}

	hash = DarlingScreenFrameHash(bgra, (size_t) width * height * 4);
	if ([_screenInput removesDuplicateFrames] && _hasLastFrameHash && hash == _lastFrameHash)
	{
		free(bgra);
		return;
	}
	_lastFrameHash = hash;
	_hasLastFrameHash = YES;
	_lastPtsNs = frame->pts_ns;

	pixelFormat = DarlingScreenOutputFormat(_videoSettings);
	sample = DarlingScreenCreateSampleBuffer(bgra, width, height,
		frame->pts_ns, durationNs, pixelFormat);
	free(bgra);
	if (!sample)
		return;

	if (atomic_load(&_inFlight) > 0 && _alwaysDiscardsLateVideoFrames)
		[self deliverSample: sample dropped: YES];
	else
		[self deliverSample: sample dropped: NO];
#else
	(void) frame;
#endif
}

- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped
{
	id delegate = [_output sampleBufferDelegate];
	SEL selector = dropped
		? @selector(captureOutput:didDropSampleBuffer:fromConnection:)
		: @selector(captureOutput:didOutputSampleBuffer:fromConnection:);

	if (!delegate || ![delegate respondsToSelector: selector])
	{
		CFRelease(sample);
		return;
	}

	if ([_output sampleBufferDelegateQueue])
	{
		AVCaptureOutput *output = [_output retain];
		AVCaptureConnection *connection = [_connection retain];
		id strongDelegate = [delegate retain];
		DarlingScreenCaptureEngine *strongEngine = [self retain];

		atomic_fetch_add(&_inFlight, 1);
		dispatch_async([_output sampleBufferDelegateQueue], ^{
			if (dropped)
				[strongDelegate captureOutput: output didDropSampleBuffer: sample
					fromConnection: connection];
			else
				[strongDelegate captureOutput: output didOutputSampleBuffer: sample
					fromConnection: connection];
			CFRelease(sample);
			[strongDelegate release];
			[output release];
			[connection release];
			[strongEngine finishDelayedSample];
			[strongEngine release];
		});
	}
	else
	{
		atomic_fetch_add(&_inFlight, 1);
		if (dropped)
			[delegate captureOutput: _output didDropSampleBuffer: sample
				fromConnection: _connection];
		else
			[delegate captureOutput: _output didOutputSampleBuffer: sample
				fromConnection: _connection];
		CFRelease(sample);
		atomic_fetch_sub(&_inFlight, 1);
	}
}

- (void)finishDelayedSample
{
	atomic_fetch_sub(&_inFlight, 1);
}

@end

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
static void DarlingScreenFrame(const struct darling_screen_capture_frame *frame, void *userData)
{
	DarlingScreenCaptureEngine *engine = (DarlingScreenCaptureEngine *) userData;

	@autoreleasepool
	{
		[engine captureFrame: frame];
	}
}

static void DarlingScreenError(int error, void *userData)
{
	(void) error;
	(void) userData;
	/* AVCaptureInput has no public error callback; stopRunning simply sees no
	 * more frames, which matches the other capture input implementations. */
}
#endif
