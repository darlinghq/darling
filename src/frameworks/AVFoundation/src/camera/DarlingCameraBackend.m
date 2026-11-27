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

#import "DarlingCameraBackend.h"

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <objc/runtime.h>

#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
static struct darling_camera_portal_api cameraPortalAPI(int timeout_ms)
{
	struct darling_camera_portal_api api;

	api.dbus = darling_dbus_get(DARLING_DBUS_HOST_FALLBACK);
	api.getenv = getenv;
	api.timeout_ms = timeout_ms;
	return api;
}
#endif

BOOL DarlingCameraBackendCameraPresent(void)
{
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	struct darling_camera_portal_api api;

	if (!darling_pipewire_available())
		return NO;
	/* A short timeout keeps repeated device queries responsive when there is
	 * no session bus; requests below use a longer one for user consent. */
	api = cameraPortalAPI(1500);
	return darling_camera_portal_is_present(&api) == DARLING_CAMERA_PORTAL_OK;
#else
	return NO;
#endif
}

int DarlingCameraBackendRequestAccess(void)
{
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	struct darling_camera_portal_api api;

	if (!darling_pipewire_available())
		return DARLING_CAMERA_RESULT_NOT_AVAILABLE;
	api = cameraPortalAPI(15000);
	return darling_camera_portal_request_access(&api);
#else
	return DARLING_CAMERA_RESULT_NOT_AVAILABLE;
#endif
}

int DarlingCameraBackendOpenRemote(int *fdOut)
{
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
	struct darling_camera_portal_api api;

	if (!fdOut)
		return DARLING_CAMERA_RESULT_NOT_AVAILABLE;
	*fdOut = -1;
	if (!darling_pipewire_available())
		return DARLING_CAMERA_RESULT_NOT_AVAILABLE;
	api = cameraPortalAPI(10000);
	return darling_camera_portal_open_remote(&api, fdOut);
#else
	(void) fdOut;
	return DARLING_CAMERA_RESULT_NOT_AVAILABLE;
#endif
}

/* Properties of the objects below are kept with the objects themselves. The
 * public headers describe them, but they have no storage in the stub classes. */
const char kDarlingCameraUniqueIDKey;
const char kDarlingCameraNodeIDKey;
const char kDarlingCameraDeviceTypeKey;
const char kDarlingCameraNameKey;
const char kDarlingCameraFormatsKey;
const char kDarlingCameraActiveFormatKey;
const char kDarlingCameraPortsKey;
const char kDarlingCameraConnectionsKey;
const char kDarlingCameraEnabledKey;
const char kDarlingCameraDeviceInputDeviceKey;
const char kDarlingCameraMediaTypeKey;

static char kCameraConnectionOutputKey;
static char kCameraFormatWidthKey, kCameraFormatHeightKey, kCameraFormatRateKey;

static id associated(id object, const void *key)
{
	return objc_getAssociatedObject(object, key);
}

