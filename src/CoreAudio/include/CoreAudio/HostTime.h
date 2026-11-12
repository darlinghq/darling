#ifndef COREAUDIOHOSTTIME_H
#define COREAUDIOHOSTTIME_H

#include "MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

UInt64 AudioGetCurrentHostTime(void);
UInt64 AudioConvertHostTimeToNanos(UInt64 inHostTime);
UInt64 AudioConvertNanosToHostTime(UInt64 inNanos);

// Number of host time ticks per second.
Float64 AudioGetHostClockFrequency(void);
// Smallest difference between two host time values the clock can report, in host ticks.
UInt32 AudioGetHostClockMinimumTimeDelta(void);

#ifdef __cplusplus
}
#endif

#endif // COREAUDIOHOSTTIME_H
