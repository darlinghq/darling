#ifndef AUDIOUNITBASE_H
#define AUDIOUNITBASE_H

#include "AudioUnit.h"
#include "AudioUnitProperties.h"
#include <vector>
#include <CoreFoundation/CFString.h>
#include <CoreServices/ComponentsInternal.h>

class AudioUnitComponent : public CarbonComponent
{
protected:
	AudioUnitComponent(std::initializer_list<CFStringRef> elements);
public:
    virtual ~AudioUnitComponent();

	virtual OSStatus init() = 0;
	virtual OSStatus deinit() = 0;
	
	virtual OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) = 0;
	
	virtual OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) = 0;

	virtual OSStatus setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize);
	virtual OSStatus getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize);
	virtual OSStatus getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable);
protected:
	std::vector<CFStringRef> m_elementNames;
	std::vector<std::pair<AudioStreamBasicDescription, AudioStreamBasicDescription>> m_config;
	//AudioStreamBasicDescription m_configOutputPlayback, m_configInputPlayback, m_configInputCapture, m_configOutputCapture;
	AudioUnitConnection m_inputUnit;
	bool m_shouldAllocateBuffer = true;
	OSStatus m_lastRenderError = 0;
	CFStringRef m_contextName = nullptr;
};

#endif

