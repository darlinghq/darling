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
#import <GameController/GCDualShockGamepad.h>
#import "GCInputInternal.h"

@implementation GCDualShockGamepad {
	GCControllerButtonInput *_touchpadButton;
	GCControllerDirectionPad *_touchpadPrimary;
	GCControllerDirectionPad *_touchpadSecondary;
}

- (instancetype)initWithController:(GCController *)controller
{
	return [self initWithController:controller style:GCProfileStyleDualShock];
}

- (instancetype)initWithController:(GCController *)controller style:(GCProfileStyle)style
{
	self = [super initWithController:controller style:style];
	if (!self)
		return nil;
	_touchpadButton = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputDualShockTouchpadButton]];
	_touchpadPrimary = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputDualShockTouchpadOne]];
	_touchpadSecondary = [[GCControllerDirectionPad alloc] initWithCollection:nil isAnalog:YES aliases:[NSSet setWithObject:GCInputDualShockTouchpadTwo]];
	[self _registerButton:_touchpadButton key:GCInputDualShockTouchpadButton];
	[self _registerDpad:_touchpadPrimary key:GCInputDualShockTouchpadOne];
	[self _registerDpad:_touchpadSecondary key:GCInputDualShockTouchpadTwo];
	[self _applyStyle:style];
	return self;
}

- (void)dealloc
{
	[_touchpadButton release];
	[_touchpadPrimary release];
	[_touchpadSecondary release];
	[super dealloc];
}

- (GCControllerButtonInput *)touchpadButton { return _touchpadButton; }
- (GCControllerDirectionPad *)touchpadPrimary { return _touchpadPrimary; }
- (GCControllerDirectionPad *)touchpadSecondary { return _touchpadSecondary; }

@end
#endif /* __LP64__ */
