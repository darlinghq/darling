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
// The error codes are those of the public CMSync.h; which code an invalid argument gives is a GUESS of the
// implementation and is pinned here. Moving times are checked with a tolerance, because they follow the host
// time clock. Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
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
	// the newer names of the same things
	CMClockOrTimebaseRef source = CMTimebaseCopySource(timebase);
	CHECK(source == host);
	CFRelease(source);
	CMClockRef sourceClock = CMTimebaseCopySourceClock(timebase);
	CHECK(sourceClock == host);
	CFRelease(sourceClock);
	CHECK(CMTimebaseCopySourceTimebase(timebase) == NULL);
	CMClockRef ultimateSource = CMTimebaseCopyUltimateSourceClock(timebase);
	CHECK(ultimateSource == host);
	CFRelease(ultimateSource);
	CMTimebaseRef viaSource = NULL;
	CHECK(CMTimebaseCreateWithSourceClock(kCFAllocatorDefault, host, &viaSource) == noErr && viaSource != NULL);
	CHECK(CMTimebaseGetMasterClock(viaSource) == host);
	CFRelease(viaSource);

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

	// the time in a given timescale: 5.5 s in whole seconds, by the requested rounding
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1, kCMTimeRoundingMethod_RoundTowardZero).value == 5);
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1, kCMTimeRoundingMethod_RoundAwayFromZero).value == 6);
	CHECK(CMTimebaseGetTimeWithTimeScale(timebase, 1000, kCMTimeRoundingMethod_Default).value == 5500);
	CHECK(!CMTIME_IS_VALID(CMTimebaseGetTimeWithTimeScale(timebase, 0, kCMTimeRoundingMethod_Default)));

	CFRelease(timebase);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
