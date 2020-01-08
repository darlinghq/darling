#include "OpenTransport.h"

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
