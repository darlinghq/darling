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

// CVGetCurrentHostTime, CVGetHostClockFrequency and CVGetHostClockMinimumTimeDelta. The host time
// base of Core Video is the one of mach_absolute_time (and of CoreAudio's host time), so the
// functions only expose that clock.

#include <CoreVideo/CVHostTime.h>
#include <mach/mach_time.h>

uint64_t CVGetCurrentHostTime(void)
{
	return mach_absolute_time();
}

// Ticks per second: mach_timebase_info gives the nanoseconds of numer / denom ticks.
double CVGetHostClockFrequency(void)
{
	mach_timebase_info_data_t info;
	// GUESS: the documentation does not cover a failing timebase query (mach_timebase_info does not fail in
	// practice); the frequency is then 0.
	if (mach_timebase_info(&info) != KERN_SUCCESS || info.numer == 0)
		return 0.0;
	return 1.0e9 * (double)info.denom / (double)info.numer;
}

uint32_t CVGetHostClockMinimumTimeDelta(void)
{
	// GUESS: the documentation only says "smallest valid increment in the system time". The host time is
	// an integer count of ticks, so one tick is the smallest step it can take.
	return 1;
}
