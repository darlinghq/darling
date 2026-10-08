/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <Foundation/Foundation.h>

#import <GameController/GCControllerButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>

@interface GCExtendedGamepad : NSObject

@property(readonly) GCControllerButtonInput *buttonA;
@property(readonly) GCControllerButtonInput *buttonB;
@property(readonly) GCControllerButtonInput *buttonX;
@property(readonly) GCControllerButtonInput *buttonY;
@property(readonly) GCControllerButtonInput *buttonMenu;
@property(readonly, nullable) GCControllerButtonInput *buttonOptions;
@property(readonly) GCControllerDirectionPad *dpad;
@property(readonly) GCControllerDirectionPad *leftThumbstick;
@property(readonly) GCControllerDirectionPad *rightThumbstick;
@property(readonly) GCControllerButtonInput *leftShoulder;
@property(readonly) GCControllerButtonInput *rightShoulder;
@property(readonly) GCControllerButtonInput *leftTrigger;
@property(readonly) GCControllerButtonInput *rightTrigger;

@end
