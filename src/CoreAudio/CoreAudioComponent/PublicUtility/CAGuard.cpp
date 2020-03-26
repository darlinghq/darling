/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

//==================================================================================================
//	Includes
//==================================================================================================

//	Self Include
#include "CAGuard.h"

#if TARGET_OS_MAC
	#include <errno.h>
#endif

//	PublicUtility Inludes
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAHostTimeBase.h"

//==================================================================================================
//	Logging
//==================================================================================================

#if CoreAudio_Debug
//	#define	Log_Ownership		1
//	#define	Log_WaitOwnership	1
//	#define	Log_TimedWaits		1
//	#define Log_Latency			1
//	#define	Log_Errors			1
#endif

//#warning		Need a try-based Locker too
//==================================================================================================
//	CAGuard
//==================================================================================================

CAGuard::CAGuard(const char* inName)
:
	CAMutex(inName)
#if	Log_Average_Latency
	,mAverageLatencyAccumulator(0.0),
	mAverageLatencyCount(0)
#endif
{
#if TARGET_OS_MAC
	OSStatus theError = pthread_cond_init(&mCondVar, NULL);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::CAGuard: Could not init the cond var");
#elif TARGET_OS_WIN32
	mEvent = CreateEvent(NULL, true, false, NULL);
	ThrowIfNULL(mEvent, CAException(GetLastError()), "CAGuard::CAGuard: Could not create the event");
#endif
}

CAGuard::~CAGuard()
{
#if TARGET_OS_MAC
	pthread_cond_destroy(&mCondVar);
#elif TARGET_OS_WIN32
	if(mEvent != NULL)
	{
		CloseHandle(mEvent);
	}
#endif
}

void	CAGuard::Wait()
{
#if TARGET_OS_MAC
	ThrowIf(!pthread_equal(pthread_self(), mOwner), CAException(1), "CAGuard::Wait: A thread has to have locked a guard before it can wait");

	mOwner = 0;

	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::Wait: thread %p is waiting on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_wait(&mCondVar, &mMutex);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::Wait: Could not wait for a signal");
	mOwner = pthread_self();

	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::Wait: thread %p waited on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif
#elif TARGET_OS_WIN32
	ThrowIf(GetCurrentThreadId() != mOwner, CAException(1), "CAGuard::Wait: A thread has to have locked a guard before it can wait");

	mOwner = 0;

	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::Wait: thread %lu is waiting on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif

	ReleaseMutex(mMutex);
	HANDLE theHandles[] = { mMutex, mEvent };
	OSStatus theError = WaitForMultipleObjects(2, theHandles, true, INFINITE);
	ThrowIfError(theError, CAException(GetLastError()), "CAGuard::Wait: Could not wait for the signal");
	mOwner = GetCurrentThreadId();
	ResetEvent(mEvent);

	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::Wait: thread %lu waited on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
#endif
}

