/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HISERVICES_CPS_H
#define HISERVICES_CPS_H
#include <stdint.h>

// Private process-serial-number calls used by programs that are not Cocoa
// applications to become a foreground application and take focus. They return
// 0 on success and a nonzero error otherwise.

#ifdef __cplusplus
extern "C" {
#endif

// Same layout as ProcessSerialNumber.
typedef struct CPSProcessSerNum
{
	uint32_t hi;
	uint32_t lo;
} CPSProcessSerNum;

int CPSGetCurrentProcess(CPSProcessSerNum* psn);
int CPSEnableForegroundOperation(CPSProcessSerNum* psn, uint32_t flags1, uint32_t flags2, uint32_t flags3, uint32_t flags4);
int CPSSetFrontProcess(CPSProcessSerNum* psn);

#ifdef __cplusplus
}
#endif

#endif
