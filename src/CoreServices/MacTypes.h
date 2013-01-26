#ifndef MACTYPES_H
#define MACTYPES_H
#include <stdint.h>

class _StringHandle;
typedef int8_t SInt8;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef int64_t SInt64;
typedef uint64_t UInt64;

typedef uint32_t OptionBits;

typedef int32_t			Fixed; // 16/16
typedef Fixed*			FixedPtr;
typedef int32_t			Fract; // 2/30
typedef Fract*			FractPtr;
typedef uint32_t		UnsignedFixed; // 16u/16
typedef UnsignedFixed*	UnsignedFixedPtr;
typedef short			ShortFixed;
typedef ShortFixed *	ShortFixedPtr; // 8/8
typedef int64_t			wide;
typedef uint64_t		UnsignedWide;
typedef uint64_t		AbsoluteTime;
typedef int32_t			Duration; // milliseconds
typedef uint8_t			Boolean;

// Pascal strings
typedef unsigned char	Str255[256];
typedef unsigned char	Str63[64];
typedef unsigned char	Str32[33];
typedef unsigned char	Str31[32];
typedef unsigned char	Str27[28];
typedef unsigned char	Str15[16];

typedef unsigned char*  StringPtr;
typedef unsigned char** StringHandle;
typedef char* Ptr;
typedef char** Handle;

typedef int32_t OSStatus;

#endif

