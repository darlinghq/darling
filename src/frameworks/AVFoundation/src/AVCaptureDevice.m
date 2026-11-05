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
#include <CoreAudio/AudioHardware.h>
#include <dispatch/dispatch.h>
#include <dlfcn.h>

typedef OSStatus (*GetPropertyDataFunc)(AudioObjectID, const AudioObjectPropertyAddress*, UInt32, const void*, UInt32*, void*);

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
	return [NSArray array];
}

+ (NSArray*)devicesWithMediaType:(AVMediaType)mediaType
{
	return [self devices];
}

+ (AVCaptureDevice*)defaultDeviceWithMediaType:(AVMediaType)mediaType
{
	return nil;
}

+ (AVAuthorizationStatus)authorizationStatusForMediaType:(AVMediaType)mediaType
{
	if (![mediaType isEqualToString: AVMediaTypeAudio] && ![mediaType isEqualToString: AVMediaTypeVideo])
	{
		[NSException raise: NSInvalidArgumentException
			format: @"Media type must be AVMediaTypeVideo or AVMediaTypeAudio, not %@", mediaType];
	}

	// There is no consent database to ask, so only what can't work is refused:
	// nothing can capture video, and the user has no setting that would change that.
	if ([mediaType isEqualToString: AVMediaTypeVideo] || !hasDefaultInputDevice())
		return AVAuthorizationStatusRestricted;

	return AVAuthorizationStatusAuthorized;
}

+ (void)requestAccessForMediaType:(AVMediaType)mediaType completionHandler:(void (^)(BOOL granted))handler
{
	const BOOL granted = [self authorizationStatusForMediaType: mediaType] == AVAuthorizationStatusAuthorized;

	// The handler is documented to run on an arbitrary queue, so it can't run inside this
	// call: a caller may be holding a lock the handler takes, or waiting for it on this thread.
	void (^copiedHandler)(BOOL) = [handler copy];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		copiedHandler(granted);
		[copiedHandler release];
	});
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
