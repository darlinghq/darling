/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef CVHOSTTIME_H
#define CVHOSTTIME_H

#include <CoreVideo/CVBase.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// The host time base, which is the one mach_absolute_time counts in.
CV_EXPORT uint64_t CVGetCurrentHostTime(void) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
CV_EXPORT double CVGetHostClockFrequency(void) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
CV_EXPORT uint32_t CVGetHostClockMinimumTimeDelta(void) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

#if defined(__cplusplus)
}
#endif

#endif
