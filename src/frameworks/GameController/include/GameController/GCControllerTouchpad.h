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

#ifndef _GCControllerTouchpad_H_
#define _GCControllerTouchpad_H_

#import <GameController/GCControllerElement.h>

@class GCControllerDirectionPad;
@class GCControllerButtonInput;

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, GCTouchState) {
	GCTouchStateUp,
	GCTouchStateDown,
	GCTouchStateMoving
} NS_SWIFT_NAME(GCControllerTouchpad.TouchState);

@interface GCControllerTouchpad : GCControllerElement

/* The button of the touch pad, pressed by pushing the surface down. */
@property (nonatomic, readonly) GCControllerButtonInput *button;

typedef void (^GCControllerTouchpadHandler)(GCControllerTouchpad *touchpad, float xValue, float yValue, float buttonValue, BOOL buttonPressed);

/* Called when a finger touches down, moves, and lifts. */
@property (nonatomic, copy, nullable) GCControllerTouchpadHandler touchDown;
@property (nonatomic, copy, nullable) GCControllerTouchpadHandler touchMoved;
@property (nonatomic, copy, nullable) GCControllerTouchpadHandler touchUp;

/* The position of the touch on the surface. */
@property (nonatomic, readonly) GCControllerDirectionPad *touchSurface;
@property (nonatomic, readonly) GCTouchState touchState;

/* Whether the surface values are absolute positions (YES) or relative to where the touch started (NO). */
@property (nonatomic) BOOL reportsAbsoluteTouchSurfaceValues;

- (void)setValueForXAxis:(float)xAxis yAxis:(float)yAxis touchDown:(BOOL)touchDown buttonValue:(float)buttonValue;

@end

NS_ASSUME_NONNULL_END

#endif
