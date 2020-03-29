/*
     File: SynthNote.cpp
 Abstract: SynthNote.h
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
	mRelativeReleaseFrame = 0;
	mRelativeKillFrame = 0;
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