static void associate(id object, const void *key, id value)
{
	objc_setAssociatedObject(object, key, value, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

static uint32_t unsignedProperty(id object, const void *key)
{
	return [associated(object, key) unsignedIntValue];
}

static BOOL DarlingCameraPresetDimensions(AVCaptureSessionPreset preset, uint32_t *width, uint32_t *height)
{
	if ([preset isEqualToString: AVCaptureSessionPreset320x240]) { *width = 320; *height = 240; }
	else if ([preset isEqualToString: AVCaptureSessionPreset352x288]) { *width = 352; *height = 288; }
	else if ([preset isEqualToString: AVCaptureSessionPreset640x480]) { *width = 640; *height = 480; }
	else if ([preset isEqualToString: AVCaptureSessionPreset960x540]) { *width = 960; *height = 540; }
	else if ([preset isEqualToString: AVCaptureSessionPreset1280x720]) { *width = 1280; *height = 720; }
	else if ([preset isEqualToString: AVCaptureSessionPreset1920x1080]) { *width = 1920; *height = 1080; }
	else if ([preset isEqualToString: AVCaptureSessionPreset3840x2160]) { *width = 3840; *height = 2160; }
	else return NO;
	return YES;
}

/* Returns the format with the closest pixel count. Substitutes the exact
 * dimensions first so a preset does not silently pick a different shape. */
static AVCaptureDeviceFormat *DarlingCameraClosestFormat(NSArray *formats, uint32_t width, uint32_t height)
{
	AVCaptureDeviceFormat *best = nil;
	uint64_t requested = (uint64_t) width * height, bestDistance = UINT64_MAX;

	for (AVCaptureDeviceFormat *format in formats)
	{
		uint64_t pixels = (uint64_t) [format darling_width] * [format darling_height];
		uint64_t distance = pixels > requested ? pixels - requested : requested - pixels;

		if ([format darling_width] == width && [format darling_height] == height)
			return format;
		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = format;
		}
	}
	return best;
}

@implementation AVCaptureDeviceFormat (DarlingCamera)

+ (AVCaptureDeviceFormat *)darling_formatWithWidth:(uint32_t)width
	height:(uint32_t)height framerate:(uint32_t)framerate
{
	AVCaptureDeviceFormat *format = [[self alloc] init];
	associate(format, &kCameraFormatWidthKey, [NSNumber numberWithUnsignedInt: width]);
	associate(format, &kCameraFormatHeightKey, [NSNumber numberWithUnsignedInt: height]);
	associate(format, &kCameraFormatRateKey, [NSNumber numberWithUnsignedInt: framerate]);
	return [format autorelease];
}

- (uint32_t)darling_width { return unsignedProperty(self, &kCameraFormatWidthKey); }
- (uint32_t)darling_height { return unsignedProperty(self, &kCameraFormatHeightKey); }
- (uint32_t)darling_framerate { return unsignedProperty(self, &kCameraFormatRateKey); }

@end

@implementation AVCaptureInputPort (DarlingCamera)

+ (AVCaptureInputPort *)darling_videoPortForDevice:(AVCaptureDevice *)device
{
	AVCaptureInputPort *port = [[self alloc] init];
	associate(port, &kDarlingCameraMediaTypeKey, AVMediaTypeVideo);
	associate(port, &kDarlingCameraDeviceInputDeviceKey, device);
	return [port autorelease];
}

- (AVMediaType)darling_mediaType { return associated(self, &kDarlingCameraMediaTypeKey); }

@end

@implementation AVCaptureDeviceInput (DarlingCamera)

+ (AVCaptureDeviceInput *)darling_inputWithDevice:(AVCaptureDevice *)device
{
	AVCaptureDeviceInput *input = [[self alloc] init];
	associate(input, &kDarlingCameraDeviceInputDeviceKey, device);
	associate(input, &kDarlingCameraPortsKey,
		[NSArray arrayWithObject: [AVCaptureInputPort darling_videoPortForDevice: device]]);
	return [input autorelease];
}

@end

@implementation AVCaptureOutput (DarlingCamera)

- (void)darling_addConnection:(AVCaptureConnection *)connection
{
	NSMutableArray *connections = associated(self, &kDarlingCameraConnectionsKey);
	if (!connections)
	{
		connections = [NSMutableArray array];
		associate(self, &kDarlingCameraConnectionsKey, connections);
	}
	if (![connections containsObject: connection])
		[connections addObject: connection];
}

- (void)darling_removeConnection:(AVCaptureConnection *)connection
{
	[associated(self, &kDarlingCameraConnectionsKey) removeObject: connection];
}

@end

@implementation AVCaptureConnection (DarlingCamera)

+ (AVCaptureConnection *)darling_connectionWithInputPorts:(NSArray *)ports
	output:(AVCaptureOutput *)output
{
	AVCaptureConnection *connection = [[self alloc] init];
	associate(connection, &kDarlingCameraPortsKey, ports);
	associate(connection, &kCameraConnectionOutputKey, output);
	return [connection autorelease];
}

- (AVCaptureOutput *)darling_output { return associated(self, &kCameraConnectionOutputKey); }

@end

static AVCaptureDevice *DarlingCameraMakeDevice(NSString *uniqueID, NSString *name, uint32_t nodeId)
{
	AVCaptureDevice *device = [[AVCaptureDevice alloc] init];
	NSArray *formats = @[
		[AVCaptureDeviceFormat darling_formatWithWidth: 640 height: 480 framerate: 30],
		[AVCaptureDeviceFormat darling_formatWithWidth: 1280 height: 720 framerate: 30],
		[AVCaptureDeviceFormat darling_formatWithWidth: 1920 height: 1080 framerate: 30],
	];

	associate(device, &kDarlingCameraUniqueIDKey, uniqueID);
	associate(device, &kDarlingCameraNodeIDKey, [NSNumber numberWithUnsignedInt: nodeId]);
	associate(device, &kDarlingCameraDeviceTypeKey, AVCaptureDeviceTypeBuiltInWideAngleCamera);
	associate(device, &kDarlingCameraNameKey, name);
	associate(device, &kDarlingCameraFormatsKey, formats);
	associate(device, &kDarlingCameraActiveFormatKey, [formats objectAtIndex: 1]);
	return [device autorelease];
}

@implementation AVCaptureDevice (DarlingCamera)

+ (NSArray<AVCaptureDevice *> *)darling_cameraDevices
{
	static NSArray *devices;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		NSMutableArray *found = [NSMutableArray array];

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
		if (darling_pw_can_connect())
		{
			struct darling_pw_video_source sources[16];
			int count = darling_pw_enumerate_video_sources(sources, 16);
			int i;

			for (i = 0; i < count; i++)
			{
				NSString *name = [NSString stringWithUTF8String: sources[i].name];
				NSString *uniqueID = [NSString stringWithFormat: @"Darling:Camera:%u:%@",
					(unsigned) sources[i].id, name];

				[found addObject: DarlingCameraMakeDevice(uniqueID, name, sources[i].id)];
			}
		}
#endif
		/* A portal camera can still be available when the node list cannot be
		 * read (for example on older compositors). Keep the generic device. */
		if ([found count] == 0 && DarlingCameraBackendCameraPresent())
			[found addObject: DarlingCameraMakeDevice(@"Darling:Camera:0", @"Camera", 0)];
		devices = [found copy];
	});
	return devices;
}

