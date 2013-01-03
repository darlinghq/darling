#ifndef MACTYPES_H
#define MACTYPES_H
#include <stdint.h>

typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;

typedef uint32_t OptionBits;

typedef int32_t                         Fixed; // 16/16
typedef Fixed *                         FixedPtr;
typedef int32_t                         Fract; // 2/30
typedef Fract *                         FractPtr;
typedef uint32_t                        UnsignedFixed; // 16u/16
typedef UnsignedFixed *                 UnsignedFixedPtr;
typedef short                           ShortFixed;
typedef ShortFixed *                    ShortFixedPtr; // 8/8
typedef int64_t							wide;
typedef uint64_t						UnsignedWide;


#endif

