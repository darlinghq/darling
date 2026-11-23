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
#include <math.h>
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

/* GUESS: the documentation lists no results for the timebase functions. The kCMTimebaseError_ codes are used
   by their names: a NULL argument is MissingRequiredParameter, an object of the wrong type or an argument
   that cannot be used is InvalidParameter. */
static OSStatus statusForBadTimebase(CMTimebaseRef timebase)
{
	return timebase == NULL ? kCMTimebaseError_MissingRequiredParameter : kCMTimebaseError_InvalidParameter;
}

/* The time times a factor (or divided by it), rounded to the nearest unit of its timescale, or of a
   nanosecond when the timescale is coarser, so that a duration in whole seconds can be scaled by a fraction.
   A factor of 1 returns the time as it is: a product through a Float64 would lose the low bits of a value
   above 2^53. */
static CMTime scaledByRate(CMTime time, Float64 factor, Boolean divide)
{
	if (!CMTIME_IS_NUMERIC(time) || factor == 1.0)
		return time;
	if (time.timescale < 1000000000)
		time = CMTimeConvertScale(time, 1000000000, kCMTimeRoundingMethod_RoundHalfAwayFromZero);
	if (!CMTIME_IS_NUMERIC(time))
		return time;
	Float64 product = divide ? (Float64)time.value / factor : (Float64)time.value * factor;
	if (product >= 9223372036854775808.0)
		return kCMTimePositiveInfinity;
	if (product <= -9223372036854775808.0)
		return kCMTimeNegativeInfinity;
	time.value = llround(product);
	if ((Float64)time.value != product)
		time.flags |= kCMTimeFlags_HasBeenRounded;
	return time;
}

/* The time of a timebase with the given anchor and rate when its master reads masterNow. GUESS: the time
   that has passed is scaled by the rate and rounded to the nearest unit of its timescale (a nanosecond if
   that is coarser), halves away from zero; the documentation does not say how a fractional rate is applied. */
static CMTime timeAtMasterTime(CMTime anchorTime, CMTime anchorMasterTime, Float64 rate, CMTime masterNow)
{
	return CMTimeAdd(anchorTime, scaledByRate(CMTimeSubtract(masterNow, anchorMasterTime), rate, false));
}

struct timebaseState {
	Float64 rate;
	CMTime anchorTime;
	CMTime anchorMasterTime;
};

static struct timebaseState stateOf(struct OpaqueCMTimebase *timebase)
{
	pthread_mutex_lock(&timebase->lock);
	struct timebaseState state = { timebase->rate, timebase->anchorTime, timebase->anchorMasterTime };
	pthread_mutex_unlock(&timebase->lock);
	return state;
}

/* The time of the timebase in a state. At rate zero the time is fixed: it does not change with the master's
   time, so the master is not asked. */
