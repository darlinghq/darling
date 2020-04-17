/*
     File: AUInstrumentBase.cpp
 Abstract: AUInstrumentBase.h
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
#include "AUInstrumentBase.h"
#include "AUMIDIDefs.h"

#if DEBUG
	#define DEBUG_PRINT 0
	#define DEBUG_PRINT_NOTE 0
	#define DEBUG_PRINT_RENDER 0
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////

const UInt32 kEventQueueSize = 1024;

AUInstrumentBase::AUInstrumentBase(
							AudioComponentInstance			inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups,
							UInt32							numParts)
	: MusicDeviceBase(inInstance, numInputs, numOutputs, numGroups), 
	mAbsoluteSampleFrame(0),
	mEventQueue(kEventQueueSize),
	mNumNotes(0),
	mNumActiveNotes(0),
	mMaxActiveNotes(0),
	mNotes(0),
	mNoteSize(0),
	mInitNumPartEls(numParts)
{
#if DEBUG_PRINT
	printf("new AUInstrumentBase\n");
#endif
	mFreeNotes.mState = kNoteState_Free;
	SetWantsRenderThreadID(true);
}
	

AUInstrumentBase::~AUInstrumentBase()
{
#if DEBUG_PRINT
	printf("delete AUInstrumentBase\n");
#endif
}

AUElement *	AUInstrumentBase::CreateElement(AudioUnitScope inScope, AudioUnitElement element)
{
	switch (inScope)
	{
		case kAudioUnitScope_Group:
			return new SynthGroupElement(this, element, new MidiControls);
		case kAudioUnitScope_Part:
			return new SynthPartElement (this, element);
	}
	return MusicDeviceBase::CreateElement(inScope, element);
}

void		AUInstrumentBase::CreateExtendedElements() 
{
	Parts().Initialize(this, kAudioUnitScope_Part, mInitNumPartEls);
}

AUScope *	AUInstrumentBase::GetScopeExtended (AudioUnitScope inScope) 
{
	if (inScope == kAudioUnitScope_Part)
		return &mPartScope;
	return NULL;
}


void		AUInstrumentBase::SetNotes(UInt32 inNumNotes, UInt32 inMaxActiveNotes, SynthNote* inNotes, UInt32 inNoteDataSize)
{
#if DEBUG_PRINT_NOTE
	printf("AUInstrumentBase::SetNotes %d %d %p %d\n", inNumNotes, inMaxActiveNotes, inNotes, inNoteDataSize);
#endif
	mNumNotes = inNumNotes;
	mMaxActiveNotes = inMaxActiveNotes;
	mNoteSize = inNoteDataSize;
	mNotes = inNotes;
	
	for (UInt32 i=0; i<mNumNotes; ++i)
	{
			SynthNote *note = GetNote(i);
			note->Reset();
			mFreeNotes.AddNote(note);
	}
}

UInt32		AUInstrumentBase::CountActiveNotes()
{
	// debugging tool.
	UInt32 sum = 0;
	for (UInt32 i=0; i<mNumNotes; ++i)
	{
		SynthNote *note = GetNote(i);
		if (note->GetState() <= kNoteState_Released) 
			sum++;
	}
	return sum;
}

void		AUInstrumentBase::AddFreeNote(SynthNote* inNote)
{
	// Fast-released notes are already considered inactive and have already decr'd the active count
	if (inNote->GetState() < kNoteState_FastReleased) {
		DecNumActiveNotes();
	}
#if DEBUG_PRINT_NOTE
	else {
			printf("AUInstrumentBase::AddFreeNote: adding fast-released note %p\n", inNote);
	}
	printf("AUInstrumentBase::AddFreeNote (%p)  mNumActiveNotes %lu\n", inNote, mNumActiveNotes);
#endif
	mFreeNotes.AddNote(inNote);
}

OSStatus			AUInstrumentBase::Initialize()
{
/*
TO DO:
	Currently ValidFormat will check and validate that the num channels is not being
	changed if the AU doesn't support the SupportedNumChannels property - which is correct
	
	What needs to happen here is that IFF the AU does support this property, (ie, the AU
	can be configured to have different num channels than its original configuration) then
	the state of the AU at Initialization needs to be validated.
	
	This is work still to be done - see AUEffectBase for the kind of logic that needs to be applied here
*/

	// override to call SetNotes
	
	mNoteIDCounter = 128; // reset this every time we initialise
	mAbsoluteSampleFrame = 0;
	return noErr;
}

