/*
     File: MusicDeviceBase.cpp
 Abstract: MusicDeviceBase.h
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
#include "MusicDeviceBase.h"

// compatibility with older OS SDK releases
typedef OSStatus
(*TEMP_MusicDeviceMIDIEventProc)(	void *			inComponentStorage,
							UInt32					inStatus,
							UInt32					inData1,
							UInt32					inData2,
							UInt32					inOffsetSampleFrame);

typedef OSStatus
(*TEMP_MusicDeviceStartNoteProc)(	void *				inComponentStorage,
						MusicDeviceInstrumentID			inInstrument,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID *				outNoteInstanceID,
						UInt32							inOffsetSampleFrame,
						const MusicDeviceNoteParams *	inParams);

typedef OSStatus
(*TEMP_MusicDeviceStopNoteProc)(void *					inComponentStorage,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID					inNoteInstanceID,
						UInt32							inOffsetSampleFrame);

#if !CA_USE_AUDIO_PLUGIN_ONLY

static OSStatus		MusicDeviceBaseMIDIEvent(void *			inComponentStorage,
						UInt32				inStatus,
						UInt32				inData1,
						UInt32				inData2,
						UInt32				inOffsetSampleFrame);

static OSStatus		MusicDeviceBaseStartNote(	void *		inComponentStorage,
						MusicDeviceInstrumentID			inInstrument,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID *				outNoteInstanceID,
						UInt32							inOffsetSampleFrame,
						const MusicDeviceNoteParams *	inParams);

static OSStatus		MusicDeviceBaseStopNote(void *			inComponentStorage,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID					inNoteInstanceID,
						UInt32							inOffsetSampleFrame);

#endif

MusicDeviceBase::MusicDeviceBase(AudioComponentInstance			inInstance, 
									UInt32						numInputs,
									UInt32						numOutputs,
									UInt32						numGroups) 
	: AUBase(inInstance, numInputs, numOutputs, numGroups),
	  AUMIDIBase(this)
{
}

OSStatus			MusicDeviceBase::GetPropertyInfo(AudioUnitPropertyID	inID,
							AudioUnitScope				inScope,
							AudioUnitElement			inElement,
							UInt32 &				outDataSize,
							Boolean &				outWritable)
{
	OSStatus result;
	
	switch (inID) 
	{
#if !TARGET_OS_IPHONE
		case kMusicDeviceProperty_InstrumentCount:
			if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
			outDataSize = sizeof(UInt32);
			outWritable = false;
			result = noErr;
			break;
#endif
		default:
			result = AUBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
			
			if (result == kAudioUnitErr_InvalidProperty)
				result = AUMIDIBase::DelegateGetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
			break;
	}
	return result;
}

OSStatus			MusicDeviceBase::GetProperty(	AudioUnitPropertyID			inID,
							AudioUnitScope 				inScope,
							AudioUnitElement		 	inElement,
							void *					outData)
{
	OSStatus result;

	switch (inID) 
	{
#if !CA_USE_AUDIO_PLUGIN_ONLY
		case kAudioUnitProperty_FastDispatch:
			if (!IsCMgrObject()) return kAudioUnitErr_InvalidProperty;
			if (inElement == kMusicDeviceMIDIEventSelect) {
				*(TEMP_MusicDeviceMIDIEventProc *)outData = MusicDeviceBaseMIDIEvent;
				return noErr;
			}
			else if (inElement == kMusicDeviceStartNoteSelect) {
				*(TEMP_MusicDeviceStartNoteProc *)outData = MusicDeviceBaseStartNote;
				return noErr;
			}	
			else if (inElement == kMusicDeviceStopNoteSelect) {
				*(TEMP_MusicDeviceStopNoteProc *)outData = MusicDeviceBaseStopNote;
				return noErr;
			}
			return kAudioUnitErr_InvalidElement;
#endif

#if !TARGET_OS_IPHONE
		case kMusicDeviceProperty_InstrumentCount:
			if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
			return GetInstrumentCount (*(UInt32*)outData);
#endif
		default:
			result = AUBase::GetProperty (inID, inScope, inElement, outData);
			
			if (result == kAudioUnitErr_InvalidProperty)
				result = AUMIDIBase::DelegateGetProperty (inID, inScope, inElement, outData);
	}
	
	return result;
}


OSStatus			MusicDeviceBase::SetProperty(	AudioUnitPropertyID 			inID,
							AudioUnitScope 				inScope,
							AudioUnitElement 			inElement,
							const void *				inData,
							UInt32 					inDataSize)

{

	OSStatus result = AUBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
		
	if (result == kAudioUnitErr_InvalidProperty)
		result = AUMIDIBase::DelegateSetProperty (inID, inScope, inElement, inData, inDataSize);
		
	return result;
}

// For a MusicDevice that doesn't support separate instruments (ie. is mono-timbral)
// then this call should return an instrument count of zero and noErr
OSStatus			MusicDeviceBase::GetInstrumentCount (UInt32 &outInstCount) const
{
	outInstCount = 0;
	return noErr;
}

OSStatus	MusicDeviceBase::HandleNoteOn(	UInt8 	inChannel,
											UInt8 	inNoteNumber,
											UInt8 	inVelocity,
											UInt32 	inStartFrame)
{
	MusicDeviceNoteParams params;
	params.argCount = 2;
	params.mPitch = inNoteNumber;
	params.mVelocity = inVelocity;
	return StartNote (kMusicNoteEvent_UseGroupInstrument, inChannel, NULL, inStartFrame, params);
}
											
OSStatus	MusicDeviceBase::HandleNoteOff(	UInt8 	inChannel,
											UInt8 	inNoteNumber,
											UInt8 	inVelocity,
											UInt32 	inStartFrame)
{
	return StopNote (inChannel, inNoteNumber, inStartFrame);
}

OSStatus			
MusicDeviceBase::HandleStartNoteMessage (MusicDeviceInstrumentID		inInstrument, 
										MusicDeviceGroupID				inGroupID, 
										NoteInstanceID *				outNoteInstanceID, 
										UInt32							inOffsetSampleFrame, 
										const MusicDeviceNoteParams *	inParams)
{
	if (inParams == NULL || outNoteInstanceID == NULL) return kAudio_ParamError;

	if (!IsInitialized()) return kAudioUnitErr_Uninitialized;
	
	return StartNote (inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, *inParams);
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
OSStatus			MusicDeviceBase::ComponentEntryDispatch(	ComponentParameters *		params,
																MusicDeviceBase *			This)
{
	if (This == NULL) return kAudio_ParamError;

	OSStatus result;
	
	switch (params->what) {
	case kMusicDeviceMIDIEventSelect:
	case kMusicDeviceSysExSelect:
		{
			result = AUMIDIBase::ComponentEntryDispatch (params, This);
		}
		break;
	case kMusicDevicePrepareInstrumentSelect:
		{
			PARAM(MusicDeviceInstrumentID, inInstrument, 0, 1);
			result = This->PrepareInstrument(inInstrument);
		}
		break;
	case kMusicDeviceReleaseInstrumentSelect:
		{
			PARAM(MusicDeviceInstrumentID, inInstrument, 0, 1);
			result = This->ReleaseInstrument(inInstrument);
		}
		break;
	case kMusicDeviceStartNoteSelect:
		{
			PARAM(MusicDeviceInstrumentID, pbinInstrument, 0, 5);
			PARAM(MusicDeviceGroupID, pbinGroupID, 1, 5);
			PARAM(NoteInstanceID *, pboutNoteInstanceID, 2, 5);
			PARAM(UInt32, pbinOffsetSampleFrame, 3, 5);
			PARAM(const MusicDeviceNoteParams *, pbinParams, 4, 5);
			result = This->HandleStartNoteMessage(pbinInstrument, pbinGroupID, pboutNoteInstanceID, pbinOffsetSampleFrame, pbinParams);
		}
		break;
	case kMusicDeviceStopNoteSelect:
		{
			PARAM(MusicDeviceGroupID, pbinGroupID, 0, 3);
			PARAM(NoteInstanceID, pbinNoteInstanceID, 1, 3);
			PARAM(UInt32, pbinOffsetSampleFrame, 2, 3);
			result = This->StopNote(pbinGroupID, pbinNoteInstanceID, pbinOffsetSampleFrame);
		}
		break;

	default:
		result = AUBase::ComponentEntryDispatch(params, This);
		break;
	}
	
	return result;
}
#endif

#if !CA_USE_AUDIO_PLUGIN_ONLY

// fast dispatch
static OSStatus		MusicDeviceBaseMIDIEvent(void *					inComponentStorage,
						UInt32					inStatus,
						UInt32					inData1,
						UInt32					inData2,
						UInt32					inOffsetSampleFrame)
{
	OSStatus result = noErr;
	try {
		MusicDeviceBase *This = static_cast<MusicDeviceBase *>(inComponentStorage);
		if (This == NULL) return kAudio_ParamError;
		result = This->MIDIEvent(inStatus, inData1, inData2, inOffsetSampleFrame);
	}
	COMPONENT_CATCH
	return result;
}

OSStatus		MusicDeviceBaseStartNote(	void *		inComponentStorage,
						MusicDeviceInstrumentID			inInstrument,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID *				outNoteInstanceID,
						UInt32							inOffsetSampleFrame,
						const MusicDeviceNoteParams *	inParams)
{
	OSStatus result = noErr;
	try {
		if (inParams == NULL || outNoteInstanceID == NULL) return kAudio_ParamError;
		MusicDeviceBase *This = static_cast<MusicDeviceBase *>(inComponentStorage);
		if (This == NULL) return kAudio_ParamError;
		result = This->StartNote(inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, *inParams);
	}
	COMPONENT_CATCH
	return result;
}

OSStatus		MusicDeviceBaseStopNote(void *			inComponentStorage,
						MusicDeviceGroupID				inGroupID,
						NoteInstanceID					inNoteInstanceID,
						UInt32							inOffsetSampleFrame)
{
	OSStatus result = noErr;
	try {
		MusicDeviceBase *This = static_cast<MusicDeviceBase *>(inComponentStorage);
		if (This == NULL) return kAudio_ParamError;
		result = This->StopNote(inGroupID, inNoteInstanceID, inOffsetSampleFrame);
	}
	COMPONENT_CATCH
	return result;
}

#endif
