/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#include "SynthNote.h"
#include "SynthElement.h"
#include "AUInstrumentBase.h"

bool SynthNote::AttackNote(
			SynthPartElement *				inPart,
			SynthGroupElement *				inGroup,
			NoteInstanceID					inNoteID, 
			UInt64							inAbsoluteSampleFrame, 
			UInt32							inOffsetSampleFrame, 
			const MusicDeviceNoteParams		&inParams)
{
#if DEBUG_PRINT
	printf("SynthNote::AttackNote %lu %lu abs frame %llu rel frame %lu\n", (UInt32)inGroup->GroupID(), (UInt32)inNoteID, inAbsoluteSampleFrame, inOffsetSampleFrame);
#endif
	mPart = inPart;
	mGroup = inGroup;
	mNoteID = inNoteID;

	mAbsoluteStartFrame = inAbsoluteSampleFrame;
	mRelativeStartFrame = inOffsetSampleFrame;
	mRelativeReleaseFrame = -1;
	mRelativeKillFrame = -1;

	mPitch = inParams.mPitch;
	mVelocity = inParams.mVelocity;
	
	
	return Attack(inParams);
}


void SynthNote::Reset()
{
	mPart = 0;
	mGroup = 0;
	mAbsoluteStartFrame = 0;
	mRelativeStartFrame = 0;
	mRelativeReleaseFrame = -1;
	mRelativeKillFrame = -1;
}

void SynthNote::Kill(UInt32 inFrame)
{
	mRelativeKillFrame = inFrame;
}

void SynthNote::Release(UInt32 inFrame)
{
	mRelativeReleaseFrame = inFrame;
}

void SynthNote::FastRelease(UInt32 inFrame)
{
	mRelativeReleaseFrame = inFrame;
}

double SynthNote::TuningA() const
{
	return 440.0;
}

double SynthNote::Frequency()
{
	return TuningA() * pow(2., (mPitch - 69. + GetPitchBend()) / 12.);
}

double SynthNote::SampleRate()
{
	return GetAudioUnit()->GetOutput(0)->GetStreamFormat().mSampleRate;
}

AUInstrumentBase* SynthNote::GetAudioUnit() const 
{ 
	return (AUInstrumentBase*)mGroup->GetAudioUnit(); 
}

Float32 SynthNote::GetGlobalParameter(AudioUnitParameterID inParamID) const 
{
	return mGroup->GetAudioUnit()->Globals()->GetParameter(inParamID);
}

void SynthNote::NoteEnded(UInt32 inFrame) 
{ 
	mGroup->NoteEnded(this, inFrame);
	mNoteID = 0xFFFFFFFF; 
}

float SynthNote::GetPitchBend() const 
{ 
	return mGroup->GetPitchBend(); 
}


