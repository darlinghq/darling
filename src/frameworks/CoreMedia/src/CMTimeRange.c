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

#include <CoreMedia/CMTimeRange.h>

const CMTimeRange kCMTimeRangeZero = {
	.start = { .value = 0, .timescale = 1, .flags = kCMTimeFlags_Valid, .epoch = 0 },
	.duration = { .value = 0, .timescale = 1, .flags = kCMTimeFlags_Valid, .epoch = 0 },
};

const CMTimeRange kCMTimeRangeInvalid = {
	.start = { .value = 0, .timescale = 0, .flags = 0, .epoch = 0 },
	.duration = { .value = 0, .timescale = 0, .flags = 0, .epoch = 0 },
};

CMTimeRange CMTimeRangeMake(CMTime start, CMTime duration)
{
	CMTimeRange r = { .start = start, .duration = duration };
	return r;
}

CMTime CMTimeRangeGetEnd(CMTimeRange range)
{
	return CMTimeAdd(range.start, range.duration);
}

CMTimeRange CMTimeRangeFromTimeToTime(CMTime start, CMTime end)
{
	// An end before the start, or a time that is not valid, gives the invalid range.
	if (!(start.flags & kCMTimeFlags_Valid) || !(end.flags & kCMTimeFlags_Valid) || CMTimeCompare(end, start) < 0)
		return kCMTimeRangeInvalid;
	return CMTimeRangeMake(start, CMTimeSubtract(end, start));
}

CMTimeRange CMTimeRangeGetUnion(CMTimeRange range, CMTimeRange otherRange)
{
	CMTime start = CMTimeMinimum(range.start, otherRange.start);
	CMTime end = CMTimeMaximum(CMTimeRangeGetEnd(range), CMTimeRangeGetEnd(otherRange));
	return CMTimeRangeMake(start, CMTimeSubtract(end, start));
}

CMTimeRange CMTimeRangeGetIntersection(CMTimeRange range, CMTimeRange otherRange)
{
	CMTime start = CMTimeMaximum(range.start, otherRange.start);
	CMTime end = CMTimeMinimum(CMTimeRangeGetEnd(range), CMTimeRangeGetEnd(otherRange));

	// Ranges that only touch, or do not overlap, intersect in the zero range.
	if (CMTimeCompare(end, start) <= 0)
		return kCMTimeRangeZero;
	return CMTimeRangeMake(start, CMTimeSubtract(end, start));
}

Boolean CMTimeRangeEqual(CMTimeRange range1, CMTimeRange range2)
{
	return CMTimeCompare(range1.start, range2.start) == 0 && CMTimeCompare(range1.duration, range2.duration) == 0;
}

Boolean CMTimeRangeContainsTime(CMTimeRange range, CMTime time)
{
	return CMTimeCompare(time, range.start) >= 0 && CMTimeCompare(time, CMTimeRangeGetEnd(range)) < 0;
}

Boolean CMTimeRangeContainsTimeRange(CMTimeRange range, CMTimeRange otherRange)
{
	// QUIRK: a range is considered to contain another one only if it contains
	// the other range's start time, so the zero range does not contain
	// itself, as an empty range contains no time. The documentation states
	// the containment of ranges without this case. Evidence: black-box
	// observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_contains_time_range_empty
	return CMTimeRangeContainsTime(range, otherRange.start)
		&& CMTimeCompare(CMTimeRangeGetEnd(otherRange), CMTimeRangeGetEnd(range)) <= 0;
}

CMTime CMTimeClampToRange(CMTime time, CMTimeRange range)
{
	if (!(time.flags & kCMTimeFlags_Valid))
		return kCMTimeInvalid;
	CMTime start = range.start;
	if (CMTimeCompare(time, start) < 0)
		return start;
	CMTime end = CMTimeRangeGetEnd(range);
	if (CMTimeCompare(time, end) > 0)
		return end;
	return time;
}
