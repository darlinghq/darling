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
// clock of the host time clock, the status of NULL arguments, drifting against another object) is what macOS
// 26.7.3 and 10.14.4 do, observed with the same checks; invalidating the singleton host time clock crashes
// there and is not tested. Prints PASS when everything holds; the exit code is the number of failed checks.

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
	// clock (the host time clock is its own reference clock, as on macOS)
	CMTime before = CMClockGetTime(clock);
	CHECK(CMClockGetAnchorTime(clock, &clockTime, &referenceTime) == noErr);
	CMTime after = CMClockGetTime(clock);
	CHECK(CMTIME_IS_NUMERIC(clockTime));
	CHECK(CMTimeCompare(before, clockTime) <= 0 && CMTimeCompare(clockTime, after) <= 0);
	CHECK(CMTimeCompare(clockTime, referenceTime) == 0);

	// A missing clock is MissingRequiredParameter; the two outputs are optional (all of this was observed on macOS
	// 26.7.3 and 10.14.4)
	CHECK(CMClockGetAnchorTime(NULL, &clockTime, &referenceTime) == kCMClockError_MissingRequiredParameter);
	CMTime onlyClock = kCMTimeInvalid, onlyReference = kCMTimeInvalid;
	CHECK(CMClockGetAnchorTime(clock, &onlyClock, NULL) == noErr && CMTIME_IS_NUMERIC(onlyClock));
	CHECK(CMClockGetAnchorTime(clock, NULL, &onlyReference) == noErr && CMTIME_IS_NUMERIC(onlyReference));
	CHECK(CMClockGetAnchorTime(clock, NULL, NULL) == noErr);

	// the host time clock is the only clock, so it cannot drift against itself
	CHECK(!CMClockMightDrift(clock, clock));
	CHECK(!CMClockMightDrift(clock, CMClockGetHostTimeClock()));
	// QUIRK test: any other object counts as another clock, which may drift (macOS 26.7.3 and 10.14.4 x86_64
	// say 1; the 10.14.4 i386 slice crashes, so it is not run there)
#ifndef __i386__
	CHECK(CMClockMightDrift(clock, (CMClockRef)CFSTR("not a clock")));
#endif

	// invalidating NULL or an object that is not a clock does nothing, and the host time clock keeps working
	// (invalidating the host time clock itself crashes macOS, so that is not tested)
	CMClockInvalidate(NULL);
	CMClockInvalidate((CMClockRef)CFSTR("not a clock"));
	CHECK(CMTIME_IS_NUMERIC(CMClockGetTime(clock)));
	CHECK(CMClockGetHostTimeClock() == clock);

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
