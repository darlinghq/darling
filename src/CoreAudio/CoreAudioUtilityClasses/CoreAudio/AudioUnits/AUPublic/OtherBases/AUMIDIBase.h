/*
     File: AUMIDIBase.h
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
