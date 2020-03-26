/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
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
