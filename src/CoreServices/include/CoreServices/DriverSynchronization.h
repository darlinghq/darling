/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#ifndef DRIVERSYNCHRONIZATION_H
#define DRIVERSYNCHRONIZATION_H

#include "MacTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

Boolean CompareAndSwap(UInt32 old, UInt32 _new, UInt32* ptr);

Boolean TestAndClear(UInt32 bit, UInt8* ptr);

Boolean TestAndSet(UInt32 bit, UInt8* ptr) ;

SInt8 IncrementAtomic8(SInt8* ptr);

SInt8 DecrementAtomic8(SInt8* ptr);

SInt8 AddAtomic8(SInt32 val, SInt8* ptr);

UInt8 BitAndAtomic8(UInt32 val, UInt8* ptr);

UInt8 BitOrAtomic8(UInt32 val, UInt8* ptr);

UInt8 BitXorAtomic8(UInt32 val, UInt8* ptr);

SInt16 IncrementAtomic16(SInt16* ptr);

SInt16 DecrementAtomic16(SInt16* ptr);

SInt16 AddAtomic16(SInt32 val, SInt16* ptr);

UInt16 BitAndAtomic16(UInt32 val, UInt16* ptr);

UInt16 BitOrAtomic16(UInt32 val, UInt16* ptr);

UInt16 BitXorAtomic16(UInt32 val, UInt16* ptr);

SInt32 IncrementAtomic(SInt32* ptr);

SInt32 DecrementAtomic(SInt32* ptr);

SInt32 AddAtomic(SInt32 val, SInt32* ptr);

UInt32 BitAndAtomic(UInt32 val, UInt32* ptr);

UInt32 BitOrAtomic(UInt32 val, UInt32* ptr);

UInt32 BitXorAtomic(UInt32 val, UInt32* ptr);

#ifdef __cplusplus
}
#endif

#endif 

