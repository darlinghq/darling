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

// The implementation is 64-bit only, like GameController on macOS (its 32-bit
// Objective-C ABI cannot hold instance variables declared in the @implementation
// and the framework is x86_64 and arm64 only there); the 32-bit slice keeps the stub.
#ifdef __LP64__
#import <GameController/_GCControllerManager.h>
#import <GameController/GCController.h>
#import <GameController/GCMouse.h>
#import <GameController/GCKeyboard.h>

#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

@interface GCController ()
- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device;
- (void)handleHIDValue:(IOHIDValueRef)value;
- (IOHIDDeviceRef)hidDevice;
@end

@interface GCMouse ()
- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device;
- (void)handleHIDValue:(IOHIDValueRef)value;
@end

@interface GCKeyboard ()
- (void)handleHIDValue:(IOHIDValueRef)value;
@end

@interface _GCControllerManager () {
	IOHIDManagerRef _hidManager;       // game controllers
	IOHIDManagerRef _inputManager;     // keyboards and mice
	NSMutableArray<GCController *> *_controllers;
	NSMapTable *_deviceToController;

	NSMutableArray<GCMouse *> *_mice;
	NSMapTable *_deviceToMouse;
	GCMouse *_currentMouse;
	GCKeyboard *_coalescedKeyboard;
	NSCountedSet *_keyboardDevices;
}

- (void)addControllerForHIDDevice:(IOHIDDeviceRef)device;
- (void)removeControllerForHIDDevice:(IOHIDDeviceRef)device;
- (void)addInputDevice:(IOHIDDeviceRef)device;
- (void)removeInputDevice:(IOHIDDeviceRef)device;
- (void)handleInputValue:(IOHIDValueRef)value forDevice:(IOHIDDeviceRef)device;

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

static void _inputDeviceMatchingCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr addInputDevice:device];
}

static void _inputDeviceRemovalCallback(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	[mgr removeInputDevice:device];
}

static void _inputValueCallback(void *context, IOReturn result, void *sender, IOHIDValueRef value)
{
	_GCControllerManager *mgr = (__bridge _GCControllerManager *)context;
	IOHIDDeviceRef device = (IOHIDDeviceRef)sender;
	[mgr handleInputValue:value forDevice:device];
}

static void _postOnMainQueue(NSString *name, id object)
{
	[object retain];
	dispatch_async(dispatch_get_main_queue(), ^{
		[[NSNotificationCenter defaultCenter] postNotificationName:name object:object];
		[object release];
	});
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
	_deviceToMouse = [[NSMapTable alloc] initWithKeyOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsObjectPointerPersonality
											   valueOptions:NSPointerFunctionsStrongMemory
												   capacity:4];
	_keyboardDevices = [[NSCountedSet alloc] init];

	[self _setupHIDManager];
	[self _setupInputManager];

	return self;
}

- (void)dealloc
{
	if (_hidManager) {
		IOHIDManagerClose(_hidManager, kIOHIDOptionsTypeNone);
		CFRelease(_hidManager);
	}
	if (_inputManager) {
		IOHIDManagerClose(_inputManager, kIOHIDOptionsTypeNone);
		CFRelease(_inputManager);
	}
	[_controllers release];
	[_deviceToController release];
	[_mice release];
	[_deviceToMouse release];
	[_currentMouse release];
	[_coalescedKeyboard release];
	[_keyboardDevices release];
	[super dealloc];
}

static NSDictionary *_matchingForUsage(int page, int usage)
{
	return @{
		(id)CFSTR(kIOHIDDeviceUsagePageKey): @(page),
		(id)CFSTR(kIOHIDDeviceUsageKey): @(usage)
	};
}

