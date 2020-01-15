#ifndef OPENTRANSPORT_H
#define OPENTRANSPORT_H
#include <MacTypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int	OTByteCount;
typedef unsigned int	OTItemCount;
typedef int32_t			OTInt32;
typedef uint32_t		OTUInt32;


#if defined(__i386__) || defined(__ppc__) // This is deprecated stuff for Carbon apps

Boolean OTAtomicSetBit(UInt8* bytePtr, OTByteCount bitNumber);
Boolean OTAtomicClearBit(UInt8* bytePtr, OTByteCount bitNumber);
Boolean OTAtomicTestBit(UInt8* bytePtr, OTByteCount bitNumber);
Boolean OTCompareAndSwapPtr(void* oldValue, void* newValue, void** dest);
Boolean OTCompareAndSwap32(UInt32 oldValue, UInt32 newValue, UInt32* dest);
Boolean OTCompareAndSwap16(UInt32 oldValue, UInt32 newValue, UInt16* dest);
Boolean OTCompareAndSwap8(UInt32 oldValue, UInt32 newValue, UInt8* dest);
SInt32 OTAtomicAdd32(SInt32 toAdd, SInt32* dest);
SInt16 OTAtomicAdd16(SInt32 toAdd, SInt16* dest);
SInt8 OTAtomicAdd8(SInt32 toAdd, SInt8* dest);

#endif

#ifdef __cplusplus
}
#endif

#endif
