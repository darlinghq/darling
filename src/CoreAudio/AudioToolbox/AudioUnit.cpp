#include "darling-config.h"
#include "AudioUnit.h"
#include "AudioOutputUnitComponent.h"
#include "AudioUnitALSA.h"
#include "stub.h"
#include <CoreServices/MacErrors.h>

OSStatus AudioUnitInitialize(AudioUnit inUnit)
{
	return ((AudioUnitComponent*)inUnit)->init();
}

OSStatus AudioUnitUninitialize(AudioUnit inUnit)
{
	return ((AudioUnitComponent*)inUnit)->deinit();
}

OSStatus AudioUnitAddRenderNotify(AudioUnit inUnit, AURenderCallback inProc, void* opaque)
{
	return ((AudioUnitComponent*)inUnit)->addRenderNotify(inProc, opaque);
}

OSStatus AudioUnitRemoveRenderNotify(AudioUnit inUnit, AURenderCallback inProc, void* opaque)
{
	return ((AudioUnitComponent*)inUnit)->removeRenderNotify(inProc, opaque);
}

OSStatus AudioUnitRender(AudioUnit inUnit, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	OSStatus status;
	
	if (inUnit == nullptr)
		return paramErr;
	
	*ioActionFlags |= kAudioUnitRenderAction_PreRender;
	status = ((AudioUnitComponent*)inUnit)->notifyListeners(ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
	
	if (status != noErr)
		return status;
	
	*ioActionFlags &= ~kAudioUnitRenderAction_PreRender;
	status = ((AudioUnitComponent*)inUnit)->render(ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
	
	if (status != noErr)
		*ioActionFlags |= kAudioUnitRenderAction_PostRenderError;
	
	*ioActionFlags |= kAudioUnitRenderAction_PostRender;
	return ((AudioUnitComponent*)inUnit)->notifyListeners(ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
}

OSStatus AudioUnitReset(AudioUnit inUnit, AudioUnitScope inScope, AudioUnitElement inElement)
{
	return ((AudioUnitComponent*)inUnit)->reset(inScope, inElement);
}

OSStatus AudioUnitGetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData, UInt32 *ioDataSize)
{
	return ((AudioUnitComponent*)inUnit)->getProperty(inID, inScope, inElement, outData, ioDataSize);
}

OSStatus AudioUnitGetPropertyInfo(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32 *outDataSize, Boolean *outWritable)
{
	return ((AudioUnitComponent*)inUnit)->getPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

OSStatus AudioUnitSetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void *inData, UInt32 inDataSize)
{
	return ((AudioUnitComponent*)inUnit)->setProperty(inID, inScope, inElement, inData, inDataSize);
}

OSStatus AudioOutputUnitStart(AudioUnit inUnit)
{
	return static_cast<AudioOutputUnitComponent*>(inUnit)->start();
}

OSStatus AudioOutputUnitStop(AudioUnit inUnit)
{
	return static_cast<AudioOutputUnitComponent*>(inUnit)->stop();
}