// Creates a manager for the matching dictionaries, delivering on the main run loop in all common modes
// (so that input keeps flowing while a menu or a modal panel runs the loop in another mode).
- (IOHIDManagerRef)_newManagerMatching:(NSArray *)matching
							  matched:(IOHIDDeviceCallback)matched
							  removed:(IOHIDDeviceCallback)removed
{
	IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
	if (!manager)
		return NULL;

	IOHIDManagerSetDeviceMatchingMultiple(manager, (__bridge CFArrayRef)matching);
	IOHIDManagerRegisterDeviceMatchingCallback(manager, matched, (__bridge void *)self);
	IOHIDManagerRegisterDeviceRemovalCallback(manager, removed, (__bridge void *)self);
	IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetMain(), kCFRunLoopCommonModes);
	IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
	return manager;
}

- (void)_setupHIDManager
{
	_hidManager = [self _newManagerMatching:@[
		_matchingForUsage(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick),
		_matchingForUsage(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad),
		_matchingForUsage(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController)
	] matched:_deviceMatchingCallback removed:_deviceRemovalCallback];
	if (!_hidManager)
		return;

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

- (void)_setupInputManager
{
	_inputManager = [self _newManagerMatching:@[
		_matchingForUsage(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard),
		_matchingForUsage(kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse)
	] matched:_inputDeviceMatchingCallback removed:_inputDeviceRemovalCallback];
	if (!_inputManager)
		return;

	CFSetRef devices = IOHIDManagerCopyDevices(_inputManager);
	if (devices) {
		CFIndex count = CFSetGetCount(devices);
		const void **devs = (const void **)malloc(sizeof(void *) * count);
		if (devs) {
			CFSetGetValues(devices, devs);
			for (CFIndex i = 0; i < count; i++) {
				[self addInputDevice:(IOHIDDeviceRef)devs[i]];
			}
			free(devs);
		}
		CFRelease(devices);
	}
}

- (void)addInputDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	BOOL isKeyboard = IOHIDDeviceConformsTo(device, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
	BOOL isMouse = IOHIDDeviceConformsTo(device, kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse);
	if (!isKeyboard && !isMouse)
		return;

	@synchronized (self) {
		if ([_deviceToMouse objectForKey:(__bridge id)device] || [_keyboardDevices containsObject:(__bridge id)device])
			return;

		if (isMouse) {
			GCMouse *mouse = [[[GCMouse alloc] initWithHIDDevice:device] autorelease];
			[_mice addObject:mouse];
			[_deviceToMouse setObject:mouse forKey:(__bridge id)device];
			if (!_currentMouse)
				_currentMouse = [mouse retain];
			_postOnMainQueue(GCMouseDidConnectNotification, mouse);
		} else {
			[_keyboardDevices addObject:(__bridge id)device];
			if (!_coalescedKeyboard) {
				_coalescedKeyboard = [[GCKeyboard alloc] init];
				_postOnMainQueue(GCKeyboardDidConnectNotification, _coalescedKeyboard);
			}
		}
	}

	IOHIDDeviceRegisterInputValueCallback(device, _inputValueCallback, (__bridge void *)self);
	IOHIDDeviceScheduleWithRunLoop(device, CFRunLoopGetMain(), kCFRunLoopCommonModes);
}

- (void)removeInputDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	IOHIDDeviceRegisterInputValueCallback(device, NULL, NULL);
	IOHIDDeviceUnscheduleFromRunLoop(device, CFRunLoopGetMain(), kCFRunLoopCommonModes);

	@synchronized (self) {
		GCMouse *mouse = [[[_deviceToMouse objectForKey:(__bridge id)device] retain] autorelease];
		if (mouse) {
			[_mice removeObject:mouse];
			[_deviceToMouse removeObjectForKey:(__bridge id)device];
			if (_currentMouse == mouse) {
				[_currentMouse release];
				_currentMouse = [[_mice firstObject] retain];
			}
			_postOnMainQueue(GCMouseDidDisconnectNotification, mouse);
		} else if ([_keyboardDevices containsObject:(__bridge id)device]) {
			[_keyboardDevices removeObject:(__bridge id)device];
			if ([_keyboardDevices count] == 0 && _coalescedKeyboard) {
				GCKeyboard *keyboard = [_coalescedKeyboard autorelease];
				_coalescedKeyboard = nil;
				_postOnMainQueue(GCKeyboardDidDisconnectNotification, keyboard);
			}
		}
	}
}

