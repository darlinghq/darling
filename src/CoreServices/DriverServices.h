/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#ifndef DRIVERSERVICES_H
#define DRIVERSERVICES_H
#include <MacTypes.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UnsignedWide Nanoseconds;

enum
{
	durationMicrosecond = -1L, 
	durationMillisecond = 1, 
	durationSecond = 1000, 
	durationMinute = 60*1000, 
	durationHour = 60*60*1000,
	durationDay = 60*60*1000*24, 
	durationNoWait = 0, 
	durationForever = INT_MAX
};

AbsoluteTime UpTime();

Nanoseconds AbsoluteToNanoseconds(AbsoluteTime absTime);

Duration AbsoluteToDuration(AbsoluteTime absTime);

AbsoluteTime NanosecondsToAbsolute(Nanoseconds ns);

AbsoluteTime DurationToAbsolute(Duration duration);

AbsoluteTime AddAbsoluteToAbsolute(AbsoluteTime time1, AbsoluteTime time2);

AbsoluteTime SubAbsoluteFromAbsolute(AbsoluteTime time1, AbsoluteTime time2);

AbsoluteTime AddNanosecondsToAbsolute(Nanoseconds ns, AbsoluteTime absTime);

AbsoluteTime AddDurationToAbsolute(Duration duration, AbsoluteTime absTime);

AbsoluteTime SubNanosecondsFromAbsolute(Nanoseconds ns, AbsoluteTime absTime);

AbsoluteTime SubDurationFromAbsolute(Duration duration, AbsoluteTime absTime);

Nanoseconds AbsoluteDeltaToNanoseconds(AbsoluteTime time1, AbsoluteTime time2);

Duration AbsoluteDeltaToDuration(AbsoluteTime time1, AbsoluteTime time2);

Nanoseconds DurationToNanoseconds(Duration duration);

Duration NanosecondsToDuration(Nanoseconds ns);

#ifdef __cplusplus
}
#endif

#endif 

