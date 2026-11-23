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
// the anchor times, the codes of invalid arguments) are pinned as the GUESS choices of the implementation.
// The error codes are those of the public CMSync.h. Prints PASS when everything holds; the exit code is the
// number of failed checks.

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
	// GUESS: a stopped timebase relative to a stopped one is a NaN too
	CHECK(isnan(CMSyncGetRelativeRate(s, s)));
	// GUESS: an argument that is neither a clock nor a timebase gives a NaN
	CHECK(isnan(CMSyncGetRelativeRate(NULL, host)));
	CHECK(isnan(CMSyncGetRelativeRate(a, CFSTR("no"))));

	// rate and anchor times: the rate, and each object's time at one instant of the host clock
	Float64 rate = -1.0;
	CMTime ofAnchor = kCMTimeInvalid, relativeToAnchor = kCMTimeInvalid;
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 4.0);
	// at a host time h, A reads 100 + 2 (h - r0) and B reads 50 + 0.5 (h - r0): A's anchor time fixes h
	Float64 sinceR0 = (CMTimeGetSeconds(ofAnchor) - 100.0) / 2.0;
	CHECK(sinceR0 >= 0.0 && sinceR0 < 5.0);
	CHECK(fabs(CMTimeGetSeconds(relativeToAnchor) - (50.0 + 0.5 * sinceR0)) < 1.0e-6);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, NULL, NULL, NULL) == noErr);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, b, &rate, NULL, NULL) == noErr && rate == 4.0);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(host, host, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(rate == 1.0 && CMTimeCompare(ofAnchor, relativeToAnchor) == 0 && CMTIME_IS_NUMERIC(ofAnchor));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, s, &rate, &ofAnchor, &relativeToAnchor) == noErr);
	CHECK(isnan(rate) && equalTime(relativeToAnchor, CMTimeMake(9, 1)));
	CHECK(CMSyncGetRelativeRateAndAnchorTime(NULL, b, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_MissingRequiredParameter);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, NULL, &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_MissingRequiredParameter);
	CHECK(CMSyncGetRelativeRateAndAnchorTime(a, CFSTR("no"), &rate, &ofAnchor, &relativeToAnchor) == kCMSyncError_InvalidParameter);

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
	// to a stopped timebase the time is its fixed time; GUESS: from one it has no time, so the result is invalid
	CHECK(equalTime(CMSyncConvertTime(hostPlus5, host, s), CMTimeMake(9, 1)));
	CHECK(equalTime(CMSyncConvertTime(CMTimeMake(110, 1), a, s), CMTimeMake(9, 1)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, host)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, a)));
	CMTimebaseRef otherStopped = newTimebase(host, 0.0, CMTimeMake(4, 1), r0);
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(9, 1), s, otherStopped)));
	CFRelease(otherStopped);
	// GUESS: a time that is not numeric is returned as it is; an object that is not a clock or a timebase
	// gives an invalid time
	CHECK(CMTIME_IS_POSITIVE_INFINITY(CMSyncConvertTime(kCMTimePositiveInfinity, a, host)));
	CHECK(CMTIME_IS_INDEFINITE(CMSyncConvertTime(kCMTimeIndefinite, a, b)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(kCMTimeInvalid, a, b)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(1, 1), NULL, a)));
	CHECK(!CMTIME_IS_VALID(CMSyncConvertTime(CMTimeMake(1, 1), a, CFSTR("no"))));

	// the host time clock is the only clock: nothing drifts relative to anything (GUESS for the arguments
	// that are not a clock or a timebase)
	CHECK(!CMSyncMightDrift(host, host));
	CHECK(!CMSyncMightDrift(a, b));
	CHECK(!CMSyncMightDrift(c, host));
	CHECK(!CMSyncMightDrift(NULL, a));

	CFRelease(c);
	CFRelease(a);
	CFRelease(b);
	CFRelease(s);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
