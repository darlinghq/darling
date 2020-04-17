/*
     File: SynthElement.cpp
 Abstract: SynthElement.h
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
#include "SynthElement.h"
#include "AUInstrumentBase.h"
#include "AUMIDIDefs.h"

#undef DEBUG_PRINT
#define DEBUG_PRINT 0
#define DEBUG_PRINT_NOTE 0
#define DEBUG_PRINT_RENDER 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////
MidiControls::MidiControls()
{
	Reset();
}

void MidiControls::Reset()
{
	memset(mControls, 0, sizeof(mControls));
	memset(mPolyPressure, 0, sizeof(mPolyPressure));
	mMonoPressure = 0;
	mProgramChange = 0;
	mPitchBend = 0;
	mActiveRPN = 0;
	mActiveNRPN = 0;
	mActiveRPValue = 0;
	mActiveNRPValue = 0;
	mControls[kMidiController_Pan] = 64;
	mControls[kMidiController_Expression] = 127;
	mPitchBendDepth = 24 << 7;
	mFPitchBendDepth = 24.0f;
	mFPitchBend = 0.0f;
}


SynthElement::SynthElement(AUInstrumentBase *audioUnit, UInt32 inElement) 
	: AUElement(audioUnit), mIndex(inElement)
{
}

SynthElement::~SynthElement()
{
}

SynthGroupElement::SynthGroupElement(AUInstrumentBase *audioUnit, UInt32 inElement, MIDIControlHandler *inHandler) 
	: SynthElement(audioUnit, inElement),
	mCurrentAbsoluteFrame(-1),
	mMidiControlHandler(inHandler),
	mSustainIsOn(false), mSostenutoIsOn(false), mOutputBus(0), mGroupID(kUnassignedGroup)
{
	for (UInt32 i=0; i<kNumberOfSoundingNoteStates; ++i)
		mNoteList[i].mState = (SynthNoteState) i;
}

SynthGroupElement::~SynthGroupElement()
{
	delete mMidiControlHandler;
}

void	SynthGroupElement::SetGroupID (MusicDeviceGroupID inGroup)
{
		// can't re-assign a group once its been assigned
	if (mGroupID != kUnassignedGroup) throw static_cast<OSStatus>(kAudioUnitErr_InvalidElement);
	mGroupID = inGroup;
}

void SynthGroupElement::Reset() 
{
#if DEBUG_PRINT
	printf("SynthGroupElement::Reset\n");
#endif
	mMidiControlHandler->Reset();
	for (UInt32 i=0; i<kNumberOfSoundingNoteStates; ++i)
		mNoteList[i].Empty();
}

SynthPartElement::SynthPartElement(AUInstrumentBase *audioUnit, UInt32 inElement) 
	: SynthElement(audioUnit, inElement)
{
}

// Return the SynthNote with the given inNoteID, if found.  If unreleasedOnly is true, only look for
// attacked and sostenutoed notes, otherwise search all states.  Return state of found note via outNoteState.

SynthNote *SynthGroupElement::GetNote(NoteInstanceID inNoteID, bool unreleasedOnly, UInt32 *outNoteState)
{
#if DEBUG_PRINT_RENDER
	printf("SynthGroupElement::GetNote %d, unreleased = %d\n", inNoteID, unreleasedOnly);
#endif
	const UInt32 lastNoteState = unreleasedOnly ? 
									(mSostenutoIsOn ? kNoteState_Sostenutoed : kNoteState_Attacked)
										: kNoteState_Released;
	SynthNote *note = NULL;
	// Search for notes in each successive state
	for (UInt32 noteState = kNoteState_Attacked; noteState <= lastNoteState; ++noteState)
	{
		if (outNoteState) *outNoteState = noteState;	// even if we find nothing
		note = mNoteList[noteState].mHead;
		while (note && note->mNoteID != inNoteID)
		{
#if DEBUG_PRINT_RENDER
			printf("   checking %p id: %d\n", note, note->mNoteID);
#endif
			note = note->mNext;
		}
		if (note)
		{
#if DEBUG_PRINT_RENDER
			printf("  found %p\n", note);
#endif
			break;
		}
	}
	return note;
}

void SynthGroupElement::NoteOn(SynthNote *note,
							   SynthPartElement *part,
							   NoteInstanceID inNoteID,
							   UInt32 inOffsetSampleFrame,
							   const MusicDeviceNoteParams &inParams)
{
#if DEBUG_PRINT_NOTE
	printf("SynthGroupElement::NoteOn %d\n", inNoteID);
#endif
	// TODO: CONSIDER FIXING this to not need to initialize mCurrentAbsoluteFrame to -1.
	UInt64 absoluteFrame = (mCurrentAbsoluteFrame == -1) ? inOffsetSampleFrame : mCurrentAbsoluteFrame + inOffsetSampleFrame;
	if (note->AttackNote(part, this, inNoteID, absoluteFrame, inOffsetSampleFrame, inParams)) {
		mNoteList[kNoteState_Attacked].AddNote(note);
	}
}

void SynthGroupElement::NoteOff(NoteInstanceID inNoteID, UInt32 inFrame)
{	
#if DEBUG_PRINT_NOTE
	printf("SynthGroupElement::NoteOff %d\n", inNoteID);
#endif
	UInt32 noteState = kNoteState_Attacked;
	SynthNote *note = GetNote(inNoteID, true, &noteState);	// asking for unreleased only
	if (note)
	{
#if DEBUG_PRINT_NOTE
		printf("  old note state: %d\n", note->mState);
#endif
		if (noteState == kNoteState_Attacked)
		{
			mNoteList[noteState].RemoveNote(note);
			if (mSustainIsOn) {
				mNoteList[kNoteState_ReleasedButSustained].AddNote(note);
			} else {
				note->Release(inFrame);
				mNoteList[kNoteState_Released].AddNote(note);
			}
#if DEBUG_PRINT_NOTE
			printf("  new note state: %d\n", note->mState);
#endif
		}
		else /* if (noteState == kNoteState_Sostenutoed) */
		{
			mNoteList[kNoteState_Sostenutoed].RemoveNote(note);
			mNoteList[kNoteState_ReleasedButSostenutoed].AddNote(note);
		}
	}
}

