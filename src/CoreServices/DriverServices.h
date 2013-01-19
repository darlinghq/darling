#ifndef DRIVERSERVICES_H
#define DRIVERSERVICES_H
#include "MacTypes.h"
#include <climits>

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