+ (AVCaptureDevice *)darling_cameraDevice
{
	return [[self darling_cameraDevices] firstObject];
}

- (AVCaptureDeviceFormat *)darling_formatForPreset:(AVCaptureSessionPreset)preset
{
	NSArray *formats = [self formats];
	uint32_t width = 0, height = 0;

	if ([formats count] == 0)
		return nil;
	if (DarlingCameraPresetDimensions(preset, &width, &height))
		return DarlingCameraClosestFormat(formats, width, height);

	if ([preset isEqualToString: AVCaptureSessionPresetLow])
		return [formats firstObject];
	if ([preset isEqualToString: AVCaptureSessionPresetMedium])
		return [formats count] > 1 ? [formats objectAtIndex: [formats count] / 2] : [formats firstObject];
	if ([preset isEqualToString: AVCaptureSessionPresetiFrame1280x720])
		return DarlingCameraClosestFormat(formats, 1280, 720);
	if ([preset isEqualToString: AVCaptureSessionPresetiFrame960x540])
		return DarlingCameraClosestFormat(formats, 960, 540);
	if ([preset isEqualToString: AVCaptureSessionPresetHigh]
		|| [preset isEqualToString: AVCaptureSessionPresetPhoto])
		return [formats lastObject];
	return nil;
}

- (AVCaptureDeviceFormat *)darling_formatForVideoSettings:(NSDictionary *)settings
{
	NSNumber *widthNumber = settings[(NSString *) kCVPixelBufferWidthKey];
	NSNumber *heightNumber = settings[(NSString *) kCVPixelBufferHeightKey];

	if (![widthNumber isKindOfClass: [NSNumber class]]
		|| ![heightNumber isKindOfClass: [NSNumber class]])
		return nil;
	return DarlingCameraClosestFormat([self formats],
		[widthNumber unsignedIntValue], [heightNumber unsignedIntValue]);
}

@end

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE

