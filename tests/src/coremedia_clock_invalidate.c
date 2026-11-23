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

// CMClockGetAnchorTime, CMClockMightDrift and CMClockInvalidate.
// The documented contract: CMClockGetAnchorTime returns the clock's time and the matching time of its
// reference clock; CMClockMightDrift says whether two clocks can drift against each other; CMClockInvalidate
// stops a clock for good, and is meant for the owner of the clock.
// https://developer.apple.com/documentation/coremedia/cmclockgetanchortime(_:clocktimeout:referenceclocktimeout:)
// https://developer.apple.com/documentation/coremedia/cmclockmightdrift(_:otherclock:)
// https://developer.apple.com/documentation/coremedia/cmclockinvalidate(_:)
// The error codes are those of the public CMSync.h. The behaviour the documentation leaves open (the reference
// clock of the host time clock, the status of invalid arguments, what invalidating the singleton does) is
// pinned as the GUESS choices of the implementation. Prints PASS when everything holds; the exit code is the
// number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	CMClockRef clock = CMClockGetHostTimeClock();
	CMTime clockTime = kCMTimeInvalid, referenceTime = kCMTimeInvalid;

	// the error codes of the public header
	CHECK(kCMClockError_MissingRequiredParameter == -12745);
	CHECK(kCMClockError_InvalidParameter == -12746);
	CHECK(kCMClockError_AllocationFailed == -12747);
	CHECK(kCMClockError_UnsupportedOperation == -12756);

	// the anchor time: the clock's time, in step with CMClockGetTime, and the same reading from the reference
	// clock (GUESS: the host time clock is its own reference clock)
	CMTime before = CMClockGetTime(clock);
	CHECK(CMClockGetAnchorTime(clock, &clockTime, &referenceTime) == noErr);
	CMTime after = CMClockGetTime(clock);
	CHECK(CMTIME_IS_NUMERIC(clockTime));
	CHECK(CMTimeCompare(before, clockTime) <= 0 && CMTimeCompare(clockTime, after) <= 0);
	CHECK(CMTimeCompare(clockTime, referenceTime) == 0);

	// missing arguments, and an object that is not a clock (GUESS: the statuses are chosen by the names)
	CHECK(CMClockGetAnchorTime(NULL, &clockTime, &referenceTime) == kCMClockError_MissingRequiredParameter);
	CHECK(CMClockGetAnchorTime(clock, NULL, &referenceTime) == kCMClockError_MissingRequiredParameter);
	CHECK(CMClockGetAnchorTime(clock, &clockTime, NULL) == kCMClockError_MissingRequiredParameter);
	CHECK(CMClockGetAnchorTime((CMClockRef)CFSTR("not a clock"), &clockTime, &referenceTime) == kCMClockError_InvalidParameter);

	// the host time clock is the only clock, so it cannot drift against itself (GUESS)
	CHECK(!CMClockMightDrift(clock, clock));
	CHECK(!CMClockMightDrift(clock, CMClockGetHostTimeClock()));
	CHECK(!CMClockMightDrift(NULL, clock));
	CHECK(!CMClockMightDrift(clock, (CMClockRef)CFSTR("not a clock")));

	// invalidating NULL or an object that is not a clock does nothing
	CMClockInvalidate(NULL);
	CMClockInvalidate((CMClockRef)CFSTR("not a clock"));
	CHECK(CMTIME_IS_NUMERIC(CMClockGetTime(clock)));

	// GUESS: invalidating the singleton host time clock, which has no owner, is ignored: it keeps working,
	// and so does everything built on it
	CMClockInvalidate(clock);
	CMClockInvalidate(clock);
	CHECK(CMClockGetHostTimeClock() == clock);
	CHECK(CMTIME_IS_NUMERIC(CMClockGetTime(clock)));
	CHECK(CMClockGetAnchorTime(clock, &clockTime, &referenceTime) == noErr);
	CHECK(CMTIME_IS_NUMERIC(clockTime) && CMTimeCompare(clockTime, referenceTime) == 0);
	CHECK(!CMClockMightDrift(clock, clock));

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
