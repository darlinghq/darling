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
#import <GameController/GCGamepad.h>
#import <GameController/GCController.h>
#import <GameController/GCGamepadSnapshot.h>
#import "GCInputInternal.h"

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@implementation GCGamepad {
	GCGamepadValueChangedHandler _valueChangedHandler;
	GCControllerDirectionPad *_dpad;
	GCControllerButtonInput *_buttonA;
	GCControllerButtonInput *_buttonB;
	GCControllerButtonInput *_buttonX;
	GCControllerButtonInput *_buttonY;
	GCControllerButtonInput *_buttonMenu;
	GCControllerButtonInput *_leftShoulder;
	GCControllerButtonInput *_rightShoulder;
}

@synthesize valueChangedHandler = _valueChangedHandler;

static GCControllerButtonInput *GCNewGamepadButton(NSString *alias)
{
	return [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:alias]];
}

- (instancetype)init
{
	return [self initWithController:nil];
}

- (instancetype)initWithController:(GCController *)controller
{
	self = [super initWithDevice:controller];
	if (!self)
		return nil;
	_dpad = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputDirectionPad]];
	_buttonA = GCNewGamepadButton(GCInputButtonA);
	_buttonB = GCNewGamepadButton(GCInputButtonB);
	_buttonX = GCNewGamepadButton(GCInputButtonX);
	_buttonY = GCNewGamepadButton(GCInputButtonY);
	_buttonMenu = GCNewGamepadButton(GCInputButtonMenu);
	_leftShoulder = GCNewGamepadButton(GCInputLeftShoulder);
	_rightShoulder = GCNewGamepadButton(GCInputRightShoulder);
	[self _registerDpad:_dpad key:GCInputDirectionPad];
	[self _registerButton:_buttonA key:GCInputButtonA];
	[self _registerButton:_buttonB key:GCInputButtonB];
	[self _registerButton:_buttonX key:GCInputButtonX];
	[self _registerButton:_buttonY key:GCInputButtonY];
	[self _registerButton:_buttonMenu key:GCInputButtonMenu];
	[self _registerButton:_leftShoulder key:GCInputLeftShoulder];
	[self _registerButton:_rightShoulder key:GCInputRightShoulder];
	return self;
}

- (void)dealloc
{
	[_dpad release];
	[_buttonA release];
	[_buttonB release];
	[_buttonX release];
	[_buttonY release];
	[_buttonMenu release];
	[_leftShoulder release];
	[_rightShoulder release];
	[_valueChangedHandler release];
	[super dealloc];
}

- (GCController *)controller { return (GCController *)[self device]; }
- (GCControllerDirectionPad *)dpad { return _dpad; }
- (GCControllerButtonInput *)buttonA { return _buttonA; }
- (GCControllerButtonInput *)buttonB { return _buttonB; }
- (GCControllerButtonInput *)buttonX { return _buttonX; }
- (GCControllerButtonInput *)buttonY { return _buttonY; }
- (GCControllerButtonInput *)leftShoulder { return _leftShoulder; }
- (GCControllerButtonInput *)rightShoulder { return _rightShoulder; }

- (GCGamepadSnapshot *)saveSnapshot
{
	return [[[GCGamepadSnapshot alloc] initWithSnapshotData:GCGamepadStateData(self)] autorelease];
}

@end

// The readers of the original data format refuse data of a later version (observed on macOS 26.7.3 for
// the extended gamepad; the other two follow the same rule).
BOOL GCGamepadSnapShotDataV100FromNSData(GCGamepadSnapShotDataV100 *snapshotData, NSData *data)
{
	if (!snapshotData || !GCSnapshotHeaderIsValid(data))
		return NO;
	uint16_t version;
	memcpy(&version, [data bytes], sizeof(version));
	if (version > 0x0100)
		return NO;
	memset(snapshotData, 0, sizeof(*snapshotData));
	memcpy(snapshotData, [data bytes], MIN([data length], sizeof(*snapshotData)));
	return YES;
}

NSData *NSDataFromGCGamepadSnapShotDataV100(GCGamepadSnapShotDataV100 *snapshotData)
{
	return snapshotData ? [NSData dataWithBytes:snapshotData length:sizeof(*snapshotData)] : nil;
}

NSData *GCGamepadStateData(GCGamepad *gamepad)
{
	GCGamepadSnapShotDataV100 d;
	memset(&d, 0, sizeof(d));
	d.version = 0x0100;
	d.size = sizeof(d);
	d.dpadX = gamepad.dpad.xAxis.value;
	d.dpadY = gamepad.dpad.yAxis.value;
	d.buttonA = gamepad.buttonA.value;
	d.buttonB = gamepad.buttonB.value;
	d.buttonX = gamepad.buttonX.value;
	d.buttonY = gamepad.buttonY.value;
	d.leftShoulder = gamepad.leftShoulder.value;
	d.rightShoulder = gamepad.rightShoulder.value;
	return NSDataFromGCGamepadSnapShotDataV100(&d);
}

BOOL GCApplyGamepadStateData(GCGamepad *gamepad, NSData *data)
{
	GCGamepadSnapShotDataV100 d;
	if (!GCGamepadSnapShotDataV100FromNSData(&d, data))
		return NO;
	[gamepad.dpad setValueForXAxis:d.dpadX yAxis:d.dpadY];
	[gamepad.buttonA setValue:d.buttonA];
	[gamepad.buttonB setValue:d.buttonB];
	[gamepad.buttonX setValue:d.buttonX];
	[gamepad.buttonY setValue:d.buttonY];
	[gamepad.leftShoulder setValue:d.leftShoulder];
	[gamepad.rightShoulder setValue:d.rightShoulder];
	return YES;
}
#else
#import <GameController/GCGamepad.h>
#import <darling-stub-forwarding.h>

@implementation GCGamepad

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
