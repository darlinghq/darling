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
#import <GameController/GCController.h>
#import <GameController/GCExtendedGamepad.h>
#import <GameController/GCXboxGamepad.h>
#import <GameController/GCDualShockGamepad.h>
#import <GameController/GCDualSenseGamepad.h>
#import <GameController/GCMicroGamepad.h>
#import <GameController/GCDeviceLight.h>
#import <GameController/GCDeviceBattery.h>
#import <GameController/GCDeviceHaptics.h>
#import <GameController/GCGamepad.h>
#import <GameController/GCMotion.h>
#import <GameController/_GCControllerManager.h>
#import "GCInputInternal.h"

#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

@interface GCController () {
	IOHIDDeviceRef _hidDevice;
	NSString *_vendorName;
	NSString *_productCategory;
	GCControllerPlayerIndex _playerIndex;
	dispatch_queue_t _handlerQueue;

	GCExtendedGamepad *_extendedGamepad;
	GCMicroGamepad *_microGamepad;

	GCDeviceBattery *_battery;
	GCDeviceLight *_light;
	GCDeviceHaptics *_haptics;

	BOOL _isPlayStation;
	BOOL _isSnapshot;
	GCMotion *_motion;
	void (^_controllerPausedHandler)(GCController *);
}

- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device;
- (void)handleHIDValue:(IOHIDValueRef)value;
- (IOHIDDeviceRef)hidDevice;

@end

@implementation GCController

+ (NSArray<GCController *> *)controllers
{
	return [[_GCControllerManager sharedInstance] controllers];
}

+ (GCController *)current
{
	NSArray<GCController *> *all = [self controllers];
	return [all firstObject];
}

static BOOL gShouldMonitorBackgroundEvents;

+ (BOOL)shouldMonitorBackgroundEvents
{
	@synchronized (self) {
		return gShouldMonitorBackgroundEvents;
	}
}

+ (void)setShouldMonitorBackgroundEvents:(BOOL)monitor
{
	@synchronized (self) {
		gShouldMonitorBackgroundEvents = monitor;
	}
}

+ (GCController *)controllerWithExtendedGamepad
{
	return [[[self alloc] _initSnapshotWithExtendedGamepad:YES] autorelease];
}

+ (GCController *)controllerWithMicroGamepad
{
	return [[[self alloc] _initSnapshotWithExtendedGamepad:NO] autorelease];
}

+ (BOOL)supportsHIDDevice:(IOHIDDeviceRef)device
{
	CFTypeRef prod = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
	CFTypeRef vid = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
	CFTypeRef pid = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
	if (!device)
		return NO;

	CFTypeRef pageRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsagePageKey));
	CFTypeRef usageRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDPrimaryUsageKey));
	int page = 0, usage = 0;
	if (pageRef && CFGetTypeID(pageRef) == CFNumberGetTypeID())
		CFNumberGetValue((CFNumberRef)pageRef, kCFNumberIntType, &page);
	if (usageRef && CFGetTypeID(usageRef) == CFNumberGetTypeID())
		CFNumberGetValue((CFNumberRef)usageRef, kCFNumberIntType, &usage);

	if (page != kHIDPage_GenericDesktop)
		return NO;

	if (usage != kHIDUsage_GD_Joystick && usage != kHIDUsage_GD_GamePad && usage != kHIDUsage_GD_MultiAxisController)
		return NO;

	CFTypeRef reportSizeRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDMaxInputReportSizeKey));
	int reportSize = 0;
	if (reportSizeRef && CFGetTypeID(reportSizeRef) == CFNumberGetTypeID())
		CFNumberGetValue((CFNumberRef)reportSizeRef, kCFNumberIntType, &reportSize);
	if (reportSize <= 0)
		return NO;

	CFTypeRef descRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDReportDescriptorKey));
	if (!descRef)
		return NO;

	return YES;
}

+ (void)startWirelessControllerDiscoveryWithCompletionHandler:(void (^)(void))completionHandler
{
	if (completionHandler) {
		dispatch_async(dispatch_get_main_queue(), ^{
			completionHandler();
		});
	}
}

