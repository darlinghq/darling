/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

//=============================================================================
//	Includes
//=============================================================================

#include "CAHostTimeBase.h"

Float64			CAHostTimeBase::sFrequency = 0;
Float64			CAHostTimeBase::sInverseFrequency = 0;
UInt32			CAHostTimeBase::sMinDelta = 0;
UInt32			CAHostTimeBase::sToNanosNumerator = 0;
UInt32			CAHostTimeBase::sToNanosDenominator = 0;
pthread_once_t	CAHostTimeBase::sIsInited = PTHREAD_ONCE_INIT;
#if Track_Host_TimeBase
UInt64			CAHostTimeBase::sLastTime = 0;
#endif

//=============================================================================
//	CAHostTimeBase
//
//	This class provides platform independent access to the host's time base.
//=============================================================================

void	CAHostTimeBase::Initialize()
{
	//	get the info about Absolute time
	#if TARGET_OS_MAC
		struct mach_timebase_info	theTimeBaseInfo;
		mach_timebase_info(&theTimeBaseInfo);
		sMinDelta = 1;
		sToNanosNumerator = theTimeBaseInfo.numer;
		sToNanosDenominator = theTimeBaseInfo.denom;

		//	the frequency of that clock is: (sToNanosDenominator / sToNanosNumerator) * 10^9
		sFrequency = static_cast<Float64>(sToNanosDenominator) / static_cast<Float64>(sToNanosNumerator);
		sFrequency *= 1000000000.0;
	#elif TARGET_OS_WIN32
		LARGE_INTEGER theFrequency;
		QueryPerformanceFrequency(&theFrequency);
		sMinDelta = 1;
		sToNanosNumerator = 1000000000ULL;
		sToNanosDenominator = *((UInt64*)&theFrequency);
		sFrequency = static_cast<Float64>(*((UInt64*)&theFrequency));
	#endif
	sInverseFrequency = 1.0 / sFrequency;
	
	#if	Log_Host_Time_Base_Parameters
		DebugPrintf("Host Time Base Parameters");
		DebugPrintf(" Minimum Delta:          %lu", (unsigned long)sMinDelta);
		DebugPrintf(" Frequency:              %f", sFrequency);
		DebugPrintf(" To Nanos Numerator:     %lu", (unsigned long)sToNanosNumerator);
		DebugPrintf(" To Nanos Denominator:   %lu", (unsigned long)sToNanosDenominator);
	#endif
}
