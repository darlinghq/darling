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


#ifndef MediaAccessibility_MADefinitions_h
#define MediaAccessibility_MADefinitions_h

#include <Availability.h>
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>

#if defined(__cplusplus)
#define MA_EXTERN_C_BEGIN extern "C" {
#define MA_EXTERN_C_END }
#define MA_EXTERN extern "C"
#else
#define MA_EXTERN_C_BEGIN
#define MA_EXTERN_C_END
#define MA_EXTERN extern
#endif

#define MA_VISIBLE __attribute__((visibility("default")))
#define MA_EXPORT MA_EXTERN MA_VISIBLE

#endif
