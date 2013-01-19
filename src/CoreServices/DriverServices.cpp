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

