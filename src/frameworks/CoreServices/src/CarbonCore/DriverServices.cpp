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

#include <CarbonCore/DriverServices.h>
#include "time.h"
#include <sys/sysctl.h>
#include <sys/time.h>

AbsoluteTime UpTime()
{
	struct timeval boottime, now;
	size_t len = sizeof(boottime);
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };
	uint64_t value;

	if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0)
		return { 0, 0};

	gettimeofday(&now, NULL);

	value = (now.tv_sec-boottime.tv_sec) * 1000000000ll
		+ (now.tv_usec-boottime.tv_usec) * 1000ll;
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

Nanoseconds AbsoluteToNanoseconds(AbsoluteTime absTime)
{
	return *reinterpret_cast<Nanoseconds*>(&absTime);
}

Duration AbsoluteToDuration(AbsoluteTime absTime)
{
	return Duration(*reinterpret_cast<int64_t*>(&absTime) / 1000000ll);
}

AbsoluteTime NanosecondsToAbsolute(Nanoseconds ns)
{
	uint64_t value = *reinterpret_cast<uint64_t*>(&ns);
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime DurationToAbsolute(Duration duration)
{
	int64_t value = duration * 1000000ll;
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime AddAbsoluteToAbsolute(AbsoluteTime time1, AbsoluteTime time2)
{
	int64_t value;
	value = *reinterpret_cast<int64_t*>(&time1);
	value += *reinterpret_cast<int64_t*>(&time2);
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime SubAbsoluteFromAbsolute(AbsoluteTime time1, AbsoluteTime time2)
{
	int64_t value;
	value = *reinterpret_cast<int64_t*>(&time1);
	value -= *reinterpret_cast<int64_t*>(&time2);
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime AddNanosecondsToAbsolute(Nanoseconds ns, AbsoluteTime absTime)
{
	int64_t value;
	value = *reinterpret_cast<int64_t*>(&absTime);
	value += *reinterpret_cast<uint64_t*>(&ns);
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime AddDurationToAbsolute(Duration duration, AbsoluteTime absTime)
{
	int64_t value;
	AbsoluteTime at2 = DurationToAbsolute(duration);

	value = *reinterpret_cast<int64_t*>(&absTime);
	value += *reinterpret_cast<int64_t*>(&at2);

	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime SubNanosecondsFromAbsolute(Nanoseconds ns, AbsoluteTime absTime)
{
	int64_t value;
	value = *reinterpret_cast<int64_t*>(&absTime);
	value -= *reinterpret_cast<uint64_t*>(&ns);
	return *reinterpret_cast<AbsoluteTime*>(&value);
}

AbsoluteTime SubDurationFromAbsolute(Duration duration, AbsoluteTime absTime)
{
	int64_t value;
	AbsoluteTime at2 = DurationToAbsolute(duration);

	value = *reinterpret_cast<int64_t*>(&absTime);
	value -= *reinterpret_cast<int64_t*>(&at2);

	return *reinterpret_cast<AbsoluteTime*>(&value);
}

Nanoseconds AbsoluteDeltaToNanoseconds(AbsoluteTime time1, AbsoluteTime time2)
{
	int64_t value = *reinterpret_cast<int64_t*>(&time1)
		            - *reinterpret_cast<int64_t*>(&time2);

	return AbsoluteToNanoseconds(*reinterpret_cast<AbsoluteTime*>(&value));
}

Duration AbsoluteDeltaToDuration(AbsoluteTime time1, AbsoluteTime time2)
{
	return AbsoluteToDuration(SubAbsoluteFromAbsolute(time1, time2));
}

Nanoseconds DurationToNanoseconds(Duration duration)
{
	uint64_t value = duration * 1000000ll;
	return *reinterpret_cast<Nanoseconds*>(&value);
}

Duration NanosecondsToDuration(Nanoseconds ns)
{
	uint64_t value = *reinterpret_cast<uint64_t*>(&ns);
	return Duration(value / 1000000ll);
}