void SynthGroupElement::NoteEnded(SynthNote *inNote, UInt32 inFrame)
{
#if DEBUG_PRINT_NOTE
	printf("SynthGroupElement::NoteEnded: id %d state %d\n", inNote->mNoteID, inNote->mState);
#endif
	if (inNote->IsSounding()) {
		SynthNoteList *list = &mNoteList[inNote->GetState()];
		list->RemoveNote(inNote);
	}
	
	GetAUInstrument()->AddFreeNote(inNote);
}

void SynthGroupElement::NoteFastReleased(SynthNote *inNote)
{
#if DEBUG_PRINT_NOTE
	printf("SynthGroupElement::NoteFastReleased id %d state %d\n", inNote->mNoteID, inNote->mState);
#endif
	if (inNote->IsActive()) {
		mNoteList[inNote->GetState()].RemoveNote(inNote);
		GetAUInstrument()->DecNumActiveNotes();
		mNoteList[kNoteState_FastReleased].AddNote(inNote);
	}
	else {
		Assert(true, "ASSERT FAILED:  Attempting to fast-release non-active note");
	}
}

bool SynthGroupElement::ChannelMessage(UInt16 controllerID, UInt16 inValue)
{
	bool handled = true;
#if DEBUG_PRINT
	printf("SynthGroupElement::ChannelMessage(0x%x, %u)\n", controllerID, inValue);
#endif
	// Sustain and sostenuto are "pedal events", and are handled during render cycle
	if (controllerID <= kMidiController_RPN_MSB && controllerID != kMidiController_Sustain && controllerID != kMidiController_Sostenuto)
		handled = mMidiControlHandler->SetController(controllerID, UInt8(inValue));
	else 
	{
		switch (controllerID)
		{
			case kMidiMessage_ProgramChange:
				handled = mMidiControlHandler->SetProgramChange(inValue);
				break;
			case kMidiMessage_PitchWheel:
				handled = mMidiControlHandler->SetPitchWheel(inValue);
				break;
			case kMidiMessage_ChannelPressure:
#if DEBUG_PRINT
				printf("SynthGroupElement::ChannelMessage: Channel Pressure %u\n", inValue);
#endif
				handled = mMidiControlHandler->SetChannelPressure(UInt8(inValue));
				break;
			case kMidiMessage_PolyPressure:
			{	UInt8 inKey = inValue >> 7;
				UInt8 val = inValue & 0x7f;
				handled = mMidiControlHandler->SetPolyPressure(inKey, val);
				break;
			}
			default:
				handled = false;
				break;
		}
	}
	return handled;
}

void SynthGroupElement::SostenutoOn(UInt32 inFrame)
{
#if DEBUG_PRINT
	printf("SynthGroupElement::SostenutoOn\n");
#endif
	if (!mSostenutoIsOn) {
		mMidiControlHandler->SetController(kMidiController_Sostenuto, 127);
		mSostenutoIsOn = true;
		mNoteList[kNoteState_Sostenutoed].TransferAllFrom(&mNoteList[kNoteState_Attacked], inFrame);
	}
}

