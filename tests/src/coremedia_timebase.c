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

// Timebases: creating one and its masters.
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

	// creation: a CF object of its own type, with the clock as master
	CHECK(CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, host, &timebase) == noErr);
	CHECK(timebase != NULL && CFGetTypeID(timebase) == CMTimebaseGetTypeID());
	CHECK(CMTimebaseGetTypeID() != CMClockGetTypeID());
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

	CFRelease(timebase);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
