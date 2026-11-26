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


#ifndef _CORETELEPHONY_DEFINES_H_
#define _CORETELEPHONY_DEFINES_H_

#include <Availability.h>

// The declaration macros of CoreTelephony's headers.
#ifdef __cplusplus
#define CORETELEPHONY_EXTERN extern "C" __attribute__((visibility("default")))
#else
#define CORETELEPHONY_EXTERN extern __attribute__((visibility("default")))
#endif

#define CORETELEPHONY_EXTERN_CLASS __attribute__((visibility("default")))
#define CORETELEPHONY_CLASS_AVAILABLE(version) __attribute__((visibility("default")))
#define CORETELEPHONY_CLASS_DEPRECATED(...) __attribute__((visibility("default")))

#endif
