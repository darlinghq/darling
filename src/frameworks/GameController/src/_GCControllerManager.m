/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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

#import <GameController/_GCControllerManager.h>
#import <GameController/GCController.h>
#import <GameController/GCMouse.h>
#import <GameController/GCKeyboard.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

@interface GCController ()
- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device;
- (void)handleHIDValue:(IOHIDValueRef)value;
- (IOHIDDeviceRef)hidDevice;
@end

@interface GCMouse ()
- (instancetype)initWithService:(io_service_t)service;
- (io_service_t)service;
@end

@interface _GCControllerManager () {
	IOHIDManagerRef _hidManager;
	NSMutableArray<GCController *> *_controllers;
	NSMapTable *_deviceToController;

	NSMutableArray<GCMouse *> *_mice;
	GCMouse *_currentMouse;
	GCKeyboard *_coalescedKeyboard;
	NSMutableSet<NSNumber *> *_keyboardServices;

	IONotificationPortRef _notifyPort;
	io_iterator_t _matchedIter;
	io_iterator_t _terminatedIter;
}

- (void)addControllerForHIDDevice:(IOHIDDeviceRef)device;
- (void)removeControllerForHIDDevice:(IOHIDDeviceRef)device;
- (void)handleInputValue:(IOHIDValueRef)value forDevice:(IOHIDDeviceRef)device;

- (void)serviceMatched:(io_iterator_t)iter;
- (void)serviceTerminated:(io_iterator_t)iter;

@end

static void _deviceMatchingCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr addControllerForHIDDevice:device];
}

static void _deviceRemovalCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr removeControllerForHIDDevice:device];
}

static void _inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	IOHIDDeviceRef device = (IOHIDDeviceRef)sender;
	[mgr handleInputValue:value forDevice:device];
}

static void _serviceMatchedCallback(void *context, io_iterator_t iterator)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr serviceMatched:iterator];
}

static void _serviceTerminatedCallback(void *context, io_iterator_t iterator)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr serviceTerminated:iterator];
}

@implementation _GCControllerManager

+ (_GCControllerManager *)sharedInstance
{
	static _GCControllerManager *s_instance = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		s_instance = [[_GCControllerManager alloc] init];
	});
	return s_instance;
}

- (instancetype)init
{
	self = [super init];
	if (!self)
		return nil;

	_controllers = [[NSMutableArray alloc] init];
	_deviceToController = [[NSMapTable alloc] initWithKeyOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsObjectPointerPersonality
												  valueOptions:NSPointerFunctionsStrongMemory
													  capacity:4];
	_mice = [[NSMutableArray alloc] init];
	_keyboardServices = [[NSMutableSet alloc] init];

	[self _setupHIDManager];
	[self _setupServiceNotifications];

	return self;
}

- (void)dealloc
{
	if (_hidManager) {
		IOHIDManagerClose(_hidManager, kIOHIDOptionsTypeNone);
		CFRelease(_hidManager);
	}
	if (_notifyPort) {
		IONotificationPortDestroy(_notifyPort);
	}
	if (_matchedIter) {
		IOObjectRelease(_matchedIter);
	}
	if (_terminatedIter) {
		IOObjectRelease(_terminatedIter);
	}
	[_controllers release];
	[_deviceToController release];
	[_mice release];
	[_currentMouse release];
	[_coalescedKeyboard release];
	[_keyboardServices release];
	[super dealloc];
}

