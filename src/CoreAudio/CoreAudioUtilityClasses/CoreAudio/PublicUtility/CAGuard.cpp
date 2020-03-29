/*
     File: CAGuard.cpp
 Abstract: CAGuard.h
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
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
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::Wait: thread %p is waiting on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_wait(&mCondVar, &mMutex);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::Wait: Could not wait for a signal");
	mOwner = pthread_self();

	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::Wait: thread %p waited on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif
#elif TARGET_OS_WIN32
	ThrowIf(GetCurrentThreadId() != mOwner, CAException(1), "CAGuard::Wait: A thread has to have locked a guard before it can wait");

	mOwner = 0;

	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::Wait: thread %lu is waiting on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif

	ReleaseMutex(mMutex);
	HANDLE theHandles[] = { mMutex, mEvent };
	OSStatus theError = WaitForMultipleObjects(2, theHandles, true, INFINITE);
	ThrowIfError(theError, CAException(GetLastError()), "CAGuard::Wait: Could not wait for the signal");
	mOwner = GetCurrentThreadId();
	ResetEvent(mEvent);

	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::Wait: thread %lu waited on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
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
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::WaitFor: thread %p is waiting on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
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
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::WaitFor: thread %p waited on %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
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
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::WaitFor: thread %lu is waiting on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
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
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::WaitFor: thread %lu waited on %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
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
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::Notify: thread %p is notifying %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_signal(&mCondVar);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::Notify: failed");
#elif TARGET_OS_WIN32
	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::Notify: thread %lu is notifying %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
	
	SetEvent(mEvent);
#endif
}

void	CAGuard::NotifyAll()
{
#if TARGET_OS_MAC
	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%p %.4f: CAGuard::NotifyAll: thread %p is notifying %s, owner: %p\n", pthread_self(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), pthread_self(), mName, mOwner);
	#endif

	OSStatus theError = pthread_cond_broadcast(&mCondVar);
	ThrowIf(theError != 0, CAException(theError), "CAGuard::NotifyAll: failed");
#elif TARGET_OS_WIN32
	#if	Log_WaitOwnership
		DebugPrintfRtn(DebugPrintfFileComma "%lu %.4f: CAGuard::NotifyAll: thread %lu is notifying %s, owner: %lu\n", GetCurrentThreadId(), ((Float64)(CAHostTimeBase::GetCurrentTimeInNanos()) / 1000000.0), GetCurrentThreadId(), mName, mOwner);
	#endif
	
	SetEvent(mEvent);
#endif
}
