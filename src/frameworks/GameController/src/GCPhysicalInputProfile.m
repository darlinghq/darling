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
#import <GameController/GCControllerTouchpad.h>
#import "GCInputInternal.h"

// The symbol of the up/down/left/right buttons of a pad follows the symbol of the pad.
static NSString *GCDirectionSymbol(NSString *padSymbol, NSString *direction)
{
	if ([padSymbol isEqualToString:@"dpad"])
		return [NSString stringWithFormat:@"dpad.%@.fill", direction];
	if ([padSymbol hasSuffix:@".joystick"])
		return [NSString stringWithFormat:@"%@.tilt.%@", padSymbol, direction];
	return nil;
}

@implementation GCPhysicalInputProfile {
	id<GCDevice> _device;
	NSTimeInterval _lastEventTimestamp;
	NSMutableDictionary<NSString *, GCDeviceElement *> *_elements;
	NSMutableDictionary<NSString *, GCDeviceButtonInput *> *_buttons;
	NSMutableDictionary<NSString *, GCDeviceAxisInput *> *_axes;
	NSMutableDictionary<NSString *, GCDeviceDirectionPad *> *_dpads;
	NSMutableDictionary<NSString *, GCDeviceTouchpad *> *_touchpads;
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
	_touchpads = [[NSMutableDictionary alloc] init];
	return self;
}

- (void)dealloc
{
	// the elements may outlive the profile in the hands of the application
	for (GCControllerElement *element in [_elements allValues])
		[element _setProfile:nil];
	[_elements release];
	[_buttons release];
	[_axes release];
	[_dpads release];
	[_touchpads release];
	[_valueDidChangeHandler release];
	[super dealloc];
}

- (id<GCDevice>)device { return _device; }

// The device does not retain its profiles to avoid a cycle; it tells them when it goes away.
- (void)_clearDevice
{
	_device = nil;
}
- (NSTimeInterval)lastEventTimestamp { @synchronized (self) { return _lastEventTimestamp; } }
- (BOOL)hasRemappedElements { return NO; }

- (NSDictionary<NSString *, GCDeviceElement *> *)elements { return _elements; }
- (NSDictionary<NSString *, GCDeviceButtonInput *> *)buttons { return _buttons; }
- (NSDictionary<NSString *, GCDeviceAxisInput *> *)axes { return _axes; }
- (NSDictionary<NSString *, GCDeviceDirectionPad *> *)dpads { return _dpads; }
- (NSDictionary<NSString *, GCDeviceTouchpad *> *)touchpads { return _touchpads; }

// An element can be listed under several aliases, the sets hold it once.
- (NSSet<GCDeviceElement *> *)allElements { return [NSSet setWithArray:[_elements allValues]]; }
- (NSSet<GCDeviceButtonInput *> *)allButtons { return [NSSet setWithArray:[_buttons allValues]]; }
- (NSSet<GCDeviceAxisInput *> *)allAxes { return [NSSet setWithArray:[_axes allValues]]; }
- (NSSet<GCDeviceDirectionPad *> *)allDpads { return [NSSet setWithArray:[_dpads allValues]]; }
- (NSSet<GCDeviceTouchpad *> *)allTouchpads { return [NSSet setWithArray:[_touchpads allValues]]; }

- (GCDeviceElement *)objectForKeyedSubscript:(NSString *)key
{
	return _elements[key];
}

- (void)_noteEvent
{
	NSTimeInterval now = [[NSProcessInfo processInfo] systemUptime];
	@synchronized (self) {
		_lastEventTimestamp = now;
	}
}

- (dispatch_queue_t)_handlerQueue
{
	id<GCDevice> device = _device;
	dispatch_queue_t queue = device ? [device handlerQueue] : nil;
	return queue ? queue : dispatch_get_main_queue();
}

- (void)_registerElement:(GCControllerElement *)element key:(NSString *)key
{
	if (element && key) {
		_elements[key] = element;
		[element _setProfile:self];
	}
}

- (void)_registerButton:(GCControllerButtonInput *)button key:(NSString *)key
{
	if (button && key) {
		[self _registerElement:button key:key];
		_buttons[key] = button;
	}
}

- (void)_registerAxis:(GCControllerAxisInput *)axis key:(NSString *)key
{
	if (axis && key) {
		[self _registerElement:axis key:key];
		_axes[key] = axis;
	}
}

// A pad is listed under its key, and its axes and directions under the key plus a suffix.
- (void)_registerDpad:(GCControllerDirectionPad *)dpad key:(NSString *)key
{
	if (!dpad || !key)
		return;
	[dpad _assignAliasesForKey:key];
	[self _registerElement:dpad key:key];
	_dpads[key] = dpad;
	for (GCControllerAxisInput *axis in @[ dpad.xAxis, dpad.yAxis ])
		[self _registerAxis:axis key:[[axis aliases] anyObject]];
	for (GCControllerButtonInput *button in @[ dpad.up, dpad.down, dpad.left, dpad.right ])
		[self _registerButton:button key:[[button aliases] anyObject]];
}

