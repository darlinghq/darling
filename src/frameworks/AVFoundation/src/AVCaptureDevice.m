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

#import <AVFoundation/AVCaptureDevice.h>
#import "camera/DarlingCameraBackend.h"
#include <CoreAudio/AudioHardware.h>
#include <dispatch/dispatch.h>
#include <dlfcn.h>
#import <objc/runtime.h>
#import <darling-stub-forwarding.h>
#include <stdatomic.h>

typedef OSStatus (*GetPropertyDataFunc)(AudioObjectID, const AudioObjectPropertyAddress*, UInt32, const void*, UInt32*, void*);

static atomic_int g_videoAuthorizationStatus = AVAuthorizationStatusNotDetermined;
static dispatch_queue_t videoAuthorizationQueue(void)
{
	static dispatch_queue_t queue;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		queue = dispatch_queue_create("org.darlinghq.avfoundation.camera.authorization", DISPATCH_QUEUE_SERIAL);
	});
	return queue;
}

// CoreAudio is only loaded if a client asks about the microphone, as it brings
// in the audio backend, which most AVFoundation clients have no use for.
static BOOL hasDefaultInputDevice(void)
{
	static GetPropertyDataFunc getPropertyData;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		void* coreAudio = dlopen("/System/Library/Frameworks/CoreAudio.framework/CoreAudio", RTLD_LAZY | RTLD_LOCAL);
		if (coreAudio)
			getPropertyData = (GetPropertyDataFunc) dlsym(coreAudio, "AudioObjectGetPropertyData");
	});

	if (!getPropertyData)
		return NO;

	AudioDeviceID device = kAudioObjectUnknown;
	UInt32 size = sizeof(device);
	AudioObjectPropertyAddress address = {
		kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};

	return getPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &device) == noErr
		&& device != kAudioObjectUnknown;
}

@implementation AVCaptureDevice

+ (NSArray*)devices
{
	return [AVCaptureDevice darling_cameraDevices];
}

+ (NSArray*)devicesWithMediaType:(AVMediaType)mediaType
{
	if ([mediaType isEqualToString: AVMediaTypeAudio])
	{
		/* Audio devices are enumerated by CoreAudio, not by the camera portal. */
		return [NSArray array];
	}
	if ([mediaType isEqualToString: AVMediaTypeVideo])
		return [AVCaptureDevice darling_cameraDevices];
	return [NSArray array];
}

+ (AVCaptureDevice*)defaultDeviceWithMediaType:(AVMediaType)mediaType
{
	return [mediaType isEqualToString: AVMediaTypeVideo] ? [AVCaptureDevice darling_cameraDevice] : nil;
}

+ (AVAuthorizationStatus)authorizationStatusForMediaType:(AVMediaType)mediaType
{
	if (![mediaType isEqualToString: AVMediaTypeAudio] && ![mediaType isEqualToString: AVMediaTypeVideo])
	{
		[NSException raise: NSInvalidArgumentException
			format: @"Media type must be AVMediaTypeVideo or AVMediaTypeAudio, not %@", mediaType];
	}

	if ([mediaType isEqualToString: AVMediaTypeVideo])
	{
		if (!DarlingCameraBackendCameraPresent())
			return AVAuthorizationStatusRestricted;
		return (AVAuthorizationStatus) atomic_load(&g_videoAuthorizationStatus);
	}

	if (!hasDefaultInputDevice())
		return AVAuthorizationStatusRestricted;

	return AVAuthorizationStatusAuthorized;
}

+ (void)requestAccessForMediaType:(AVMediaType)mediaType completionHandler:(void (^)(BOOL granted))handler
{
	if (![mediaType isEqualToString: AVMediaTypeAudio] && ![mediaType isEqualToString: AVMediaTypeVideo])
	{
		[NSException raise: NSInvalidArgumentException
			format: @"Media type must be AVMediaTypeVideo or AVMediaTypeAudio, not %@", mediaType];
	}

	void (^copiedHandler)(BOOL) = [handler copy];
	if (!copiedHandler)
		return;

	if ([mediaType isEqualToString: AVMediaTypeAudio])
	{
		const BOOL granted = [self authorizationStatusForMediaType: mediaType] == AVAuthorizationStatusAuthorized;
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			copiedHandler(granted);
			[copiedHandler release];
		});
		return;
	}

	dispatch_async(videoAuthorizationQueue(), ^{
		BOOL granted = NO;
		const AVAuthorizationStatus status = (AVAuthorizationStatus) atomic_load(&g_videoAuthorizationStatus);
		if (status == AVAuthorizationStatusAuthorized || status == AVAuthorizationStatusDenied)
		{
			granted = status == AVAuthorizationStatusAuthorized;
		}
		else if (!DarlingCameraBackendCameraPresent())
		{
			atomic_store(&g_videoAuthorizationStatus, AVAuthorizationStatusRestricted);
			granted = NO;
		}
		else
		{
			const int result = DarlingCameraBackendRequestAccess();
			if (result == DARLING_CAMERA_RESULT_OK)
			{
				atomic_store(&g_videoAuthorizationStatus, AVAuthorizationStatusAuthorized);
				granted = YES;
			}
			else if (result == DARLING_CAMERA_RESULT_DENIED)
			{
				atomic_store(&g_videoAuthorizationStatus, AVAuthorizationStatusDenied);
				granted = NO;
			}
			else
			{
				granted = NO;
			}
		}

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			copiedHandler(granted);
			[copiedHandler release];
		});
	});
}

- (NSString *)uniqueID
{
	return objc_getAssociatedObject(self, &kDarlingCameraUniqueIDKey);
}

- (AVCaptureDeviceType)deviceType
{
	return objc_getAssociatedObject(self, &kDarlingCameraDeviceTypeKey);
}

- (NSString *)localizedName
{
	return objc_getAssociatedObject(self, &kDarlingCameraNameKey);
}

- (NSArray<AVCaptureDeviceFormat *> *)formats
{
	return objc_getAssociatedObject(self, &kDarlingCameraFormatsKey) ?: [NSArray array];
}

- (AVCaptureDeviceFormat *)activeFormat
{
	return objc_getAssociatedObject(self, &kDarlingCameraActiveFormatKey)
		?: [[self formats] firstObject];
}

- (void)setActiveFormat:(AVCaptureDeviceFormat *)format
{
	if ([[self formats] containsObject: format])
		objc_setAssociatedObject(self, &kDarlingCameraActiveFormatKey, format,
			OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (BOOL)hasMediaType:(AVMediaType)mediaType
{
	return [mediaType isEqualToString: AVMediaTypeVideo];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
