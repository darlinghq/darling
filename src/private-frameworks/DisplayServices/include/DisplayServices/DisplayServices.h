/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _DISPLAYSERVICES_H_
#define _DISPLAYSERVICES_H_

#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Brightness control is not available on Darling: no display can change its
// brightness through this interface, but queries and requests succeed
// (return 0) so that callers that only read the current value keep working.
int DisplayServicesGetBrightness(uint32_t display, float *brightness);
int DisplayServicesSetBrightness(uint32_t display, float brightness);
int DisplayServicesGetLinearBrightness(uint32_t display, float *brightness);
int DisplayServicesSetLinearBrightness(uint32_t display, float brightness);
int DisplayServicesCanChangeBrightness(uint32_t display);

#ifdef __cplusplus
}
#endif

#endif
