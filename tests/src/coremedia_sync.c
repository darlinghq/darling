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

// CMSyncGetRelativeRate, CMSyncGetRelativeRateAndAnchorTime, CMSyncConvertTime and CMSyncMightDrift.
// The documented contract (public CMSync.h and the pages below): with a common master the calculations
// are made purely from the rates and offsets in the tree under it; the rate of a moving timebase relative
// to a stopped timebase is a NaN; CMTimebaseGetEffectiveRate(t) is CMSyncGetRelativeRate(t, its ultimate
// master clock); timebases can drift against each other only if their ultimate master clocks can.
// https://developer.apple.com/documentation/coremedia/cmsyncgetrelativerate(_:relativeto:)
// https://developer.apple.com/documentation/coremedia/cmsyncgetrelativerateandanchortime(_:relativeto:relativerateout:anchortimeout:relativetoanchortimeout:)
// https://developer.apple.com/documentation/coremedia/cmsyncconverttime(_:from:to:)
// https://developer.apple.com/documentation/coremedia/cmsyncmightdrift(_:_:)
// The cases the documentation leaves open (stopped against stopped, converting a time of a stopped timebase,
// the anchor times, times that are not numeric, NULL arguments) are what macOS 26.7.3 and 10.14.4 do, observed
// with the same checks (objects that are neither clocks nor timebases crash there and are not tested). The error
// codes are those of the public CMSync.h. Prints PASS when everything holds; the exit code is the number of
// failed checks.

#include <CoreMedia/CoreMedia.h>
#include <math.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static int equalTime(CMTime a, CMTime b) { return CMTIME_IS_NUMERIC(a) && CMTimeCompare(a, b) == 0; }

static CMTimebaseRef newTimebase(CMClockOrTimebaseRef master, Float64 rate, CMTime time, CMTime masterTime)
{
	CMTimebaseRef timebase = NULL;
	OSStatus status = CFGetTypeID(master) == CMClockGetTypeID()
		? CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, (CMClockRef)master, &timebase)
		: CMTimebaseCreateWithMasterTimebase(kCFAllocatorDefault, (CMTimebaseRef)master, &timebase);
	CHECK(status == noErr);
	CHECK(CMTimebaseSetRateAndAnchorTime(timebase, rate, time, masterTime) == noErr);
	return timebase;
}

