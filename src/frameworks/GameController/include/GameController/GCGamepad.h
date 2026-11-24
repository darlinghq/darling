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

#ifndef _GCGamepad_H_
#define _GCGamepad_H_

#import <Foundation/Foundation.h>
#import <GameController/GCPhysicalInputProfile.h>

@class GCController;
@class GCGamepad;
@class GCGamepadSnapshot;

NS_ASSUME_NONNULL_BEGIN

typedef void (^GCGamepadValueChangedHandler)(GCGamepad *gamepad, GCControllerElement *element);

/* The original gamepad profile; GCExtendedGamepad replaces it. */
@interface GCGamepad : GCPhysicalInputProfile

@property (nonatomic, readonly, weak) GCController *controller;
@property (nonatomic, copy, nullable) GCGamepadValueChangedHandler valueChangedHandler;

- (GCGamepadSnapshot *)saveSnapshot;

@property (nonatomic, readonly) GCControllerDirectionPad *dpad;
@property (nonatomic, readonly) GCControllerButtonInput *buttonA;
@property (nonatomic, readonly) GCControllerButtonInput *buttonB;
@property (nonatomic, readonly) GCControllerButtonInput *buttonX;
@property (nonatomic, readonly) GCControllerButtonInput *buttonY;
@property (nonatomic, readonly) GCControllerButtonInput *leftShoulder;
@property (nonatomic, readonly) GCControllerButtonInput *rightShoulder;

@end

NS_ASSUME_NONNULL_END

#endif
