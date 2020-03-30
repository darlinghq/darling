/*
     File: CAGuard.h
 Abstract: Part of CoreAudio Utility Classes
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
