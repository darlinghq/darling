/*
     File: AUMIDIBase.cpp
 Abstract: AUMIDIBase.h
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
#include "AUMIDIBase.h"
#include <CoreMIDI/CoreMIDI.h>
#include "CAXException.h"

//temporaray location
enum
{
	kMidiMessage_NoteOff 			= 0x80,
	kMidiMessage_NoteOn 			= 0x90,
	kMidiMessage_PolyPressure 		= 0xA0,
	kMidiMessage_ControlChange 		= 0xB0,
	kMidiMessage_ProgramChange 		= 0xC0,
	kMidiMessage_ChannelPressure 	= 0xD0,
	kMidiMessage_PitchWheel 		= 0xE0,

	kMidiController_AllSoundOff			= 120,
	kMidiController_ResetAllControllers	= 121,
	kMidiController_AllNotesOff			= 123
};

AUMIDIBase::AUMIDIBase(AUBase* inBase) 
	: mAUBaseInstance (*inBase) 
{
#if CA_AUTO_MIDI_MAP
	mMapManager = new CAAUMIDIMapManager();
#endif
}

AUMIDIBase::~AUMIDIBase() 
{
#if CA_AUTO_MIDI_MAP
	if (mMapManager) 
		delete mMapManager;
#endif
}

#if TARGET_API_MAC_OSX
OSStatus			AUMIDIBase::DelegateGetPropertyInfo(AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable)
{
	OSStatus result = noErr;
	
	switch (inID) {
#if !TARGET_OS_IPHONE
	case kMusicDeviceProperty_MIDIXMLNames:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		if (GetXMLNames(NULL) == noErr) {
			outDataSize = sizeof(CFURLRef);
			outWritable = false;
		} else
			result = kAudioUnitErr_InvalidProperty;
		break;
#endif		
#if CA_AUTO_MIDI_MAP				
	case kAudioUnitProperty_AllParameterMIDIMappings:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		outWritable = true;
		outDataSize = sizeof (AUParameterMIDIMapping)*mMapManager->NumMaps();
		result = noErr;
		break;
	
	case kAudioUnitProperty_HotMapParameterMIDIMapping:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		outWritable = true;
		outDataSize = sizeof (AUParameterMIDIMapping); 
		result = noErr;
		break;
		
	case kAudioUnitProperty_AddParameterMIDIMapping:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		outWritable = true;
		outDataSize = sizeof (AUParameterMIDIMapping);
		result = noErr;
		break;
		
	case kAudioUnitProperty_RemoveParameterMIDIMapping:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		outWritable = true;
		outDataSize = sizeof (AUParameterMIDIMapping); 
		result = noErr;
		break;
#endif

	default:
		result = kAudioUnitErr_InvalidProperty;
		break;
	}
	return result;

#if CA_AUTO_MIDI_MAP || (!TARGET_OS_IPHONE)
InvalidScope:
	return kAudioUnitErr_InvalidScope;
InvalidElement:
	return kAudioUnitErr_InvalidElement;
#endif
}

OSStatus			AUMIDIBase::DelegateGetProperty(	AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData)
{
	OSStatus result;
	
	switch (inID) {
#if !TARGET_OS_IPHONE
	case kMusicDeviceProperty_MIDIXMLNames:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		result = GetXMLNames((CFURLRef *)outData);
		break;
#endif		
#if CA_AUTO_MIDI_MAP
	case kAudioUnitProperty_AllParameterMIDIMappings:{
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		AUParameterMIDIMapping*  maps =  (static_cast<AUParameterMIDIMapping*>(outData));
		mMapManager->GetMaps(maps);
//		printf ("GETTING MAPS\n");
//		mMapManager->Print();
		result = noErr;
		break;
	}
		
	case kAudioUnitProperty_HotMapParameterMIDIMapping:{
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		ca_require(inElement == 0, InvalidElement);
		AUParameterMIDIMapping *  map =  (static_cast<AUParameterMIDIMapping*>(outData));
		mMapManager->GetHotParameterMap (*map);
		result = noErr;
		break;
	}
#endif
	
	default:
		result = kAudioUnitErr_InvalidProperty;
		break;
	}
	return result;

#if CA_AUTO_MIDI_MAP || (!TARGET_OS_IPHONE)
InvalidScope:
	return kAudioUnitErr_InvalidScope;
InvalidElement:
	return kAudioUnitErr_InvalidElement;
#endif
}

OSStatus			AUMIDIBase::DelegateSetProperty(	AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														const void *					inData,
														UInt32							inDataSize)
{
	OSStatus result;
	
	switch (inID) {
#if CA_AUTO_MIDI_MAP
		case kAudioUnitProperty_AddParameterMIDIMapping:{
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			ca_require(inElement == 0, InvalidElement);
			AUParameterMIDIMapping * maps = (AUParameterMIDIMapping*)inData;
			mMapManager->SortedInsertToParamaterMaps (maps, (inDataSize / sizeof(AUParameterMIDIMapping)), mAUBaseInstance);
			mAUBaseInstance.PropertyChanged (kAudioUnitProperty_AllParameterMIDIMappings, kAudioUnitScope_Global, 0);	 
			result = noErr;
			break;
		}
			
		case kAudioUnitProperty_RemoveParameterMIDIMapping:{
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			ca_require(inElement == 0, InvalidElement);
			AUParameterMIDIMapping * maps = (AUParameterMIDIMapping*)inData;
			bool didChange;
			mMapManager->SortedRemoveFromParameterMaps(maps, (inDataSize / sizeof(AUParameterMIDIMapping)), didChange);
			if (didChange)
				mAUBaseInstance.PropertyChanged (kAudioUnitProperty_AllParameterMIDIMappings, kAudioUnitScope_Global, 0);	 
			result = noErr;
			break;
		}
					
		case kAudioUnitProperty_HotMapParameterMIDIMapping:{
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			ca_require(inElement == 0, InvalidElement);
			AUParameterMIDIMapping & map = *((AUParameterMIDIMapping*)inData);
			mMapManager->SetHotMapping (map);			
			result = noErr;
			break;
		}
		case kAudioUnitProperty_AllParameterMIDIMappings:{
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			ca_require(inElement == 0, InvalidElement);
			AUParameterMIDIMapping * mappings = (AUParameterMIDIMapping*)inData;
			mMapManager->ReplaceAllMaps (mappings, (inDataSize / sizeof(AUParameterMIDIMapping)), mAUBaseInstance);
			result = noErr;
			break;
		}
#endif
		
	default:
		result = kAudioUnitErr_InvalidProperty;
		break;
	}
	return result;
#if CA_AUTO_MIDI_MAP
	InvalidScope:
		return kAudioUnitErr_InvalidScope;
	InvalidElement:
		return kAudioUnitErr_InvalidElement;
#endif
}



#endif //TARGET_API_MAC_OSX


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____MidiDispatch


inline const Byte *	NextMIDIEvent(const Byte *event, const Byte *end)
{
	Byte c = *event;
	switch (c >> 4) {
	default:	// data byte -- assume in sysex
		while ((*++event & 0x80) == 0 && event < end)
			;
		break;
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xE:
		event += 3;
		break;
	case 0xC:
	case 0xD:
		event += 2;
		break;
	case 0xF:
		switch (c) {
		case 0xF0:
			while ((*++event & 0x80) == 0 && event < end)
				;
			break;
		case 0xF1:
		case 0xF3:
			event += 2;
			break;
		case 0xF2:
			event += 3;
			break;
		default:
			++event;
			break;
		}
	}
	return (event >= end) ? end : event;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMIDIBase::HandleMIDIPacketList
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			AUMIDIBase::HandleMIDIPacketList(const MIDIPacketList *pktlist)
{
	if (!mAUBaseInstance.IsInitialized()) return kAudioUnitErr_Uninitialized;
	
	int nPackets = pktlist->numPackets;
	const MIDIPacket *pkt = pktlist->packet;
	
	while (nPackets-- > 0) {
		const Byte *event = pkt->data, *packetEnd = event + pkt->length;
		long startFrame = (long)pkt->timeStamp;
		while (event < packetEnd) {
			Byte status = event[0];
			if (status & 0x80) {
				// really a status byte (not sysex continuation)
				HandleMidiEvent(status & 0xF0, status & 0x0F, event[1], event[2], static_cast<UInt32>(startFrame));
					// note that we're generating a bogus channel number for system messages (0xF0-FF)
			}
			event = NextMIDIEvent(event, packetEnd);
		}
		pkt = reinterpret_cast<const MIDIPacket *>(packetEnd);
	}
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMIDIBase::HandleMidiEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus 	AUMIDIBase::HandleMidiEvent(UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inStartFrame)
{
	if (!mAUBaseInstance.IsInitialized()) return kAudioUnitErr_Uninitialized;
		
#if CA_AUTO_MIDI_MAP	
// you potentially have a choice to make here - if a param mapping matches, do you still want to process the 
// MIDI event or not. The default behaviour is to continue on with the MIDI event.
	if (mMapManager->HandleHotMapping (status, channel, data1, mAUBaseInstance)) {
		mAUBaseInstance.PropertyChanged (kAudioUnitProperty_HotMapParameterMIDIMapping, kAudioUnitScope_Global, 0);
	}
	else {
		mMapManager->FindParameterMapEventMatch(status, channel, data1, data2, inStartFrame, mAUBaseInstance);
	}	
#endif	
	
	OSStatus result = noErr;
	
	switch(status)
	{
		case kMidiMessage_NoteOn:
			if(data2)
			{
				result = HandleNoteOn(channel, data1, data2, inStartFrame);
			}
			else
			{
				// zero velocity translates to note off
				result = HandleNoteOff(channel, data1, data2, inStartFrame);
			}
			break;
			
		case kMidiMessage_NoteOff:
			result = HandleNoteOff(channel, data1, data2, inStartFrame);
			break;
		
		default:
			result = HandleNonNoteEvent (status, channel, data1, data2, inStartFrame);
			break;
	}
	
	return result;
}

OSStatus	AUMIDIBase::HandleNonNoteEvent (UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inStartFrame)
{
	OSStatus result = noErr;

	switch (status)
	{
		case kMidiMessage_PitchWheel:
			result = HandlePitchWheel(channel, data1, data2, inStartFrame);
			break;
			
		case kMidiMessage_ProgramChange:
			result = HandleProgramChange(channel, data1);
			break;
			
		case kMidiMessage_ChannelPressure:
			result = HandleChannelPressure(channel, data1, inStartFrame);
			break;
			
		case kMidiMessage_ControlChange:
		{
			switch (data1) {
				case kMidiController_AllNotesOff:
					result = HandleAllNotesOff(channel);
					break;

				case kMidiController_ResetAllControllers:
					result = HandleResetAllControllers(channel);
					break;

				case kMidiController_AllSoundOff:
					result = HandleAllSoundOff(channel);
					break;
								
				default:
					result = HandleControlChange(channel, data1, data2, inStartFrame);
					break;
			}
			break;
		}
		case kMidiMessage_PolyPressure:
			result = HandlePolyPressure (channel, data1, data2, inStartFrame);
			break;
	}
	return result;
}

OSStatus 	AUMIDIBase::SysEx (const UInt8 *	inData, 
										UInt32			inLength)
{
	if (!mAUBaseInstance.IsInitialized()) return kAudioUnitErr_Uninitialized;

	return HandleSysEx(inData, inLength );
}



#if TARGET_OS_MAC
	#if __LP64__
		// comp instance, parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index + 1];
	#else
		// parameters in reverse order, then comp instance
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_nparams - 1 - _index];
	#endif
#elif TARGET_OS_WIN32
		// (no comp instance), parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index];
#endif

#if !CA_USE_AUDIO_PLUGIN_ONLY
OSStatus			AUMIDIBase::ComponentEntryDispatch(	ComponentParameters *			params,
															AUMIDIBase *				This)
{
	if (This == NULL) return kAudio_ParamError;

	OSStatus result;
	
	switch (params->what) {
	case kMusicDeviceMIDIEventSelect:
		{
			PARAM(UInt32, pbinStatus, 0, 4);
			PARAM(UInt32, pbinData1, 1, 4);
			PARAM(UInt32, pbinData2, 2, 4);
			PARAM(UInt32, pbinOffsetSampleFrame, 3, 4);
			result = This->MIDIEvent(pbinStatus, pbinData1, pbinData2, pbinOffsetSampleFrame);
		}
		break;
	case kMusicDeviceSysExSelect:
		{
			PARAM(const UInt8 *, pbinData, 0, 2);
			PARAM(UInt32, pbinLength, 1, 2);
			result = This->SysEx(pbinData, pbinLength);
		}
		break;

	default:
		result = badComponentSelector;
		break;
	}
	
	return result;
}
#endif