@interface DarlingCameraCaptureEngine ()
- (void)deliverFrames;
- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped toDelegate:(id)delegate;
- (void)finishDelayedSample;
@end

@implementation DarlingCameraCaptureEngine

- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection
	delegate:(id)delegate queue:(dispatch_queue_t)queue
	format:(AVCaptureDeviceFormat *)format
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames
{
	self = [super init];
	if (!self)
		return nil;

	m_output = [output retain];
	m_connection = [connection retain];
	/* The output owns the delegate slot; the engine reads it for each frame so
	 * delegate changes and delegate teardown are observed immediately. */
	(void) delegate;
	if (queue)
		dispatch_retain(queue);
	m_queue = queue;
	m_format = [format retain];
	m_videoSettings = [videoSettings copy];
	m_alwaysDiscardsLateVideoFrames = alwaysDiscardsLateVideoFrames;
	m_remoteFd = -1;
	atomic_store(&m_running, false);
	atomic_store(&m_threadValid, false);
	atomic_store(&m_inFlight, 0);
	m_lastPtsNs = -1;
	return self;
}

- (void)dealloc
{
	[self stop];
	[m_output release];
	[m_connection release];
	if (m_queue)
		dispatch_release(m_queue);
	[m_format release];
	[m_videoSettings release];
	[super dealloc];
}

static void *cameraPump(void *context)
{
	DarlingCameraCaptureEngine *engine = (DarlingCameraCaptureEngine *) context;
	[engine deliverFrames];
	return NULL;
}

/* Copies every plane while the stream is locked; PipeWire may recycle the
 * buffers as soon as they are queued again. */
static int DarlingCameraCopyPlanes(const struct darling_pw_buffer *buffer,
	uint8_t *planes[4], uint32_t strides[4])
{
	size_t total = 0, offset = 0;
	uint8_t *block;
	uint32_t i;

	memset(planes, 0, 4 * sizeof(*planes));
	memset(strides, 0, 4 * sizeof(*strides));
	for (i = 0; i < 4; i++)
		total += buffer->plane_sizes[i];
	block = (uint8_t *) malloc(total ? total : 1);
	if (!block)
		return -1;
	for (i = 0; i < 4; i++)
	{
		if (!buffer->planes[i] || buffer->plane_sizes[i] == 0)
			continue;
		memcpy(block + offset, buffer->planes[i], buffer->plane_sizes[i]);
		planes[i] = block + offset;
		strides[i] = buffer->plane_strides[i];
		offset += buffer->plane_sizes[i];
	}
	return 0;
}

/* The converter trusts the reported strides. PipeWire's chunk may describe a
 * smaller region than a full frame (for example on a partially reused buffer),
 * so reject that buffer instead of reading past the copied planes. */
static BOOL DarlingCameraBufferHasCompleteFrame(const struct darling_pw_buffer *buffer,
	enum darling_camera_pixel_layout layout, uint32_t width, uint32_t height)
{
	uint64_t chromaHeight = ((uint64_t) height + 1) / 2;
	uint64_t needed0 = 0, needed1 = 0, needed2 = 0;

	if (!width || !height)
		return NO;
	switch (layout)
	{
		case DARLING_CAMERA_PIXEL_I420:
			if (!buffer->planes[1] || !buffer->planes[2])
				return NO;
			needed0 = (uint64_t) height * buffer->plane_strides[0];
			needed1 = chromaHeight * buffer->plane_strides[1];
			needed2 = chromaHeight * buffer->plane_strides[2];
			break;
		case DARLING_CAMERA_PIXEL_NV12:
			if (!buffer->planes[1])
				return NO;
			needed0 = (uint64_t) height * buffer->plane_strides[0];
			needed1 = chromaHeight * buffer->plane_strides[1];
			break;
		default:
			needed0 = (uint64_t) height * buffer->plane_strides[0];
			break;
	}
	return buffer->plane_sizes[0] >= needed0
		&& buffer->plane_sizes[1] >= needed1
		&& buffer->plane_sizes[2] >= needed2;
}