static CMTime timeInState(struct OpaqueCMTimebase *timebase, struct timebaseState state)
{
	if (state.rate == 0.0)
		return state.anchorTime;
	return timeAtMasterTime(state.anchorTime, state.anchorMasterTime, state.rate, CMSyncGetTime(timebase->master));
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

#pragma mark - Time

CMTime CMSyncGetTime(CMClockOrTimebaseRef clockOrTimebase)
{
	if (isClock(clockOrTimebase))
		return CMClockGetTime((CMClockRef)clockOrTimebase);
	/* GUESS: the documentation does not say what an object that is neither gives; an invalid time. */
	return CMTimebaseGetTime((CMTimebaseRef)clockOrTimebase);
}

CMTime CMTimebaseGetTime(CMTimebaseRef timebase)
{
	/* GUESS: the documentation lists no error result for the functions that return a time; an invalid time
	   stands for an argument that is not a timebase and for a master that gives no time. */
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return kCMTimeInvalid;
	return timeInState(object, stateOf(object));
}

CMTime CMTimebaseGetTimeWithTimeScale(CMTimebaseRef timebase, CMTimeScale timescale, CMTimeRoundingMethod method)
{
	return CMTimeConvertScale(CMTimebaseGetTime(timebase), timescale, method);
}

OSStatus CMTimebaseGetTimeAndRate(CMTimebaseRef timebase, CMTime *timeOut, Float64 *rateOut)
{
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return statusForBadTimebase(timebase);
	struct timebaseState state = stateOf(object);
	if (rateOut != NULL)
		*rateOut = state.rate;
	if (timeOut != NULL)
		*timeOut = timeInState(object, state);
	return noErr;
}

/* Sets the anchor and the rate in one step. GUESS: a time that is not numeric, or a rate that is not finite,
   is InvalidParameter; the documentation does not say. */
static OSStatus setAnchor(struct OpaqueCMTimebase *timebase, const Float64 *rate, CMTime time, CMTime masterTime)
{
	if (!CMTIME_IS_NUMERIC(time) || !CMTIME_IS_NUMERIC(masterTime) || (rate != NULL && !isfinite(*rate)))
		return kCMTimebaseError_InvalidParameter;
	pthread_mutex_lock(&timebase->lock);
	timebase->anchorTime = time;
	timebase->anchorMasterTime = masterTime;
	if (rate != NULL)
		timebase->rate = *rate;
	pthread_mutex_unlock(&timebase->lock);
	return noErr;
}

/* The current time of the master. GUESS: a master that gives no time makes the setters fail with
   InvalidParameter. */
static OSStatus masterNow(struct OpaqueCMTimebase *timebase, CMTime *now)
{
	*now = CMSyncGetTime(timebase->master);
	return CMTIME_IS_NUMERIC(*now) ? noErr : kCMTimebaseError_InvalidParameter;
}

OSStatus CMTimebaseSetAnchorTime(CMTimebaseRef timebase, CMTime timebaseTime, CMTime immediateMasterTime)
{
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return statusForBadTimebase(timebase);
	return setAnchor(object, NULL, timebaseTime, immediateMasterTime);
}

OSStatus CMTimebaseSetTime(CMTimebaseRef timebase, CMTime time)
{
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return statusForBadTimebase(timebase);
	CMTime now;
	OSStatus status = masterNow(object, &now);
	return status != noErr ? status : setAnchor(object, NULL, time, now);
}

#pragma mark - Rate

Float64 CMTimebaseGetRate(CMTimebaseRef timebase)
{
	/* GUESS: the documentation does not say what the rate of an argument that is not a timebase is; 0.0. */
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	return object != NULL ? stateOf(object).rate : 0.0;
}

Float64 CMTimebaseGetEffectiveRate(CMTimebaseRef timebase)
{
	/* The product of the rates of the timebase and of all its master timebases (a clock runs at rate 1). */
	if (asTimebase(timebase) == NULL)
		return 0.0;
	Float64 rate = 1.0;
	for (CFTypeRef object = timebase; asTimebase(object) != NULL; object = ((struct OpaqueCMTimebase *)object)->master)
		rate *= stateOf((struct OpaqueCMTimebase *)object).rate;
	return rate;
}

OSStatus CMTimebaseSetRateAndAnchorTime(CMTimebaseRef timebase, Float64 rate, CMTime timebaseTime, CMTime immediateMasterTime)
{
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return statusForBadTimebase(timebase);
	return setAnchor(object, &rate, timebaseTime, immediateMasterTime);
}

OSStatus CMTimebaseSetRate(CMTimebaseRef timebase, Float64 rate)
{
	/* The documented equivalent is CMTimebaseSetRateAndAnchorTime(timebase, rate, CMTimebaseGetTime(timebase),
	   the master's time), without the time jump notification. The time is taken at the master time that
	   becomes the anchor, so the timebase does not move across the change. */
	struct OpaqueCMTimebase *object = asTimebase(timebase);
	if (object == NULL)
		return statusForBadTimebase(timebase);
	if (!isfinite(rate))
		return kCMTimebaseError_InvalidParameter;
	CMTime now;
	OSStatus status = masterNow(object, &now);
	if (status != noErr)
		return status;
	pthread_mutex_lock(&object->lock);
	CMTime time = object->rate == 0.0 ? object->anchorTime
		: timeAtMasterTime(object->anchorTime, object->anchorMasterTime, object->rate, now);
	if (CMTIME_IS_NUMERIC(time)) {
		object->anchorTime = time;
		object->anchorMasterTime = now;
		object->rate = rate;
	}
	pthread_mutex_unlock(&object->lock);
	return CMTIME_IS_NUMERIC(time) ? noErr : kCMTimebaseError_InvalidParameter;
}

#pragma mark - Synchronising clocks and timebases

/* The time of a clock or timebase when the clock at the root of the chain of masters reads rootTime. */
static CMTime timeAtRootTime(CFTypeRef object, CMTime rootTime)
{
	struct OpaqueCMTimebase *timebase = asTimebase(object);
	if (timebase == NULL)
		return isClock(object) ? rootTime : kCMTimeInvalid;
	struct timebaseState state = stateOf(timebase);
	if (state.rate == 0.0)
		return state.anchorTime;
	return timeAtMasterTime(state.anchorTime, state.anchorMasterTime, state.rate, timeAtRootTime(timebase->master, rootTime));
}

/* The time of the root clock at which a clock or timebase reads time: the reverse of timeAtRootTime. GUESS: a
   stopped timebase passes through no root times, so its time has none and the result is an invalid time. */
static CMTime rootTimeAtTime(CFTypeRef object, CMTime time)
{
	struct OpaqueCMTimebase *timebase = asTimebase(object);
	if (timebase == NULL)
		return isClock(object) ? time : kCMTimeInvalid;
	struct timebaseState state = stateOf(timebase);
	if (state.rate == 0.0)
		return kCMTimeInvalid;
	CMTime masterTime = CMTimeAdd(state.anchorMasterTime, scaledByRate(CMTimeSubtract(time, state.anchorTime), state.rate, true));
	return rootTimeAtTime(timebase->master, masterTime);
}

static Boolean isClockOrTimebase(CFTypeRef object)
{
	return isClock(object) || asTimebase(object) != NULL;
}

static CFTypeRef rootClockOf(CFTypeRef object)
{
	return asTimebase(object) != NULL ? ultimateMasterClock((CMTimebaseRef)object) : object;
}

CMTime CMSyncConvertTime(CMTime time, CMClockOrTimebaseRef fromClockOrTimebase, CMClockOrTimebaseRef toClockOrTimebase)
{
	/* GUESS: the documentation lists no error result; an object that is neither a clock nor a timebase gives
	   an invalid time. A time that is not numeric (invalid, infinite, indefinite) is returned as it is. */
	if (!isClockOrTimebase(fromClockOrTimebase) || !isClockOrTimebase(toClockOrTimebase))
		return kCMTimeInvalid;
	if (!CMTIME_IS_NUMERIC(time) || fromClockOrTimebase == toClockOrTimebase)
		return time;
	/* The only clock is the host time clock, so the two always have a common master and the conversion is made
	   purely from the rates and anchors, through the time of that clock. */
	CMTime rootTime = rootTimeAtTime(fromClockOrTimebase, time);
	return CMTIME_IS_NUMERIC(rootTime) ? timeAtRootTime(toClockOrTimebase, rootTime) : kCMTimeInvalid;
}

/* The rate of a relative to b: the ratio of their rates relative to the common root clock. */
static Float64 relativeRate(CFTypeRef a, CFTypeRef b)
{
	Float64 rateOfA = asTimebase(a) != NULL ? CMTimebaseGetEffectiveRate((CMTimebaseRef)a) : 1.0;
	Float64 rateOfB = asTimebase(b) != NULL ? CMTimebaseGetEffectiveRate((CMTimebaseRef)b) : 1.0;
	/* The rate of anything moving relative to a stopped timebase is a NaN, as documented. GUESS: so is that of
	   a stopped timebase relative to a stopped one. */
	return rateOfB == 0.0 ? NAN : rateOfA / rateOfB;
}

Float64 CMSyncGetRelativeRate(CMClockOrTimebaseRef ofClockOrTimebase, CMClockOrTimebaseRef relativeToClockOrTimebase)
{
	/* GUESS: the documentation lists no error result; a NaN, which is also the documented result for a
	   stopped timebase, stands for an object that is neither a clock nor a timebase. */
	if (!isClockOrTimebase(ofClockOrTimebase) || !isClockOrTimebase(relativeToClockOrTimebase))
		return NAN;
	return relativeRate(ofClockOrTimebase, relativeToClockOrTimebase);
}

OSStatus CMSyncGetRelativeRateAndAnchorTime(CMClockOrTimebaseRef ofClockOrTimebase, CMClockOrTimebaseRef relativeToClockOrTimebase,
	Float64 *outRelativeRate, CMTime *outOfClockOrTimebaseAnchorTime, CMTime *outRelativeToClockOrTimebaseAnchorTime)
{
	/* GUESS: the kCMSyncError_ codes are used by their names: NULL is MissingRequiredParameter, an object that
	   is neither a clock nor a timebase, or a root clock with no time, is InvalidParameter. */
	if (ofClockOrTimebase == NULL || relativeToClockOrTimebase == NULL)
		return kCMSyncError_MissingRequiredParameter;
	if (!isClockOrTimebase(ofClockOrTimebase) || !isClockOrTimebase(relativeToClockOrTimebase))
		return kCMSyncError_InvalidParameter;
	/* GUESS: the documentation does not say which times are the anchors "at which the relative rate went into
	   effect". The time of the root clock now is used as that instant, so each anchor is the time of its clock
	   or timebase at it; the rejected alternative is the time of the last change of rate or time in either
	   chain, which a timebase does not record. */
	CMTime rootNow = CMClockGetTime((CMClockRef)rootClockOf(ofClockOrTimebase));
	if (!CMTIME_IS_NUMERIC(rootNow))
		return kCMSyncError_InvalidParameter;
	if (outRelativeRate != NULL)
		*outRelativeRate = relativeRate(ofClockOrTimebase, relativeToClockOrTimebase);
	if (outOfClockOrTimebaseAnchorTime != NULL)
		*outOfClockOrTimebaseAnchorTime = timeAtRootTime(ofClockOrTimebase, rootNow);
	if (outRelativeToClockOrTimebaseAnchorTime != NULL)
		*outRelativeToClockOrTimebaseAnchorTime = timeAtRootTime(relativeToClockOrTimebase, rootNow);
	return noErr;
}

Boolean CMSyncMightDrift(CMClockOrTimebaseRef clockOrTimebase1, CMClockOrTimebaseRef clockOrTimebase2)
{
	/* Two timebases can drift when the clocks at their roots can. The host time clock is the only clock, so
	   they never can. GUESS: an object that is neither a clock nor a timebase gives false too. */
	(void)clockOrTimebase1;
	(void)clockOrTimebase2;
	return false;
}
