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

#include <CarbonCore/DriverSynchronization.h>


Boolean CompareAndSwap(UInt32 old, UInt32 _new, UInt32* ptr)
{
	return __sync_bool_compare_and_swap(ptr, old, _new);
}

Boolean TestAndClear(UInt32 bit, UInt8* ptr)
{
	unsigned long mask = (1 << bit);
	return !!(__sync_fetch_and_and((unsigned long*) ptr, ~mask) & mask);
}

Boolean TestAndSet(UInt32 bit, UInt8* ptr)
{
	unsigned long mask = (1 << bit);
	return !!(__sync_fetch_and_or((unsigned long*) ptr, mask) & mask);
}

SInt8 IncrementAtomic8(SInt8* ptr)
{
	return __sync_add_and_fetch(ptr, 1);
}

SInt8 DecrementAtomic8(SInt8* ptr)
{
	return __sync_sub_and_fetch(ptr, 1);
}

SInt8 AddAtomic8(SInt32 val, SInt8* ptr)
{
	return __sync_add_and_fetch(ptr, val);
}

UInt8 BitAndAtomic8(UInt32 val, UInt8* ptr)
{
	return __sync_and_and_fetch(ptr, val);
}

UInt8 BitOrAtomic8(UInt32 val, UInt8* ptr)
{
	return __sync_or_and_fetch(ptr, val);
}

UInt8 BitXorAtomic8(UInt32 val, UInt8* ptr)
{
	return __sync_xor_and_fetch(ptr, val);
}

SInt16 IncrementAtomic16(SInt16* ptr)
{
	return __sync_add_and_fetch(ptr, 1);
}

SInt16 DecrementAtomic16(SInt16* ptr)
{
	return __sync_sub_and_fetch(ptr, 1);
}

SInt16 AddAtomic16(SInt32 val, SInt16* ptr)
{
	return __sync_add_and_fetch(ptr, val);
}

UInt16 BitAndAtomic16(UInt32 val, UInt16* ptr)
{
	return __sync_and_and_fetch(ptr, val);
}

UInt16 BitOrAtomic16(UInt32 val, UInt16* ptr)
{
	return __sync_or_and_fetch(ptr, val);
}

UInt16 BitXorAtomic16(UInt32 val, UInt16* ptr)
{
	return __sync_xor_and_fetch(ptr, val);
}

SInt32 IncrementAtomic(SInt32* ptr)
{
	return __sync_add_and_fetch(ptr, 1);
}

SInt32 DecrementAtomic(SInt32* ptr)
{
	return __sync_sub_and_fetch(ptr, 1);
}

SInt32 AddAtomic(SInt32 val, SInt32* ptr)
{
	return __sync_add_and_fetch(ptr, val);
}

UInt32 BitAndAtomic(UInt32 val, UInt32* ptr)
{
	return __sync_and_and_fetch(ptr, val);
}

UInt32 BitOrAtomic(UInt32 val, UInt32* ptr)
{
	return __sync_or_and_fetch(ptr, val);
}

UInt32 BitXorAtomic(UInt32 val, UInt32* ptr)
{
	return __sync_xor_and_fetch(ptr, val);
}

