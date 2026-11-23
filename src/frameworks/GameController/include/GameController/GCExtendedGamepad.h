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

#ifndef _GCExtendedGamepad_H_
#define _GCExtendedGamepad_H_

#import <Foundation/Foundation.h>
#import <GameController/GCPhysicalInputProfile.h>
#import <GameController/GCControllerButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>

@class GCController;
@class GCExtendedGamepadSnapshot;

NS_ASSUME_NONNULL_BEGIN

@class GCExtendedGamepad;

typedef void (^GCExtendedGamepadValueChangedHandler)(GCExtendedGamepad *gamepad, GCControllerElement *element);

@interface GCExtendedGamepad : GCPhysicalInputProfile

@property (nonatomic, readonly, weak, nullable) GCController *controller;
@property (nonatomic, copy, nullable) GCExtendedGamepadValueChangedHandler valueChangedHandler;

@property (nonatomic, readonly) GCControllerDirectionPad *dpad;
@property (nonatomic, readonly) GCControllerButtonInput *buttonA;
@property (nonatomic, readonly) GCControllerButtonInput *buttonB;
@property (nonatomic, readonly) GCControllerButtonInput *buttonX;
@property (nonatomic, readonly) GCControllerButtonInput *buttonY;
@property (nonatomic, readonly) GCControllerButtonInput *buttonMenu;
@property (nonatomic, readonly, nullable) GCControllerButtonInput *buttonOptions;
@property (nonatomic, readonly, nullable) GCControllerButtonInput *buttonHome;

@property (nonatomic, readonly) GCControllerDirectionPad *leftThumbstick;
@property (nonatomic, readonly) GCControllerDirectionPad *rightThumbstick;

@property (nonatomic, readonly) GCControllerButtonInput *leftShoulder;
@property (nonatomic, readonly) GCControllerButtonInput *rightShoulder;
@property (nonatomic, readonly) GCControllerButtonInput *leftTrigger;
@property (nonatomic, readonly) GCControllerButtonInput *rightTrigger;

@property (nonatomic, readonly, nullable) GCControllerButtonInput *leftThumbstickButton;
@property (nonatomic, readonly, nullable) GCControllerButtonInput *rightThumbstickButton;

/* Deprecated by +[GCController controllerWithExtendedGamepad] on macOS: a snapshot of the state. */
- (GCExtendedGamepadSnapshot *)saveSnapshot;

/* Copies the state of another gamepad; has no effect on the profile of a real device. */
- (void)setStateFromExtendedGamepad:(GCExtendedGamepad *)extendedGamepad;

@end

NS_ASSUME_NONNULL_END

#endif
