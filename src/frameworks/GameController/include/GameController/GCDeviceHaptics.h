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

#ifndef _GCDEVICEHAPTICS_H_
#define _GCDEVICEHAPTICS_H_

#import <Foundation/Foundation.h>

// The locations of the haptic actuators of a controller.
typedef NSString *GCHapticsLocality NS_TYPED_ENUM;

extern GCHapticsLocality const GCHapticsLocalityDefault;
extern GCHapticsLocality const GCHapticsLocalityAll;
extern GCHapticsLocality const GCHapticsLocalityHandles;
extern GCHapticsLocality const GCHapticsLocalityLeftHandle;
extern GCHapticsLocality const GCHapticsLocalityRightHandle;
extern GCHapticsLocality const GCHapticsLocalityTriggers;
extern GCHapticsLocality const GCHapticsLocalityLeftTrigger;
extern GCHapticsLocality const GCHapticsLocalityRightTrigger;

// A haptic event of this duration lasts until it is stopped.
extern const float GCHapticDurationInfinite;

#endif