+ (void)stopWirelessControllerDiscovery
{
}

- (instancetype)initWithHIDDevice:(IOHIDDeviceRef)device
{
	self = [super init];
	if (!self)
		return nil;

	if (device)
		_hidDevice = (IOHIDDeviceRef)CFRetain(device);

	_playerIndex = GCControllerPlayerIndexUnset;
	_handlerQueue = dispatch_get_main_queue();

	int vendorID = 0;
	int productID = 0;
	NSString *productName = @"Game Controller";

	if (device) {
		CFTypeRef vRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
		CFTypeRef pRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
		CFTypeRef nRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));

		if (vRef && CFGetTypeID(vRef) == CFNumberGetTypeID())
			CFNumberGetValue((CFNumberRef)vRef, kCFNumberIntType, &vendorID);
		if (pRef && CFGetTypeID(pRef) == CFNumberGetTypeID())
			CFNumberGetValue((CFNumberRef)pRef, kCFNumberIntType, &productID);
		if (nRef && CFGetTypeID(nRef) == CFStringGetTypeID())
			productName = [NSString stringWithString:(__bridge NSString *)nRef];
	}

	_vendorName = [productName copy];

	// Determine category and layout
	if (vendorID == 0x045e || [productName rangeOfString:@"Xbox" options:NSCaseInsensitiveSearch].location != NSNotFound) {
		_productCategory = [GCProductCategoryXboxOne retain];
		_extendedGamepad = [[GCXboxGamepad alloc] initWithController:self];
	} else if (vendorID == 0x054c || [productName rangeOfString:@"DualShock" options:NSCaseInsensitiveSearch].location != NSNotFound || [productName rangeOfString:@"DualSense" options:NSCaseInsensitiveSearch].location != NSNotFound) {
		_isPlayStation = YES;
		if (productID == 0x0ce6 || productID == 0x0df2 || [productName rangeOfString:@"DualSense" options:NSCaseInsensitiveSearch].location != NSNotFound) {
			_productCategory = [GCProductCategoryDualSense retain];
			_extendedGamepad = [[GCDualSenseGamepad alloc] initWithController:self];
		} else {
			_productCategory = [GCProductCategoryDualShock4 retain];
			_extendedGamepad = [[GCDualShockGamepad alloc] initWithController:self];
		}
	} else {
		_productCategory = [GCProductCategoryHID retain];
		_extendedGamepad = [[GCExtendedGamepad alloc] initWithController:self];
	}

	_microGamepad = [[GCMicroGamepad alloc] initWithController:self];

	// No battery, light or haptics objects: the HID services publish none of the properties they would
	// be made from, and macOS answers nil for a device without them.

	return self;
}

- (instancetype)_initSnapshotWithExtendedGamepad:(BOOL)extended
{
	self = [super init];
	if (!self)
		return nil;
	_isSnapshot = YES;
	// QUIRK: a controller created with +controllerWithExtendedGamepad/MicroGamepad has player index 1
	// (0), not Unset like the controllers of devices; observed on macOS 26.7.3 (tier 2). Test:
	// gamecontroller_public_api, "snapshot: no battery, light, haptics; has motion; player index 0".
	_playerIndex = GCControllerPlayerIndex1;
	_handlerQueue = dispatch_get_main_queue();
	if (extended) {
		_vendorName = [@"ExtendedGamepad" copy];
		_productCategory = [GCProductCategoryMFi retain];
		_extendedGamepad = [[GCExtendedGamepad alloc] initWithController:self];
		_motion = [[GCMotion alloc] initWithController:self attitudeAndRotationRate:NO];
	} else {
		_vendorName = [@"MicroGamepad" copy];
		// the remote of a micro gamepad has no constant of its own
		_productCategory = [@"Siri Remote" copy];
		_microGamepad = [[GCMicroGamepad alloc] initWithController:self];
		_motion = [[GCMotion alloc] initWithController:self attitudeAndRotationRate:YES];
	}
	return self;
}

