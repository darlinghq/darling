/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(2)
typedef struct UTCDateTime
{
	uint16_t highSeconds;
	uint32_t lowSeconds;
	uint16_t fraction;
} UTCDateTime;
#pragma pack()

typedef UTCDateTime* UTCDateTimePtr;
typedef UTCDateTime** UTCDateTimeHandle;
typedef UTCDateTime LocalDateTime;
typedef LocalDateTime* LocalDateTimePtr;
typedef LocalDateTime** LocalDateTimeHandle;

#ifdef __cplusplus
}
#endif

#endif
