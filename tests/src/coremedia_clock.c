// CFLAGS: -framework CoreMedia -framework CoreVideo -framework CoreFoundation
/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// CMClockGetHostTimeClock, CMClockGetTime, CMClockGetTypeID and the two host time conversions.
// The documented contract: the host time clock is a singleton that reads mach_absolute_time and
// reports it with a large integer timescale; the conversions are scale conversions to and from
// the native units of mach_absolute_time.
// https://developer.apple.com/documentation/coremedia/cmclockgethosttimeclock()
// https://developer.apple.com/documentation/coremedia/cmclockgettime(_:)
// https://developer.apple.com/documentation/coremedia/cmclockconverthosttimetosystemunits(_:)
// https://developer.apple.com/documentation/coremedia/cmclockmakehosttimefromsystemunits(_:)
// The cases the documentation does not cover (invalid input, rounding) are only checked to
// give a defined answer, not a particular one. Prints PASS when everything holds; the exit
// code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <mach/mach_time.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	mach_timebase_info_data_t info;
	CHECK(mach_timebase_info(&info) == KERN_SUCCESS);

	// the singleton, and its type
	CMClockRef clock = CMClockGetHostTimeClock();
	CHECK(clock != NULL);
	CHECK(CMClockGetHostTimeClock() == clock);
	CHECK(CFGetTypeID(clock) == CMClockGetTypeID());
	CFRetain(clock);
	CFRelease(clock);
	CHECK(CMClockGetHostTimeClock() == clock);

	// the current time is mach_absolute_time in a large integer timescale
	uint64_t before = mach_absolute_time();
	CMTime now = CMClockGetTime(clock);
	uint64_t after = mach_absolute_time();
	CHECK(CMTIME_IS_NUMERIC(now));
	CHECK(now.timescale >= 1000000);
	uint64_t ticks = CMClockConvertHostTimeToSystemUnits(now);
	CHECK(before <= ticks && ticks <= after);

	// the host time of CoreVideo is the same clock: the clock's time, converted back to ticks, lies between two
	// CVGetCurrentHostTime reads, and one second of CVGetHostClockFrequency ticks is one second of clock time
	uint64_t cvBefore = CVGetCurrentHostTime();
	CMTime sample = CMClockGetTime(clock);
	uint64_t cvAfter = CVGetCurrentHostTime();
	uint64_t sampleTicks = CMClockConvertHostTimeToSystemUnits(sample);
	CHECK(cvBefore <= sampleTicks && sampleTicks <= cvAfter);
	CMTime oneSecond = CMClockMakeHostTimeFromSystemUnits((uint64_t)CVGetHostClockFrequency());
	CHECK(fabs(CMTimeGetSeconds(oneSecond) - 1.0) < 1.0e-6);

	// it advances, and by about the elapsed monotonic time
	struct timespec t0, t1, pause = { 0, 20 * 1000 * 1000 };
	clock_gettime(CLOCK_MONOTONIC, &t0);
	CMTime a = CMClockGetTime(clock);
	nanosleep(&pause, NULL);
	CMTime b = CMClockGetTime(clock);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	CHECK(CMTimeCompare(b, a) > 0);
	double byClock = CMTimeGetSeconds(CMTimeSubtract(b, a));
	double byMonotonic = (double)(t1.tv_sec - t0.tv_sec) + (double)(t1.tv_nsec - t0.tv_nsec) / 1.0e9;
	CHECK(fabs(byClock - byMonotonic) < 0.005);

	// the conversion from native units: one second of ticks is one second of time
	double ticksPerSecond = 1.0e9 * info.denom / info.numer;
	CMTime second = CMClockMakeHostTimeFromSystemUnits((uint64_t)ticksPerSecond);
	CHECK(CMTIME_IS_NUMERIC(second) && second.timescale >= 1000000);
	CHECK(fabs(CMTimeGetSeconds(second) - 1.0) < 1.0e-6);
	CHECK(CMTimeCompare(CMClockMakeHostTimeFromSystemUnits(0), kCMTimeZero) == 0);

	// ... and to native units, exactly for whole ticks: info.denom ticks last info.numer nanoseconds
	CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(info.numer, 1000000000)) == info.denom);
	CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(5 * (int64_t)info.numer, 1000000000)) == 5 * (uint64_t)info.denom);
	CHECK(CMClockConvertHostTimeToSystemUnits(kCMTimeZero) == 0);

	// the two conversions are inverse of each other, up to the rounding of the nanosecond value
	uint64_t samples[] = { 1, 1000, 123456789, 24 * 3600 * (uint64_t)1000000000, 1ull << 40 };
	for (unsigned i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
		uint64_t tick = CMClockConvertHostTimeToSystemUnits(CMClockMakeHostTimeFromSystemUnits(samples[i]));
		uint64_t tolerance = info.denom / info.numer + 1;
		CHECK(tick + tolerance >= samples[i] && tick <= samples[i] + tolerance);
	}

	// a different timescale is a scale conversion: 3 seconds is 3 * ticksPerSecond ticks
	uint64_t three = CMClockConvertHostTimeToSystemUnits(CMTimeMake(3 * 600, 600));
	CHECK(fabs((double)three - 3.0 * ticksPerSecond) <= 1.0);

	// the rounding is to the nearest tick, halves away from zero (observed on macOS 26.7.3 and 10.14.4); with
	// one nanosecond per tick, half a tick is a time of (2k + 1) / 2e9 seconds
	if (info.numer == 1 && info.denom == 1) {
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(5, 2000000000)) == 3);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(4, 2000000000)) == 2);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(3, 2000000000)) == 2);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(1, 2000000000)) == 1);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(1, 3)) == 333333333);
		// negative times round the same way (see the quirk test below)
		CHECK((int64_t)CMClockConvertHostTimeToSystemUnits(CMTimeMake(-1, 2000000000)) == -1);
		CHECK((int64_t)CMClockConvertHostTimeToSystemUnits(CMTimeMake(-3, 2000000000)) == -2);
		CHECK((int64_t)CMClockConvertHostTimeToSystemUnits(CMTimeMake(-5, 2000000000)) == -3);
	}

	// QUIRK test, host time above INT64_MAX wraps: the nanosecond value of a host time that is not below 2^63
	// is not rejected but wraps to a negative one (macOS 26.7.3 and 10.14.4, 1:1 timebase).
	if (info.numer == 1 && info.denom == 1) {
		CMTime wrapped = CMClockMakeHostTimeFromSystemUnits(UINT64_MAX);
		CHECK(CMTIME_IS_VALID(wrapped) && wrapped.value == -1 && wrapped.timescale == 1000000000);
		wrapped = CMClockMakeHostTimeFromSystemUnits(1ull << 63);
		CHECK(CMTIME_IS_VALID(wrapped) && wrapped.value == INT64_MIN);
		wrapped = CMClockMakeHostTimeFromSystemUnits((uint64_t)INT64_MAX);
		CHECK(CMTIME_IS_VALID(wrapped) && wrapped.value == INT64_MAX);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(INT64_MAX, 1000000000)) == (uint64_t)INT64_MAX);
		// no 64-bit tick count holds INT64_MAX seconds
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(INT64_MAX, 1)) == 0);
	}

	// QUIRK test, negative host time wraps: a negative time is not rejected; its tick count is negated modulo
	// 2^64, also for a negative timescale (macOS 26.7.3 and 10.14.4, 1:1 timebase).
	if (info.numer == 1 && info.denom == 1) {
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(-5, 1000000000)) == (uint64_t)0 - 5);
		CHECK(CMClockConvertHostTimeToSystemUnits(CMTimeMake(-1, 1)) == (uint64_t)0 - 1000000000);
		CMTime negScale = CMTimeMake(5, 1000000000);
		negScale.timescale = -1000000000;
		CHECK(CMClockConvertHostTimeToSystemUnits(negScale) == (uint64_t)0 - 5);
	}

	// a flag that only says the value was rounded does not make the time invalid, and the epoch is ignored
	CMTime rounded = CMTimeMake(5, 1000000000);
	rounded.flags |= kCMTimeFlags_HasBeenRounded;
	rounded.epoch = 7;
	CHECK(CMClockConvertHostTimeToSystemUnits(rounded) == CMClockConvertHostTimeToSystemUnits(CMTimeMake(5, 1000000000)));
	CHECK(CMClockConvertHostTimeToSystemUnits(rounded) != 0);

	// an infinity, even with a valid timescale and a value that would otherwise convert, a zero timescale and
	// an invalid time give 0
	CMTime zeroScale = CMTimeMake(5, 1000000000);
	zeroScale.timescale = 0;
	CHECK(CMClockConvertHostTimeToSystemUnits(zeroScale) == 0);
	CHECK(CMClockConvertHostTimeToSystemUnits(kCMTimeNegativeInfinity) == 0);
	CMTime infinite = CMTimeMake(5, 1000);
	infinite.flags |= kCMTimeFlags_PositiveInfinity;
	CHECK(CMClockConvertHostTimeToSystemUnits(infinite) == 0);
	CMTime indefinite = CMTimeMake(5, 1000);
	indefinite.flags |= kCMTimeFlags_Indefinite;
	CHECK(CMClockConvertHostTimeToSystemUnits(indefinite) == 0);
	CHECK(CMClockConvertHostTimeToSystemUnits(kCMTimeInvalid) == 0);
	CHECK(CMClockConvertHostTimeToSystemUnits(kCMTimePositiveInfinity) == 0);
	CHECK(CMClockConvertHostTimeToSystemUnits(kCMTimeIndefinite) == 0);
	CHECK(!CMTIME_IS_VALID(CMClockGetTime(NULL)));
	CHECK(!CMTIME_IS_VALID(CMClockGetTime((CMClockRef)CFSTR("not a clock"))));

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