- (void)dealloc
{
	[_extendedGamepad _clearDevice];
	[_microGamepad _clearDevice];
	[_motion release];
	[_controllerPausedHandler release];
	if (_hidDevice)
		CFRelease(_hidDevice);
	[_vendorName release];
	[_productCategory release];
	[_extendedGamepad release];
	[_microGamepad release];
	[_battery release];
	[_light release];
	[_haptics release];
	[super dealloc];
}

- (IOHIDDeviceRef)hidDevice { return _hidDevice; }
- (NSString *)vendorName { return _vendorName; }
- (NSString *)productCategory { return _productCategory; }
- (GCControllerPlayerIndex)playerIndex { return _playerIndex; }
- (void)setPlayerIndex:(GCControllerPlayerIndex)playerIndex { _playerIndex = playerIndex; }

- (dispatch_queue_t)handlerQueue { return _handlerQueue; }
- (void)setHandlerQueue:(dispatch_queue_t)queue
{
	_handlerQueue = queue ? queue : dispatch_get_main_queue();
}

- (GCExtendedGamepad *)extendedGamepad { return _extendedGamepad; }

// QUIRK: on a controller created with +controllerWithExtendedGamepad, -microGamepad answers the extended
// profile object itself, as -gamepad does on every controller that has an extended profile; macOS 26
// answers that way (black-box observation, tier 2). Test: gamecontroller_public_api, "snapshot profiles".
- (GCMicroGamepad *)microGamepad { return _isSnapshot && _extendedGamepad ? (GCMicroGamepad *)_extendedGamepad : _microGamepad; }
- (GCGamepad *)gamepad { return (GCGamepad *)_extendedGamepad; }
- (GCPhysicalInputProfile *)physicalInputProfile { return _extendedGamepad ? _extendedGamepad : _microGamepad; }
- (GCMotion *)motion { return _motion; }
// The live input of the new input API (GCControllerLiveInput) is not implemented.
- (GCControllerLiveInput *)input { return nil; }
- (void (^)(GCController *))controllerPausedHandler { return _controllerPausedHandler; }
- (void)setControllerPausedHandler:(void (^)(GCController *))handler
{
	if (handler != _controllerPausedHandler) {
		[_controllerPausedHandler release];
		_controllerPausedHandler = [handler copy];
	}
}
- (BOOL)isSnapshot { return _isSnapshot; }

- (GCController *)capture
{
	GCController *copy = [[[GCController alloc] _initSnapshotWithExtendedGamepad:_extendedGamepad != nil] autorelease];
	GCPhysicalInputProfile *profile = copy.physicalInputProfile;
	[profile _forceStateFromPhysicalInput:self.physicalInputProfile];
	return copy;
}

- (GCDeviceBattery *)battery { return _battery; }
- (GCDeviceLight *)light { return _light; }
- (GCDeviceHaptics *)haptics { return _haptics; }

- (BOOL)isAttachedToDevice { return !_isSnapshot; }

