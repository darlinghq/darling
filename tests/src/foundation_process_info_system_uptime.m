/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// -[NSProcessInfo systemUptime] is the time since the system started: the
// same clock as mach_absolute_time() (converted to seconds), much smaller than
// the time since 1970, and increasing.  Build without ARC or with; no windowing needed.

#import <Foundation/Foundation.h>
#include <mach/mach_time.h>
#include <stdio.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static double machSeconds(void)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	return (double) mach_absolute_time() * tb.numer / tb.denom / 1e9;
}

int main(void)
{
	@autoreleasepool {
		NSProcessInfo *info = [NSProcessInfo processInfo];

		double before = machSeconds();
		NSTimeInterval uptime = [info systemUptime];
		double after = machSeconds();

		CHECK(uptime > 0, "systemUptime %f is not positive", uptime);
		CHECK(uptime >= before - 1.0 && uptime <= after + 1.0,
		      "systemUptime %f is not within 1 s of mach_absolute_time seconds [%f, %f]", uptime, before, after);
		CHECK(uptime < [[NSDate date] timeIntervalSince1970],
		      "systemUptime %f is not less than the time since 1970 (%f)", uptime, [[NSDate date] timeIntervalSince1970]);

		usleep(200000);
		NSTimeInterval later = [info systemUptime];
		CHECK(later > uptime, "systemUptime did not increase: %f then %f", uptime, later);
		CHECK(later - uptime < 5.0, "systemUptime advanced %f s during a 0.2 s sleep", later - uptime);
		CHECK(later - uptime >= 0.15, "systemUptime advanced only %f s during a 0.2 s sleep", later - uptime);
	}
	if (failures) {
		printf("FAILED: %d checks\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
