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


#ifndef _CMSYNC_H_
#define _CMSYNC_H_

#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTime.h>

// Clocks and timebases: the types. Their functions are still the generated declarations in
// CoreMedia.h.
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMClock *CMClockRef;
typedef struct CF_BRIDGED_TYPE(id) OpaqueCMTimebase *CMTimebaseRef;
typedef CFTypeRef CMClockOrTimebaseRef;

#endif