void				AUInstrumentBase::Cleanup()
{
	mFreeNotes.Empty();
}


OSStatus			AUInstrumentBase::Reset(			AudioUnitScope 					inScope,
														AudioUnitElement 				inElement)
{
#if DEBUG_PRINT
	printf("AUInstrumentBase::Reset\n");
#endif
	if (inScope == kAudioUnitScope_Global)
	{
		// kill all notes..
		mFreeNotes.Empty();
		for (UInt32 i=0; i<mNumNotes; ++i)
		{
			SynthNote *note = GetNote(i);
			if (note->IsSounding()) 
				note->Kill(0);
			note->ListRemove();
			mFreeNotes.AddNote(note);
		}
		mNumActiveNotes = 0;
		mAbsoluteSampleFrame = 0;

		// empty lists.
		UInt32 numGroups = Groups().GetNumberOfElements();
		for (UInt32 j = 0; j < numGroups; ++j)
		{
			SynthGroupElement *group = (SynthGroupElement*)Groups().GetElement(j);
			group->Reset();
		}
	}
	return MusicDeviceBase::Reset(inScope, inElement);
}

void		AUInstrumentBase::PerformEvents(const AudioTimeStamp& inTimeStamp)
{
#if DEBUG_PRINT_RENDER
	printf("AUInstrumentBase::PerformEvents\n");
#endif
	SynthEvent *event;
	SynthGroupElement *group;
	
	while ((event = mEventQueue.ReadItem()) != NULL)
	{
#if DEBUG_PRINT_RENDER
		printf("event %08X %d\n", event, event->GetEventType());
#endif
		switch(event->GetEventType())
		{
			case SynthEvent::kEventType_NoteOn :
				RealTimeStartNote(GetElForGroupID (event->GetGroupID()), event->GetNoteID(),
									event->GetOffsetSampleFrame(), *event->GetParams());
				break;
			case SynthEvent::kEventType_NoteOff :
				RealTimeStopNote(event->GetGroupID(), event->GetNoteID(),
					event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_SustainOn :
				group = GetElForGroupID (event->GetGroupID());
				group->SustainOn(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_SustainOff :
				group = GetElForGroupID (event->GetGroupID());
				group->SustainOff(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_SostenutoOn :
				group = GetElForGroupID (event->GetGroupID());
				group->SostenutoOn(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_SostenutoOff :
				group = GetElForGroupID (event->GetGroupID());
				group->SostenutoOff(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_AllNotesOff :
				group = GetElForGroupID (event->GetGroupID());
				group->AllNotesOff(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_AllSoundOff :
				group = GetElForGroupID (event->GetGroupID());
				group->AllSoundOff(event->GetOffsetSampleFrame());
				break;
			case SynthEvent::kEventType_ResetAllControllers :
				group = GetElForGroupID (event->GetGroupID());
				group->ResetAllControllers(event->GetOffsetSampleFrame());
				break;
		}
		
		mEventQueue.AdvanceReadPtr();
	}
}

														
OSStatus			AUInstrumentBase::Render(   AudioUnitRenderActionFlags &	ioActionFlags,
												const AudioTimeStamp &			inTimeStamp,
												UInt32							inNumberFrames)
{
	PerformEvents(inTimeStamp);

	AUScope &outputs = Outputs();
	UInt32 numOutputs = outputs.GetNumberOfElements();
	for (UInt32 j = 0; j < numOutputs; ++j)
	{
		GetOutput(j)->PrepareBuffer(inNumberFrames);	// AUBase::DoRenderBus() only does this for the first output element
		AudioBufferList& bufferList = GetOutput(j)->GetBufferList();
		for (UInt32 k = 0; k < bufferList.mNumberBuffers; ++k)
		{
			memset(bufferList.mBuffers[k].mData, 0, bufferList.mBuffers[k].mDataByteSize);
		}
	}
	UInt32 numGroups = Groups().GetNumberOfElements();
	for (UInt32 j = 0; j < numGroups; ++j)
	{
		SynthGroupElement *group = (SynthGroupElement*)Groups().GetElement(j);
		OSStatus err = group->Render((SInt64)inTimeStamp.mSampleTime, inNumberFrames, outputs);
		if (err) return err;
	}
	mAbsoluteSampleFrame += inNumberFrames;
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUInstrumentBase::ValidFormat
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool				AUInstrumentBase::ValidFormat(	AudioUnitScope					inScope,
													AudioUnitElement				inElement,
													const CAStreamBasicDescription  & inNewFormat)
{	
		// if the AU supports this, then we should just let this go through to the Init call
	if (SupportedNumChannels (NULL)) 
		return MusicDeviceBase::ValidFormat(inScope, inElement, inNewFormat);

	bool isGood = MusicDeviceBase::ValidFormat (inScope, inElement, inNewFormat);
	if (!isGood) return false;
	
		// if we get to here, then the basic criteria is that the
		// num channels cannot change on an existing bus
	AUIOElement *el = GetIOElement (inScope, inElement);
	return (el->GetStreamFormat().NumberChannels() == inNewFormat.NumberChannels()); 
}


bool				AUInstrumentBase::StreamFormatWritable(	AudioUnitScope					scope,
															AudioUnitElement				element)
{
	return IsInitialized() ? false : true;
}

OSStatus			AUInstrumentBase::RealTimeStartNote(	SynthGroupElement 			*inGroup,
															NoteInstanceID 				inNoteInstanceID, 
															UInt32 						inOffsetSampleFrame, 
															const MusicDeviceNoteParams &inParams)
{
	return noErr;
}

SynthPartElement *	AUInstrumentBase::GetPartElement (AudioUnitElement inPartElement)
{
	AUScope & parts = Parts();
	unsigned int numEls = parts.GetNumberOfElements();
	for (unsigned int i = 0; i < numEls; ++i) {
		SynthPartElement* el = reinterpret_cast<SynthPartElement*>(parts.GetElement(i));
		if (el->GetIndex() == inPartElement) {
			return el;
		}
	}
	return NULL;
}

SynthGroupElement *	AUInstrumentBase::GetElForGroupID (MusicDeviceGroupID	inGroupID)
{
	AUScope & groups = Groups();
	unsigned int numEls = groups.GetNumberOfElements();
	SynthGroupElement* unassignedEl = NULL;
	
	for (unsigned int i = 0; i < numEls; ++i) {
		SynthGroupElement* el = reinterpret_cast<SynthGroupElement*>(groups.GetElement(i));
		if (el->GroupID() == inGroupID) 
			return el;
		if (el->GroupID() == SynthGroupElement::kUnassignedGroup) {
			unassignedEl = el;
			break; // we fill this up from the start of the group scope vector
		}
	}
	if (unassignedEl) {
		unassignedEl->SetGroupID(inGroupID);
		return unassignedEl;
	}
	throw static_cast<OSStatus>(kAudioUnitErr_InvalidElement);
}

OSStatus			AUInstrumentBase::RealTimeStopNote(
												MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame)
{
#if DEBUG_PRINT
	printf("AUInstrumentBase::RealTimeStopNote ch %d id %d\n", inGroupID, inNoteInstanceID);
#endif
	
	SynthGroupElement *gp = (inGroupID == kMusicNoteEvent_Unused
								? GetElForNoteID (inNoteInstanceID)
								: GetElForGroupID(inGroupID));
	if (gp)
	{
		gp->NoteOff (inNoteInstanceID, inOffsetSampleFrame);
	}
	
	return noErr;
}

SynthGroupElement *	AUInstrumentBase::GetElForNoteID (NoteInstanceID inNoteID)
{
#if DEBUG_PRINT
	printf("GetElForNoteID id %u\n", inNoteID);
#endif
	AUScope & groups = Groups();
	unsigned int numEls = groups.GetNumberOfElements();
	
	for (unsigned int i = 0; i < numEls; ++i) {
		SynthGroupElement* el = reinterpret_cast<SynthGroupElement*>(groups.GetElement(i));
		if (el->GetNote(inNoteID) != NULL)	// searches for any note state
			return el;
	}
	throw static_cast<OSStatus>(kAudioUnitErr_InvalidElement);
}

OSStatus			AUInstrumentBase::StartNote(	MusicDeviceInstrumentID 	inInstrument, 
													MusicDeviceGroupID 			inGroupID, 
													NoteInstanceID *			outNoteInstanceID, 
													UInt32 						inOffsetSampleFrame, 
													const MusicDeviceNoteParams &inParams)
{
	OSStatus err = noErr;
	
	NoteInstanceID noteID; 
	if (outNoteInstanceID) {
		noteID = NextNoteID();
		*outNoteInstanceID = noteID;
	} else
		noteID = (UInt32)inParams.mPitch;
	
#if DEBUG_PRINT
	printf("AUInstrumentBase::StartNote ch %u, key %u, offset %u\n", inGroupID, (unsigned) inParams.mPitch, inOffsetSampleFrame);
#endif

	if (InRenderThread ())
	{		
		err = RealTimeStartNote(
					GetElForGroupID(inGroupID),
					noteID,
					inOffsetSampleFrame,
					inParams);
	}
	else
	{
		SynthEvent *event = mEventQueue.WriteItem();
		if (!event) return -1; // queue full

		event->Set(
			SynthEvent::kEventType_NoteOn,
			inGroupID,
			noteID,
			inOffsetSampleFrame,
			&inParams
		);
		
		mEventQueue.AdvanceWritePtr();
	}
	return err;
}

OSStatus			AUInstrumentBase::StopNote( MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame)
{
#if DEBUG_PRINT
	printf("AUInstrumentBase::StopNote ch %u, id %u, offset %u\n", (unsigned)inGroupID, (unsigned)inNoteInstanceID, inOffsetSampleFrame);
#endif
	OSStatus err = noErr;

	if (InRenderThread ())
	{		
		err = RealTimeStopNote(
			inGroupID,
			inNoteInstanceID,
			inOffsetSampleFrame);
	}
	else
	{
		SynthEvent *event = mEventQueue.WriteItem();
		if (!event) return -1; // queue full

		event->Set(
			SynthEvent::kEventType_NoteOff,
			inGroupID,
			inNoteInstanceID,
			inOffsetSampleFrame,
			NULL
		);
		
		mEventQueue.AdvanceWritePtr();
	}
	return err;
}

OSStatus	AUInstrumentBase::SendPedalEvent(MusicDeviceGroupID inGroupID, UInt32 inEventType, UInt32 inOffsetSampleFrame)
{
	
	if (InRenderThread ())
	{
		SynthGroupElement *group = GetElForGroupID(inGroupID);
		if (!group)
			return kAudioUnitErr_InvalidElement;
		
		switch (inEventType)
		{
			case SynthEvent::kEventType_SustainOn :
				group->SustainOn(inOffsetSampleFrame);
				break;
			case SynthEvent::kEventType_SustainOff :
				group->SustainOff(inOffsetSampleFrame);
				break;
			case SynthEvent::kEventType_SostenutoOn :
				group->SostenutoOn(inOffsetSampleFrame);
				break;
			case SynthEvent::kEventType_SostenutoOff :
				group->SostenutoOff(inOffsetSampleFrame);
				break;
			case SynthEvent::kEventType_AllNotesOff :
				group->AllNotesOff(inOffsetSampleFrame);
				mNumActiveNotes = CountActiveNotes();
				break;
			case SynthEvent::kEventType_AllSoundOff :
				group->AllSoundOff(inOffsetSampleFrame);
				mNumActiveNotes = CountActiveNotes();
				break;
			case SynthEvent::kEventType_ResetAllControllers :
				group->ResetAllControllers(inOffsetSampleFrame);
				break;
		}
	}
	else
	{
		SynthEvent *event = mEventQueue.WriteItem();
		if (!event) return -1; // queue full

		event->Set(inEventType, inGroupID, 0, 0, NULL);
		
		mEventQueue.AdvanceWritePtr();
	}
	return noErr;
}

OSStatus	AUInstrumentBase::HandleControlChange(	UInt8 	inChannel,
													UInt8 	inController,
													UInt8 	inValue,
													UInt32	inStartFrame)
{
#if DEBUG_PRINT
	printf("AUInstrumentBase::HandleControlChange ch %u ctlr: %u val: %u frm: %u\n", inChannel, inController, inValue, inStartFrame);
#endif
	SynthGroupElement *gp = GetElForGroupID(inChannel);
	if (gp)
	{
		gp->ChannelMessage(inController, inValue);
	}
	else
		return kAudioUnitErr_InvalidElement;
	switch (inController)
	{
		case kMidiController_Sustain :
			if (inValue >= 64)
				SendPedalEvent(inChannel, SynthEvent::kEventType_SustainOn, inStartFrame);
			else
				SendPedalEvent(inChannel, SynthEvent::kEventType_SustainOff, inStartFrame);
			break;
		case kMidiController_Sostenuto :
			if (inValue >= 64)
				SendPedalEvent(inChannel, SynthEvent::kEventType_SostenutoOn, inStartFrame);
			else
				SendPedalEvent(inChannel, SynthEvent::kEventType_SostenutoOff, inStartFrame);
			break;
        case kMidiController_OmniModeOff:
        case kMidiController_OmniModeOn:
        case kMidiController_MonoModeOn:
        case kMidiController_MonoModeOff:
            HandleAllSoundOff(inChannel);
            break;
	}
	return noErr;
}
												
OSStatus	AUInstrumentBase::HandlePitchWheel(		UInt8 	inChannel,
													UInt8 	inPitch1,	// LSB
													UInt8 	inPitch2,	// MSB
													UInt32	inStartFrame)
{
	SynthGroupElement *gp = GetElForGroupID(inChannel);
	if (gp)
	{
		gp->ChannelMessage(kMidiMessage_PitchWheel, (inPitch2 << 7) | inPitch1);
		return noErr;
	}
	else
		return kAudioUnitErr_InvalidElement;
}

												
OSStatus	AUInstrumentBase::HandleChannelPressure(UInt8 	inChannel,
													UInt8 	inValue,
													UInt32	inStartFrame)
{
	SynthGroupElement *gp = GetElForGroupID(inChannel);
	if (gp)
	{
		gp->ChannelMessage(kMidiMessage_ChannelPressure, inValue);
		return noErr;
	}
	else
		return kAudioUnitErr_InvalidElement;
}


OSStatus	AUInstrumentBase::HandleProgramChange(	UInt8 	inChannel,
													UInt8 	inValue)
{
#if DEBUG_PRINT
	printf("AUInstrumentBase::HandleProgramChange %u %u\n", inChannel, inValue);
#endif
	SynthGroupElement *gp = GetElForGroupID(inChannel);
	if (gp)
	{
		gp->ChannelMessage(kMidiMessage_ProgramChange, inValue);
		return noErr;
	}
	else
		return kAudioUnitErr_InvalidElement;
}


OSStatus	AUInstrumentBase::HandlePolyPressure(	UInt8 	inChannel,
													UInt8 	inKey,
													UInt8	inValue,
													UInt32	inStartFrame)
{
	SynthGroupElement *gp = GetElForGroupID(inChannel);
	if (gp)
	{
		// Combine key and value into single argument.  UGLY!
		gp->ChannelMessage(kMidiMessage_PolyPressure, (inKey << 7) | inValue);
		return noErr;
	}
	else
		return kAudioUnitErr_InvalidElement;
}


OSStatus	AUInstrumentBase::HandleResetAllControllers(	UInt8 	inChannel)
{
	return SendPedalEvent (inChannel, SynthEvent::kEventType_ResetAllControllers, 0);
}

	
OSStatus	AUInstrumentBase::HandleAllNotesOff(			UInt8 	inChannel)
{
	return SendPedalEvent (inChannel, SynthEvent::kEventType_AllNotesOff, 0);
}

	
OSStatus	AUInstrumentBase::HandleAllSoundOff(			UInt8 	inChannel)
{
	return SendPedalEvent (inChannel, SynthEvent::kEventType_AllSoundOff, 0);
}

SynthNote*  AUInstrumentBase::GetAFreeNote(UInt32 inFrame)
{
#if DEBUG_PRINT_NOTE
	printf("AUInstrumentBase::GetAFreeNote:  %lu available\n", mFreeNotes.Length());
#endif
	SynthNote *note = mFreeNotes.mHead;
	if (note)
	{
		mFreeNotes.RemoveNote(note);
		return note;
	}
	
	return VoiceStealing(inFrame, true);
}

SynthNote*  AUInstrumentBase::VoiceStealing(UInt32 inFrame, bool inKillIt)
{

#if DEBUG_PRINT_NOTE
	printf("AUInstrumentBase::VoiceStealing\n");
#endif
	// free list was empty so we need to kill a note.
	UInt32 startState = inKillIt ? kNoteState_FastReleased : kNoteState_Released;
	for (UInt32 i = startState; i <= startState; --i)
	{
#if DEBUG_PRINT_NOTE
		printf(" checking state %d...\n", i);
#endif
		UInt32 numGroups = Groups().GetNumberOfElements();
		for (UInt32 j = 0; j < numGroups; ++j)
		{
			SynthGroupElement *group = (SynthGroupElement*)Groups().GetElement(j);
#if DEBUG_PRINT_NOTE
			printf("\tsteal group %d   size %d\n", j, group->mNoteList[i].Length());
#endif
			if (group->mNoteList[i].NotEmpty()) {
#if DEBUG_PRINT_NOTE
				printf("\t-- not empty\n");
#endif
				SynthNote *note = group->mNoteList[i].FindMostQuietNote();
				if (inKillIt) {
#if DEBUG_PRINT_NOTE
					printf("\t--=== KILL ===---\n");
#endif
					note->Kill(inFrame);
					group->mNoteList[i].RemoveNote(note);
					if (i != kNoteState_FastReleased)
						DecNumActiveNotes();
					return note;
				} else {
#if DEBUG_PRINT_NOTE
					printf("\t--=== FAST RELEASE ===---\n");
#endif
					group->mNoteList[i].RemoveNote(note);
					note->FastRelease(inFrame);
					group->mNoteList[kNoteState_FastReleased].AddNote(note);
					DecNumActiveNotes(); // kNoteState_FastReleased counts as inactive for voice stealing purposes.
					return NULL;
				}
			}
		}
	}
#if DEBUG_PRINT_NOTE
	printf("no notes to steal????\n");
#endif
	return NULL; // It should be impossible to get here. It means there were no notes to kill in any state. 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

AUMonotimbralInstrumentBase::AUMonotimbralInstrumentBase(
							AudioComponentInstance			inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups,
							UInt32							numParts)
	: AUInstrumentBase(inInstance, numInputs, numOutputs, numGroups, numParts)
{
}

OSStatus			AUMonotimbralInstrumentBase::RealTimeStartNote(	
															SynthGroupElement 			*inGroup, 
															NoteInstanceID 				inNoteInstanceID, 
															UInt32 						inOffsetSampleFrame, 
															const MusicDeviceNoteParams &inParams)
{
#if DEBUG_PRINT_RENDER
	printf("AUMonotimbralInstrumentBase::RealTimeStartNote %d\n", inNoteInstanceID);
#endif

	if (NumActiveNotes() + 1 > MaxActiveNotes()) 
	{
		VoiceStealing(inOffsetSampleFrame, false);
	}
	SynthNote *note = GetAFreeNote(inOffsetSampleFrame);
	if (!note) return -1;
	
	SynthPartElement *part = GetPartElement (0);	// Only one part for monotimbral
	
	IncNumActiveNotes();
	inGroup->NoteOn(note, part, inNoteInstanceID, inOffsetSampleFrame, inParams);
	
	return noErr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////


OSStatus			AUMultitimbralInstrumentBase::GetPropertyInfo(AudioUnitPropertyID	inID,
												AudioUnitScope				inScope,
												AudioUnitElement			inElement,
												UInt32 &					outDataSize,
												Boolean &					outWritable)
{
	OSStatus result = noErr;
	
	switch (inID) 
	{
#if !TARGET_OS_IPHONE
		case kMusicDeviceProperty_PartGroup:
			if (inScope != kAudioUnitScope_Part) return kAudioUnitErr_InvalidScope;
			outDataSize = sizeof(UInt32);
			outWritable = true;
			break;
#endif
		default:
			result = AUInstrumentBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
	}
	return result;
}

OSStatus			AUMultitimbralInstrumentBase::GetProperty(	AudioUnitPropertyID 	inID,
												AudioUnitScope 				inScope,
												AudioUnitElement		 	inElement,
												void *						outData)
{
	OSStatus result = noErr;

	switch (inID) 
	{
#if !TARGET_OS_IPHONE
		case kMusicDeviceProperty_PartGroup:
			if (inScope != kAudioUnitScope_Group) return kAudioUnitErr_InvalidScope;
			// ??
			return -1; //unimpl
			break;
#endif
		default:
			result = AUInstrumentBase::GetProperty (inID, inScope, inElement, outData);
	}
	
	return result;
}



OSStatus			AUMultitimbralInstrumentBase::SetProperty(  AudioUnitPropertyID 			inID,
																AudioUnitScope 					inScope,
																AudioUnitElement 				inElement,
																const void *					inData,
																UInt32 							inDataSize)
{
	OSStatus result = noErr;

	switch (inID) 
	{
#if !TARGET_OS_IPHONE
		case kMusicDeviceProperty_PartGroup:
			if (inScope != kAudioUnitScope_Group) return kAudioUnitErr_InvalidScope;
			// ??
			return -1; //unimpl
			break;
#endif
		default:
			result = MusicDeviceBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
	}
	
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