void SynthGroupElement::SostenutoOff(UInt32 inFrame)
{
#if DEBUG_PRINT
	printf("SynthGroupElement::SostenutoOff\n");
#endif
	if (mSostenutoIsOn) {
		mMidiControlHandler->SetController(kMidiController_Sostenuto, 0);
		mSostenutoIsOn = false;
		mNoteList[kNoteState_Attacked].TransferAllFrom(&mNoteList[kNoteState_Sostenutoed], inFrame);
		if (mSustainIsOn) 
			mNoteList[kNoteState_ReleasedButSustained].TransferAllFrom(&mNoteList[kNoteState_ReleasedButSostenutoed], inFrame);
		else
			mNoteList[kNoteState_Released].TransferAllFrom(&mNoteList[kNoteState_ReleasedButSostenutoed], inFrame);
	}
}


void SynthGroupElement::SustainOn(UInt32 inFrame)
{
#if DEBUG_PRINT
//	printf("SynthGroupElement::SustainOn\n");
#endif
	if (!mSustainIsOn) {
		mMidiControlHandler->SetController(kMidiController_Sustain, 127);
		mSustainIsOn = true;
	}
}

void SynthGroupElement::SustainOff(UInt32 inFrame)
{
#if DEBUG_PRINT
//	printf("SynthGroupElement::SustainOff\n");
#endif
	if (mSustainIsOn) {
		mMidiControlHandler->SetController(kMidiController_Sustain, 0);
		mSustainIsOn = false;
	
		mNoteList[kNoteState_Released].TransferAllFrom(&mNoteList[kNoteState_ReleasedButSustained], inFrame);
	}
}

void SynthGroupElement::AllNotesOff(UInt32 inFrame)
{
#if DEBUG_PRINT
	printf("SynthGroupElement::AllNotesOff\n");
#endif
	SynthNote *note;
	for (UInt32 i=0 ; i<=kNoteState_Sostenutoed; ++i)
	{
		UInt32 newState = (i == kNoteState_Attacked) ?
			kNoteState_Released : kNoteState_ReleasedButSostenutoed;
		note = mNoteList[i].mHead;
		while (note)
		{
			SynthNote *nextNote = note->mNext;
			
			mNoteList[i].RemoveNote(note);
			note->Release(inFrame);
			mNoteList[newState].AddNote(note);
			
			note = nextNote;
		}
	}	
}

void SynthGroupElement::AllSoundOff(UInt32 inFrame)
{
#if DEBUG_PRINT
	printf("SynthGroupElement::AllSoundOff\n");
#endif
	SynthNote *note;
	
	for (UInt32 i=0 ; i<kNumberOfActiveNoteStates; ++i)
	{
		note = mNoteList[i].mHead;
		while (note)
		{
			SynthNote *nextNote = note->mNext;
			
			mNoteList[i].RemoveNote(note);
			note->FastRelease(inFrame);
			mNoteList[kNoteState_FastReleased].AddNote(note);
			GetAUInstrument()->DecNumActiveNotes();
			note = nextNote;
		}
	}	
}

void SynthGroupElement::ResetAllControllers(UInt32 inFrame)
{
#if DEBUG_PRINT
	printf("SynthGroupElement::ResetAllControllers\n");
#endif
	mMidiControlHandler->Reset();
}

OSStatus SynthGroupElement::Render(SInt64 inAbsoluteSampleFrame, UInt32 inNumberFrames, AUScope &outputs)
{
	// Avoid duplicate calls at same sample offset
	if (inAbsoluteSampleFrame != mCurrentAbsoluteFrame)
	{
		mCurrentAbsoluteFrame = inAbsoluteSampleFrame;
		AudioBufferList* buffArray[16];
		UInt32 numOutputs = outputs.GetNumberOfElements();
		for (UInt32 outBus = 0; outBus < numOutputs && outBus < 16; ++outBus)
		{
			buffArray[outBus] = &GetAudioUnit()->GetOutput(outBus)->GetBufferList();
		}
		
		for (UInt32 i=0 ; i<kNumberOfSoundingNoteStates; ++i)
		{
			SynthNote *note = mNoteList[i].mHead;
			while (note)
			{
#if DEBUG_PRINT_RENDER
				printf("SynthGroupElement::Render: state %d, note %p\n", i, note);
#endif
				SynthNote *nextNote = note->mNext;
				
				OSStatus err = note->Render(inAbsoluteSampleFrame, inNumberFrames, buffArray, numOutputs);
				if (err) return err;
				
				note = nextNote;
			}
		}
	}
	return noErr;
}


