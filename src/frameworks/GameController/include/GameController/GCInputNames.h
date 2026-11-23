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

#ifndef _GCInputNames_H_
#define _GCInputNames_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/* Marker protocols that give the names of the different element kinds distinct types. */
__attribute__((objc_non_runtime_protocol)) NS_REFINED_FOR_SWIFT
@protocol GCPhysicalInputElementName
@end
__attribute__((objc_non_runtime_protocol)) NS_REFINED_FOR_SWIFT
@protocol GCButtonElementName <GCPhysicalInputElementName>
@end
__attribute__((objc_non_runtime_protocol)) NS_REFINED_FOR_SWIFT
@protocol GCAxisElementName <GCPhysicalInputElementName>
@end
__attribute__((objc_non_runtime_protocol)) NS_REFINED_FOR_SWIFT
@protocol GCSwitchElementName <GCPhysicalInputElementName>
@end
__attribute__((objc_non_runtime_protocol)) NS_REFINED_FOR_SWIFT
@protocol GCDirectionPadElementName <GCPhysicalInputElementName>
@end

typedef NSString<GCPhysicalInputElementName> *const GCInputElementName NS_REFINED_FOR_SWIFT NS_TYPED_EXTENSIBLE_ENUM;
typedef NSString<GCButtonElementName> *const GCInputButtonName NS_REFINED_FOR_SWIFT NS_TYPED_EXTENSIBLE_ENUM;
typedef NSString<GCAxisElementName> *const GCInputAxisName NS_REFINED_FOR_SWIFT NS_TYPED_EXTENSIBLE_ENUM;
typedef NSString<GCSwitchElementName> *const GCInputSwitchName NS_REFINED_FOR_SWIFT NS_TYPED_EXTENSIBLE_ENUM;
typedef NSString<GCDirectionPadElementName> *const GCInputDirectionPadName NS_REFINED_FOR_SWIFT NS_TYPED_EXTENSIBLE_ENUM;

extern GCInputButtonName GCInputButtonA;
extern GCInputButtonName GCInputButtonB;
extern GCInputButtonName GCInputButtonX;
extern GCInputButtonName GCInputButtonY;
extern GCInputDirectionPadName GCInputDirectionPad;
extern GCInputDirectionPadName GCInputThumbstick;
extern GCInputDirectionPadName GCInputLeftThumbstick;
extern GCInputDirectionPadName GCInputRightThumbstick;
extern GCInputButtonName GCInputThumbstickButton;
extern GCInputButtonName GCInputLeftThumbstickButton;
extern GCInputButtonName GCInputRightThumbstickButton;
extern GCInputButtonName GCInputGripButton;
extern GCInputButtonName GCInputLeftShoulder;
extern GCInputButtonName GCInputRightShoulder;
extern GCInputButtonName GCInputLeftBumper;
extern GCInputButtonName GCInputRightBumper;
extern GCInputButtonName GCInputTrigger;
extern GCInputButtonName GCInputLeftTrigger;
extern GCInputButtonName GCInputRightTrigger;
extern GCInputButtonName GCInputLeftSideButton;
extern GCInputButtonName GCInputRightSideButton;
extern GCInputButtonName GCInputBackLeftButton(NSInteger position);
extern GCInputButtonName GCInputBackRightButton(NSInteger position);
extern GCInputButtonName GCInputButtonHome;
extern GCInputButtonName GCInputButtonMenu;
extern GCInputButtonName GCInputButtonOptions;
extern GCInputButtonName GCInputButtonShare;
extern GCInputButtonName GCInputXboxPaddleOne;
extern GCInputButtonName GCInputXboxPaddleTwo;
extern GCInputButtonName GCInputXboxPaddleThree;
extern GCInputButtonName GCInputXboxPaddleFour;
extern GCInputDirectionPadName GCInputDualShockTouchpadOne;
extern GCInputDirectionPadName GCInputDualShockTouchpadTwo;
extern GCInputButtonName GCInputDualShockTouchpadButton;
extern GCInputAxisName GCInputSteeringWheel;
extern GCInputElementName GCInputShifter;
extern GCInputButtonName GCInputPedalAccelerator;
extern GCInputButtonName GCInputPedalBrake;
extern GCInputButtonName GCInputPedalClutch;
extern GCInputButtonName GCInputLeftPaddle;
extern GCInputButtonName GCInputRightPaddle;
extern GCInputButtonName GCInputArcadeButtonName(NSInteger row, NSInteger column);

NS_ASSUME_NONNULL_END

#endif
