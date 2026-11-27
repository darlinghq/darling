/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef _CMTIMEMAPPING_H_
#define _CMTIMEMAPPING_H_

#include <CoreMedia/CMTimeRange.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

// The map from a range of source time to a range of target time. An empty mapping has an invalid
// source range and only a target range.
typedef struct
{
	CMTimeRange source;
	CMTimeRange target;
}
CMTimeMapping;

#pragma pack(pop)

CMTimeMapping CMTimeMappingMake(CMTimeRange source, CMTimeRange target);
CMTimeMapping CMTimeMappingMakeEmpty(CMTimeRange target);

#ifdef __cplusplus
}
#endif

#endif