int main(void)
{
	CMClockRef host = CMClockGetHostTimeClock();

	CHECK(kCMSyncError_MissingRequiredParameter == -12752);
	CHECK(kCMSyncError_InvalidParameter == -12753);
	CHECK(kCMSyncError_AllocationFailed == -12754);
	CHECK(kCMSyncError_RateMustBeNonZero == -12755);

	// A runs at 2 with 100 s at host time r0, B at 0.5 with 50 s at r0, C runs at 3 relative to A with 0 s at
	// A time 100 s, and S is stopped at 9 s
	CMTime r0 = CMClockGetTime(host);
	CMTimebaseRef a = newTimebase(host, 2.0, CMTimeMake(100, 1), r0);
	CMTimebaseRef b = newTimebase(host, 0.5, CMTimeMake(50, 1), r0);
	CMTimebaseRef c = newTimebase(a, 3.0, kCMTimeZero, CMTimeMake(100, 1));
	CMTimebaseRef s = newTimebase(host, 0.0, CMTimeMake(9, 1), r0);
	CMTimebaseRef s2 = newTimebase(host, 0.0, CMTimeMake(4, 1), r0);

	// relative rates: ratios of the effective rates, a clock having rate 1
	CHECK(CMSyncGetRelativeRate(host, host) == 1.0);
	CHECK(CMSyncGetRelativeRate(a, host) == 2.0);
	CHECK(CMSyncGetRelativeRate(host, a) == 0.5);
	CHECK(CMSyncGetRelativeRate(a, b) == 4.0);
	CHECK(CMSyncGetRelativeRate(b, a) == 0.25);
	CHECK(CMSyncGetRelativeRate(c, host) == 6.0);
	CHECK(CMSyncGetRelativeRate(c, a) == 3.0);
	CHECK(fabs(CMSyncGetRelativeRate(a, c) - 1.0 / 3.0) < 1.0e-12);
	CHECK(CMSyncGetRelativeRate(a, a) == 1.0);
	CHECK(CMSyncGetRelativeRate(a, host) == CMTimebaseGetEffectiveRate(a));
	CHECK(CMSyncGetRelativeRate(c, host) == CMTimebaseGetEffectiveRate(c));
	// a stopped timebase: the rate relative to it of a moving one is a NaN; relative to a moving one it is 0
	CHECK(isnan(CMSyncGetRelativeRate(a, s)));
	CHECK(isnan(CMSyncGetRelativeRate(host, s)));
	CHECK(CMSyncGetRelativeRate(s, a) == 0.0);
	CHECK(CMSyncGetRelativeRate(s, host) == 0.0 && CMTimebaseGetEffectiveRate(s) == 0.0);
	// a stopped timebase relative to another stopped one is a NaN too, relative to itself it is 1
	CHECK(isnan(CMSyncGetRelativeRate(s, s2)));
	CHECK(CMSyncGetRelativeRate(s, s) == 1.0 && CMSyncGetRelativeRate(a, a) == 1.0 && CMSyncGetRelativeRate(host, host) == 1.0);
	CHECK(isnan(CMSyncGetRelativeRate(NULL, host)));
	CHECK(isnan(CMSyncGetRelativeRate(host, NULL)));

	// rate and anchor times. QUIRK test (the choice of the two instants is not documented): when the second
	// object is a clock, the anchors are the first one's own anchor and the time of the clock at that instant;
	// otherwise the instant at which the second reads zero: the time of the first then, and zero. Observed on
	// macOS 26.7.3 and 10.14.4 (26.7.3 also uses the anchor of a timebase for its master timebase; 10.14.4 does
	// not, and that is followed).
	Float64 rate = -1.0;
	CMTime ofAnchor = kCMTimeInvalid, relativeToAnchor = kCMTimeInvalid;
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, host, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 2.0 && equalTime(ofAnchor, CMTimeMake(100, 1)) && equalTime(relativeToAnchor, r0));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(c, a, &rate, &ofAnchor, &relativeToAnchor) == noErr && rate == 3.0);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(c, host, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 6.0 && equalTime(ofAnchor, kCMTimeZero) && equalTime(relativeToAnchor, r0));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 4.0 && equalTime(ofAnchor, CMTimeMake(-100, 1)) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(b, a, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 0.25 && equalTime(ofAnchor, CMTimeMake(25, 1)) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(host, a, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 0.5 && equalTime(ofAnchor, CMTimeSubtract(r0, CMTimeMake(50, 1))) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(c, b, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 12.0 && equalTime(ofAnchor, CMTimeMake(-600, 1)) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, a, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 1.0 && equalTime(ofAnchor, kCMTimeZero) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(host, host, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 1.0 && equalTime(ofAnchor, kCMTimeZero) && equalTime(relativeToAnchor, kCMTimeZero));
	// a stopped first object: its fixed time, and the same rules for the second
	CHECK(CMSyncGetRelativeRateAndAnchorTime(s, a, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 0.0 && equalTime(ofAnchor, CMTimeMake(9, 1)) && equalTime(relativeToAnchor, kCMTimeZero));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(s, host, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 0.0 && equalTime(ofAnchor, CMTimeMake(9, 1)) && equalTime(relativeToAnchor, r0));
	// relative to a stopped timebase: RateMustBeNonZero, and the outputs are not written
	rate = -77.0; ofAnchor = relativeToAnchor = kCMTimeInvalid;
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, s, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_RateMustBeNonZero);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(host, s, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_RateMustBeNonZero);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(s, s2, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_RateMustBeNonZero);
	CHECK(rate == -77.0 && !CMTIME_IS_VALID(ofAnchor) && !CMTIME_IS_VALID(relativeToAnchor));
	// every output is optional; NULL objects are MissingRequiredParameter
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, NULL, NULL, NULL) == noErr);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, &rate, NULL, NULL) == noErr && rate == 4.0);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(NULL, b, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_MissingRequiredParameter);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, NULL, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_MissingRequiredParameter);

	// conversions: A reads 110 s when the host reads r0 + 5 s, and the other way round
	CMTime hostPlus5 = CMTimeAdd(r0, CMTimeMake(5, 1));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(110, 1), a, host), hostPlus5));
	CHECK(equalTime(CMSyncConvertTime(hostPlus5, host, a), CMTimeMake(110, 1)));
	// from one timebase to another: B reads 50 + 0.5 * 5 s at that moment
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(110, 1), a, b), CMTimeMake(105, 2)));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(105, 2), b, a), CMTimeMake(110, 1)));
	// C reads 30 s when A reads 110 s: between a timebase and its master, and across the tree
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(30, 1), c, a), CMTimeMake(110, 1)));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(110, 1), a, c), CMTimeMake(30, 1)));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(30, 1), c, host), hostPlus5));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(30, 1), c, b), CMTimeMake(105, 2)));
	// a rate that does not divide evenly is exact to the nanosecond: C reads 1 s a third of a second of A later
	CMTime third = CMSyncConvertTime(CMTimeAdd(CMTimeMake(30, 1), CMTimeMake(1, 1)), c, a);
	CHECK(fabs(CMTimeGetSeconds(CMTimeSubtract(third, CMTimeMake(110, 1))) - 1.0 / 3.0) < 2.0e-9);
	// the same object, and a clock to itself, leave the time as it is
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(7, 1), a, a), CMTimeMake(7, 1)));
	CHECK(equalTime(CMSyncConvertTime(hostPlus5, host, host), hostPlus5));
	// to a stopped timebase the time is its fixed time; from one it has no time, so the result is invalid (the same object excepted)
	CHECK(equalTime(CMSyncConvertTime(hostPlus5, host, s), CMTimeMake(9, 1)));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(110, 1), a, s), CMTimeMake(9, 1)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, host)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, a)));
	CMTimebaseRef otherStopped = newTimebase(host, 0.0, CMTimeMake(4, 1), r0);
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, otherStopped)));
	CFRelease(otherStopped);
	// QUIRK test: a time that is not numeric (invalid, infinite, indefinite) converts to an invalid time, even to the
	// same object, and so does a NULL object; observed on macOS 26.7.3 and 10.14.4
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(kCMTimePositiveInfinity, a, host)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(kCMTimeIndefinite, a, b)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(kCMTimeNegativeInfinity, host, host)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(kCMTimeInvalid, a, b)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(1, 1), NULL, a)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(1, 1), a, NULL)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(1, 1), NULL, NULL)));

	// the host time clock is the only clock: nothing drifts relative to anything. QUIRK test: a NULL argument
	// (either) drifts, observed on macOS 26.7.3 and 10.14.4
	CHECK(!CMSyncMightDrift(host, host));
	CHECK(!CMSyncMightDrift(a, b));
	CHECK(!CMSyncMightDrift(c, host));
	CHECK(!CMSyncMightDrift(s, s2));
	CHECK(CMSyncMightDrift(NULL, a));
	CHECK(CMSyncMightDrift(a, NULL));
	CHECK(CMSyncMightDrift(NULL, NULL));

	CFRelease(c);
	CFRelease(a);
	CFRelease(b);
	CFRelease(s);
	CFRelease(s2);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
