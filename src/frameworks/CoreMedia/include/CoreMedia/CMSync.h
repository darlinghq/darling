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


#ifndef _CMSYNC_H_
#define _CMSYNC_H_

#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>

// Clocks and timebases: the types. Of their functions only the clock ones are declared and implemented
// here; the rest are still the generated declarations in CoreMedia.h.
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMClock *CMClockRef;
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMTimebase *CMTimebaseRef;
typedef CFTypeRef CMClockOrTimebaseRef;

// CMClock error codes
enum {
	kCMClockError_MissingRequiredParameter = -12745,
	kCMClockError_InvalidParameter = -12746,
	kCMClockError_AllocationFailed = -12747,
	kCMClockError_UnsupportedOperation = -12756,
};

CF_IMPLICIT_BRIDGING_ENABLED

CM_EXPORT CFTypeID CMClockGetTypeID(void);

// The singleton clock of the host time, which is mach_absolute_time. It is not retained for the caller.
CM_EXPORT CMClockRef CMClockGetHostTimeClock(void);

CM_EXPORT CMTime CMClockGetTime(CMClockRef clock);

// Returns the clock's time and the matching time of its reference clock, which for the host time clock is the
// host time clock itself.
CM_EXPORT OSStatus CMClockGetAnchorTime(CMClockRef clock, CMTime *clockTimeOut, CMTime *referenceClockTimeOut);

// Whether two clocks can drift relative to each other.
CM_EXPORT Boolean CMClockMightDrift(CMClockRef clock, CMClockRef otherClock);

// Stops a clock that its owner is done with. The host time clock has no owner, so this has no effect on it.
CM_EXPORT void CMClockInvalidate(CMClockRef clock);

// A scale conversion between a CMTime and the native units of the host time (mach_absolute_time ticks).
CM_EXPORT uint64_t CMClockConvertHostTimeToSystemUnits(CMTime hostTime);
CM_EXPORT CMTime CMClockMakeHostTimeFromSystemUnits(uint64_t hostTime);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
