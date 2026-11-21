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


#ifndef MediaAccessibility_MAVideoAccommodations_h
#define MediaAccessibility_MAVideoAccommodations_h

#include <MediaAccessibility/MADefinitions.h>
#include <CoreFoundation/CoreFoundation.h>

MA_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

// Whether the user asked for flashing or strobing lights in video to be dimmed.
MA_EXPORT bool MADimFlashingLightsEnabled(void);

// CFNotification posted when the setting above changes.
MA_EXPORT const CFStringRef kMADimFlashingLightsChangedNotification;

CF_ASSUME_NONNULL_END
MA_EXTERN_C_END

#endif