- (void)start
{
	struct darling_pw_stream_config config;
	struct darling_pw_stream_events events;
	int result;

	/* The Camera portal hands out a remote with cameras in it only to a client
	 * that has been through AccessCamera; the call returns at once when the
	 * user has already decided. */
	if (DarlingCameraBackendRequestAccess() != DARLING_CAMERA_RESULT_OK)
		return;
	if (DarlingCameraBackendOpenRemote(&m_remoteFd) != DARLING_CAMERA_RESULT_OK)
		return;

	memset(&config, 0, sizeof(config));
	config.media = DARLING_PW_MEDIA_VIDEO;
	config.capture = 1;
	config.name = "DarlingCamera";
	config.role = "Camera";
	{
		AVCaptureInputPort *port = [[m_connection videoPorts] firstObject];
		AVCaptureDevice *device = associated(port, &kDarlingCameraDeviceInputDeviceKey);
		config.target_id = [associated(device, &kDarlingCameraNodeIDKey) unsignedIntValue];
	}
	config.width = [m_format darling_width];
	config.height = [m_format darling_height];
	config.framerate = [m_format darling_framerate];

	memset(&events, 0, sizeof(events));
	events.version = 1;
	/* darling_pw_stream_create() takes ownership of the fd on every path. */
	{
		int fd = m_remoteFd;
		m_remoteFd = -1;
		result = darling_pw_stream_create(&config, fd, &events, self, &m_stream);
	}
	if (result != 0)
		return;

	if (darling_pw_stream_wait(m_stream, 2000) < 0)
	{
		[self stop];
		return;
	}
	atomic_store(&m_running, true);
	if (pthread_create(&m_thread, NULL, cameraPump, self) != 0)
	{
		atomic_store(&m_running, false);
		[self stop];
		return;
	}
	atomic_store(&m_threadValid, true);
}

/* videoSettings may ask for a format the converter cannot fill. Fall back to
 * BGRA rather than asking CoreVideo to allocate a buffer we cannot write. */
static OSType DarlingCameraOutputFormatFromSettings(NSDictionary *videoSettings)
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

static enum darling_camera_output_layout DarlingCameraOutputForPixelFormat(OSType format)
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

static enum darling_camera_pixel_layout DarlingCameraLayoutForFormat(enum darling_pw_video_format format)
{
	switch (format)
	{
		case DARLING_PW_VIDEO_FORMAT_RGBX: return DARLING_CAMERA_PIXEL_RGBX;
		case DARLING_PW_VIDEO_FORMAT_XRGB: return DARLING_CAMERA_PIXEL_XRGB;
		case DARLING_PW_VIDEO_FORMAT_XBGR: return DARLING_CAMERA_PIXEL_XBGR;
		case DARLING_PW_VIDEO_FORMAT_RGB24: return DARLING_CAMERA_PIXEL_RGB24;
		case DARLING_PW_VIDEO_FORMAT_BGR24: return DARLING_CAMERA_PIXEL_BGR24;
		case DARLING_PW_VIDEO_FORMAT_I420: return DARLING_CAMERA_PIXEL_I420;
		case DARLING_PW_VIDEO_FORMAT_NV12: return DARLING_CAMERA_PIXEL_NV12;
		case DARLING_PW_VIDEO_FORMAT_YUY2: return DARLING_CAMERA_PIXEL_YUY2;
		case DARLING_PW_VIDEO_FORMAT_UYVY: return DARLING_CAMERA_PIXEL_UYVY;
		case DARLING_PW_VIDEO_FORMAT_P010: return DARLING_CAMERA_PIXEL_P010;
		default: return DARLING_CAMERA_PIXEL_BGRX;
	}
}

