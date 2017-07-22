/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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


#ifndef UTCUTILS_H
#define UTCUTILS_H
#include <stdint.h>
#include <time.h>
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

typedef struct UTCDateTime LocalDateTime;
typedef struct UTCDateTime* UTCDateTimePtr;
typedef struct UTCDateTime** UTCDateTimeHandle;
typedef LocalDateTime* LocalDateTimePtr;
typedef LocalDateTime** LocalDateTimeHandle;

#ifdef __cplusplus
extern "C" {
#endif

OSErr UCConvertUTCDateTimeToCFAbsoluteTime(const UTCDateTimePtr in, CFAbsoluteTime* out);
OSErr UCConvertSecondsToCFAbsoluteTime(uint32_t seconds, CFAbsoluteTime* out);
OSErr UCConvertLongDateTimeToCFAbsoluteTime(int64_t seconds, CFAbsoluteTime* out);
OSErr UCConvertCFAbsoluteTimeToUTCDateTime(CFAbsoluteTime in, UTCDateTimePtr out);
OSErr UCConvertCFAbsoluteTimeToSeconds(CFAbsoluteTime in, uint32_t* out);
OSErr UCConvertCFAbsoluteTimeToLongDateTime(CFAbsoluteTime in, int64_t* out);

// Seconds since 1.1.1904
void GetDateTime(unsigned long* secs);

#ifdef __cplusplus
}

namespace Darling
{
	UTCDateTime time_tToUTC(time_t t);
}

#endif

#endif