- (void)addControllerForHIDDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	GCController *controller = nil;
	@synchronized (self) {
		if ([_deviceToController objectForKey:(__bridge id)device])
			return;

		if (![GCController supportsHIDDevice:device])
			return;

		controller = [[[GCController alloc] initWithHIDDevice:device] autorelease];
		[_controllers addObject:controller];
		[_deviceToController setObject:controller forKey:(__bridge id)device];
	}

	IOHIDDeviceRegisterInputValueCallback(device, _inputValueCallback, (__bridge void *)self);
	IOHIDDeviceScheduleWithRunLoop(device, CFRunLoopGetMain(), kCFRunLoopCommonModes);

	_postOnMainQueue(GCControllerDidConnectNotification, controller);
}

- (void)removeControllerForHIDDevice:(IOHIDDeviceRef)device
{
	if (!device)
		return;

	GCController *controller = nil;
	@synchronized (self) {
		controller = [[[_deviceToController objectForKey:(__bridge id)device] retain] autorelease];
		if (!controller)
			return;
		[_controllers removeObject:controller];
		[_deviceToController removeObjectForKey:(__bridge id)device];
	}

	IOHIDDeviceRegisterInputValueCallback(device, NULL, NULL);
	IOHIDDeviceUnscheduleFromRunLoop(device, CFRunLoopGetMain(), kCFRunLoopCommonModes);

	_postOnMainQueue(GCControllerDidDisconnectNotification, controller);
}

- (void)handleInputValue:(IOHIDValueRef)value forDevice:(IOHIDDeviceRef)device
{
	GCController *controller = nil;
	GCMouse *mouse = nil;
	GCKeyboard *keyboard = nil;
	@synchronized (self) {
		controller = [[[_deviceToController objectForKey:(__bridge id)device] retain] autorelease];
		mouse = [[[_deviceToMouse objectForKey:(__bridge id)device] retain] autorelease];
		if ([_keyboardDevices containsObject:(__bridge id)device])
			keyboard = [[_coalescedKeyboard retain] autorelease];
	}

	if (controller) {
		[controller handleHIDValue:value];
	} else if (mouse) {
		[self _makeCurrent:mouse];
		[mouse handleHIDValue:value];
	} else if (keyboard) {
		[keyboard handleHIDValue:value];
	}
}

// The current mouse is the one that most recently produced input.
- (void)_makeCurrent:(GCMouse *)mouse
{
	GCMouse *previous = nil;
	@synchronized (self) {
		if (_currentMouse == mouse)
			return;
		previous = _currentMouse;
		_currentMouse = [mouse retain];
	}
	if (previous) {
		_postOnMainQueue(GCMouseDidStopBeingCurrentNotification, previous);
		[previous release];
	}
	_postOnMainQueue(GCMouseDidBecomeCurrentNotification, mouse);
}

- (NSArray<GCController *> *)controllers
{
	@synchronized (self) {
		return [NSArray arrayWithArray:_controllers];
	}
}

- (NSArray<GCMouse *> *)mice
{
	@synchronized (self) {
		return [NSArray arrayWithArray:_mice];
	}
}

- (GCMouse *)currentMouse
{
	@synchronized (self) {
		return [[_currentMouse retain] autorelease];
	}
}

- (GCKeyboard *)coalescedKeyboard
{
	@synchronized (self) {
		return [[_coalescedKeyboard retain] autorelease];
	}
}

@end

#else
#import <GameController/_GCControllerManager.h>
#import <darling-stub-forwarding.h>

@implementation _GCControllerManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif /* __LP64__ */