- (void)_setupHIDManager
{
	_hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
	if (!_hidManager) {
		return;
	}

	NSArray *matching = @[
		@{
			(id)CFSTR(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop),
			(id)CFSTR(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_Joystick)
		},
		@{
			(id)CFSTR(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop),
			(id)CFSTR(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)
		},
		@{
			(id)CFSTR(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop),
			(id)CFSTR(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_MultiAxisController)
		}
	];

	IOHIDManagerSetDeviceMatchingMultiple(_hidManager, (__bridge CFArrayRef)matching);
	IOHIDManagerRegisterDeviceMatchingCallback(_hidManager, _deviceMatchingCallback, (__bridge void *)self);
	IOHIDManagerRegisterDeviceRemovalCallback(_hidManager, _deviceRemovalCallback, (__bridge void *)self);

	CFRunLoopRef runLoop = CFRunLoopGetMain();
	if (!runLoop)
		runLoop = CFRunLoopGetCurrent();
	if (runLoop) {
		IOHIDManagerScheduleWithRunLoop(_hidManager, runLoop, kCFRunLoopDefaultMode);
	}

	IOHIDManagerOpen(_hidManager, kIOHIDOptionsTypeNone);

	CFSetRef devices = IOHIDManagerCopyDevices(_hidManager);
	if (devices) {
		CFIndex count = CFSetGetCount(devices);
		const void **devs = (const void **)malloc(sizeof(void *) * count);
		if (devs) {
			CFSetGetValues(devices, devs);
			for (CFIndex i = 0; i < count; i++) {
				[self addControllerForHIDDevice:(IOHIDDeviceRef)devs[i]];
			}
			free(devs);
		}
		CFRelease(devices);
	}
}

- (void)_setupServiceNotifications
{
	_notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
	if (!_notifyPort)
		return;

	CFRunLoopSourceRef source = IONotificationPortGetRunLoopSource(_notifyPort);
	if (source) {
		CFRunLoopRef runLoop = CFRunLoopGetMain();
		if (!runLoop)
			runLoop = CFRunLoopGetCurrent();
		if (runLoop) {
			CFRunLoopAddSource(runLoop, source, kCFRunLoopDefaultMode);
		}
	}

	CFMutableDictionaryRef matchDict = IOServiceMatching("IOHIDDevice");
	if (matchDict) {
		IOServiceAddMatchingNotification(_notifyPort,
										 kIOMatchedNotification,
										 matchDict,
										 _serviceMatchedCallback,
										 (__bridge void *)self,
										 &_matchedIter);
		if (_matchedIter) {
			[self serviceMatched:_matchedIter];
		}
	}

	CFMutableDictionaryRef termDict = IOServiceMatching("IOHIDDevice");
	if (termDict) {
		IOServiceAddMatchingNotification(_notifyPort,
										 kIOTerminatedNotification,
										 termDict,
										 _serviceTerminatedCallback,
										 (__bridge void *)self,
										 &_terminatedIter);
		if (_terminatedIter) {
			[self serviceTerminated:_terminatedIter];
		}
	}
}

- (void)serviceMatched:(io_iterator_t)iterator
{
	io_service_t service;
	while ((service = IOIteratorNext(iterator))) {
		CFTypeRef pageRef = IORegistryEntryCreateCFProperty(service, CFSTR("PrimaryUsagePage"), NULL, 0);
		CFTypeRef usageRef = IORegistryEntryCreateCFProperty(service, CFSTR("PrimaryUsage"), NULL, 0);
		int page = 0, usage = 0;

		if (pageRef && CFGetTypeID(pageRef) == CFNumberGetTypeID())
			CFNumberGetValue((CFNumberRef)pageRef, kCFNumberIntType, &page);
		if (usageRef && CFGetTypeID(usageRef) == CFNumberGetTypeID())
			CFNumberGetValue((CFNumberRef)usageRef, kCFNumberIntType, &usage);

		if (pageRef) CFRelease(pageRef);
		if (usageRef) CFRelease(usageRef);

		if (page == 1) {
			if (usage == 2) {
				// Mouse service
				GCMouse *mouse = [[[GCMouse alloc] initWithService:service] autorelease];
				[_mice addObject:mouse];
				if (!_currentMouse)
					_currentMouse = [mouse retain];

				dispatch_async(dispatch_get_main_queue(), ^{
					[[NSNotificationCenter defaultCenter] postNotificationName:GCMouseDidConnectNotification object:mouse];
				});
			} else if (usage == 6) {
				// Keyboard service
				[_keyboardServices addObject:@(service)];
				if (!_coalescedKeyboard) {
					_coalescedKeyboard = [[GCKeyboard alloc] init];
					GCKeyboard *kb = _coalescedKeyboard;
					dispatch_async(dispatch_get_main_queue(), ^{
						[[NSNotificationCenter defaultCenter] postNotificationName:GCKeyboardDidConnectNotification object:kb];
					});
				}
			}
		}
		IOObjectRelease(service);
	}
}

