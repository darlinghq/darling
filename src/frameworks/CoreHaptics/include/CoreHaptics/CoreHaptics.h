/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#ifndef _CoreHaptics_H_
#define _CoreHaptics_H_

#import <Foundation/Foundation.h>

#import <CoreHaptics/CHHapticEvent.h>
#import <CoreHaptics/CHHapticEventParameter.h>
#import <CoreHaptics/CHHapticDynamicParameter.h>
#import <CoreHaptics/CHHapticPattern.h>

FOUNDATION_EXTERN NSString * const CHHapticDynamicParameterIDHapticIntensityControl;
FOUNDATION_EXTERN NSString * const CHHapticDynamicParameterIDHapticSharpnessControl;
FOUNDATION_EXTERN NSString * const CHHapticEventParameterIDHapticIntensity;
FOUNDATION_EXTERN NSString * const CHHapticEventParameterIDHapticSharpness;
FOUNDATION_EXTERN NSString * const CHHapticEventTypeHapticContinuous;

#endif
