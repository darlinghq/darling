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

#import <GameController/GCInputNames.h>

GCInputButtonName GCInputButtonA = (id)@"Button A";
GCInputButtonName GCInputButtonB = (id)@"Button B";
GCInputButtonName GCInputButtonX = (id)@"Button X";
GCInputButtonName GCInputButtonY = (id)@"Button Y";
GCInputDirectionPadName GCInputDirectionPad = (id)@"Direction Pad";
GCInputDirectionPadName GCInputThumbstick = (id)@"Thumbstick";
GCInputDirectionPadName GCInputLeftThumbstick = (id)@"Left Thumbstick";
GCInputDirectionPadName GCInputRightThumbstick = (id)@"Right Thumbstick";
GCInputButtonName GCInputThumbstickButton = (id)@"Thumbstick Button";
GCInputButtonName GCInputLeftThumbstickButton = (id)@"Left Thumbstick Button";
GCInputButtonName GCInputRightThumbstickButton = (id)@"Right Thumbstick Button";
GCInputButtonName GCInputGripButton = (id)@"Grip";
GCInputButtonName GCInputLeftShoulder = (id)@"Left Shoulder";
GCInputButtonName GCInputRightShoulder = (id)@"Right Shoulder";
GCInputButtonName GCInputLeftBumper = (id)@"Left Bumper";
GCInputButtonName GCInputRightBumper = (id)@"Right Bumper";
GCInputButtonName GCInputTrigger = (id)@"Trigger";
GCInputButtonName GCInputLeftTrigger = (id)@"Left Trigger";
GCInputButtonName GCInputRightTrigger = (id)@"Right Trigger";
GCInputButtonName GCInputLeftSideButton = (id)@"Left Side Button";
GCInputButtonName GCInputRightSideButton = (id)@"Right Side Button";
GCInputButtonName GCInputButtonHome = (id)@"Button Home";
GCInputButtonName GCInputButtonMenu = (id)@"Button Menu";
GCInputButtonName GCInputButtonOptions = (id)@"Button Options";
GCInputButtonName GCInputButtonShare = (id)@"Button Share";
GCInputButtonName GCInputXboxPaddleOne = (id)@"Paddle 1";
GCInputButtonName GCInputXboxPaddleTwo = (id)@"Paddle 2";
GCInputButtonName GCInputXboxPaddleThree = (id)@"Paddle 3";
GCInputButtonName GCInputXboxPaddleFour = (id)@"Paddle 4";
GCInputDirectionPadName GCInputDualShockTouchpadOne = (id)@"Touchpad 1";
GCInputDirectionPadName GCInputDualShockTouchpadTwo = (id)@"Touchpad 2";
GCInputButtonName GCInputDualShockTouchpadButton = (id)@"Touchpad Button";
GCInputAxisName GCInputSteeringWheel = (id)@"Steering Wheel";
GCInputElementName GCInputShifter = (id)@"Shifter";
GCInputButtonName GCInputPedalAccelerator = (id)@"Accelerator Pedal";
GCInputButtonName GCInputPedalBrake = (id)@"Brake Pedal";
GCInputButtonName GCInputPedalClutch = (id)@"Clutch Pedal";
GCInputButtonName GCInputLeftPaddle = (id)@"Left Paddle";
GCInputButtonName GCInputRightPaddle = (id)@"Right Paddle";

GCInputButtonName GCInputBackLeftButton(NSInteger position)
{
	return (id)[NSString stringWithFormat:@"Back Left Button %ld", (long)position];
}

GCInputButtonName GCInputBackRightButton(NSInteger position)
{
	return (id)[NSString stringWithFormat:@"Back Right Button %ld", (long)position];
}

GCInputButtonName GCInputArcadeButtonName(NSInteger row, NSInteger column)
{
	return (id)[NSString stringWithFormat:@"Arcade Button %ld, %ld", (long)row, (long)column];
}

NSString *const GCInputDirectionalDpad = @"Direction Pad";
NSString *const GCInputDirectionalTouchSurfaceButton = @"Button A";
NSString *const GCInputDirectionalCardinalDpad = @"Cardinal Direction Pad";
NSString *const GCInputDirectionalCenterButton = @"Button Center";
NSString *const GCInputMicroGamepadDpad = @"Direction Pad";
NSString *const GCInputMicroGamepadButtonA = @"Button A";
NSString *const GCInputMicroGamepadButtonX = @"Button X";
NSString *const GCInputMicroGamepadButtonMenu = @"Button Menu";
