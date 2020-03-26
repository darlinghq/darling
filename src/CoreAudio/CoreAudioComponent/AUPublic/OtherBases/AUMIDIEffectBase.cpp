/*
 <codex>
 <abstract>AUMIDIEffectBase.h</abstract>
 </codex>
*/
#include "AUMIDIEffectBase.h"

// compatibility with older OS SDK releases
typedef OSStatus
(*TEMP_MusicDeviceMIDIEventProc)(	void *				inComponentStorage,
				UInt32					inStatus,
				UInt32					inData1,
				UInt32					inData2,
				UInt32					inOffsetSampleFrame);

#if !CA_USE_AUDIO_PLUGIN_ONLY
static OSStatus		AUMIDIEffectBaseMIDIEvent(void *				inComponentStorage,
						UInt32					inStatus,
						UInt32					inData1,
						UInt32					inData2,
						UInt32					inOffsetSampleFrame);
#endif

AUMIDIEffectBase::AUMIDIEffectBase(		AudioComponentInstance				inInstance,
						bool 						inProcessesInPlace ) 
	: AUEffectBase(inInstance, inProcessesInPlace),
	  AUMIDIBase(this)
{
}

OSStatus			AUMIDIEffectBase::GetPropertyInfo(AudioUnitPropertyID			inID,
							AudioUnitScope				inScope,
							AudioUnitElement			inElement,
							UInt32 &				outDataSize,
							Boolean &				outWritable)
{
	OSStatus result;
	
	result = AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
	
	if (result == kAudioUnitErr_InvalidProperty)
		result = AUMIDIBase::DelegateGetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
	
	return result;
}

OSStatus			AUMIDIEffectBase::GetProperty(	AudioUnitPropertyID		inID,
													AudioUnitScope 			inScope,
													AudioUnitElement		inElement,
													void *					outData)
{
	OSStatus result;

#if !CA_USE_AUDIO_PLUGIN_ONLY
	if (inID == kAudioUnitProperty_FastDispatch) {
		if (inElement == kMusicDeviceMIDIEventSelect) {
			*(TEMP_MusicDeviceMIDIEventProc *)outData = AUMIDIEffectBaseMIDIEvent;
			return noErr;
		}
		return kAudioUnitErr_InvalidElement;
	}
#endif
	
	result = AUEffectBase::GetProperty (inID, inScope, inElement, outData);
	
	if (result == kAudioUnitErr_InvalidProperty)
		result = AUMIDIBase::DelegateGetProperty (inID, inScope, inElement, outData);
	
	return result;
}

OSStatus			AUMIDIEffectBase::SetProperty(	AudioUnitPropertyID			inID,
							AudioUnitScope 				inScope,
							AudioUnitElement		 	inElement,
							const void *				inData,
							UInt32					inDataSize)
{

	OSStatus result = AUEffectBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
		
	if (result == kAudioUnitErr_InvalidProperty)
		result = AUMIDIBase::DelegateSetProperty (inID, inScope, inElement, inData, inDataSize);
		
	return result;
}

#if !CA_USE_AUDIO_PLUGIN_ONLY
#if !TARGET_OS_IPHONE
OSStatus			AUMIDIEffectBase::ComponentEntryDispatch(ComponentParameters *			params,
								AUMIDIEffectBase *			This)
{
	if (This == NULL) return paramErr;

	OSStatus result;
	
	switch (params->what) {
	case kMusicDeviceMIDIEventSelect:
	case kMusicDeviceSysExSelect:
		result = AUMIDIBase::ComponentEntryDispatch (params, This);
		break;
	default:
		result = AUEffectBase::ComponentEntryDispatch(params, This);
		break;
	}
	
	return result;
}
#endif

// fast dispatch
static OSStatus		AUMIDIEffectBaseMIDIEvent(void *				inComponentStorage,
						UInt32					inStatus,
						UInt32					inData1,
						UInt32					inData2,
						UInt32					inOffsetSampleFrame)
{
	OSStatus result = noErr;
	try {
		AUMIDIEffectBase *This = static_cast<AUMIDIEffectBase *>(inComponentStorage);
		if (This == NULL) return paramErr;
		result = This->AUMIDIBase::MIDIEvent(inStatus, inData1, inData2, inOffsetSampleFrame);
	}
	COMPONENT_CATCH
	return result;
}
#endif
