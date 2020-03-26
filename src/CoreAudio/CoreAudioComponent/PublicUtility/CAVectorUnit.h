/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#ifndef __CAVectorUnit_h__
#define __CAVectorUnit_h__

#include <TargetConditionals.h>
#include "CAVectorUnitTypes.h"
#include <stdlib.h>
#include <stdio.h>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreFoundation/CFBase.h>
#else
	#include "CFBase.h"
#endif

// Unify checks for vector units.
// Allow setting an environment variable "CA_NoVector" to turn off vectorized code at runtime (very useful for performance testing).

extern int gCAVectorUnitType;

#ifdef __cplusplus
extern "C" {
#endif

extern SInt32 CAVectorUnit_Examine();	// expensive. use GetType() for lazy initialization and caching.

static inline SInt32 CAVectorUnit_GetType()
{
	int x = gCAVectorUnitType;
	return (x != kVecUninitialized) ? x : CAVectorUnit_Examine();
}

static inline Boolean CAVectorUnit_HasVectorUnit()
{
	return CAVectorUnit_GetType() > kVecNone;
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
class CAVectorUnit {
public:
	static SInt32		GetVectorUnitType() { return CAVectorUnit_GetType(); }
	static bool			HasVectorUnit() { return GetVectorUnitType() > kVecNone; }
	static bool			HasAltivec() { return GetVectorUnitType() == kVecAltivec; }
	static bool			HasSSE2() { return GetVectorUnitType() >= kVecSSE2; }
	static bool			HasSSE3() { return GetVectorUnitType() >= kVecSSE3; }
	static bool			HasAVX1() { return GetVectorUnitType() >= kVecAVX1; }
	static bool			HasNeon() { return GetVectorUnitType() == kVecNeon; }
};
#endif

#endif // __CAVectorUnit_h__
