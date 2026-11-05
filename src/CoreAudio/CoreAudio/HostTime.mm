#include "CoreAudio/HostTime.h"
#include <mach/mach_time.h>

// Without a 128-bit type on every target: divides first, so that the multiplication can't overflow.
static UInt64 scale(UInt64 value, UInt32 multiplier, UInt32 divisor)
{
	return (value / divisor) * multiplier + (value % divisor) * multiplier / divisor;
}

static const mach_timebase_info_data_t& timebase()
{
	static mach_timebase_info_data_t tb = [] {
		mach_timebase_info_data_t t = {0, 0};
		mach_timebase_info(&t);
		if (!t.numer || !t.denom)
			t.numer = t.denom = 1;
		return t;
	}();
	return tb;
}

UInt64 AudioGetCurrentHostTime(void)
{
	return mach_absolute_time();
}

UInt64 AudioConvertHostTimeToNanos(UInt64 inHostTime)
{
	return scale(inHostTime, timebase().numer, timebase().denom);
}

UInt64 AudioConvertNanosToHostTime(UInt64 inNanos)
{
	return scale(inNanos, timebase().denom, timebase().numer);
}
