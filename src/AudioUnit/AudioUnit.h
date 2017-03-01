#ifndef AUDIOUNIT_H
#define AUDIOUNIT_H
#include <MacTypes.h>
#include <CoreAudio/CoreAudioTypes.h>
#include "AUComponent.h"

typedef AudioComponentInstance AudioUnit;
typedef UInt32 AudioUnitRenderActionFlags;
typedef UInt32 AudioUnitElement;
typedef UInt32 AudioUnitScope;
typedef UInt32 AudioUnitPropertyID;

typedef OSStatus (*AURenderCallback) (void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);

enum {
	kAudioUnitRenderAction_PreRender            = (1 << 2),
	kAudioUnitRenderAction_PostRender           = (1 << 3),
	kAudioUnitRenderAction_OutputIsSilence      = (1 << 4),
	kAudioOfflineUnitRenderAction_Preflight     = (1 << 5),
	kAudioOfflineUnitRenderAction_Render        = (1 << 6),
	kAudioOfflineUnitRenderAction_Complete      = (1 << 7),
	kAudioUnitRenderAction_PostRenderError      = (1 << 8),
	kAudioUnitRenderAction_DoNotCheckRenderArgs = (1 << 9)
};

extern "C" {

OSStatus AudioUnitInitialize(AudioUnit inUnit);
OSStatus AudioUnitUninitialize(AudioUnit inUnit);

OSStatus AudioUnitAddRenderNotify(AudioUnit inUnit, AURenderCallback inProc, void* opaque);
OSStatus AudioUnitRemoveRenderNotify(AudioUnit inUnit, AURenderCallback inProc, void* opaque);

OSStatus AudioUnitRender(AudioUnit inUnit, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
OSStatus AudioUnitReset(AudioUnit inUnit, AudioUnitScope inScope, AudioUnitElement inElement);

OSStatus AudioUnitGetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData, UInt32 *ioDataSize);
OSStatus AudioUnitGetPropertyInfo(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32 *outDataSize, Boolean *outWritable);
OSStatus AudioUnitSetProperty(AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void *inData, UInt32 inDataSize);

OSStatus AudioOutputUnitStart(AudioUnit inUnit);
OSStatus AudioOutputUnitStop(AudioUnit inUnit);

}

#endif

