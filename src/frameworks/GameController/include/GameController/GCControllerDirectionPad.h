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

#ifndef _GCControllerDirectionPad_H_
#define _GCControllerDirectionPad_H_

#import <Foundation/Foundation.h>
#import <GameController/GCControllerElement.h>

@class GCControllerAxisInput;
@class GCControllerButtonInput;

NS_ASSUME_NONNULL_BEGIN

@class GCControllerDirectionPad;

typedef void (^GCControllerDirectionPadValueChangedHandler)(GCControllerDirectionPad *dpad, float xValue, float yValue);

@interface GCControllerDirectionPad : GCControllerElement

@property (nonatomic, copy, nullable) GCControllerDirectionPadValueChangedHandler valueChangedHandler;

@property (nonatomic, readonly) GCControllerAxisInput *xAxis;
@property (nonatomic, readonly) GCControllerAxisInput *yAxis;

@property (nonatomic, readonly) GCControllerButtonInput *up;
@property (nonatomic, readonly) GCControllerButtonInput *down;
@property (nonatomic, readonly) GCControllerButtonInput *left;
@property (nonatomic, readonly) GCControllerButtonInput *right;

- (void)setValueForXAxis:(float)xAxis yAxis:(float)yAxis;

@end

NS_ASSUME_NONNULL_END

#endif
