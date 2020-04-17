/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#include <CoreServices/OpenTransport.h>

#if defined(__i386__) || defined(__ppc__) // This is deprecated stuff for Carbon apps

Boolean OTAtomicSetBit(UInt8* bytePtr, OTByteCount bitNumber)
{
	UInt8 bitValue = 1 << bitNumber;
	
	while (true)
	{
		UInt8 value = *bytePtr;
		if (value & bitValue)
			return false;
		
		if (__sync_bool_compare_and_swap(bytePtr, value, value | bitValue))
			return true;
	}
}

Boolean OTAtomicClearBit(UInt8* bytePtr, OTByteCount bitNumber)
{
	UInt8 bitValue = 1 << bitNumber;
	
	while (true)
	{
		UInt8 value = *bytePtr;
		if ((value & bitValue) == 0)
			return false;
		
		if (__sync_bool_compare_and_swap(bytePtr, value, value & ~bitValue))
			return true;
	}
}

Boolean OTAtomicTestBit(UInt8* bytePtr, OTByteCount bitNumber)
{
	return *bytePtr & (1 << bitNumber);
}

Boolean OTCompareAndSwapPtr(void* oldValue, void* newValue, void** dest)
{
	return __sync_bool_compare_and_swap(dest, oldValue, newValue);
}

Boolean OTCompareAndSwap32(UInt32 oldValue, UInt32 newValue, UInt32* dest)
{
	return __sync_bool_compare_and_swap(dest, oldValue, newValue);
}

Boolean OTCompareAndSwap16(UInt32 oldValue, UInt32 newValue, UInt16* dest)
{
	return __sync_bool_compare_and_swap(dest, oldValue, newValue);
}

Boolean OTCompareAndSwap8(UInt32 oldValue, UInt32 newValue, UInt8* dest)
{
	return __sync_bool_compare_and_swap(dest, oldValue, newValue);
}

SInt32 OTAtomicAdd32(SInt32 toAdd, SInt32* dest)
{
	return __sync_add_and_fetch(dest, toAdd);
}

SInt16 OTAtomicAdd16(SInt32 toAdd, SInt16* dest)
{
	return __sync_add_and_fetch(dest, toAdd);
}

SInt8 OTAtomicAdd8(SInt32 toAdd, SInt8* dest)
{
	return __sync_add_and_fetch(dest, toAdd);
}

#endif
