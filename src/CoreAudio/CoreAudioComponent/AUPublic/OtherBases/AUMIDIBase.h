/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#ifndef __AUMIDIBase_h__
#define __AUMIDIBase_h__

#include "AUBase.h"

#if CA_AUTO_MIDI_MAP
	#include "CAAUMIDIMapManager.h"
#endif

struct MIDIPacketList;

// ________________________________________________________________________
//	MusicDeviceBase
//
	/*! @class AUMIDIBase */
class AUMIDIBase {
public:
									// this is NOT a copy constructor!
	/*! @ctor AUMIDIBase */
								AUMIDIBase(AUBase* inBase);
	/*! @dtor ~AUMIDIBase */
	virtual						~AUMIDIBase();
	
	/*! @method MIDIEvent */
	virtual OSStatus	MIDIEvent(		UInt32 						inStatus, 
										UInt32 						inData1, 
										UInt32 						inData2, 
										UInt32 						inOffsetSampleFrame)
	{
		UInt32 strippedStatus = inStatus & 0xf0;
		UInt32 channel = inStatus & 0x0f;
	
		return HandleMidiEvent(strippedStatus, channel, inData1, inData2, inOffsetSampleFrame);
	}
	
	/*! @method HandleMIDIPacketList */
	OSStatus			HandleMIDIPacketList(const MIDIPacketList *pktlist);
	
	/*! @method SysEx */
	virtual OSStatus	SysEx(			const UInt8 *				inData, 
										UInt32 						inLength);

#if TARGET_API_MAC_OSX
	/*! @method DelegateGetPropertyInfo */
	virtual OSStatus			DelegateGetPropertyInfo(AudioUnitPropertyID			inID,
														AudioUnitScope				inScope,
														AudioUnitElement			inElement,
														UInt32 &					outDataSize,
														Boolean &					outWritable);

	/*! @method DelegateGetProperty */
	virtual OSStatus			DelegateGetProperty(	AudioUnitPropertyID 		inID,
														AudioUnitScope 				inScope,
														AudioUnitElement		 	inElement,
														void *						outData);
														
	/*! @method DelegateSetProperty */
	virtual OSStatus			DelegateSetProperty(	AudioUnitPropertyID 		inID,
														AudioUnitScope 				inScope,
														AudioUnitElement		 	inElement,
														const void *				inData,
														UInt32						inDataSize);
#endif

protected:
	// MIDI dispatch
	/*! @method HandleMidiEvent */
	virtual OSStatus	HandleMidiEvent(		UInt8 	inStatus,
												UInt8 	inChannel,
												UInt8 	inData1,
												UInt8 	inData2,
												UInt32 	inStartFrame);

	/*! @method HandleNonNoteEvent */
	virtual OSStatus	HandleNonNoteEvent (	UInt8	status, 
												UInt8	channel, 
												UInt8	data1, 
												UInt8	data2, 
												UInt32	inStartFrame);

#if TARGET_API_MAC_OSX
	/*! @method GetXMLNames */
	virtual OSStatus			GetXMLNames(CFURLRef *outNameDocument) 
	{ return kAudioUnitErr_InvalidProperty; }	// if not overridden, it's unsupported
#endif

// channel messages
	/*! @method HandleNoteOn */
	virtual OSStatus	HandleNoteOn(			UInt8 	inChannel,
												UInt8 	inNoteNumber,
												UInt8 	inVelocity,
												UInt32 	inStartFrame) { return noErr; }
												
	/*! @method HandleNoteOff */
	virtual OSStatus	HandleNoteOff(			UInt8 	inChannel,
												UInt8 	inNoteNumber,
												UInt8 	inVelocity,
												UInt32 	inStartFrame) { return noErr; }
												
	/*! @method HandleControlChange */
	virtual OSStatus	HandleControlChange(	UInt8 	inChannel,
												UInt8 	inController,
												UInt8 	inValue,
												UInt32	inStartFrame) { return noErr; }
												
	/*! @method HandlePitchWheel */
	virtual OSStatus	HandlePitchWheel(		UInt8 	inChannel,
												UInt8 	inPitch1,
												UInt8 	inPitch2,
												UInt32	inStartFrame) { return noErr; }
												
	/*! @method HandleChannelPressure */
	virtual OSStatus	HandleChannelPressure(	UInt8 	inChannel,
												UInt8 	inValue,
												UInt32	inStartFrame) { return noErr; }

	/*! @method HandleProgramChange */
	virtual OSStatus	HandleProgramChange(	UInt8 	inChannel,
												UInt8 	inValue) { return noErr; }

	/*! @method HandlePolyPressure */
	virtual OSStatus	HandlePolyPressure(		UInt8 	inChannel,
												UInt8 	inKey,
												UInt8	inValue,
												UInt32	inStartFrame) { return noErr; }

	/*! @method HandleResetAllControllers */
	virtual OSStatus	HandleResetAllControllers(UInt8 inChannel) { return noErr; }
	
	/*! @method HandleAllNotesOff */
	virtual OSStatus	HandleAllNotesOff(		UInt8	inChannel) { return noErr; }
	
	/*! @method HandleAllSoundOff */
	virtual OSStatus	HandleAllSoundOff(		UInt8	inChannel) { return noErr; }


//System messages   
	/*! @method HandleSysEx */
	virtual OSStatus	HandleSysEx(			const UInt8 *	inData,
                                        		UInt32			inLength ) { return noErr; }

#if CA_AUTO_MIDI_MAP
	/* map manager */
	CAAUMIDIMapManager			*GetMIDIMapManager() {return mMapManager;};
	
#endif

												
private:
	/*! @var mAUBaseInstance */
	AUBase						& mAUBaseInstance;
	
#if CA_AUTO_MIDI_MAP
	/* map manager */
	CAAUMIDIMapManager			* mMapManager;
#endif
	
public:
#if !CA_USE_AUDIO_PLUGIN_ONLY
	// component dispatcher
	/*! @method ComponentEntryDispatch */
	static OSStatus			ComponentEntryDispatch(	ComponentParameters 		*params,
														AUMIDIBase 					*This);
#endif
};

#endif // __AUMIDIBase_h__
