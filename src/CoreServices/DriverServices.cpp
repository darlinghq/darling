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

#include "DriverServices.h"
#include "time.h"
#include <sys/sysinfo.h>

AbsoluteTime UpTime()
{
	struct sysinfo si;
	sysinfo(&si);
	return si.uptime * 1000000000ll;
}

Nanoseconds AbsoluteToNanoseconds(AbsoluteTime absTime)
{
	return absTime;
}

Duration AbsoluteToDuration(AbsoluteTime absTime)
{
	return Duration(absTime / 1000000ll);
}

AbsoluteTime NanosecondsToAbsolute(Nanoseconds ns)
{
	return ns;
}

AbsoluteTime DurationToAbsolute(Duration duration)
{
	return duration * 1000000ll;
}

AbsoluteTime AddAbsoluteToAbsolute(AbsoluteTime time1, AbsoluteTime time2)
{
	return time1+time2;
}

AbsoluteTime SubAbsoluteFromAbsolute(AbsoluteTime time1, AbsoluteTime time2)
{
	return time1-time2;
}

AbsoluteTime AddNanosecondsToAbsolute(Nanoseconds ns, AbsoluteTime absTime)
{
	return absTime + NanosecondsToAbsolute(ns);
}

AbsoluteTime AddDurationToAbsolute(Duration duration, AbsoluteTime absTime)
{
	return absTime + DurationToAbsolute(duration);
}

AbsoluteTime SubNanosecondsFromAbsolute(Nanoseconds ns, AbsoluteTime absTime)
{
	return absTime - NanosecondsToAbsolute(ns);
}

AbsoluteTime SubDurationFromAbsolute(Duration duration, AbsoluteTime absTime)
{
	return absTime - DurationToAbsolute(duration);
}

Nanoseconds AbsoluteDeltaToNanoseconds(AbsoluteTime time1, AbsoluteTime time2)
{
	return AbsoluteToNanoseconds(time1 - time2);
}

Duration AbsoluteDeltaToDuration(AbsoluteTime time1, AbsoluteTime time2)
{
	return AbsoluteToDuration(time1 - time2);
}

Nanoseconds DurationToNanoseconds(Duration duration)
{
	return duration * 1000000ll;
}

Duration NanosecondsToDuration(Nanoseconds ns)
{
	return Duration(ns / 1000000ll);
}

