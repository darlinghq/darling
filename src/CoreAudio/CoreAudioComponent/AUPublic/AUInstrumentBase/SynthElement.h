/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#ifndef __SynthElement__
#define __SynthElement__

#include <AudioUnit/AudioUnit.h>
#include "MusicDeviceBase.h"
#include "SynthNoteList.h"
#include "MIDIControlHandler.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AUInstrumentBase;

class SynthElement : public AUElement
{
public:
	SynthElement(AUInstrumentBase *audioUnit, UInt32 inElement);
	virtual ~SynthElement();

	UInt32 GetIndex() const { return mIndex; }
	
	AUInstrumentBase* GetAUInstrument() { return (AUInstrumentBase*)GetAudioUnit(); }
	
private:
	UInt32 mIndex;
};

class MidiControls : public MIDIControlHandler
{
	enum { kMaxControls = 128 };
public:
	MidiControls();
	virtual ~MidiControls() {}
	virtual void	Reset();
	virtual bool	SetProgramChange(UInt16	inProgram) { mProgramChange = inProgram; return true; }
	virtual bool	SetPitchWheel(UInt16 inValue) {
		mPitchBend = inValue;
		mFPitchBend = (float)(((SInt16)mPitchBend - 8192) / 8192.);
		return true;
	}
	virtual bool	SetChannelPressure(UInt8 inValue) { mMonoPressure = inValue; return true; }
	virtual bool	SetPolyPressure(UInt8 inKey, UInt8 inValue) {
		mPolyPressure[inKey] = inValue;
		return true;
	}
	virtual bool	SetController(UInt8 inControllerNumber, UInt8 inValue) {
		if (inControllerNumber < kMaxControls) {
			mControls[inControllerNumber] = inValue;
			return true;
		}
		return false;
	}
	virtual bool	SetSysex(void *inSysexMsg) { return false; }

	virtual float GetPitchBend() const { return mFPitchBend * mFPitchBendDepth; }

	SInt16 GetHiResControl(UInt32 inIndex) const 
	{   
		return ((mControls[inIndex] & 127) << 7) | (mControls[inIndex + 32] & 127);
	}
	
	float GetControl(UInt32 inIndex) const
	{
		if (inIndex < 32) {
			return (float)(mControls[inIndex] + (mControls[inIndex + 32] / 127.));
		} else {
			return (float)mControls[inIndex];
		}
	}
	
	
private:
	
	UInt8 mControls[128];
	UInt8 mPolyPressure[128];
	UInt8 mMonoPressure;
	UInt8 mProgramChange;
	UInt16 mPitchBend;
	UInt16 mActiveRPN;
	UInt16 mActiveNRPN;
	UInt16 mActiveRPValue;
	UInt16 mActiveNRPValue;
	
	UInt16 mPitchBendDepth;
	float mFPitchBendDepth;
	float mFPitchBend;
	
	void SetHiResControl(UInt32 inIndex, UInt8 inMSB, UInt8 inLSB)
		{ 
			mControls[inIndex] = inMSB;
			mControls[inIndex + 32] = inLSB;
		}
		
};


class SynthGroupElement : public SynthElement
{
public:
	enum {
		kUnassignedGroup = 0xFFFFFFFF
	};
	
	SynthGroupElement(AUInstrumentBase *audioUnit, UInt32 inElement, MIDIControlHandler *inHandler);
	virtual					~SynthGroupElement();

	virtual void			NoteOn(SynthNote *note, SynthPartElement *part, NoteInstanceID inNoteID, UInt32 inOffsetSampleFrame, const MusicDeviceNoteParams &inParams);
	virtual void			NoteOff(NoteInstanceID inNoteID, UInt32 inOffsetSampleFrame);
	void					SustainOn(UInt32 inFrame);
	void					SustainOff(UInt32 inFrame);
	void					SostenutoOn(UInt32 inFrame);
	void					SostenutoOff(UInt32 inFrame);

	void					NoteEnded(SynthNote *inNote, UInt32 inFrame);
	void					NoteFastReleased(SynthNote *inNote);
	
	virtual bool			ChannelMessage(UInt16 controlID, UInt16 controlValue);
	virtual void			AllNotesOff(UInt32 inFrame);
	virtual void			AllSoundOff(UInt32 inFrame);
	void					ResetAllControllers(UInt32 inFrame);
	
	SynthNote *				GetNote(NoteInstanceID inNoteID, bool unreleasedOnly=false, UInt32 *outNoteState=NULL);
	
	void					Reset();
	
	virtual OSStatus		Render(SInt64 inAbsoluteSampleFrame, UInt32 inNumberFrames, AUScope &outputs);
	
	float					GetPitchBend() const { return mMidiControlHandler->GetPitchBend(); }
	SInt64					GetCurrentAbsoluteFrame() const { return mCurrentAbsoluteFrame; }
	
	MusicDeviceGroupID		GroupID () const { return mGroupID; }
	virtual void			SetGroupID (MusicDeviceGroupID inGroup);

	MIDIControlHandler *	GetMIDIControlHandler() const { return mMidiControlHandler; }
	
protected:	
	SInt64					mCurrentAbsoluteFrame;
	SynthNoteList 			mNoteList[kNumberOfSoundingNoteStates];
	MIDIControlHandler		*mMidiControlHandler;

private:
	friend class AUInstrumentBase;
	friend class AUMonotimbralInstrumentBase;
	friend class AUMultitimbralInstrumentBase;
	
	bool					mSustainIsOn;
	bool					mSostenutoIsOn;
	UInt32					mOutputBus;
	MusicDeviceGroupID		mGroupID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SynthKeyZone
{
	UInt8 mLoNote;
	UInt8 mHiNote;
	UInt8 mLoVelocity;
	UInt8 mHiVelocity;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

const UInt32 kUnlimitedPolyphony = 0xFFFFFFFF;

class SynthPartElement : public SynthElement
{
public:
	SynthPartElement(AUInstrumentBase *audioUnit, UInt32 inElement);

	UInt32		GetGroupIndex() const { return mGroupIndex; }
	bool		InRange(Float32 inNote, Float32 inVelocity);
	
	UInt32		GetMaxPolyphony() const { return mMaxPolyphony; }
	void		SetMaxPolyphony(UInt32 inMaxPolyphony) { mMaxPolyphony = inMaxPolyphony; }
	
private:
	UInt32							mGroupIndex;
	UInt32							mPatchIndex;
	UInt32							mMaxPolyphony;
	SynthKeyZone					mKeyZone;	
};

#endif