static CMSampleBufferRef DarlingCameraCreateSampleBuffer(
	const uint8_t *const planes[4], const uint32_t strides[4],
	enum darling_camera_pixel_layout layout, uint32_t width, uint32_t height,
	int64_t pts_ns, int64_t duration_ns, OSType pixelFormat)
{
	CVPixelBufferRef pixelBuffer = NULL;
	CMVideoFormatDescriptionRef description = NULL;
	CMSampleBufferRef sample = NULL;
	CMSampleTimingInfo timing;
	const enum darling_camera_output_layout output = DarlingCameraOutputForPixelFormat(pixelFormat);
	size_t planeCount, i;

	if (!planes[0] || width == 0 || height == 0 || duration_ns <= 0)
		return NULL;
	if (CVPixelBufferCreate(NULL, width, height, pixelFormat, NULL, &pixelBuffer) != kCVReturnSuccess)
		return NULL;

	if (CVPixelBufferLockBaseAddress(pixelBuffer, 0) == kCVReturnSuccess)
	{
		const uint8_t *dstPlanes[4] = { NULL, NULL, NULL, NULL };
		uint32_t dstStrides[4] = { 0, 0, 0, 0 };

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
			DarlingCameraConvertFrame(planes, strides, layout, dstPlanes, dstStrides, output, width, height);
		CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
	}

	if (CMVideoFormatDescriptionCreateForImageBuffer(NULL, pixelBuffer, &description) == noErr)
	{
		memset(&timing, 0, sizeof(timing));
		/* PipeWire timestamps are monotonic nanoseconds, the natural timescale
		 * for a CMSampleBuffer. Fall back to the host clock only when the source
		 * supplied nothing usable. */
		if (pts_ns >= 0)
			timing.presentationTimeStamp = CMTimeMake(pts_ns, 1000000000);
		else
		{
			CMTime host = CMTimeConvertScale(CMClockGetTime(CMClockGetHostTimeClock()),
				1000000000, kCMTimeRoundingMethod_RoundHalfAwayFromZero);
			timing.presentationTimeStamp = host;
		}
		timing.duration = CMTimeMake(duration_ns, 1000000000);
		CMSampleBufferCreateReadyWithImageBuffer(NULL, pixelBuffer, description, &timing, &sample);
	}

	if (description)
		CFRelease(description);
	CVPixelBufferRelease(pixelBuffer);
	return sample;
}

- (BOOL)darling_shouldDeliverFrame
{
	if (!atomic_load(&m_running))
		return NO;
	if (![m_connection isEnabled])
		return NO;
	return YES;
}

