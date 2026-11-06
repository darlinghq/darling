#ifndef COREAUDIOHOSTTIME_H
#define COREAUDIOHOSTTIME_H

#include "MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

UInt64 AudioGetCurrentHostTime(void);
UInt64 AudioConvertHostTimeToNanos(UInt64 inHostTime);
UInt64 AudioConvertNanosToHostTime(UInt64 inNanos);

#ifdef __cplusplus
}
#endif

#endif // COREAUDIOHOSTTIME_H
