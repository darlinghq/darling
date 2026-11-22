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

#import <GameController/GCMicroGamepad.h>
#import <GameController/GCController.h>
#import "GCInputInternal.h"

@implementation GCMicroGamepad {
	GCController *_controller;
	GCMicroGamepadValueChangedHandler _valueChangedHandler;

	GCControllerDirectionPad *_dpad;
	GCControllerButtonInput *_buttonA;
	GCControllerButtonInput *_buttonX;
	GCControllerButtonInput *_buttonMenu;

	BOOL _reportsAbsoluteDpadValues;
	BOOL _allowsRotation;
}

@synthesize valueChangedHandler = _valueChangedHandler;
@synthesize reportsAbsoluteDpadValues = _reportsAbsoluteDpadValues;
@synthesize allowsRotation = _allowsRotation;

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
	_buttonX = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputButtonX]];
	_buttonMenu = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonMenu]];

	[self _registerDpad:_dpad key:GCInputDirectionPad];
	[self _registerButton:_buttonA key:GCInputButtonA];
	[self _registerButton:_buttonX key:GCInputButtonX];
	[self _registerButton:_buttonMenu key:GCInputButtonMenu];

	return self;
}

- (void)dealloc
{
	[_dpad release];
	[_buttonA release];
	[_buttonX release];
	[_buttonMenu release];
	[_valueChangedHandler release];
	[super dealloc];
}

- (GCController *)controller { return _controller; }
- (GCControllerDirectionPad *)dpad { return _dpad; }
- (GCControllerButtonInput *)buttonA { return _buttonA; }
- (GCControllerButtonInput *)buttonX { return _buttonX; }
- (GCControllerButtonInput *)buttonMenu { return _buttonMenu; }

@end
