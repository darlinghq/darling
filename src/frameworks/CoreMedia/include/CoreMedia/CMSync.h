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

// Clocks and timebases: the types. Of their functions only the host time clock ones and the timebase ones
// for creating, time and rate are declared and implemented here; the rest are still the generated
// declarations in CoreMedia.h.
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMClock *CMClockRef;
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMTimebase *CMTimebaseRef;
typedef CFTypeRef CMClockOrTimebaseRef;

CF_IMPLICIT_BRIDGING_ENABLED

CM_EXPORT CFTypeID CMClockGetTypeID(void);

// The singleton clock of the host time, which is mach_absolute_time. It is not retained for the caller.
CM_EXPORT CMClockRef CMClockGetHostTimeClock(void);

CM_EXPORT CMTime CMClockGetTime(CMClockRef clock);

// A scale conversion between a CMTime and the native units of the host time (mach_absolute_time ticks).
CM_EXPORT uint64_t CMClockConvertHostTimeToSystemUnits(CMTime hostTime);
CM_EXPORT CMTime CMClockMakeHostTimeFromSystemUnits(uint64_t hostTime);

// CMTimebase error codes
enum {
	kCMTimebaseError_MissingRequiredParameter = -12748,
	kCMTimebaseError_InvalidParameter = -12749,
	kCMTimebaseError_AllocationFailed = -12750,
	kCMTimebaseError_TimerIntervalTooShort = -12751,
	kCMTimebaseError_ReadOnly = -12757,
};

// A timebase is a timeline under the control of the caller: it runs at a rate relative to its master, a clock
// or another timebase, and starts with rate zero and time zero. "Source" is the newer name of "master".
CM_EXPORT CFTypeID CMTimebaseGetTypeID(void);

CM_EXPORT OSStatus CMTimebaseCreateWithMasterClock(CFAllocatorRef allocator, CMClockRef masterClock, CMTimebaseRef *timebaseOut);
CM_EXPORT OSStatus CMTimebaseCreateWithMasterTimebase(CFAllocatorRef allocator, CMTimebaseRef masterTimebase, CMTimebaseRef *timebaseOut);
CM_EXPORT OSStatus CMTimebaseCreateWithSourceClock(CFAllocatorRef allocator, CMClockRef sourceClock, CMTimebaseRef *timebaseOut);
CM_EXPORT OSStatus CMTimebaseCreateWithSourceTimebase(CFAllocatorRef allocator, CMTimebaseRef sourceTimebase, CMTimebaseRef *timebaseOut);

// The Copy functions return a new reference; the Get functions (deprecated) do not retain.
CM_EXPORT CMTimebaseRef CMTimebaseCopyMasterTimebase(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseCopyMasterClock(CMTimebaseRef timebase);
CM_EXPORT CMClockOrTimebaseRef CMTimebaseCopyMaster(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseCopyUltimateMasterClock(CMTimebaseRef timebase);
CM_EXPORT CMTimebaseRef CMTimebaseGetMasterTimebase(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseGetMasterClock(CMTimebaseRef timebase);
CM_EXPORT CMClockOrTimebaseRef CMTimebaseGetMaster(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseGetUltimateMasterClock(CMTimebaseRef timebase);
CM_EXPORT CMTimebaseRef CMTimebaseCopySourceTimebase(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseCopySourceClock(CMTimebaseRef timebase);
CM_EXPORT CMClockOrTimebaseRef CMTimebaseCopySource(CMTimebaseRef timebase);
CM_EXPORT CMClockRef CMTimebaseCopyUltimateSourceClock(CMTimebaseRef timebase);

// The time of a clock or of a timebase, whichever the object is.
CM_EXPORT CMTime CMSyncGetTime(CMClockOrTimebaseRef clockOrTimebase);

CM_EXPORT CMTime CMTimebaseGetTime(CMTimebaseRef timebase);
CM_EXPORT CMTime CMTimebaseGetTimeWithTimeScale(CMTimebaseRef timebase, CMTimeScale timescale, CMTimeRoundingMethod method);
CM_EXPORT OSStatus CMTimebaseGetTimeAndRate(CMTimebaseRef timebase, CMTime *timeOut, Float64 *rateOut);
CM_EXPORT OSStatus CMTimebaseSetTime(CMTimebaseRef timebase, CMTime time);
// Sets the time of the timebase at a given time of its master; its time is interpolated from there.
CM_EXPORT OSStatus CMTimebaseSetAnchorTime(CMTimebaseRef timebase, CMTime timebaseTime, CMTime immediateMasterTime);

// The rate is relative to the master; the effective rate also includes the rates of all its master timebases.
CM_EXPORT Float64 CMTimebaseGetRate(CMTimebaseRef timebase);
CM_EXPORT Float64 CMTimebaseGetEffectiveRate(CMTimebaseRef timebase);
CM_EXPORT OSStatus CMTimebaseSetRate(CMTimebaseRef timebase, Float64 rate);
CM_EXPORT OSStatus CMTimebaseSetRateAndAnchorTime(CMTimebaseRef timebase, Float64 rate, CMTime timebaseTime, CMTime immediateMasterTime);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
