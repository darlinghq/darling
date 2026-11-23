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

#ifndef _GCMicroGamepad_H_
#define _GCMicroGamepad_H_

#import <Foundation/Foundation.h>
#import <GameController/GCPhysicalInputProfile.h>
#import <GameController/GCControllerButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>

@class GCController;
@class GCMicroGamepadSnapshot;

NS_ASSUME_NONNULL_BEGIN

/* Names of the elements of a micro gamepad. */
extern NSString *const GCInputMicroGamepadDpad;
extern NSString *const GCInputMicroGamepadButtonA;
extern NSString *const GCInputMicroGamepadButtonX;
extern NSString *const GCInputMicroGamepadButtonMenu;

@class GCMicroGamepad;

typedef void (^GCMicroGamepadValueChangedHandler)(GCMicroGamepad *gamepad, GCControllerElement *element);

@interface GCMicroGamepad : GCPhysicalInputProfile

@property (nonatomic, readonly, weak, nullable) GCController *controller;
@property (nonatomic, copy, nullable) GCMicroGamepadValueChangedHandler valueChangedHandler;

@property (nonatomic, readonly, strong) GCControllerDirectionPad *dpad;
@property (nonatomic, readonly, strong) GCControllerButtonInput *buttonA;
@property (nonatomic, readonly, strong) GCControllerButtonInput *buttonX;
@property (nonatomic, readonly, strong) GCControllerButtonInput *buttonMenu;

@property (nonatomic) BOOL reportsAbsoluteDpadValues;
@property (nonatomic) BOOL allowsRotation;

- (GCMicroGamepadSnapshot *)saveSnapshot;
- (void)setStateFromMicroGamepad:(GCMicroGamepad *)microGamepad;

@end

NS_ASSUME_NONNULL_END

#endif