- (void)_registerTouchpad:(GCControllerTouchpad *)touchpad key:(NSString *)key
{
	if (touchpad && key) {
		[self _registerElement:touchpad key:key];
		_touchpads[key] = touchpad;
	}
}

// Lists an element that is registered under one key under one more.
- (void)_registerAlias:(NSString *)alias forElement:(GCControllerElement *)element
{
	NSMutableSet *aliases = [[element aliases] mutableCopy];
	[aliases addObject:alias];
	[element _setAliases:aliases];
	[aliases release];
	if ([element isKindOfClass:[GCControllerButtonInput class]])
		[self _registerButton:(GCControllerButtonInput *)element key:alias];
	else if ([element isKindOfClass:[GCControllerAxisInput class]])
		[self _registerAxis:(GCControllerAxisInput *)element key:alias];
	else {
		[self _registerElement:element key:alias];
		if ([element isKindOfClass:[GCControllerDirectionPad class]])
			_dpads[alias] = (GCControllerDirectionPad *)element;
	}
}

- (void)_applyStyle:(GCProfileStyle)style
{
	size_t count = 0;
	const GCElementSpec *specs = GCElementSpecsForStyle(style, &count);
	for (size_t i = 0; i < count; i++) {
		NSString *key = @(specs[i].key);
		GCControllerElement *element = _elements[key];
		if (!element)
			continue;
		NSString *name = specs[i].localizedName ? @(specs[i].localizedName) : nil;
		NSString *symbol = specs[i].sfSymbolsName ? @(specs[i].sfSymbolsName) : nil;
		[element _setAnalog:specs[i].analog];
		[element _setBoundToSystemGesture:specs[i].boundToSystemGesture];
		[element setLocalizedName:name];
		[element setUnmappedLocalizedName:name];
		[element setSfSymbolsName:symbol];
		[element setUnmappedSfSymbolsName:symbol];
		if (![element isKindOfClass:[GCControllerDirectionPad class]])
			continue;
		GCControllerDirectionPad *pad = (GCControllerDirectionPad *)element;
		NSArray *children = @[ pad.xAxis, pad.yAxis, pad.up, pad.down, pad.left, pad.right ];
		NSArray *suffixes = @[ @"Horizontal", @"Vertical", @"Up", @"Down", @"Left", @"Right" ];
		NSArray *directions = @[ @"", @"", @"up", @"down", @"left", @"right" ];
		for (NSUInteger c = 0; c < 6; c++) {
			GCControllerElement *child = children[c];
			NSString *childName = name ? [NSString stringWithFormat:@"%@ (%@)", name, suffixes[c]] : nil;
			NSString *childSymbol = ([directions[c] length] && symbol) ? GCDirectionSymbol(symbol, directions[c]) : nil;
			[child setLocalizedName:childName];
			[child setUnmappedLocalizedName:childName];
			[child setSfSymbolsName:childSymbol];
			[child setUnmappedSfSymbolsName:childSymbol];
		}
	}
}

- (instancetype)capture
{
	GCPhysicalInputProfile *copy = [[[[self class] alloc] init] autorelease];
	[copy _forceStateFromPhysicalInput:self];
	return copy;
}

- (void)setStateFromPhysicalInput:(GCPhysicalInputProfile *)physicalInput
{
	BOOL writable = YES;
	id<GCDevice> device = _device;
	if (device) {
		// only the profile of a controller the application created accepts the state
		writable = NO;
		if ([device respondsToSelector:@selector(isSnapshot)])
			writable = [(id)device isSnapshot];
	}
	if (writable)
		[self _forceStateFromPhysicalInput:physicalInput];
}

- (void)_forceStateFromPhysicalInput:(GCPhysicalInputProfile *)physicalInput
{
	// the pads copy their children, so only elements that are not children are copied one by one
	for (NSString *key in [_elements allKeys]) {
		GCControllerElement *mine = _elements[key];
		GCControllerElement *theirs = physicalInput.elements[key];
		if (!theirs || [mine collection])
			continue;
		[mine _copyStateFromElement:theirs];
	}
}

- (NSString *)mappedElementAliasForPhysicalInputName:(NSString *)inputName
{
	return inputName;
}

- (NSSet<NSString *> *)mappedPhysicalInputNamesForElementAlias:(NSString *)elementAlias
{
	return elementAlias ? [NSSet setWithObject:elementAlias] : [NSSet set];
}

@end
#else
#import <GameController/GCPhysicalInputProfile.h>

@implementation GCPhysicalInputProfile

@end

#endif /* __LP64__ */
