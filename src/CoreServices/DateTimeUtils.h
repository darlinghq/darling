#ifndef UTCUTILS_H
#define UTCUTILS_H
#include <stdint.h>
#include <ctime>
#include <CoreFoundation/CFDate.h>
#include "MacErrors.h"

#define UNIX_OFFSET 2082844800ll // 1970 - 1904
#define CF_OFFSET 3061152000ll // 2001 - 1904: "Seconds" is since 2001+

#pragma pack(2)
struct UTCDateTime
{
	uint16_t highSeconds;
	uint32_t lowSeconds;
	uint16_t fraction;
};
#pragma pack()

typedef UTCDateTime LocalDateTime;
typedef UTCDateTime* UTCDateTimePtr;
typedef UTCDateTime** UTCDateTimeHandle;
typedef LocalDateTime* LocalDateTimePtr;
typedef LocalDateTime** LocalDateTimeHandle;

extern "C" {

OSErr UCConvertUTCDateTimeToCFAbsoluteTime(const UTCDateTime* in, CFAbsoluteTime* out);
OSErr UCConvertSecondsToCFAbsoluteTime(uint32_t seconds, CFAbsoluteTime* out);
OSErr UCConvertLongDateTimeToCFAbsoluteTime(int64_t seconds, CFAbsoluteTime* out);
OSErr UCConvertCFAbsoluteTimeToUTCDateTime(CFAbsoluteTime in, UTCDateTime* out);
OSErr UCConvertCFAbsoluteTimeToSeconds(CFAbsoluteTime in, uint32_t* out);
OSErr UCConvertCFAbsoluteTimeToLongDateTime(CFAbsoluteTime in, int64_t* out);

}

namespace Darling
{
	UTCDateTime time_tToUTC(time_t t);
}

#endif

