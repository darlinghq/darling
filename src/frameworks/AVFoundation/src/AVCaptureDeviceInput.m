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

#import <AVFoundation/AVCaptureDeviceInput.h>
#import <AVFoundation/AVError.h>
#import "camera/DarlingCameraBackend.h"
#import <objc/runtime.h>
#import <darling-stub-forwarding.h>

@implementation AVCaptureDeviceInput

+ (instancetype)deviceInputWithDevice:(AVCaptureDevice *)device error:(NSError **)error
{
	if (!device)
	{
		if (error)
			// AVErrorDeviceNotConnected, from the public AVError enum.
			*error = [NSError errorWithDomain: AVFoundationErrorDomain code: 11800 userInfo: nil];
		return nil;
	}
	return [AVCaptureDeviceInput darling_inputWithDevice: device];
}

- (AVCaptureDevice *)device
{
	return objc_getAssociatedObject(self, &kDarlingCameraDeviceInputDeviceKey);
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
