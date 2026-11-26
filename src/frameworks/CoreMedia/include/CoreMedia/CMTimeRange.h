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

#ifndef _CMTIMERANGE_H_
#define _CMTIMERANGE_H_

#include <CoreMedia/CMTime.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

// The half-open interval [start, start + duration).
typedef struct
{
	CMTime start;
	CMTime duration;
}
CMTimeRange;

#pragma pack(pop)

extern const CMTimeRange kCMTimeRangeZero;
extern const CMTimeRange kCMTimeRangeInvalid;

CMTimeRange CMTimeRangeMake(CMTime start, CMTime duration) CM_SWIFT_NAME(CMTimeRangeMake(start:duration:));
CMTimeRange CMTimeRangeFromTimeToTime(CMTime start, CMTime end) CM_SWIFT_NAME(CMTimeRangeFromTimeToTime(start:end:));

CMTime CMTimeRangeGetEnd(CMTimeRange range);
CMTimeRange CMTimeRangeGetUnion(CMTimeRange range, CMTimeRange otherRange) CM_SWIFT_NAME(CMTimeRangeGetUnion(_:otherRange:));
CMTimeRange CMTimeRangeGetIntersection(CMTimeRange range, CMTimeRange otherRange) CM_SWIFT_NAME(CMTimeRangeGetIntersection(_:otherRange:));

Boolean CMTimeRangeEqual(CMTimeRange range1, CMTimeRange range2) CM_SWIFT_NAME(CMTimeRangeEqual(_:_:));
Boolean CMTimeRangeContainsTime(CMTimeRange range, CMTime time) CM_SWIFT_NAME(CMTimeRangeContainsTime(_:time:));
Boolean CMTimeRangeContainsTimeRange(CMTimeRange range, CMTimeRange otherRange) CM_SWIFT_NAME(CMTimeRangeContainsTimeRange(_:otherRange:));

CMTime CMTimeClampToRange(CMTime time, CMTimeRange range) CM_SWIFT_NAME(CMTimeClampToRange(_:range:));

#ifdef __cplusplus
}
#endif

#endif
