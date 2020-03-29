/*
     File: SynthEvent.h
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
/* You can either fill in code here or remove this and create or add new files. */

#ifndef __SynthEvent__
#define __SynthEvent__

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include "MusicDeviceBase.h"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////////////////////////////////


class SynthEvent
{
public:
	enum {
		kEventType_NoteOn = 1,
		kEventType_NoteOff = 2,
		kEventType_SustainOn = 3,
		kEventType_SustainOff = 4,
		kEventType_SostenutoOn = 5,
		kEventType_SostenutoOff = 6,
		kEventType_AllNotesOff = 7,
		kEventType_AllSoundOff = 8,
		kEventType_ResetAllControllers = 9
	};


	SynthEvent() {}
	~SynthEvent() {}

	void Set(   
				UInt32							inEventType,
				MusicDeviceGroupID				inGroupID,
				NoteInstanceID					inNoteID,
				UInt32							inOffsetSampleFrame,
				const MusicDeviceNoteParams*	inNoteParams
		)
	{
		mEventType = inEventType;
		mGroupID = inGroupID;
		mNoteID = inNoteID;
		mOffsetSampleFrame = inOffsetSampleFrame;
		
		if (inNoteParams)
		{
			UInt32 paramSize = offsetof(MusicDeviceNoteParams, mControls) + (inNoteParams->argCount-2)*sizeof(NoteParamsControlValue); 
			mNoteParams = inNoteParams->argCount > 3 
								? (MusicDeviceNoteParams*)malloc(paramSize) 
								: &mSmallNoteParams;
			memcpy(mNoteParams, inNoteParams, paramSize);
		}
		else 
			mNoteParams = NULL;
	}
	
	
	void Free()
	{
		if (mNoteParams)
		{
			if (mNoteParams->argCount > 3)
				free(mNoteParams);
			mNoteParams = NULL;
		}
	}
	
	UInt32					GetEventType() const { return mEventType; }
	MusicDeviceGroupID		GetGroupID() const { return mGroupID; }
	NoteInstanceID			GetNoteID() const { return mNoteID; }
	UInt32					GetOffsetSampleFrame() const { return mOffsetSampleFrame; }
	
	MusicDeviceNoteParams*  GetParams() const { return mNoteParams; }

	UInt32					GetArgCount() const { return mNoteParams->argCount; }
	UInt32					NumberParameters() const { return mNoteParams->argCount - 2; }
	
	Float32					GetNote() const { return mNoteParams->mPitch; }
	Float32					GetVelocity() const { return mNoteParams->mVelocity; }
	
	NoteParamsControlValue  GetParameter(UInt32 inIndex) const 
							{
								if (inIndex >= NumberParameters()) 
									throw std::runtime_error("index out of range");
								return mNoteParams->mControls[inIndex]; 
							}
	
private:
	UInt32					mEventType;
	MusicDeviceGroupID		mGroupID;
	NoteInstanceID			mNoteID;
	UInt32					mOffsetSampleFrame;
	MusicDeviceNoteParams*  mNoteParams;
	MusicDeviceNoteParams   mSmallNoteParams; // inline a small one to eliminate malloc for the simple case.
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