bool	CAGuard::WaitFor(UInt64 inNanos)
{
	bool theAnswer = false;

#if TARGET_OS_MAC
	ThrowIf(!pthread_equal(pthread_self(), mOwner), CAException(1), "CAGuard::WaitFor: A thread has to have locked a guard be for it can wait");

	#if	Log_TimedWaits
		DebugMessageN1("CAGuard::WaitFor: waiting %.0f", (Float64)inNanos);
	#endif

	struct timespec	theTimeSpec;
	static const UInt64	kNanosPerSecond = 1000000000ULL;
	if(inNanos >= kNanosPerSecond)
	{
		theTimeSpec.tv_sec = static_cast<long>(inNanos / kNanosPerSecond);
		theTimeSpec.tv_nsec = static_cast<long>(inNanos % kNanosPerSecond);
	}
	else
	{
		theTimeSpec.tv_sec = 0;
		theTimeSpec.tv_nsec = static_cast<long>(inNanos);
	}
	
	#if	Log_TimedWaits || Log_Latency || Log_Average_Latency
		UInt64	theStartNanos = CAHostTimeBase::GetCurrentTimeInNanos();
	#endif

	mOwner = 0;

	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::WaitFor: thread %p is waiting on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_timedwait_relative_np(&mCondVar, &mMutex, &theTimeSpec);
	ThrowIf((theError != 0) && (theError != ETIMEDOUT), CAException(theError), "CAGuard::WaitFor: Wait got an error");
	mOwner = pthread_self();
	
	#if	Log_TimedWaits || Log_Latency || Log_Average_Latency
		UInt64	theEndNanos = CAHostTimeBase::GetCurrentTimeInNanos();
	#endif
	
	#if	Log_TimedWaits
		DebugMessageN1("CAGuard::WaitFor: waited  %.0f", (Float64)(theEndNanos - theStartNanos));
	#endif
	
	#if	Log_Latency
		DebugMessageN1("CAGuard::WaitFor: latency  %.0f", (Float64)((theEndNanos - theStartNanos) - inNanos));
	#endif
	
	#if	Log_Average_Latency
		++mAverageLatencyCount;
		mAverageLatencyAccumulator += (theEndNanos - theStartNanos) - inNanos;
		if(mAverageLatencyCount >= 50)
		{
			DebugMessageN2("CAGuard::WaitFor: average latency  %.3f ns over %ld waits", mAverageLatencyAccumulator / mAverageLatencyCount, mAverageLatencyCount);
			mAverageLatencyCount = 0;
			mAverageLatencyAccumulator = 0.0;
		}
	#endif

	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::WaitFor: thread %p waited on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	theAnswer = theError == ETIMEDOUT;
#elif TARGET_OS_WIN32
	ThrowIf(GetCurrentThreadId() != mOwner, CAException(1), "CAGuard::WaitFor: A thread has to have locked a guard be for it can wait");

	#if	Log_TimedWaits
		DebugMessageN1("CAGuard::WaitFor: waiting %.0f", (Float64)inNanos);
	#endif

	//	the time out is specified in milliseconds(!)
	UInt32 theWaitTime = static_cast<UInt32>(inNanos / 1000000ULL);

	#if	Log_TimedWaits || Log_Latency || Log_Average_Latency
		UInt64	theStartNanos = CAHostTimeBase::GetCurrentTimeInNanos();
	#endif

	mOwner = 0;

	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::WaitFor: thread %lu is waiting on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
	
	ReleaseMutex(mMutex);
	HANDLE theHandles[] = { mMutex, mEvent };
	OSStatus theError = WaitForMultipleObjects(2, theHandles, true, theWaitTime);
	ThrowIf((theError != WAIT_OBJECT_0) && (theError != WAIT_TIMEOUT), CAException(GetLastError()), "CAGuard::WaitFor: Wait got an error");
	mOwner = GetCurrentThreadId();
	ResetEvent(mEvent);
	// This mutex should be locked again when time out happens.rdar://12270555
	if(theError == WAIT_TIMEOUT) {
		DWORD dwError = WaitForSingleObject(mMutex, INFINITE);
		ThrowIf((dwError != WAIT_OBJECT_0), CAException(GetLastError()), "CAGuard::WaitFor: failed to acquire the mutex back when timeout happened\n");
	}
	#if	Log_TimedWaits || Log_Latency || Log_Average_Latency
		UInt64	theEndNanos = CAHostTimeBase::GetCurrentTimeInNanos();
	#endif
	
	#if	Log_TimedWaits
		DebugMessageN1("CAGuard::WaitFor: waited  %.0f", (Float64)(theEndNanos - theStartNanos));
	#endif
	
	#if	Log_Latency
		DebugMessageN1("CAGuard::WaitFor: latency  %.0f", (Float64)((theEndNanos - theStartNanos) - inNanos));
	#endif
	
	#if	Log_Average_Latency
		++mAverageLatencyCount;
		mAverageLatencyAccumulator += (theEndNanos - theStartNanos) - inNanos;
		if(mAverageLatencyCount >= 50)
		{
			DebugMessageN2("CAGuard::WaitFor: average latency  %.3f ns over %ld waits", mAverageLatencyAccumulator / mAverageLatencyCount, mAverageLatencyCount);
			mAverageLatencyCount = 0;
			mAverageLatencyAccumulator = 0.0;
		}
	#endif

	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::WaitFor: thread %lu waited on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif

	theAnswer = theError == WAIT_TIMEOUT;
#endif

	return theAnswer;
}

bool	CAGuard::WaitUntil(UInt64 inNanos)
{
	bool	theAnswer = false;
	UInt64	theCurrentNanos = CAHostTimeBase::GetCurrentTimeInNanos();
	
#if	Log_TimedWaits
	DebugMessageN2("CAGuard::WaitUntil: now: %.0f, requested: %.0f", (double)theCurrentNanos, (double)inNanos);
#endif
	
	if(inNanos > theCurrentNanos)
	{
#if Log_Errors
		if((inNanos - theCurrentNanos) > 1000000000ULL)
		{
			DebugMessage("CAGuard::WaitUntil: about to wait for more than a second");
		}
#endif
		theAnswer = WaitFor(inNanos - theCurrentNanos);
	}
	else
	{
#if	Log_Errors
		DebugMessageN2("CAGuard::WaitUntil: Time has expired before waiting, now: %.0f, requested: %.0f", (double)theCurrentNanos, (double)inNanos);
#endif
		theAnswer = true;
	}

	return theAnswer;
}

void	CAGuard::Notify()
{
#if TARGET_OS_MAC
	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::Notify: thread %p is notifying %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_signal(&mCondVar);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::Notify: failed");
#elif TARGET_OS_WIN32
	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::Notify: thread %lu is notifying %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
	
	SetEvent(mEvent);
#endif
}

void	CAGuard::NotifyAll()
{
#if TARGET_OS_MAC
	#if	Log_WaitOwnership
		DebugPrintf("%p %.4f: CAGuard::NotifyAll: thread %p is notifying %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_broadcast(&mCondVar);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::NotifyAll: failed");
#elif TARGET_OS_WIN32
	#if	Log_WaitOwnership
		DebugPrintf("%lu %.4f: CAGuard::NotifyAll: thread %lu is notifying %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
	
	SetEvent(mEvent);
#endif
}
