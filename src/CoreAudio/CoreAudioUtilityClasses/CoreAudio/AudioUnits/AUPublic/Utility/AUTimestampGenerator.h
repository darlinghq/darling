/*
     File: AUTimestampGenerator.h
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
#ifndef __AUTimestampGenerator_h__
#define __AUTimestampGenerator_h__

#include <math.h>
#include "CAHostTimeBase.h"
#include <stdio.h>

#define TSGFMT "0x%10qx"
//#define TSGFMT "%10qd"

// This class generates a continuously increasing series of timestamps based
// on a series of potentially discontinuous timestamps (as can be delivered from
// CoreAudio in the event of an overload or major engine change).
// N.B.: "output" = downstream (source) timestamp
//		 "input"  = upstream (derived) timestamp
class AUTimestampGenerator {
public:
	AUTimestampGenerator(bool hostTimeDiscontinuityCorrection = false)
	{
		mState.mStartInputAtZero = true;
		mState.mBypassed = false;
		mState.mHostTimeDiscontinuityCorrection = hostTimeDiscontinuityCorrection;
#if DEBUG
		mVerbosity = 0;
		snprintf(mDebugName, sizeof(mDebugName), "tsg @ %p", this);
#endif
		// CAHostTimeBase should be used instead of the calls in <CoreAudio/HostTime.h>
		// we make this call here to ensure that this is initialized, otherwise the first time
		// you do actually call CAHostTimeBase to do work, can be on the render thread, and lead to unwanted VM faults
		CAHostTimeBase::GetFrequency();
		Reset();
	}
	
	void	SetStartInputAtZero(bool b) { mState.mStartInputAtZero = b; }
	bool	GetStartInputAtZero() const { return mState.mStartInputAtZero; }
	
	// bypassing is intended for a narrow special case. the upstream sample time will always be the same as the downstream time.
	void	SetBypassed(bool b) { mState.mBypassed = b; }
	bool	GetBypassed() const { return mState.mBypassed; }
		
	// Call this to reset the timeline.
	void	Reset()
	{
		mState.mCurrentInputTime.mSampleTime = 0.;
		mState.mNextInputSampleTime = 0.;
		mState.mCurrentOutputTime.mSampleTime = 0.;
		mState.mNextOutputSampleTime = 0.;
		mState.mLastOutputTime.mFlags = 0;
		mState.mRateScalarAdj = 1.;
		
		mFirstTime = true;
#if DEBUG
		if (mVerbosity)
			printf("%-20.20s: Reset\n", mDebugName);
#endif
	}
	
	// Call this once per render cycle with the downstream timestamp.
	// expectedDeltaFrames is the expected difference between the current and NEXT 
	//	downstream timestamps.
	// sampleRate is the OUTPUT sample rate.
	void	AddOutputTime(const AudioTimeStamp &inTimeStamp, Float64 expectedDeltaFrames, double outputSampleRate, double rateScalarAdj=1.0);
	
	// Call this once per render cycle to obtain the upstream timestamp.
	// framesToAdvance is the number of frames the input timeline is to be
	//	advanced during this render cycle.
	// sampleRate is the INPUT sample rate.
	const AudioTimeStamp &	GenerateInputTime(Float64 framesToAdvance, double inputSampleRate, bool advanceHostTime = false);
		
	// this can be called to override the setting of the next input sample time in GenerateInputTime
	void					Advance(Float64 framesToAdvance)
	{
#if DEBUG
		if (mVerbosity > 1)
			printf("%-20.20s:	ADVANCE         in = " TSGFMT "                    advance = " TSGFMT "\n", mDebugName, (SInt64)mState.mCurrentInputTime.mSampleTime, (SInt64)framesToAdvance);
#endif
		mState.mNextInputSampleTime = mState.mCurrentInputTime.mSampleTime + framesToAdvance;
	}

	struct State {
		AudioTimeStamp		mCurrentInputTime;
		Float64				mNextInputSampleTime;
		Float64				mNextOutputSampleTime;
		Float64				mInputSampleTimeForOutputPull;
		
		AudioTimeStamp		mLastOutputTime;
		AudioTimeStamp		mCurrentOutputTime;

		bool				mStartInputAtZero;  // if true, input timeline starts at 0, else it starts
												// synced with the output timeline
		bool				mDiscontinuous;
		bool				mBypassed;
		Float64				mDiscontinuityDeltaSamples;
		
		double				mRateScalarAdj;
		
		bool				mHostTimeDiscontinuityCorrection; // If true, propagate timestamp discontinuities using host time.
	};
	
	void GetState(State& outState) const { outState = mState; }
	void SetState(State const& inState) { mState = inState; mFirstTime = false; }
	
private:
	
	struct State		mState;

	bool				mFirstTime;	
	
#if DEBUG
public:
	int					mVerbosity;
	char				mDebugName[64];
#endif
};


#endif // __AUTimestampGenerator_h__
