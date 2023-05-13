/*
This file is part of Darling.

Copyright (C) 2012-2016 Lubos Dolezel

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

#include <CarbonCore/DateTimeUtils.h>
#include <climits>
#include <ctime>

UTCDateTime Darling::time_tToUTC(time_t t)
{
	UTCDateTime rv;
	uint64_t time = t + UNIX_OFFSET;

	rv.fraction = 0;
	rv.lowSeconds = time & 0xffffffff;
	rv.highSeconds = time >> 32;

	return rv;
}

OSErr UCConvertUTCDateTimeToCFAbsoluteTime(const UTCDateTime* in, CFAbsoluteTime* out)
{
	uint64_t seconds = in->lowSeconds;
	seconds |= uint64_t(in->highSeconds) << 32;
	*out = seconds - CF_OFFSET;
	*out += (1.0/65535) * in->fraction;
	return noErr;
}

OSErr UCConvertSecondsToCFAbsoluteTime(uint32_t seconds, CFAbsoluteTime* out)
{
	*out = seconds;
	return noErr;
}

OSErr UCConvertLongDateTimeToCFAbsoluteTime(int64_t seconds, CFAbsoluteTime* out)
{
	*out = seconds;
	return noErr;
}

OSErr UCConvertCFAbsoluteTimeToUTCDateTime(CFAbsoluteTime in, UTCDateTime* out)
{
	in += CF_OFFSET;

	if (in > 2.0e48)
		return paramErr;

	uint64_t seconds = uint64_t(in);

	out->lowSeconds = seconds & 0xffffffff;
	out->highSeconds = seconds >> 32;
	out->fraction = uint16_t((in-seconds) * 65535);

	return noErr;
}

OSErr UCConvertCFAbsoluteTimeToSeconds(CFAbsoluteTime in, uint32_t* out)
{
	if (in <= UINT_MAX)
	{
		*out = uint32_t(in);
		return noErr;
	}
	else
	{
		*out = 0;
		return paramErr;
	}
}

OSErr UCConvertCFAbsoluteTimeToLongDateTime(CFAbsoluteTime in, int64_t* out)
{
	if (in <= LLONG_MAX)
	{
		*out = int64_t(in);
		return noErr;
	}
	else
	{
		*out = 0;
		return paramErr;
	}
}

void GetDateTime(unsigned long* secs)
{
	time_t t;
	time(&t);
	
	*secs = t + 2082844800l;
}
