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

#ifndef _CMBASE_H_
#define _CMBASE_H_

#include <CoreFoundation/CoreFoundation.h>
#include <MacTypes.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define CM_EXPORT extern "C"
#else
#define CM_EXPORT extern
#endif

#ifndef CM_NONNULL
#define CM_NONNULL __nonnull
#endif
#define CM_RETURNS_RETAINED_PARAMETER CF_RETURNS_RETAINED

#define CM_SWIFT_NAME(_name) CF_SWIFT_NAME(_name)

typedef signed long CMItemCount;
typedef signed long CMItemIndex;

#endif
