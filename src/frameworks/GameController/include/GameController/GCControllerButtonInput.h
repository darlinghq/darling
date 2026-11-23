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

#ifndef _GCControllerButtonInput_H_
#define _GCControllerButtonInput_H_

#import <Foundation/Foundation.h>
#import <GameController/GCControllerElement.h>

NS_ASSUME_NONNULL_BEGIN

@class GCControllerButtonInput;

typedef void (^GCControllerButtonValueChangedHandler)(GCControllerButtonInput *button, float value, BOOL pressed);
typedef void (^GCControllerButtonTouchedChangedHandler)(GCControllerButtonInput *button, float value, BOOL pressed, BOOL touched);

@interface GCControllerButtonInput : GCControllerElement

@property (nonatomic, copy, nullable) GCControllerButtonValueChangedHandler valueChangedHandler;
@property (nonatomic, copy, nullable) GCControllerButtonValueChangedHandler pressedChangedHandler;
@property (nonatomic, copy, nullable) GCControllerButtonTouchedChangedHandler touchedChangedHandler;

@property (nonatomic, readonly) float value;
@property (nonatomic, readonly, getter=isPressed) BOOL pressed;
@property (nonatomic, readonly, getter=isTouched) BOOL touched;

- (void)setValue:(float)value;

@end

NS_ASSUME_NONNULL_END

#endif
