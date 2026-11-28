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

/* Netscape Portable Runtime (NSPR) scalar types. */
#ifndef _PRTYPES_H_
#define _PRTYPES_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t PRInt8;
typedef uint8_t PRUint8;
typedef int16_t PRInt16;
typedef uint16_t PRUint16;
typedef int32_t PRInt32;
typedef uint32_t PRUint32;
typedef int64_t PRInt64;
typedef uint64_t PRUint64;
typedef int PRIntn;
typedef unsigned int PRUintn;
typedef uintptr_t PRWord;
typedef ptrdiff_t PRPtrdiff;
typedef int PRStatus;
#define PR_SUCCESS 0
#define PR_FAILURE -1

#ifdef __cplusplus
}
#endif

#endif
