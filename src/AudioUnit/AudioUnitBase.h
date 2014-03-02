#ifndef AUDIOUNITBASE_H
#define AUDIOUNITBASE_H

#include "AudioUnit.h"
#include "AudioUnitProperties.h"
#include <CoreServices/ComponentsInternal.h>

class AudioUnitComponent : public CarbonComponent
{
public:
	AudioUnitComponent();

	virtual OSStatus init() = 0;
	virtual OSStatus deinit() = 0;
	
	virtual OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) = 0;
	
	virtual OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) = 0;

	virtual OSStatus setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize);
	virtual OSStatus getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize);
	virtual OSStatus getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable);
protected:
	AudioStreamBasicDescription m_configOutputPlayback, m_configInputPlayback, m_configInputCapture, m_configOutputCapture;
	AURenderCallbackStruct m_renderCallback;
	bool m_shouldAllocateBuffer = true;
};

#endif

