// CFLAGS: -framework CoreMedia -framework CoreFoundation
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

// Timebases: creating one, its masters, time and rate.
// The documented contract (public CMSync.h and the pages below): a timebase created from a clock or from
// another timebase starts with rate zero and time zero; at rate 0 its time is fixed, at rate 2 it advances two
// seconds per second of its master; its effective rate is the product of its rate and the rates of its master
// timebases; CMTimebaseSetTime(t, time) is CMTimebaseSetAnchorTime(t, time, the master's time); the Copy
// functions return a new reference and the deprecated Get functions do not.
// https://developer.apple.com/documentation/coremedia/cmtimebasecreatewithsourceclock(allocator:sourceclock:timebaseout:)
// https://developer.apple.com/documentation/coremedia/cmtimebasegettime(_:)
// https://developer.apple.com/documentation/coremedia/cmtimebasesetrate(_:rate:)
// https://developer.apple.com/documentation/coremedia/cmtimebasegeteffectiverate(_:)
// The error codes are those of the public CMSync.h; what the functions do with unusual values (rates that are not
// numbers, times that are not numeric, a timescale of 0, how a fractional rate is applied) is what macOS 26.7.3
// and 10.14.4 do, observed with the same checks (the Master names are used because 10.14.4 has no others; the
// Source names are looked up at run time). Moving times are checked with a tolerance, because they follow the host
// time clock. Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static void sleepMilliseconds(long ms)
{
	struct timespec pause = { 0, ms * 1000 * 1000 };
	nanosleep(&pause, NULL);
}

static Float64 seconds(CMTime time) { return CMTimeGetSeconds(time); }

#define NEAR(time, expected, tolerance) (CMTIME_IS_NUMERIC(time) && fabs(seconds(time) - (expected)) <= (tolerance))

