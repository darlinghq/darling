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

#ifndef _GCDualSenseGamepad_H_
#define _GCDualSenseGamepad_H_

#import <GameController/GCExtendedGamepad.h>
#import <GameController/GCDualSenseAdaptiveTrigger.h>

NS_ASSUME_NONNULL_BEGIN

@interface GCDualSenseGamepad : GCExtendedGamepad

@property (nonatomic, readonly) GCControllerButtonInput *touchpadButton;
@property (nonatomic, readonly) GCControllerDirectionPad *touchpadPrimary;
@property (nonatomic, readonly) GCControllerDirectionPad *touchpadSecondary;
@property (nonatomic, readonly) GCDualSenseAdaptiveTrigger *leftTrigger;
@property (nonatomic, readonly) GCDualSenseAdaptiveTrigger *rightTrigger;

@end

NS_ASSUME_NONNULL_END

#endif
