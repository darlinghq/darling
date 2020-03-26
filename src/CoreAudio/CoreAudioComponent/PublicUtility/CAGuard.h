/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#if !defined(__CAGuard_h__)
#define __CAGuard_h__

//==================================================================================================
//	Includes
//=============================================================================

//	Super Class Includes
#include "CAMutex.h"

#if CoreAudio_Debug
//	#define	Log_Average_Latency	1
#endif

//==================================================================================================
//	CAGuard
//
//	This is your typical mutex with signalling implemented via pthreads.
//	Lock() will return true if and only if the guard is locked on that call.
//	A thread that already has the guard will receive 'false' if it locks it
//	again. Use of the stack-based CAGuard::Locker class is highly recommended
//	to properly manage the recursive nesting. The Wait calls with timeouts
//	will return true if and only if the timeout period expired. They will
//	return false if they receive notification any other way.
//==================================================================================================

class	CAGuard : public CAMutex
{

//	Construction/Destruction
public:
					CAGuard(const char* inName);
	virtual			~CAGuard();

//	Actions
public:
	virtual void	Wait();
	virtual bool	WaitFor(UInt64 inNanos);
	virtual bool	WaitUntil(UInt64 inNanos);
	
	virtual void	Notify();
	virtual void	NotifyAll();

//	Implementation
protected:
#if TARGET_OS_MAC
	pthread_cond_t	mCondVar;
#else
	HANDLE			mEvent;
#endif
#if	Log_Average_Latency
	Float64			mAverageLatencyAccumulator;
	UInt32			mAverageLatencyCount;
#endif
	
//	Helper class to manage taking and releasing recursively
public:
	class			Locker
	{
	
	//	Construction/Destruction
	public:
					Locker(CAGuard& inGuard) : mGuard(inGuard), mNeedsRelease(false) { mNeedsRelease = mGuard.Lock(); }
					~Locker() { if(mNeedsRelease) { mGuard.Unlock(); } }
	
	private:
					Locker(const Locker&);
		Locker&		operator=(const Locker&);
	
	//	Actions
	public:
		void		Wait() { mGuard.Wait(); }
		bool		WaitFor(UInt64 inNanos) { return mGuard.WaitFor(inNanos); }
		bool		WaitUntil(UInt64 inNanos) { return mGuard.WaitUntil(inNanos); }
		
		void		Notify() { mGuard.Notify(); }
		void		NotifyAll() { mGuard.NotifyAll(); }

	//	Implementation
	private:
		CAGuard&	mGuard;
		bool		mNeedsRelease;	
	};

};

#endif
