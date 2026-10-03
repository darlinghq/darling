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

#import <GameController/GCController.h>
#import <GameController/_GCController.h>

/* No controller hardware exists here, so these are never posted. They are defined
 * because dyld fails to load any binary importing an undefined symbol, which is a hard
 * launch failure for every game that watches for controller connections. */

NSString *const GCControllerDidConnectNotification = @"GCControllerDidConnectNotification";
NSString *const GCControllerDidDisconnectNotification = @"GCControllerDidDisconnectNotification";

NSString *const GCInputButtonA = @"GCInputButtonA";
NSString *const GCInputButtonB = @"GCInputButtonB";
NSString *const GCInputButtonX = @"GCInputButtonX";
NSString *const GCInputButtonY = @"GCInputButtonY";
NSString *const GCInputButtonMenu = @"GCInputButtonMenu";
NSString *const GCInputButtonOptions = @"GCInputButtonOptions";
NSString *const GCInputLeftShoulder = @"GCInputLeftShoulder";
NSString *const GCInputRightShoulder = @"GCInputRightShoulder";
NSString *const GCInputLeftTrigger = @"GCInputLeftTrigger";
NSString *const GCInputRightTrigger = @"GCInputRightTrigger";
NSString *const GCInputLeftThumbstick = @"GCInputLeftThumbstick";
NSString *const GCInputDirectionPad = @"GCInputDirectionPad";
