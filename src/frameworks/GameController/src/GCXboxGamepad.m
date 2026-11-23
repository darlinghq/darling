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

#import <GameController/GCXboxGamepad.h>
#import "GCInputInternal.h"

@implementation GCXboxGamepad {
	GCControllerButtonInput *_paddleButton1;
	GCControllerButtonInput *_paddleButton2;
	GCControllerButtonInput *_paddleButton3;
	GCControllerButtonInput *_paddleButton4;
	GCControllerButtonInput *_buttonShare;
}

- (instancetype)initWithController:(GCController *)controller
{
	self = [super initWithController:controller];
	if (!self)
		return nil;

	_paddleButton1 = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputXboxPaddleOne]];
	_paddleButton2 = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputXboxPaddleTwo]];
	_paddleButton3 = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputXboxPaddleThree]];
	_paddleButton4 = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputXboxPaddleFour]];
	_buttonShare = [[GCControllerButtonInput alloc] initWithCollection:nil isAnalog:NO aliases:[NSSet setWithObject:GCInputButtonShare]];

	[self _registerButton:_paddleButton1 key:GCInputXboxPaddleOne];
	[self _registerButton:_paddleButton2 key:GCInputXboxPaddleTwo];
	[self _registerButton:_paddleButton3 key:GCInputXboxPaddleThree];
	[self _registerButton:_paddleButton4 key:GCInputXboxPaddleFour];
	[self _registerButton:_buttonShare key:GCInputButtonShare];

	return self;
}

- (void)dealloc
{
	[_paddleButton1 release];
	[_paddleButton2 release];
	[_paddleButton3 release];
	[_paddleButton4 release];
	[_buttonShare release];
	[super dealloc];
}

- (GCControllerButtonInput *)paddleButton1 { return _paddleButton1; }
- (GCControllerButtonInput *)paddleButton2 { return _paddleButton2; }
- (GCControllerButtonInput *)paddleButton3 { return _paddleButton3; }
- (GCControllerButtonInput *)paddleButton4 { return _paddleButton4; }
- (GCControllerButtonInput *)buttonShare { return _buttonShare; }

@end
