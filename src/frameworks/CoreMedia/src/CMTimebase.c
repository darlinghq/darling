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

/*
 * Timebases: a timeline driven by a master (a clock or another timebase) through a rate and an anchor.
 * The time of a timebase is anchorTime + (master time - anchorMasterTime) * rate.
 */

#include <CoreMedia/CoreMedia.h>
#include "CMClass.h"
#include <pthread.h>

struct OpaqueCMTimebase {
	CFRuntimeBase base;
	pthread_mutex_t lock;
	/* Set when the timebase is created and never changed (retained: a clock or a timebase). */
	CFTypeRef master;
	/* Guarded by the lock. */
	Float64 rate;
	CMTime anchorTime;
	CMTime anchorMasterTime;
};

static void timebaseFinalize(CFTypeRef cf)
{
	struct OpaqueCMTimebase *timebase = (struct OpaqueCMTimebase *)cf;
	if (timebase->master != NULL)
		CFRelease(timebase->master);
	pthread_mutex_destroy(&timebase->lock);
}

CM_CLASS(CMTimebase_, "CMTimebase", .finalize = timebaseFinalize)

CFTypeID CMTimebaseGetTypeID(void) { return CMTimebase_GetTypeID(); }

#pragma mark - Helpers

static struct OpaqueCMTimebase *asTimebase(CFTypeRef object)
{
	return object != NULL && CFGetTypeID(object) == CMTimebase_GetTypeID() ? (struct OpaqueCMTimebase *)object : NULL;
}

static Boolean isClock(CFTypeRef object)
{
	return object != NULL && CFGetTypeID(object) == CMClockGetTypeID();
}

#pragma mark - Creating timebases

static OSStatus createTimebase(CFAllocatorRef allocator, CFTypeRef master, Boolean masterIsClock, CMTimebaseRef *timebaseOut)
{
	if (master == NULL || timebaseOut == NULL)
		return kCMTimebaseError_MissingRequiredParameter;
	if (masterIsClock ? !isClock(master) : asTimebase(master) == NULL)
		return kCMTimebaseError_InvalidParameter;
	struct OpaqueCMTimebase *timebase = CM_NEW(struct OpaqueCMTimebase *, CMTimebase_, allocator);
	if (timebase == NULL)
		return kCMTimebaseError_AllocationFailed;
	pthread_mutex_init(&timebase->lock, NULL);
	timebase->master = CFRetain(master);
	/* A new timebase has rate zero and time zero. */
	timebase->rate = 0.0;
	timebase->anchorTime = kCMTimeZero;
	timebase->anchorMasterTime = kCMTimeZero;
	*timebaseOut = timebase;
	return noErr;
}

OSStatus CMTimebaseCreateWithMasterClock(CFAllocatorRef allocator, CMClockRef masterClock, CMTimebaseRef *timebaseOut)
{
	return createTimebase(allocator, masterClock, true, timebaseOut);
}

OSStatus CMTimebaseCreateWithMasterTimebase(CFAllocatorRef allocator, CMTimebaseRef masterTimebase, CMTimebaseRef *timebaseOut)
{
	return createTimebase(allocator, masterTimebase, false, timebaseOut);
}

OSStatus CMTimebaseCreateWithSourceClock(CFAllocatorRef allocator, CMClockRef sourceClock, CMTimebaseRef *timebaseOut)
{
	return createTimebase(allocator, sourceClock, true, timebaseOut);
}

OSStatus CMTimebaseCreateWithSourceTimebase(CFAllocatorRef allocator, CMTimebaseRef sourceTimebase, CMTimebaseRef *timebaseOut)
{
	return createTimebase(allocator, sourceTimebase, false, timebaseOut);
}

#pragma mark - Masters

/* The immediate master of a timebase, not retained, or NULL for an argument that is not a timebase. */
static CFTypeRef masterOf(CMTimebaseRef timebase)
{
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	return object != NULL ? object->master : NULL;
}

static CFTypeRef retained(CFTypeRef object)
{
	return object != NULL ? CFRetain(object) : NULL;
}

static CFTypeRef masterIfTimebase(CMTimebaseRef timebase)
{
	CFTypeRef master = masterOf(timebase);
	return asTimebase(master);
}

static CFTypeRef masterIfClock(CMTimebaseRef timebase)
{
	CFTypeRef master = masterOf(timebase);
	return isClock(master) ? master : NULL;
}

/* The clock at the root of the chain of masters. */
static CFTypeRef ultimateMasterClock(CMTimebaseRef timebase)
{
	CFTypeRef master = masterOf(timebase);
	while (asTimebase(master) != NULL)
		master = ((struct OpaqueCMTimebase *)master)->master;
	return master;
}

CMTimebaseRef CMTimebaseCopyMasterTimebase(CMTimebaseRef timebase) { return (CMTimebaseRef)retained(masterIfTimebase(timebase)); }
CMClockRef CMTimebaseCopyMasterClock(CMTimebaseRef timebase) { return (CMClockRef)retained(masterIfClock(timebase)); }
CMClockOrTimebaseRef CMTimebaseCopyMaster(CMTimebaseRef timebase) { return retained(masterOf(timebase)); }
CMClockRef CMTimebaseCopyUltimateMasterClock(CMTimebaseRef timebase) { return (CMClockRef)retained(ultimateMasterClock(timebase)); }

CMTimebaseRef CMTimebaseGetMasterTimebase(CMTimebaseRef timebase) { return (CMTimebaseRef)masterIfTimebase(timebase); }
CMClockRef CMTimebaseGetMasterClock(CMTimebaseRef timebase) { return (CMClockRef)masterIfClock(timebase); }
CMClockOrTimebaseRef CMTimebaseGetMaster(CMTimebaseRef timebase) { return masterOf(timebase); }
CMClockRef CMTimebaseGetUltimateMasterClock(CMTimebaseRef timebase) { return (CMClockRef)ultimateMasterClock(timebase); }

CMTimebaseRef CMTimebaseCopySourceTimebase(CMTimebaseRef timebase) { return CMTimebaseCopyMasterTimebase(timebase); }
CMClockRef CMTimebaseCopySourceClock(CMTimebaseRef timebase) { return CMTimebaseCopyMasterClock(timebase); }
CMClockOrTimebaseRef CMTimebaseCopySource(CMTimebaseRef timebase) { return CMTimebaseCopyMaster(timebase); }
CMClockRef CMTimebaseCopyUltimateSourceClock(CMTimebaseRef timebase) { return CMTimebaseCopyUltimateMasterClock(timebase); }