- (void)serviceTerminated:(io_iterator_t)iterator
{
	io_service_t service;
	while ((service = IOIteratorNext(iterator))) {
		// Check if mouse
		GCMouse *foundMouse = nil;
		for (GCMouse *m in _mice) {
			if ([m service] == service) {
				foundMouse = m;
				break;
			}
		}
		if (foundMouse) {
			[foundMouse retain];
			[_mice removeObject:foundMouse];
			if (_currentMouse == foundMouse) {
				[_currentMouse release];
				_currentMouse = [[_mice firstObject] retain];
			}
			dispatch_async(dispatch_get_main_queue(), ^{
				[[NSNotificationCenter defaultCenter] postNotificationName:GCMouseDidDisconnectNotification object:foundMouse];
				[foundMouse release];
			});
		}

		// Check if keyboard
		NSNumber *svcNum = @(service);
		if ([_keyboardServices containsObject:svcNum]) {
			[_keyboardServices removeObject:svcNum];
			if ([_keyboardServices count] == 0 && _coalescedKeyboard) {
				GCKeyboard *kb = [_coalescedKeyboard retain];
				[_coalescedKeyboard release];
				_coalescedKeyboard = nil;
				dispatch_async(dispatch_get_main_queue(), ^{
					[[NSNotificationCenter defaultCenter] postNotificationName:GCKeyboardDidDisconnectNotification object:kb];
					[kb release];
				});
			}
		}
		IOObjectRelease(service);
	}
}

- (void)addControllerForHIDDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	if ([_deviceToController objectForKey:(__bridge id)device]) {
		return;
	}

	BOOL supported = [GCController supportsHIDDevice:device];
	if (!supported)
		return;

	GCController *controller = [[[GCController alloc] initWithHIDDevice:device] autorelease];
	[_controllers addObject:controller];
	[_deviceToController setObject:controller forKey:(__bridge id)device];

	IOHIDDeviceRegisterInputValueCallback(device, _inputValueCallback, (__bridge void *)self);
	CFRunLoopRef runLoop = CFRunLoopGetMain();
	if (!runLoop)
		runLoop = CFRunLoopGetCurrent();
	if (runLoop) {
		IOHIDDeviceScheduleWithRunLoop(device, runLoop, kCFRunLoopDefaultMode);
	}

	dispatch_async(dispatch_get_main_queue(), ^{
		[[NSNotificationCenter defaultCenter] postNotificationName:GCControllerDidConnectNotification object:controller];
	});
}

- (void)removeControllerForHIDDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	GCController *controller = [_deviceToController objectForKey:(__bridge id)device];
	if (!controller)
		return;

	IOHIDDeviceRegisterInputValueCallback(device, NULL, NULL);
	CFRunLoopRef runLoop = CFRunLoopGetMain();
	if (!runLoop)
		runLoop = CFRunLoopGetCurrent();
	if (runLoop) {
		IOHIDDeviceUnscheduleFromRunLoop(device, runLoop, kCFRunLoopDefaultMode);
	}

	[controller retain];
	[_controllers removeObject:controller];
	[_deviceToController removeObjectForKey:(__bridge id)device];

	dispatch_async(dispatch_get_main_queue(), ^{
		[[NSNotificationCenter defaultCenter] postNotificationName:GCControllerDidDisconnectNotification object:controller];
		[controller release];
	});
}

- (void)handleInputValue:(IOHIDValueRef)value forDevice:(IOHIDDeviceRef)device
{
	GCController *controller = [_deviceToController objectForKey:(__bridge id)device];
	if (controller) {
		[controller handleHIDValue:value];
	}
}

- (NSArray<GCController *> *)controllers
{
	return [NSArray arrayWithArray:_controllers];
}

- (NSArray<GCMouse *> *)mice
{
	return [NSArray arrayWithArray:_mice];
}

- (GCMouse *)currentMouse
{
	return _currentMouse;
}

- (GCKeyboard *)coalescedKeyboard
{
	return _coalescedKeyboard;
}

@end
