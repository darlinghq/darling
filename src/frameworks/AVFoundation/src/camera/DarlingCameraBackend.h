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

#ifndef DARLING_CAMERA_BACKEND_H_
#define DARLING_CAMERA_BACKEND_H_

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

#import <AVFoundation/AVCaptureDevice.h>
#import <AVFoundation/AVCaptureDeviceFormat.h>
#import <AVFoundation/AVCaptureDeviceInput.h>
#import <AVFoundation/AVCaptureInputPort.h>
#import <AVFoundation/AVCaptureOutput.h>
#import <AVFoundation/AVCaptureConnection.h>
#import <AVFoundation/AVCaptureVideoDataOutput.h>
#import <AVFoundation/AVCaptureSession.h>

#include "DarlingCameraFrame.h"

#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
#include <darling_dbus.h>
#include <camera_portal.h>
#include <pipe_client.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
/* Exported by the optional PipeWire ELF wrapper. */
extern int darling_pipewire_available(void);
#endif

/* These mirror the portal results but stay usable in builds without PipeWire. */
enum
{
	DARLING_CAMERA_RESULT_OK = 0,
	DARLING_CAMERA_RESULT_NOT_AVAILABLE = -1,
	DARLING_CAMERA_RESULT_DENIED = -2,
	DARLING_CAMERA_RESULT_FAILED = -3,
	DARLING_CAMERA_RESULT_CANCELLED = -4,
};

/* Non-zero when the Camera portal can be reached and reports a camera. */
BOOL DarlingCameraBackendCameraPresent(void);
int DarlingCameraBackendRequestAccess(void);
/* On success the returned descriptor is owned by the caller. */
int DarlingCameraBackendOpenRemote(int *fdOut);

/* Associated-object keys shared with the primary class implementations. */
extern const char kDarlingCameraUniqueIDKey;
extern const char kDarlingCameraNodeIDKey;
extern const char kDarlingCameraDeviceTypeKey;
extern const char kDarlingCameraNameKey;
extern const char kDarlingCameraFormatsKey;
extern const char kDarlingCameraActiveFormatKey;
extern const char kDarlingCameraPortsKey;
extern const char kDarlingCameraConnectionsKey;
extern const char kDarlingCameraEnabledKey;
extern const char kDarlingCameraDeviceInputDeviceKey;
extern const char kDarlingCameraMediaTypeKey;

@interface AVCaptureDevice (DarlingCamera)
/* One device per PipeWire video source, or the portal camera when the source
 * list is unavailable. The Camera portal still performs source selection when
 * opening a capture stream. */
+ (NSArray<AVCaptureDevice *> *)darling_cameraDevices;
+ (AVCaptureDevice *)darling_cameraDevice;
/* Picks the device format that best matches a session preset. Returns nil
 * when the preset is unknown or the device has no formats. */
- (AVCaptureDeviceFormat *)darling_formatForPreset:(AVCaptureSessionPreset)preset;
/* Applies the width and height keys of videoSettings by selecting a supported
 * capture format. Returns nil when no format matches. */
- (AVCaptureDeviceFormat *)darling_formatForVideoSettings:(NSDictionary *)settings;
@end

@interface AVCaptureDeviceFormat (DarlingCamera)
+ (AVCaptureDeviceFormat *)darling_formatWithWidth:(uint32_t)width height:(uint32_t)height framerate:(uint32_t)framerate;
- (uint32_t)darling_width;
- (uint32_t)darling_height;
- (uint32_t)darling_framerate;
@end

@interface AVCaptureInputPort (DarlingCamera)
+ (AVCaptureInputPort *)darling_videoPortForDevice:(AVCaptureDevice *)device;
- (AVMediaType)darling_mediaType;
@end

@interface AVCaptureDeviceInput (DarlingCamera)
+ (AVCaptureDeviceInput *)darling_inputWithDevice:(AVCaptureDevice *)device;
@end

@interface AVCaptureOutput (DarlingCamera)
- (void)darling_addConnection:(AVCaptureConnection *)connection;
- (void)darling_removeConnection:(AVCaptureConnection *)connection;
@end

@interface AVCaptureConnection (DarlingCamera)
+ (AVCaptureConnection *)darling_connectionWithInputPorts:(NSArray *)ports output:(AVCaptureOutput *)output;
- (AVCaptureOutput *)darling_output;
@end

/* Owns one PipeWire capture stream and its delivery thread. The session owns
 * the engine; stop joins every thread before destroying the stream. */
@interface DarlingCameraCaptureEngine : NSObject
#if defined(HAVE_PIPEWIRE) && HAVE_PIPEWIRE
{
	AVCaptureVideoDataOutput *m_output;
	AVCaptureConnection *m_connection;
	dispatch_queue_t m_queue;
	AVCaptureDeviceFormat *m_format;
	NSDictionary *m_videoSettings;
	BOOL m_alwaysDiscardsLateVideoFrames;
	struct darling_pw_stream *m_stream;
	int m_remoteFd;
	pthread_t m_thread;
	atomic_bool m_running;
	atomic_bool m_threadValid;
	atomic_int m_inFlight;
	int64_t m_lastPtsNs;
}
#endif
- (instancetype)initWithOutput:(AVCaptureVideoDataOutput *)output
	connection:(AVCaptureConnection *)connection
	delegate:(id)delegate
	queue:(dispatch_queue_t)queue
	format:(AVCaptureDeviceFormat *)format
	videoSettings:(NSDictionary *)videoSettings
	alwaysDiscardsLateVideoFrames:(BOOL)alwaysDiscardsLateVideoFrames;
- (void)start;
- (void)stop;
@end

#endif
