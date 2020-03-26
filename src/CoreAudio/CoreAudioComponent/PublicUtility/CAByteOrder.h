/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#if !defined(__CAByteOrder_h__)
#define __CAByteOrder_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include "CoreFoundation.h"
#endif


#if defined(__cplusplus)
extern "C" {
#endif

CF_INLINE Float32 CASwapFloat32 (Float32 arg) {
	union {
		Float32 f;
		UInt32 i;
	} flip;

	flip.f = arg;
	flip.i = CFSwapInt32 (flip.i);
	
	return flip.f;
}

CF_INLINE Float64 CASwapFloat64 (Float64 arg) {
	union {
		Float64 f;
		UInt64 i;
	} flip;

	flip.f = arg;
	flip.i = CFSwapInt64 (flip.i);
	
	return flip.f;
}

#pragma mark -Flippers

CF_INLINE Float32 CASwapFloat32BigToHost(Float32 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64BigToHost(Float64 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32HostToBig(Float32 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64HostToBig(Float64 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32LittleToHost(Float32 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64LittleToHost(Float64 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32HostToLittle(Float32 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64HostToLittle(Float64 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}


#if defined(__cplusplus)
}
#endif

#endif
