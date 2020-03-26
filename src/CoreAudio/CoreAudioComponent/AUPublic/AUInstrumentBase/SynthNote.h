/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#ifndef __SynthNote__
#define __SynthNote__

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include "MusicDeviceBase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SynthNoteState {
	kNoteState_Attacked = 0,
	kNoteState_Sostenutoed = 1,
	kNoteState_ReleasedButSostenutoed = 2,
	kNoteState_ReleasedButSustained = 3,
	kNoteState_Released = 4,
	kNoteState_FastReleased = 5,
	kNoteState_Free = 6,
	kNumberOfActiveNoteStates = 5,
	kNumberOfSoundingNoteStates = 6,
	kNumberOfNoteStates = 7,
	kNoteState_Unset = kNumberOfNoteStates
};

/*
		This table describes the state transitions for SynthNotes

        EVENT                   CURRENT STATE                                   NEW STATE
        note on                 free                                            attacked
        note off                attacked (and sustain on)                       released but sustained
        note off                attacked                                        released
        note off                sostenutoed                                     released but sostenutoed
        sustain on              -- no changes --
        sustain off             released but sustained                          released
        sostenuto on            attacked                                        sostenutoed
        sostenuto off           sostenutoed                                     attacked
        sostenuto off           released but sostenutoed (and sustain on)       released but sustained
        sostenuto off           released but sostenutoed                        released
        end of note             any state                                       free
		soft voice stealing     any state                                       fast released
		hard voice stealing     any state                                       free
		
		soft voice stealing happens when there is a note on event and NumActiveNotes > MaxActiveNotes
		hard voice stealing happens when there is a note on event and NumActiveNotes == NumNotes (no free notes)
		voice stealing removes the quietest note in the highest numbered state that has sounding notes.
*/

class SynthGroupElement;
class SynthPartElement;
class AUInstrumentBase;

struct SynthNote
{
	SynthNote() :
		mPrev(0), mNext(0), mPart(0), mGroup(0),
		mNoteID(0xffffffff),
		mState(kNoteState_Unset),
		mAbsoluteStartFrame(0),
		mRelativeStartFrame(0),
		mRelativeReleaseFrame(-1),
		mRelativeKillFrame(-1),
		mPitch(0.0f),
		mVelocity(0.0f)
	{
	}
	
	virtual					~SynthNote() {}
	
	virtual void			Reset();
	//! Returns true if active note resulted from this call, otherwise false
	virtual bool			AttackNote(
									SynthPartElement *				inPart,
									SynthGroupElement *				inGroup,
									NoteInstanceID					inNoteID, 
									UInt64							inAbsoluteSampleFrame, 
									UInt32							inOffsetSampleFrame, 
									const MusicDeviceNoteParams		&inParams
							);
								
	virtual OSStatus		Render(UInt64 inAbsoluteSampleFrame, UInt32 inNumFrames, AudioBufferList** inBufferList, UInt32 inOutBusCount) = 0;
	//! Returns true if active note resulted from this call, otherwise false
	virtual bool			Attack(const MusicDeviceNoteParams &inParams) = 0;
	virtual void			Kill(UInt32 inFrame); // voice is being stolen.
	virtual void			Release(UInt32 inFrame);
	virtual void			FastRelease(UInt32 inFrame);
	virtual Float32			Amplitude() = 0; // used for finding quietest note for voice stealing.

	virtual void			NoteEnded(UInt32 inFrame);

	SynthGroupElement*		GetGroup() const { return mGroup; }
	SynthPartElement*		GetPart() const { return mPart; }
	
	AUInstrumentBase*		GetAudioUnit() const;

	Float32					GetGlobalParameter(AudioUnitParameterID inParamID) const;

	NoteInstanceID			GetNoteID() const { return mNoteID; }
	SynthNoteState			GetState() const { return mState; }
	UInt8					GetMidiKey() const { return (UInt8) mPitch; }
	UInt8					GetMidiVelocity() const { return (UInt8) mVelocity; }
	
	Boolean					IsSounding() const { return mState < kNumberOfSoundingNoteStates; }
	Boolean					IsActive() const { return mState < kNumberOfActiveNoteStates; }
	UInt64					GetAbsoluteStartFrame() const { return mAbsoluteStartFrame; }
	SInt32					GetRelativeStartFrame() const { return mRelativeStartFrame; }
	SInt32					GetRelativeReleaseFrame() const { return mRelativeReleaseFrame; }
	SInt32					GetRelativeKillFrame() const { return mRelativeKillFrame; }

	void					ListRemove() { mPrev = mNext = 0; } // only use when lists will be reset.

	float					GetPitchBend() const;
	double					TuningA() const;
	
	Float32					GetPitch() const { return mPitch; }	// returns raw pitch from MusicDeviceNoteParams
	virtual double			Frequency(); // returns the frequency of note + pitch bend.
	virtual double			SampleRate();

	// linked list pointers
	SynthNote				*mPrev;
	SynthNote				*mNext;
	
	friend class			SynthGroupElement;
	friend struct			SynthNoteList;
protected:
	void					SetState(SynthNoteState inState) { mState = inState; }
private:
	SynthPartElement*		mPart;
	SynthGroupElement*	mGroup;
		
	NoteInstanceID			mNoteID;
	SynthNoteState			mState;
	UInt64					mAbsoluteStartFrame;
	SInt32					mRelativeStartFrame;
	SInt32					mRelativeReleaseFrame;
	SInt32					mRelativeKillFrame;
	
	Float32					mPitch;
	Float32					mVelocity;
};

#endif

