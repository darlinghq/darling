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

#import <GameController/GCMouse.h>
#import <GameController/GCProductCategories.h>
#import <GameController/_GCControllerManager.h>
#import "GCInputInternal.h"
#include <IOKit/IOKitLib.h>

NSString *const GCMouseDidConnectNotification = @"GCMouseDidConnectNotification";
NSString *const GCMouseDidDisconnectNotification = @"GCMouseDidDisconnectNotification";
NSString *const GCMouseDidBecomeCurrentNotification = @"GCMouseDidBecomeCurrentNotification";
NSString *const GCMouseDidStopBeingCurrentNotification = @"GCMouseDidStopBeingCurrentNotification";

@implementation GCMouseInput {
	GCDeviceButtonInput *_leftButton;
	GCDeviceButtonInput *_rightButton;
	GCDeviceButtonInput *_middleButton;
	GCDeviceDirectionPad *_scroll;
}

@synthesize mouseMovedHandler = _mouseMovedHandler;

- (instancetype)initWithMouse:(GCMouse *)mouse
{
	self = [super initWithDevice:mouse];
	if (!self)
		return nil;
	_leftButton = [[GCDeviceButtonInput alloc] init];
	_rightButton = [[GCDeviceButtonInput alloc] init];
	_middleButton = [[GCDeviceButtonInput alloc] init];
	_scroll = [[GCDeviceDirectionPad alloc] init];
	return self;
}

- (void)dealloc
{
	[_mouseMovedHandler release];
	[_leftButton release];
	[_rightButton release];
	[_middleButton release];
	[_scroll release];
	[super dealloc];
}

- (GCDeviceButtonInput *)leftButton { return _leftButton; }
- (GCDeviceButtonInput *)rightButton { return _rightButton; }
- (GCDeviceButtonInput *)middleButton { return _middleButton; }
- (NSArray *)auxiliaryButtons { return @[]; }
- (GCDeviceDirectionPad *)scroll { return _scroll; }

@end

@implementation GCMouse {
	io_service_t _service;
	NSString *_vendorName;
	GCMouseInput *_mouseInput;
	dispatch_queue_t _handlerQueue;
}

- (instancetype)init
{
	return [self initWithService:0];
}

- (instancetype)initWithService:(io_service_t)service
{
	self = [super init];
	if (!self)
		return nil;

	_service = service;
	if (_service)
		IOObjectRetain(_service);

	_handlerQueue = dispatch_get_main_queue();
	_mouseInput = [[GCMouseInput alloc] initWithMouse:self];

	NSString *name = @"Mouse";
	if (_service) {
		CFTypeRef prop = IORegistryEntryCreateCFProperty(_service, CFSTR("Product"), NULL, 0);
		if (prop && CFGetTypeID(prop) == CFStringGetTypeID()) {
			name = [NSString stringWithString:(__bridge NSString *)prop];
		}
		if (prop) CFRelease(prop);
	}
	_vendorName = [name copy];

	return self;
}

- (void)dealloc
{
	if (_service)
		IOObjectRelease(_service);
	[_vendorName release];
	[_mouseInput release];
	[super dealloc];
}

- (io_service_t)service { return _service; }

+ (NSArray<GCMouse *> *)mice
{
	return [[_GCControllerManager sharedInstance] mice];
}

+ (GCMouse *)current
{
	return [[_GCControllerManager sharedInstance] currentMouse];
}

- (GCMouseInput *)mouseInput
{
	return _mouseInput;
}

- (GCPhysicalInputProfile *)physicalInputProfile
{
	return _mouseInput;
}

- (NSString *)vendorName
{
	return _vendorName;
}

- (NSString *)productCategory
{
	return GCProductCategoryMouse;
}

- (BOOL)isAttachedToDevice
{
	return YES;
}

- (dispatch_queue_t)handlerQueue
{
	return _handlerQueue;
}

- (void)setHandlerQueue:(dispatch_queue_t)queue
{
	_handlerQueue = queue ? queue : dispatch_get_main_queue();
}

@end
