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
#import <GameController/GCXboxGamepad.h>
#import "GCInputInternal.h"

@implementation GCXboxGamepad {
	GCControllerButtonInput *_buttonShare;
}

- (instancetype)initWithController:(GCController *)controller
{
	self = [super initWithController:controller style:GCProfileStyleXbox];
	if (!self)
		return nil;
	// a profile without a controller has no share button; one with a controller is assumed to have it
	if (controller) {
		_buttonShare = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonShare]];
		[self _registerButton:_buttonShare key:GCInputButtonShare];
	}
	return self;
}

- (void)dealloc
{
	[_buttonShare release];
	[super dealloc];
}

// The paddles are the back buttons of the profile (Paddle 1 and 2 on the right, 3 and 4 on the left);
// a profile that belongs to no controller answers nil for them, as macOS does.
- (GCControllerButtonInput *)_paddleNamed:(NSString *)name { return [self controller] ? self.buttons[name] : nil; }
- (GCControllerButtonInput *)paddleButton1 { return [self _paddleNamed:GCInputXboxPaddleOne]; }
- (GCControllerButtonInput *)paddleButton2 { return [self _paddleNamed:GCInputXboxPaddleTwo]; }
- (GCControllerButtonInput *)paddleButton3 { return [self _paddleNamed:GCInputXboxPaddleThree]; }
- (GCControllerButtonInput *)paddleButton4 { return [self _paddleNamed:GCInputXboxPaddleFour]; }
- (GCControllerButtonInput *)buttonShare { return _buttonShare; }

@end
#endif /* __LP64__ */
