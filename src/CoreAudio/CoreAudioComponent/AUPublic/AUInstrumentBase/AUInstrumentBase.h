/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#ifndef __AUInstrumentBase__
#define __AUInstrumentBase__

#include <vector>
#include <stdexcept>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <libkern/OSAtomic.h>
#include "MusicDeviceBase.h"
#include "LockFreeFIFO.h"
#include "SynthEvent.h"
#include "SynthNote.h"
#include "SynthElement.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef LockFreeFIFOWithFree<SynthEvent> SynthEventQueue;

class AUInstrumentBase : public MusicDeviceBase
{
public:
			AUInstrumentBase(
							AudioComponentInstance			inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups = 16,
							UInt32							numParts = 1);
	virtual ~AUInstrumentBase();

	virtual OSStatus			Initialize();
	
	/*! @method Parts */
	AUScope &					Parts()	{ return mPartScope; }

	/*! @method GetPart */
	AUElement *					GetPart( AudioUnitElement inElement)
	{
		return mPartScope.SafeGetElement(inElement);
	}

	virtual AUScope *			GetScopeExtended (AudioUnitScope inScope);

	virtual AUElement *			CreateElement(			AudioUnitScope					inScope,
														AudioUnitElement				inElement);

	virtual void				CreateExtendedElements();

	virtual void				Cleanup();
	
	virtual OSStatus			Reset(					AudioUnitScope 					inScope,
														AudioUnitElement 				inElement);
														
	virtual bool				ValidFormat(			AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														const CAStreamBasicDescription  & inNewFormat);

	virtual bool				StreamFormatWritable(	AudioUnitScope					scope,
														AudioUnitElement				element);

	virtual bool				CanScheduleParameters() const { return false; }

	virtual OSStatus			Render(					AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inNumberFrames);

	virtual OSStatus			StartNote(		MusicDeviceInstrumentID 	inInstrument, 
												MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID *			outNoteInstanceID, 
												UInt32 						inOffsetSampleFrame, 
												const MusicDeviceNoteParams &inParams);

	virtual OSStatus			StopNote(		MusicDeviceGroupID 			inGroupID, 
												NoteInstanceID 				inNoteInstanceID, 
												UInt32 						inOffsetSampleFrame);

	virtual OSStatus			RealTimeStartNote(		SynthGroupElement 			*inGroup,
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame, 
														const MusicDeviceNoteParams &inParams);
	
	virtual OSStatus			RealTimeStopNote(		MusicDeviceGroupID 			inGroupID, 
														NoteInstanceID 				inNoteInstanceID, 
														UInt32 						inOffsetSampleFrame);
	
	virtual OSStatus	HandleControlChange(	UInt8	inChannel,
												UInt8 	inController,
												UInt8 	inValue,
												UInt32	inStartFrame);
												
	virtual OSStatus	HandlePitchWheel(		UInt8 	inChannel,
												UInt8 	inPitch1,
												UInt8 	inPitch2,
												UInt32	inStartFrame);
												
	virtual OSStatus	HandleChannelPressure(	UInt8 	inChannel,
												UInt8 	inValue,
												UInt32	inStartFrame);

	virtual OSStatus	HandleProgramChange(	UInt8	inChannel,
												UInt8 	inValue);

	virtual OSStatus	HandlePolyPressure(		UInt8 	inChannel,
												UInt8 	inKey,
												UInt8	inValue,
												UInt32	inStartFrame);

	virtual OSStatus	HandleResetAllControllers(		UInt8 	inChannel);
	
	virtual OSStatus	HandleAllNotesOff(				UInt8 	inChannel);
	
	virtual OSStatus	HandleAllSoundOff(				UInt8 	inChannel);

	SynthNote*			GetNote(UInt32 inIndex) 
						{ 
							if (!mNotes)
								throw std::runtime_error("no notes");
							return (SynthNote*)((char*)mNotes + inIndex * mNoteSize); 
						}
	
	SynthNote*			GetAFreeNote(UInt32 inFrame);
	void				AddFreeNote(SynthNote* inNote);
	
	friend class SynthGroupElement;
protected:

	UInt32				NextNoteID() { return OSAtomicIncrement32((int32_t *)&mNoteIDCounter); }
	
	
	// call SetNotes in your Initialize() method to give the base class your note structures and to set the maximum 
	// number of active notes. inNoteData should be an array of size inMaxActiveNotes.
	void				SetNotes(UInt32 inNumNotes, UInt32 inMaxActiveNotes, SynthNote* inNotes, UInt32 inNoteSize);
	
	void				PerformEvents(   const AudioTimeStamp &			inTimeStamp);
	OSStatus			SendPedalEvent(MusicDeviceGroupID inGroupID, UInt32 inEventType, UInt32 inOffsetSampleFrame);
	virtual SynthNote*  VoiceStealing(UInt32 inFrame, bool inKillIt);
	UInt32				MaxActiveNotes() const { return mMaxActiveNotes; }
	UInt32				NumActiveNotes() const { return mNumActiveNotes; }
	void				IncNumActiveNotes() { ++mNumActiveNotes; }
	void				DecNumActiveNotes() { --mNumActiveNotes; }
	UInt32				CountActiveNotes();
	
	SynthPartElement *	GetPartElement (AudioUnitElement inPartElement);
	
			// this call throws if there's no assigned element for the group ID
	virtual SynthGroupElement *	GetElForGroupID (MusicDeviceGroupID	inGroupID);
	virtual SynthGroupElement *	GetElForNoteID (NoteInstanceID inNoteID);

	SInt64 mAbsoluteSampleFrame;

	
private:
				
	SInt32 mNoteIDCounter;
	
	SynthEventQueue mEventQueue;
	
	UInt32 mNumNotes;
	UInt32 mNumActiveNotes;
	UInt32 mMaxActiveNotes;
	SynthNote* mNotes;	
	SynthNoteList mFreeNotes;
	UInt32 mNoteSize;
	
	AUScope			mPartScope;
	const UInt32	mInitNumPartEls;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AUMonotimbralInstrumentBase : public AUInstrumentBase
{
public:
	AUMonotimbralInstrumentBase(
							AudioComponentInstance			inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups = 16,
							UInt32							numParts = 1);
										
	virtual OSStatus			RealTimeStartNote(			SynthGroupElement 			*inGroup, 
															NoteInstanceID 				inNoteInstanceID, 
															UInt32 						inOffsetSampleFrame, 
															const MusicDeviceNoteParams &inParams);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// this is a work in progress! The mono-timbral one is finished though!
class AUMultitimbralInstrumentBase : public AUInstrumentBase
{
public:
	AUMultitimbralInstrumentBase(
							AudioComponentInstance			inInstance, 
							UInt32							numInputs,
							UInt32							numOutputs,
							UInt32							numGroups,
							UInt32							numParts);
							
	virtual OSStatus			GetPropertyInfo(		AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable);

	virtual OSStatus			GetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData);

	virtual OSStatus			SetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);

private:

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

