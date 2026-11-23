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

// The implementation is 64-bit only, like GameController on macOS (its 32-bit
// Objective-C ABI cannot hold instance variables declared in the @implementation
// and the framework is x86_64 and arm64 only there); the 32-bit slice keeps the stub.
#ifdef __LP64__
#import <GameController/GCPhysicalInputProfile.h>
#import "GCInputInternal.h"

@implementation GCPhysicalInputProfile {
	id<GCDevice> _device;
	NSTimeInterval _lastEventTimestamp;
	NSMutableDictionary<NSString *, GCDeviceElement *> *_elements;
	NSMutableDictionary<NSString *, GCDeviceButtonInput *> *_buttons;
	NSMutableDictionary<NSString *, GCDeviceAxisInput *> *_axes;
	NSMutableDictionary<NSString *, GCDeviceDirectionPad *> *_dpads;
	void (^_valueDidChangeHandler)(GCPhysicalInputProfile *, GCControllerElement *);
}

@synthesize valueDidChangeHandler = _valueDidChangeHandler;

- (instancetype)init
{
	return [self initWithDevice:nil];
}

- (instancetype)initWithDevice:(id<GCDevice>)device
{
	self = [super init];
	if (!self)
		return nil;
	_device = device;
	_elements = [[NSMutableDictionary alloc] init];
	_buttons = [[NSMutableDictionary alloc] init];
	_axes = [[NSMutableDictionary alloc] init];
	_dpads = [[NSMutableDictionary alloc] init];
	return self;
}

- (void)dealloc
{
	[_elements release];
	[_buttons release];
	[_axes release];
	[_dpads release];
	[_valueDidChangeHandler release];
	[super dealloc];
}

- (id<GCDevice>)device { return _device; }
- (NSTimeInterval)lastEventTimestamp { return _lastEventTimestamp; }
- (BOOL)hasRemappedElements { return NO; }

- (NSDictionary<NSString *, GCDeviceElement *> *)elements { return _elements; }
- (NSDictionary<NSString *, GCDeviceButtonInput *> *)buttons { return _buttons; }
- (NSDictionary<NSString *, GCDeviceAxisInput *> *)axes { return _axes; }
- (NSDictionary<NSString *, GCDeviceDirectionPad *> *)dpads { return _dpads; }

- (NSSet<GCDeviceElement *> *)allElements { return [NSSet setWithArray:[_elements allValues]]; }
- (NSSet<GCDeviceButtonInput *> *)allButtons { return [NSSet setWithArray:[_buttons allValues]]; }
- (NSSet<GCDeviceAxisInput *> *)allAxes { return [NSSet setWithArray:[_axes allValues]]; }
- (NSSet<GCDeviceDirectionPad *> *)allDpads { return [NSSet setWithArray:[_dpads allValues]]; }

- (GCDeviceElement *)objectForKeyedSubscript:(NSString *)key
{
	return _elements[key];
}

- (void)_registerElement:(GCControllerElement *)element key:(NSString *)key
{
	if (element && key)
		_elements[key] = element;
}

- (void)_registerButton:(GCControllerButtonInput *)button key:(NSString *)key
{
	if (button && key) {
		_elements[key] = button;
		_buttons[key] = button;
	}
}

- (void)_registerAxis:(GCControllerAxisInput *)axis key:(NSString *)key
{
	if (axis && key) {
		_elements[key] = axis;
		_axes[key] = axis;
	}
}

- (void)_registerDpad:(GCControllerDirectionPad *)dpad key:(NSString *)key
{
	if (dpad && key) {
		_elements[key] = dpad;
		_dpads[key] = dpad;
	}
}

@end

#else
#import <GameController/GCPhysicalInputProfile.h>

@implementation GCPhysicalInputProfile

@end

#endif /* __LP64__ */
