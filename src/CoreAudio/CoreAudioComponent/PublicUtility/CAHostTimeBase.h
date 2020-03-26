/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#if !defined(__CAHostTimeBase_h__)
#define __CAHostTimeBase_h__

//=============================================================================
//	Includes
//=============================================================================

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
#else
	#include <CoreAudioTypes.h>
#endif

#if TARGET_OS_MAC
	#include <mach/mach_time.h>
	#include <pthread.h>
#elif TARGET_OS_WIN32
	#include <windows.h>
	#include "WinPThreadDefs.h"
#else
	#error	Unsupported operating system
#endif

#include "CADebugPrintf.h"

//=============================================================================
//	CAHostTimeBase
//
//	This class provides platform independent access to the host's time base.
//=============================================================================

#if CoreAudio_Debug
//	#define Log_Host_Time_Base_Parameters	1
//	#define Track_Host_TimeBase				1
#endif

class	CAHostTimeBase
{

public:
	static UInt64			ConvertToNanos(UInt64 inHostTime);
	static UInt64			ConvertFromNanos(UInt64 inNanos);

	static UInt64			GetTheCurrentTime();
#if TARGET_OS_MAC
	static UInt64			GetCurrentTime() { return GetTheCurrentTime(); }
#endif
	static UInt64			GetCurrentTimeInNanos();

	static Float64			GetFrequency() { pthread_once(&sIsInited, Initialize); return sFrequency; }
	static Float64			GetInverseFrequency() { pthread_once(&sIsInited, Initialize); return sInverseFrequency; }
	static UInt32			GetMinimumDelta() { pthread_once(&sIsInited, Initialize); return sMinDelta; }

	static UInt64			AbsoluteHostDeltaToNanos(UInt64 inStartTime, UInt64 inEndTime);
	static SInt64			HostDeltaToNanos(UInt64 inStartTime, UInt64 inEndTime);

	static UInt64			MultiplyByRatio(UInt64 inMuliplicand, UInt32 inNumerator, UInt32 inDenominator);
	
private:
	static void				Initialize();
	
	static pthread_once_t	sIsInited;
	
	static Float64			sFrequency;
	static Float64			sInverseFrequency;
	static UInt32			sMinDelta;
	static UInt32			sToNanosNumerator;
	static UInt32			sToNanosDenominator;
#if Track_Host_TimeBase
	static UInt64			sLastTime;
#endif
};

inline UInt64	CAHostTimeBase::GetTheCurrentTime()
{
	UInt64 theTime = 0;

	#if TARGET_OS_MAC
		theTime = mach_absolute_time();
	#elif TARGET_OS_WIN32
		LARGE_INTEGER theValue;
		QueryPerformanceCounter(&theValue);
		theTime = *((UInt64*)&theValue);
	#endif
	
	#if	Track_Host_TimeBase
		if(sLastTime != 0)
		{
			if(theTime <= sLastTime)
			{
				DebugPrintf("CAHostTimeBase::GetTheCurrentTime: the current time is earlier than the last time, now: %qd, then: %qd", theTime, sLastTime);
			}
			sLastTime = theTime;
		}
		else
		{
			sLastTime = theTime;
		}
	#endif

	return theTime;
}

inline UInt64	CAHostTimeBase::ConvertToNanos(UInt64 inHostTime)
{
	pthread_once(&sIsInited, Initialize);
	
	UInt64 theAnswer = MultiplyByRatio(inHostTime, sToNanosNumerator, sToNanosDenominator);
	#if CoreAudio_Debug
		if(((sToNanosNumerator > sToNanosDenominator) && (theAnswer < inHostTime)) || ((sToNanosDenominator > sToNanosNumerator) && (theAnswer > inHostTime)))
		{
			DebugPrintf("CAHostTimeBase::ConvertToNanos: The conversion wrapped");
		}
	#endif
	
	return theAnswer;
}

inline UInt64	CAHostTimeBase::ConvertFromNanos(UInt64 inNanos)
{
	pthread_once(&sIsInited, Initialize);

	UInt64 theAnswer = MultiplyByRatio(inNanos, sToNanosDenominator, sToNanosNumerator);
	#if CoreAudio_Debug
		if(((sToNanosDenominator > sToNanosNumerator) && (theAnswer < inNanos)) || ((sToNanosNumerator > sToNanosDenominator) && (theAnswer > inNanos)))
		{
			DebugPrintf("CAHostTimeBase::ConvertFromNanos: The conversion wrapped");
		}
	#endif

	return theAnswer;
}

inline UInt64	CAHostTimeBase::GetCurrentTimeInNanos()
{
	return ConvertToNanos(GetTheCurrentTime());
}

inline UInt64	CAHostTimeBase::AbsoluteHostDeltaToNanos(UInt64 inStartTime, UInt64 inEndTime)
{
	UInt64 theAnswer;
	
	if(inStartTime <= inEndTime)
	{
		theAnswer = inEndTime - inStartTime;
	}
	else
	{
		theAnswer = inStartTime - inEndTime;
	}
	
	return ConvertToNanos(theAnswer);
}

inline SInt64	CAHostTimeBase::HostDeltaToNanos(UInt64 inStartTime, UInt64 inEndTime)
{
	SInt64 theAnswer;
	SInt64 theSign = 1;
	
	if(inStartTime <= inEndTime)
	{
		theAnswer = static_cast<SInt64>(inEndTime - inStartTime);
	}
	else
	{
		theAnswer = static_cast<SInt64>(inStartTime - inEndTime);
		theSign = -1;
	}
	
	return theSign * static_cast<SInt64>(ConvertToNanos(static_cast<UInt64>(theAnswer)));
}

inline UInt64	CAHostTimeBase::MultiplyByRatio(UInt64 inMuliplicand, UInt32 inNumerator, UInt32 inDenominator)
{
#if TARGET_OS_MAC && TARGET_RT_64_BIT
	__uint128_t theAnswer = inMuliplicand;
#else
	long double theAnswer = inMuliplicand;
#endif
	if(inNumerator != inDenominator)
	{
		theAnswer *= inNumerator;
		theAnswer /= inDenominator;
	}
	return static_cast<UInt64>(theAnswer);
}

#endif
