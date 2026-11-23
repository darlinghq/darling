// CFLAGS: -framework CoreVideo -framework CoreFoundation
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

// CVGetCurrentHostTime, CVGetHostClockFrequency and CVGetHostClockMinimumTimeDelta. Core Video's
// documentation says its host time base is the one of mach_absolute_time, so the expected values
// are derived from mach_absolute_time and mach_timebase_info:
// https://developer.apple.com/documentation/corevideo/cvgetcurrenthosttime()
// https://developer.apple.com/documentation/corevideo/cvgethostclockfrequency()
// https://developer.apple.com/documentation/corevideo/cvgethostclockminimumtimedelta()
// Prints PASS when everything holds; the exit code is the number of failed checks.

#include <CoreVideo/CoreVideo.h>
#include <math.h>
#include <mach/mach_time.h>
#include <stdio.h>
#include <time.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	// Interchangeable with mach_absolute_time: a read between two reads of it lies between them.
	uint64_t before = mach_absolute_time();
	uint64_t now = CVGetCurrentHostTime();
	uint64_t after = mach_absolute_time();
	CHECK(before <= now && now <= after);

	// The clock advances.
	struct timespec pause = { 0, 2 * 1000 * 1000 };
	nanosleep(&pause, NULL);
	CHECK(CVGetCurrentHostTime() > now);

	// Frequency is ticks per second of the same base: nanoseconds per tick are numer / denom.
	mach_timebase_info_data_t info;
	CHECK(mach_timebase_info(&info) == KERN_SUCCESS);
	double frequency = CVGetHostClockFrequency();
	CHECK(frequency > 0.0);
	CHECK(fabs(frequency - 1.0e9 * info.denom / info.numer) < 1.0);

	// Elapsed ticks divided by the frequency agree with the elapsed nanoseconds of a monotonic clock.
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	uint64_t h0 = CVGetCurrentHostTime();
	struct timespec wait = { 0, 20 * 1000 * 1000 };
	nanosleep(&wait, NULL);
	uint64_t h1 = CVGetCurrentHostTime();
	clock_gettime(CLOCK_MONOTONIC, &t1);
	double byHost = (double)(h1 - h0) / frequency;
	double byClock = (double)(t1.tv_sec - t0.tv_sec) + (double)(t1.tv_nsec - t0.tv_nsec) / 1.0e9;
	CHECK(fabs(byHost - byClock) < 0.005);

	// The smallest increment is a positive number of ticks. Which value is a GUESS (see CVHostTime.c); the
	// check holds for any valid reading, and the host time never moves by less than it.
	uint32_t minimum = CVGetHostClockMinimumTimeDelta();
	CHECK(minimum >= 1);
	uint64_t a = CVGetCurrentHostTime(), b = CVGetCurrentHostTime();
	CHECK(b == a || b - a >= minimum);

	if (failures == 0)
		printf("PASS\n");
	return failures;
}