- (void)handleHIDValue:(IOHIDValueRef)value
{
	IOHIDElementRef element = IOHIDValueGetElement(value);
	if (!element)
		return;

	uint32_t usagePage = IOHIDElementGetUsagePage(element);
	uint32_t usage = IOHIDElementGetUsage(element);
	CFIndex intVal = IOHIDValueGetIntegerValue(value);
	CFIndex min = IOHIDElementGetLogicalMin(element);
	CFIndex max = IOHIDElementGetLogicalMax(element);

	dispatch_queue_t queue = _handlerQueue;

	void (^extProfileHandler)(GCControllerElement *) = ^(GCControllerElement *elem) {
		if (_extendedGamepad.valueChangedHandler)
			_extendedGamepad.valueChangedHandler(_extendedGamepad, elem);
		if (_extendedGamepad.valueDidChangeHandler)
			_extendedGamepad.valueDidChangeHandler(_extendedGamepad, elem);
	};

	void (^microProfileHandler)(GCControllerElement *) = ^(GCControllerElement *elem) {
		if (_microGamepad.valueChangedHandler)
			_microGamepad.valueChangedHandler(_microGamepad, elem);
		if (_microGamepad.valueDidChangeHandler)
			_microGamepad.valueDidChangeHandler(_microGamepad, elem);
	};

	if (usagePage == kHIDPage_GenericDesktop) {
		if (usage == 0x39) {
			// Hat switch
			// Standard/Joystick: 1..8, 0 is centered.
			// PlayStation: 0..7, 8 is centered.
			BOOL up = NO, down = NO, left = NO, right = NO;
			if (_isPlayStation) {
				if (intVal >= 0 && intVal <= 7) {
					up = (intVal == 7 || intVal == 0 || intVal == 1);
					right = (intVal == 1 || intVal == 2 || intVal == 3);
					down = (intVal == 3 || intVal == 4 || intVal == 5);
					left = (intVal == 5 || intVal == 6 || intVal == 7);
				}
			} else {
				if (intVal >= 1 && intVal <= 8) {
					up = (intVal == 8 || intVal == 1 || intVal == 2);
					right = (intVal == 2 || intVal == 3 || intVal == 4);
					down = (intVal == 4 || intVal == 5 || intVal == 6);
					left = (intVal == 6 || intVal == 7 || intVal == 8);
				}
			}
			float x = right ? 1.0f : (left ? -1.0f : 0.0f);
			float y = up ? 1.0f : (down ? -1.0f : 0.0f);

			[_extendedGamepad.dpad _setXAxis:x yAxis:y handlerQueue:queue profileHandler:extProfileHandler];
			[_microGamepad.dpad _setXAxis:x yAxis:y handlerQueue:queue profileHandler:microProfileHandler];
		} else if (usage == 0x30) {
			// Left stick X
			float x = (max > min) ? ((float)(intVal - min) / (float)(max - min) * 2.0f - 1.0f) : 0.0f;
			[_extendedGamepad.leftThumbstick _setXAxis:x yAxis:_extendedGamepad.leftThumbstick.yAxis.value handlerQueue:queue profileHandler:extProfileHandler];
		} else if (usage == 0x31) {
			// Left stick Y (inverted: 0 is top)
			float y = (max > min) ? -((float)(intVal - min) / (float)(max - min) * 2.0f - 1.0f) : 0.0f;
			[_extendedGamepad.leftThumbstick _setXAxis:_extendedGamepad.leftThumbstick.xAxis.value yAxis:y handlerQueue:queue profileHandler:extProfileHandler];
		} else if (usage == 0x32) {
			// Right stick X
			float x = (max > min) ? ((float)(intVal - min) / (float)(max - min) * 2.0f - 1.0f) : 0.0f;
			[_extendedGamepad.rightThumbstick _setXAxis:x yAxis:_extendedGamepad.rightThumbstick.yAxis.value handlerQueue:queue profileHandler:extProfileHandler];
		} else if (usage == 0x35) {
			// Right stick Y (inverted)
			float y = (max > min) ? -((float)(intVal - min) / (float)(max - min) * 2.0f - 1.0f) : 0.0f;
			[_extendedGamepad.rightThumbstick _setXAxis:_extendedGamepad.rightThumbstick.xAxis.value yAxis:y handlerQueue:queue profileHandler:extProfileHandler];
		} else if (_isPlayStation && usage == 0x33) {
			// Left trigger (Rx in PlayStation)
			float val = (max > min) ? (float)(intVal - min) / (float)(max - min) : 0.0f;
			[_extendedGamepad.leftTrigger _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
		} else if (_isPlayStation && usage == 0x34) {
			// Right trigger (Ry in PlayStation)
			float val = (max > min) ? (float)(intVal - min) / (float)(max - min) : 0.0f;
			[_extendedGamepad.rightTrigger _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
		}
	} else if (usagePage == kHIDPage_Simulation) {
		if (!_isPlayStation && usage == 0xC5) {
			// Left trigger (Brake in Standard)
			float val = (max > min) ? (float)(intVal - min) / (float)(max - min) : 0.0f;
			[_extendedGamepad.leftTrigger _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
		} else if (!_isPlayStation && usage == 0xC4) {
			// Right trigger (Accelerator in Standard)
			float val = (max > min) ? (float)(intVal - min) / (float)(max - min) : 0.0f;
			[_extendedGamepad.rightTrigger _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
		}
	} else if (usagePage == kHIDPage_Button) {
		float val = (intVal != 0) ? 1.0f : 0.0f;
		if (_isPlayStation) {
			switch (usage) {
				case 1:
					[_extendedGamepad.buttonX _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonX _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 2:
					[_extendedGamepad.buttonA _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonA _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 3:
					[_extendedGamepad.buttonB _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 4:
					[_extendedGamepad.buttonY _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 5:
					[_extendedGamepad.leftShoulder _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 6:
					[_extendedGamepad.rightShoulder _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 9:
					[_extendedGamepad.buttonOptions _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 10:
					[_extendedGamepad.buttonMenu _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonMenu _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 11:
					[_extendedGamepad.leftThumbstickButton _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 12:
					[_extendedGamepad.rightThumbstickButton _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 13:
					[_extendedGamepad.buttonHome _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 14:
					if ([_extendedGamepad respondsToSelector:@selector(touchpadButton)]) {
						[(GCDualShockGamepad *)_extendedGamepad touchpadButton];
						[((GCDualShockGamepad *)_extendedGamepad).touchpadButton _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					}
					break;
				default:
					break;
			}
		} else {
			// Standard layout (Xbox / Generic)
			switch (usage) {
				case 1:
					[_extendedGamepad.buttonA _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonA _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 2:
					[_extendedGamepad.buttonB _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 4:
					[_extendedGamepad.buttonX _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonX _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 5:
					[_extendedGamepad.buttonY _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 7:
					[_extendedGamepad.leftShoulder _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 8:
					[_extendedGamepad.rightShoulder _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 11:
					[_extendedGamepad.buttonOptions _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 12:
					[_extendedGamepad.buttonMenu _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					[_microGamepad.buttonMenu _setValue:val handlerQueue:queue profileHandler:microProfileHandler];
					break;
				case 13:
					[_extendedGamepad.buttonHome _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 14:
					[_extendedGamepad.leftThumbstickButton _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 15:
					[_extendedGamepad.rightThumbstickButton _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 16:
					if ([_extendedGamepad isKindOfClass:[GCXboxGamepad class]])
						[((GCXboxGamepad *)_extendedGamepad).paddleButton1 _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 17:
					if ([_extendedGamepad isKindOfClass:[GCXboxGamepad class]])
						[((GCXboxGamepad *)_extendedGamepad).paddleButton2 _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 18:
					if ([_extendedGamepad isKindOfClass:[GCXboxGamepad class]])
						[((GCXboxGamepad *)_extendedGamepad).paddleButton3 _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				case 19:
					if ([_extendedGamepad isKindOfClass:[GCXboxGamepad class]])
						[((GCXboxGamepad *)_extendedGamepad).paddleButton4 _setValue:val handlerQueue:queue profileHandler:extProfileHandler];
					break;
				default:
					break;
			}
		}
	}
}

@end

#else
#import <GameController/GCController.h>
#import <GameController/GCMicroGamepad.h>
#import <GameController/GCPhysicalInputProfile.h>
#import <darling-stub-forwarding.h>

@implementation GCController

+ (NSArray *)controllers {
	return @[];
}

- (GCExtendedGamepad *)extendedGamepad {
	return nil;
}

- (GCMicroGamepad *)microGamepad {
	return nil;
}

- (GCPhysicalInputProfile *)physicalInputProfile {
	return nil;
}

- (NSString *)vendorName {
	return nil;
}

- (NSString *)productCategory {
	return @"";
}

- (GCControllerPlayerIndex)playerIndex {
	return GCControllerPlayerIndexUnset;
}

- (void)setPlayerIndex:(GCControllerPlayerIndex)playerIndex {
}

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
    return [super methodSignatureForSelector:aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    // Class methods that are not implemented are accepted, ignored and answer zero.
    void *result = NULL;
    [anInvocation setReturnValue:&result];
}

@end

#endif /* __LP64__ */