int main(void)
{
	CMClockRef host = CMClockGetHostTimeClock();
	CMTimebaseRef timebase = NULL;

	CHECK(kCMTimebaseError_MissingRequiredParameter == -12748);
	CHECK(kCMTimebaseError_InvalidParameter == -12749);
	CHECK(kCMTimebaseError_AllocationFailed == -12750);
	CHECK(kCMTimebaseError_TimerIntervalTooShort == -12751);
	CHECK(kCMTimebaseError_ReadOnly == -12757);

	// creation: rate zero, time zero, a CF object of its own type, with the clock as master
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, &timebase) == noErr);
	CHECK(timebase != NULL && CFGetTypeID(timebase) == CMTimebaseGetTypeID());
	CHECK(CMTimebaseGetTypeID() != CMClockGetTypeID());
	CHECK(CMTimebaseGetRate(timebase) == 0.0);
	CHECK(CMTimebaseGetEffectiveRate(timebase) == 0.0);
	CHECK(CMTimeCompare(CMTimebaseGetTime(timebase), kCMTimeZero) == 0);
	CMClockOrTimebaseRef master = CMTimebaseCopyMaster(timebase);
	CHECK(master == host);
	CFRelease(master);
	CMClockRef masterClock = CMTimebaseCopyMasterClock(timebase);
	CHECK(masterClock == host);
	CFRelease(masterClock);
	CHECK(CMTimebaseCopyMasterTimebase(timebase) == NULL);
	CHECK(CMTimebaseGetMasterClock(timebase) == host && CMTimebaseGetMaster(timebase) == host);
	CHECK(CMTimebaseGetMasterTimebase(timebase) == NULL);
	CMClockRef ultimate = CMTimebaseCopyUltimateMasterClock(timebase);
	CHECK(ultimate == host);
	CFRelease(ultimate);
	CHECK(CMTimebaseGetUltimateMasterClock(timebase) == host);
	// the newer names of the same things (macOS 12 and later, so looked up at run time)
	CMClockOrTimebaseRef (*copySource)(CMTimebaseRef) = dlsym(RTLD_DEFAULT, "CMTimebaseCopySource");
	CMClockRef (*copySourceClock)(CMTimebaseRef) = dlsym(RTLD_DEFAULT, "CMTimebaseCopySourceClock");
	CMTimebaseRef (*copySourceTimebase)(CMTimebaseRef) = dlsym(RTLD_DEFAULT, "CMTimebaseCopySourceTimebase");
	CMClockRef (*copyUltimateSource)(CMTimebaseRef) = dlsym(RTLD_DEFAULT, "CMTimebaseCopyUltimateSourceClock");
	OSStatus (*createWithSourceClock)(CFAllocatorRef, CMClockRef, CMTimebaseRef *) = dlsym(RTLD_DEFAULT, "CMTimebaseCreateWithSourceClock");
	if (copySource != NULL && copySourceClock != NULL && copySourceTimebase != NULL && copyUltimateSource != NULL
			&& createWithSourceClock != NULL) {
		CMClockOrTimebaseRef source = copySource(timebase);
		CHECK(source == host);
		CFRelease(source);
		CMClockRef sourceClock = copySourceClock(timebase);
		CHECK(sourceClock == host);
		CFRelease(sourceClock);
		CHECK(copySourceTimebase(timebase) == NULL);
		CMClockRef ultimateSource = copyUltimateSource(timebase);
		CHECK(ultimateSource == host);
		CFRelease(ultimateSource);
		CMTimebaseRef viaSource = NULL;
		CHECK(createWithSourceClock(kCFAllocatorDefault, host, &viaSource) == noErr && viaSource != NULL);
		CHECK(CMTimebaseGetMasterClock(viaSource) == host);
		CFRelease(viaSource);
	}

	// rate zero fixes the time
	CMTime first = CMTimebaseGetTime(timebase);
	sleepMilliseconds(20);
	CHECK(CMTimeCompare(CMTimebaseGetTime(timebase), first) == 0);
	CHECK(CMTimebaseSetTime(timebase, CMTimeMake(5, 1)) == noErr);
	sleepMilliseconds(20);
	CHECK(CMTimeCompare(CMTimebaseGetTime(timebase), CMTimeMake(5, 1)) == 0);

	// rate 1 from time 5: the time advances like the master, and the change of rate does not make it jump
	CHECK(CMTimebaseSetRate(timebase, 1.0) == noErr);
	CHECK(CMTimebaseGetRate(timebase) == 1.0 && CMTimebaseGetEffectiveRate(timebase) == 1.0);
	CMTime justAfter = CMTimebaseGetTime(timebase);
	CHECK(NEAR(justAfter, 5.0, 0.02));
	CMTime hostBefore = CMClockGetTime(host);
	sleepMilliseconds(50);
	CMTime later = CMTimebaseGetTime(timebase);
	CMTime hostAfter = CMClockGetTime(host);
	CHECK(CMTimeCompare(later, justAfter) > 0);
	CHECK(fabs((seconds(later) - seconds(justAfter)) - (seconds(hostAfter) - seconds(hostBefore))) < 0.02);

	// the rate changes at the current time: from rate 1 to rate 2 the time is continuous, then runs twice as fast
	CMTime beforeChange = CMTimebaseGetTime(timebase);
	CHECK(CMTimebaseSetRate(timebase, 2.0) == noErr);
	CMTime afterChange = CMTimebaseGetTime(timebase);
	CHECK(CMTimeCompare(afterChange, beforeChange) >= 0 && seconds(afterChange) - seconds(beforeChange) < 0.02);
	hostBefore = CMClockGetTime(host);
	CMTime start = CMTimebaseGetTime(timebase);
	sleepMilliseconds(50);
	CMTime end = CMTimebaseGetTime(timebase);
	hostAfter = CMClockGetTime(host);
	CHECK(fabs((seconds(end) - seconds(start)) - 2.0 * (seconds(hostAfter) - seconds(hostBefore))) < 0.04);

	// stopping again: the time freezes at the current value
	CHECK(CMTimebaseSetRate(timebase, 0.0) == noErr);
	CMTime frozen = CMTimebaseGetTime(timebase);
	sleepMilliseconds(20);
	CHECK(CMTimeCompare(CMTimebaseGetTime(timebase), frozen) == 0);

	// an anchor in the past: at rate 2 with time 100 s one second of master time ago, the time is about 102 s;
	// at rate -1 it is about 99 s
	CMTime now = CMClockGetTime(host);
	CHECK(CMTimebaseSetRateAndAnchorTime(timebase, 2.0, CMTimeMake(100, 1), CMTimeSubtract(now, CMTimeMake(1, 1))) == noErr);
	CHECK(NEAR(CMTimebaseGetTime(timebase), 102.0, 0.05));
	now = CMClockGetTime(host);
	CHECK(CMTimebaseSetRateAndAnchorTime(timebase, -1.0, CMTimeMake(100, 1), CMTimeSubtract(now, CMTimeMake(1, 1))) == noErr);
	CHECK(NEAR(CMTimebaseGetTime(timebase), 99.0, 0.05));
	CHECK(CMTimebaseGetRate(timebase) == -1.0);
	// CMTimebaseSetAnchorTime keeps the rate and moves the anchor
	now = CMClockGetTime(host);
	CHECK(CMTimebaseSetRate(timebase, 1.0) == noErr);
	CHECK(CMTimebaseSetAnchorTime(timebase, CMTimeMake(10, 1), CMTimeSubtract(CMClockGetTime(host), CMTimeMake(2, 1))) == noErr);
	CHECK(CMTimebaseGetRate(timebase) == 1.0);
	CHECK(NEAR(CMTimebaseGetTime(timebase), 12.0, 0.05));

	// time and rate in one step; each output is optional
	CHECK(CMTimebaseSetRate(timebase, 0.0) == noErr);
	CHECK(CMTimebaseSetTime(timebase, CMTimeMake(11, 2)) == noErr);
	CMTime snapshotTime = kCMTimeInvalid;
	Float64 snapshotRate = -1.0;
	CHECK(CMTimebaseGetTimeAndRate(timebase, &snapshotTime, &snapshotRate) == noErr);
	CHECK(CMTimeCompare(snapshotTime, CMTimeMake(11, 2)) == 0 && snapshotRate == 0.0);
	CHECK(CMTimebaseGetTimeAndRate(timebase, NULL, &snapshotRate) == noErr);
	CHECK(CMTimebaseGetTimeAndRate(timebase, &snapshotTime, NULL) == noErr);
	CHECK(CMTimebaseGetTimeAndRate(timebase, NULL, NULL) == noErr);

	// a child of a stopped parent: the time that passed is scaled by the rate. A whole-number rate multiplies the
	// value and keeps the timescale; any other rate rounds to a nanosecond, halves away from zero
	CMTimebaseRef frozenParent = NULL, exact = NULL;
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, &frozenParent) == noErr);
	CHECK(CMTimebaseSetTime(frozenParent, CMTimeMake(10000000000000000, 1000000000)) == noErr);
	CHECK(CMTimebaseCreateWithMasterTimebase(kCFAllocatorDefault, frozenParent, &exact) == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 0.5, kCMTimeZero, CMTimeMake(10000000000000000 - 1000000001, 1000000000)) == noErr);
	CMTime half = CMTimebaseGetTime(exact);
	CHECK(half.value == 500000001 && half.timescale == 1000000000);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, -0.5, kCMTimeZero, CMTimeMake(10000000000000000 - 1000000001, 1000000000)) == noErr);
	CHECK(CMTimebaseGetTime(exact).value == -500000001);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 0.25, CMTimeMake(1, 1), CMTimeMake(10000000000000000 - 4000000000, 1000000000)) == noErr);
	CMTime quarter = CMTimebaseGetTime(exact);
	CHECK(CMTimeCompare(quarter, CMTimeMake(2, 1)) == 0 && !(quarter.flags & kCMTimeFlags_HasBeenRounded));
	// a duration in whole seconds is still scaled to the nanosecond: a third of 1 s
	CHECK(CMTimebaseSetTime(frozenParent, CMTimeMake(1, 1)) == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 1.0 / 3.0, kCMTimeZero, kCMTimeZero) == noErr);
	CMTime aThird = CMTimebaseGetTime(exact);
	CHECK(aThird.value == 333333333 && aThird.timescale == 1000000000);
	CHECK(CMTimebaseSetTime(frozenParent, CMTimeMake(10, 1)) == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 0.5, kCMTimeZero, kCMTimeZero) == noErr);
	CMTime five = CMTimebaseGetTime(exact);
	CHECK(CMTimeCompare(five, CMTimeMake(5, 1)) == 0 && five.timescale == 1000000000);
	// QUIRK test: a whole-number rate keeps the timescale of the time that passed (3/2 s at rate 2 is 6/2, 7/600 s
	// at rate -2 is -14/600)
	CHECK(CMTimebaseSetTime(frozenParent, CMTimeMake(3, 2)) == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 2.0, kCMTimeZero, kCMTimeZero) == noErr);
	CMTime keeps = CMTimebaseGetTime(exact);
	CHECK(keeps.value == 6 && keeps.timescale == 2);
	CHECK(CMTimebaseSetTime(frozenParent, CMTimeMake(7, 600)) == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, -2.0, kCMTimeZero, kCMTimeZero) == noErr);
	keeps = CMTimebaseGetTime(exact);
	CHECK(keeps.value == -14 && keeps.timescale == 600);
	// a time with a coarse timescale goes to nanoseconds before a fractional rate is applied: 7/600 s is
	// 11666667 ns, half of it 5833333.5 ns, which rounds to 5833334
	CHECK(CMTimebaseSetRateAndAnchorTime(exact, 0.5, kCMTimeZero, kCMTimeZero) == noErr);
	keeps = CMTimebaseGetTime(exact);
	CHECK(keeps.value == 5833334 && keeps.timescale == 1000000000);
	CFRelease(exact);
	CFRelease(frozenParent);

	// the time in a given timescale: 5.5 s in whole seconds, by the requested rounding
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1, kCMTimeRoundingMethod_RoundTowardZero).value == 5);
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1, kCMTimeRoundingMethod_RoundAwayFromZero).value == 6);
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1000, kCMTimeRoundingMethod_Default).value == 5500);
	// QUIRK test: a timescale of 0 gives the time as it is, a negative one an invalid time
	CMTime asIs = CMTimebaseGetTimeWithTimeScale(timebase, 0, kCMTimeRoundingMethod_Default);
	CHECK(CMTimeCompare(asIs, CMTimebaseGetTime(timebase)) == 0 && asIs.timescale == CMTimebaseGetTime(timebase).timescale);
	CHECK(!CMTIME_IS_VALID(CMTimebaseGetTimeWithTimeScale(timebase, -5, kCMTimeRoundingMethod_Default)));

	// a timebase of a timebase: the effective rate is the product, the masters are as created, and a parent
	// that is stopped stops its child
	CMTimebaseRef parent = timebase, child = NULL;
	CHECK(CMTimebaseSetRate(parent, 2.0) == noErr);
	CHECK(CMTimebaseCreateWithMasterTimebase(kCFAllocatorDefault, parent, &child) == noErr && child != NULL);
	CHECK(CMTimebaseSetRate(child, 3.0) == noErr);
	CHECK(CMTimebaseGetRate(child) == 3.0 && CMTimebaseGetEffectiveRate(child) == 6.0);
	CHECK(CMTimebaseGetMasterTimebase(child) == parent && CMTimebaseGetMasterClock(child) == NULL);
	CHECK(CMTimebaseGetMaster(child) == parent);
	CMTimebaseRef copiedParent = CMTimebaseCopyMasterTimebase(child);
	CHECK(copiedParent == parent);
	CFRelease(copiedParent);
	CHECK(CMTimebaseCopyMasterClock(child) == NULL);
	CHECK(CMTimebaseGetUltimateMasterClock(child) == host);
	CMClockRef ultimateOfChild = CMTimebaseCopyUltimateMasterClock(child);
	CHECK(ultimateOfChild == host);
	CFRelease(ultimateOfChild);
	OSStatus (*createWithSourceTimebase)(CFAllocatorRef, CMTimebaseRef, CMTimebaseRef *) = dlsym(RTLD_DEFAULT, "CMTimebaseCreateWithSourceTimebase");
	CMTimebaseRef (*copySourceTimebaseOf)(CMTimebaseRef) = dlsym(RTLD_DEFAULT, "CMTimebaseCopySourceTimebase");
	if (createWithSourceTimebase != NULL && copySourceTimebaseOf != NULL) {
		CMTimebaseRef viaSourceTimebase = NULL;
		CHECK(createWithSourceTimebase(kCFAllocatorDefault, parent, &viaSourceTimebase) == noErr);
		CMTimebaseRef copiedSource = copySourceTimebaseOf(viaSourceTimebase);
		CHECK(copiedSource == parent);
		CFRelease(copiedSource);
		CFRelease(viaSourceTimebase);
	}
	// a timebase keeps its master alive: the only other reference to the master is dropped
	CMTimebaseRef shortLived = NULL, dependent = NULL;
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, &shortLived) == noErr);
	CHECK(CMTimebaseCreateWithMasterTimebase(kCFAllocatorDefault, shortLived, &dependent) == noErr);
	CFRelease(shortLived);
	CMTimebaseRef survivor = CMTimebaseCopyMasterTimebase(dependent);
	CHECK(survivor == shortLived && CFGetTypeID(survivor) == CMTimebaseGetTypeID());
	CHECK(CMTimeCompare(CMTimebaseGetTime(survivor), kCMTimeZero) == 0);
	CFRelease(survivor);
	CFRelease(dependent);
	CHECK(CMTimebaseSetRate(child, 1.0) == noErr);
	CHECK(CMTimebaseSetRate(parent, 0.0) == noErr);
	CHECK(CMTimebaseGetEffectiveRate(child) == 0.0);
	CMTime parentStopped = CMTimebaseGetTime(parent);
	sleepMilliseconds(20);
	CHECK(CMTimeCompare(CMTimebaseGetTime(parent), parentStopped) == 0);
	// a child at rate 1 of a parent that stands still stays where it is
	CHECK(CMTimebaseSetTime(child, CMTimeMake(7, 1)) == noErr);
	sleepMilliseconds(20);
	CHECK(CMTimeCompare(CMTimebaseGetTime(child), CMTimeMake(7, 1)) == 0);

	// CMSyncGetTime reads a clock or a timebase
	CHECK(CMTIME_IS_NUMERIC(CMSyncGetTime(host)));
	CHECK(CMTimeCompare(CMSyncGetTime(child), CMTimebaseGetTime(child)) == 0);

	// errors: NULL is MissingRequiredParameter everywhere (an object of the wrong type crashes macOS and is not
	// tested)
	CMTimebaseRef created = NULL;
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, NULL, &created) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, NULL) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseCreateWithMasterTimebase(kCFAllocatorDefault, NULL, &created) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(created == NULL);
	CHECK(CMTimebaseSetTime(NULL, kCMTimeZero) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetRate(NULL, 1.0) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetAnchorTime(NULL, kCMTimeZero, kCMTimeZero) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetRateAndAnchorTime(NULL, 1.0, kCMTimeZero, kCMTimeZero) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseGetTimeAndRate(NULL, &snapshotTime, &snapshotRate) == kCMTimebaseError_MissingRequiredParameter);

	// unusual values, each on a timebase of its own that stands at 7 s
	CMTimebaseRef scratch = NULL;
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, &scratch) == noErr);
	CHECK(CMTimebaseSetTime(scratch, CMTimeMake(7, 1)) == noErr);
	// QUIRK test: SetTime with a time that is not numeric is MissingRequiredParameter and changes nothing
	CHECK(CMTimebaseSetTime(scratch, kCMTimeInvalid) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetTime(scratch, kCMTimePositiveInfinity) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetTime(scratch, kCMTimeNegativeInfinity) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimebaseSetTime(scratch, kCMTimeIndefinite) == kCMTimebaseError_MissingRequiredParameter);
	CHECK(CMTimeCompare(CMTimebaseGetTime(scratch), CMTimeMake(7, 1)) == 0 && CMTimebaseGetRate(scratch) == 0.0);
	// QUIRK test: SetRateAndAnchorTime with a master time that is not valid is InvalidParameter and changes nothing
	CHECK(CMTimebaseSetRateAndAnchorTime(scratch, 2.0, CMTimeMake(3, 1), kCMTimeInvalid) == kCMTimebaseError_InvalidParameter);
	CHECK(CMTimeCompare(CMTimebaseGetTime(scratch), CMTimeMake(7, 1)) == 0 && CMTimebaseGetRate(scratch) == 0.0);
	// everything else is accepted: a rate that is not a number or is infinite is stored, and the time is then
	// invalid or infinite; so are anchor times that are not numeric
	CHECK(CMTimebaseSetRateAndAnchorTime(scratch, NAN, CMTimeMake(3, 1), CMClockGetTime(host)) == noErr);
	// (the time is then invalid on macOS 26.7.3; 10.14.4 gives an arbitrary number, so it is not checked)
	CHECK(isnan(CMTimebaseGetRate(scratch)));
	CHECK(CMTimebaseSetRateAndAnchorTime(scratch, 0.0, CMTimeMake(3, 1), CMClockGetTime(host)) == noErr);
	CHECK(CMTimebaseSetAnchorTime(scratch, CMTimeMake(5, 1), kCMTimeInvalid) == noErr);
	CHECK(CMTimeCompare(CMTimebaseGetTime(scratch), CMTimeMake(5, 1)) == 0);
	CHECK(CMTimebaseSetTime(scratch, CMTimeMake(7, 1)) == noErr);
	CHECK(CMTimebaseSetRate(scratch, INFINITY) == noErr && CMTimebaseGetRate(scratch) == INFINITY);
	CHECK(CMTimebaseSetRate(scratch, -INFINITY) == noErr && CMTimebaseGetRate(scratch) == -INFINITY);
	CFRelease(scratch);
	// the getters of NULL
	CHECK(!CMTIME_IS_VALID(CMTimebaseGetTime(NULL)));
	CHECK(!CMTIME_IS_VALID(CMSyncGetTime(NULL)));
	CHECK(CMTimebaseGetRate(NULL) == 0.0 && CMTimebaseGetEffectiveRate(NULL) == 0.0);
	CHECK(CMTimebaseCopyMaster(NULL) == NULL && CMTimebaseGetMaster(NULL) == NULL);
	CHECK(CMTimebaseCopyUltimateMasterClock(NULL) == NULL && CMTimebaseCopyMasterTimebase(NULL) == NULL);

	CFRelease(child);
	CFRelease(parent);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
