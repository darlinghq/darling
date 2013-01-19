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

