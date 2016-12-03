#ifndef MACTYPES_H
#define MACTYPES_H
#include <stdint.h>
#include <CoreFoundation/CFBase.h>

#ifdef __cplusplus
class _StringHandle;
#endif

#if 0 // declared in CFBase.h
typedef int8_t SInt8;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef int64_t SInt64;
typedef uint64_t UInt64;
typedef float Float32;
typedef double Float64;
#endif

typedef Fixed*			FixedPtr;
typedef Fract*			FractPtr;
typedef UnsignedFixed*	UnsignedFixedPtr;
typedef short			ShortFixed;
typedef ShortFixed *	ShortFixedPtr; // 8/8
//typedef int64_t			wide;
//typedef uint64_t		UnsignedWide;
//typedef uint64_t		AbsoluteTime;
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

typedef int16_t OSErr;

enum {
	kDurationImmediate = 0,
	kDurationForever = 0x7FFFFFFF,
	kDurationMillisecond = 1,
	kDurationMicrosecond = -1
};

#endif

