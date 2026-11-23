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
#import <GameController/GCExtendedGamepad.h>
#import <GameController/GCController.h>
#import "GCInputInternal.h"

@implementation GCExtendedGamepad {
	GCController *_controller;
	GCExtendedGamepadValueChangedHandler _valueChangedHandler;

	GCControllerDirectionPad *_dpad;
	GCControllerButtonInput *_buttonA;
	GCControllerButtonInput *_buttonB;
	GCControllerButtonInput *_buttonX;
	GCControllerButtonInput *_buttonY;
	GCControllerButtonInput *_buttonMenu;
	GCControllerButtonInput *_buttonOptions;
	GCControllerButtonInput *_buttonHome;

	GCControllerDirectionPad *_leftThumbstick;
	GCControllerDirectionPad *_rightThumbstick;

	GCControllerButtonInput *_leftShoulder;
	GCControllerButtonInput *_rightShoulder;
	GCControllerButtonInput *_leftTrigger;
	GCControllerButtonInput *_rightTrigger;

	GCControllerButtonInput *_leftThumbstickButton;
	GCControllerButtonInput *_rightThumbstickButton;
}

@synthesize valueChangedHandler = _valueChangedHandler;

- (instancetype)init
{
	return [self initWithController:nil];
}

- (instancetype)initWithController:(GCController *)controller
{
	self = [super initWithDevice:controller];
	if (!self)
		return nil;

	_controller = controller;

	_dpad = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputDirectionPad]];
	_buttonA = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputButtonA]];
	_buttonB = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputButtonB]];
	_buttonX = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputButtonX]];
	_buttonY = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputButtonY]];
	_buttonMenu = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonMenu]];
	_buttonOptions = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonOptions]];
	_buttonHome = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonHome]];

	_leftThumbstick = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputLeftThumbstick]];
	_rightThumbstick = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputRightThumbstick]];

	_leftShoulder = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputLeftShoulder]];
	_rightShoulder = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputRightShoulder]];
	_leftTrigger = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputLeftTrigger]];
	_rightTrigger = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputRightTrigger]];

	_leftThumbstickButton = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputLeftThumbstickButton]];
	_rightThumbstickButton = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputRightThumbstickButton]];

	[self _registerDpad:_dpad key:GCInputDirectionPad];
	[self _registerButton:_buttonA key:GCInputButtonA];
	[self _registerButton:_buttonB key:GCInputButtonB];
	[self _registerButton:_buttonX key:GCInputButtonX];
	[self _registerButton:_buttonY key:GCInputButtonY];
	[self _registerButton:_buttonMenu key:GCInputButtonMenu];
	[self _registerButton:_buttonOptions key:GCInputButtonOptions];
	[self _registerButton:_buttonHome key:GCInputButtonHome];

	[self _registerDpad:_leftThumbstick key:GCInputLeftThumbstick];
	[self _registerDpad:_rightThumbstick key:GCInputRightThumbstick];

	[self _registerButton:_leftShoulder key:GCInputLeftShoulder];
	[self _registerButton:_rightShoulder key:GCInputRightShoulder];
	[self _registerButton:_leftTrigger key:GCInputLeftTrigger];
	[self _registerButton:_rightTrigger key:GCInputRightTrigger];

	[self _registerButton:_leftThumbstickButton key:GCInputLeftThumbstickButton];
	[self _registerButton:_rightThumbstickButton key:GCInputRightThumbstickButton];

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
	[_buttonOptions release];
	[_buttonHome release];
	[_leftThumbstick release];
	[_rightThumbstick release];
	[_leftShoulder release];
	[_rightShoulder release];
	[_leftTrigger release];
	[_rightTrigger release];
	[_leftThumbstickButton release];
	[_rightThumbstickButton release];
	[_valueChangedHandler release];
	[super dealloc];
}

- (GCController *)controller { return _controller; }
- (GCControllerDirectionPad *)dpad { return _dpad; }
- (GCControllerButtonInput *)buttonA { return _buttonA; }
- (GCControllerButtonInput *)buttonB { return _buttonB; }
- (GCControllerButtonInput *)buttonX { return _buttonX; }
- (GCControllerButtonInput *)buttonY { return _buttonY; }
- (GCControllerButtonInput *)buttonMenu { return _buttonMenu; }
- (GCControllerButtonInput *)buttonOptions { return _buttonOptions; }
- (GCControllerButtonInput *)buttonHome { return _buttonHome; }
- (GCControllerDirectionPad *)leftThumbstick { return _leftThumbstick; }
- (GCControllerDirectionPad *)rightThumbstick { return _rightThumbstick; }
- (GCControllerButtonInput *)leftShoulder { return _leftShoulder; }
- (GCControllerButtonInput *)rightShoulder { return _rightShoulder; }
- (GCControllerButtonInput *)leftTrigger { return _leftTrigger; }
- (GCControllerButtonInput *)rightTrigger { return _rightTrigger; }
- (GCControllerButtonInput *)leftThumbstickButton { return _leftThumbstickButton; }
- (GCControllerButtonInput *)rightThumbstickButton { return _rightThumbstickButton; }

@end

#else
#import <GameController/GCExtendedGamepad.h>
#import <darling-stub-forwarding.h>

@implementation GCExtendedGamepad

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