- (void)deliverSample:(CMSampleBufferRef)sample dropped:(BOOL)dropped toDelegate:(id)delegate
{
	SEL selector = dropped
		? @selector(captureOutput:didDropSampleBuffer:fromConnection:)
		: @selector(captureOutput:didOutputSampleBuffer:fromConnection:);

	if (!delegate || ![delegate respondsToSelector: selector])
	{
		CFRelease(sample);
		return;
	}

	if (m_queue)
	{
		AVCaptureOutput *output = [m_output retain];
		AVCaptureConnection *connection = [m_connection retain];
		id strongDelegate = [delegate retain];
		/* Keep the engine alive for the queued callback. stopRunning joins the
		 * pump thread but must not wait on a delegate queue that can call it. */
		DarlingCameraCaptureEngine *strongEngine = [self retain];

		atomic_fetch_add(&m_inFlight, 1);
		dispatch_async(m_queue, ^{
			if (dropped)
				[strongDelegate captureOutput: output didDropSampleBuffer: sample fromConnection: connection];
			else
				[strongDelegate captureOutput: output didOutputSampleBuffer: sample fromConnection: connection];
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
		atomic_fetch_add(&m_inFlight, 1);
		if (dropped)
			[delegate captureOutput: m_output didDropSampleBuffer: sample fromConnection: m_connection];
		else
			[delegate captureOutput: m_output didOutputSampleBuffer: sample fromConnection: m_connection];
		CFRelease(sample);
		atomic_fetch_sub(&m_inFlight, 1);
	}
}

- (void)finishDelayedSample
{
	atomic_fetch_sub(&m_inFlight, 1);
}

- (void)deliverFrames
{
	while (atomic_load(&m_running))
	{
		@autoreleasepool
		{
			struct darling_pw_buffer buffer;
			struct darling_pw_format format;
			void *cookie = NULL;
			uint32_t width, height, rateNum, rateDenom;
			uint8_t *planes[4] = { NULL, NULL, NULL, NULL };
			uint32_t strides[4] = { 0, 0, 0, 0 };
			uint8_t *copyBlock = NULL;
			enum darling_camera_pixel_layout layout;
			OSType pixelFormat;
			int64_t ptsNs, durationNs;
			id delegate;
			BOOL droppedFrame;
			int count;

			if (darling_pw_stream_lock(m_stream) != 0)
				break;
			count = darling_pw_stream_dequeue(m_stream, &buffer, &cookie);
			if (count <= 0)
			{
				darling_pw_stream_unlock(m_stream);
				usleep(2000);
				continue;
			}

			memset(&format, 0, sizeof(format));
			darling_pw_stream_get_format(m_stream, &format);
			width = format.width ? format.width : [m_format darling_width];
			height = format.height ? format.height : [m_format darling_height];
			rateNum = format.framerate_num ? format.framerate_num : [m_format darling_framerate];
			rateDenom = format.framerate_denom ? format.framerate_denom : 1;
			if (rateNum == 0)
				rateNum = 30;
			if (rateDenom == 0)
				rateDenom = 1;
			durationNs = (int64_t) 1000000000 * rateDenom / rateNum;
			layout = DarlingCameraLayoutForFormat(format.video_format);
			if (!DarlingCameraBufferHasCompleteFrame(&buffer, layout, width, height))
			{
				darling_pw_stream_queue(m_stream, cookie, buffer.filled);
				darling_pw_stream_unlock(m_stream);
				continue;
			}

			ptsNs = buffer.pts_ns;
			droppedFrame = (buffer.flags & DARLING_PW_BUFFER_FLAG_CORRUPTED) != 0;
			if (buffer.flags & DARLING_PW_BUFFER_FLAG_DISCONT)
				m_lastPtsNs = -1;
			if (ptsNs < 0)
				ptsNs = m_lastPtsNs >= 0 ? m_lastPtsNs + durationNs : -1;
			if (ptsNs >= 0)
				m_lastPtsNs = ptsNs;

			if (DarlingCameraCopyPlanes(&buffer, planes, strides) == 0)
				copyBlock = planes[0];
			darling_pw_stream_queue(m_stream, cookie, buffer.filled);
			darling_pw_stream_unlock(m_stream);

			if (!copyBlock)
				continue;
			if (![self darling_shouldDeliverFrame])
			{
				free(copyBlock);
				continue;
			}

			delegate = [m_output sampleBufferDelegate];
			pixelFormat = DarlingCameraOutputFormatFromSettings(m_videoSettings);
			CMSampleBufferRef sample = DarlingCameraCreateSampleBuffer(
				(const uint8_t * const *) planes, strides, layout, width, height,
				ptsNs, durationNs, pixelFormat);
			free(copyBlock);
			if (!sample)
				continue;

			if (droppedFrame || (m_alwaysDiscardsLateVideoFrames && atomic_load(&m_inFlight) > 0))
				[self deliverSample: sample dropped: YES toDelegate: delegate];
			else
				[self deliverSample: sample dropped: NO toDelegate: delegate];
		}
	}
}

- (void)stop
{
	atomic_store(&m_running, false);
	if (atomic_exchange(&m_threadValid, false))
	{
		if (!pthread_equal(pthread_self(), m_thread))
			pthread_join(m_thread, NULL);
	}
	/* Queued callbacks retain the engine, so they cannot outlive it even if a
	 * delegate calls stopRunning from its own queue. */
	if (m_stream)
	{
		darling_pw_stream_destroy(m_stream);
		m_stream = NULL;
	}
	if (m_remoteFd >= 0)
	{
		close(m_remoteFd);
		m_remoteFd = -1;
	}
}

@end

#else /* !HAVE_PIPEWIRE */

@implementation DarlingCameraCaptureEngine
- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection delegate:(id)delegate
	queue:(dispatch_queue_t)queue format:(AVCaptureDeviceFormat *)format
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames
{
	(void) output; (void) connection; (void) delegate; (void) queue; (void) format;
	(void) videoSettings; (void) alwaysDiscardsLateVideoFrames;
	[self release];
	return nil;
}
- (void)start {}
- (void)stop {}
@end

#endif
