#ifndef AUDIOUNITBASE_H
#define AUDIOUNITBASE_H

#include "AudioUnit.h"
#include "AudioUnitProperties.h"
#include <CoreServices/ComponentsInternal.h>

class AudioUnitComponent : public CarbonComponent
{
public:
	AudioUnitComponent();

	virtual int cardIndex() const = 0;

	virtual OSStatus init() = 0;
	virtual OSStatus deinit() = 0;

	virtual OSStatus start() = 0;
	virtual OSStatus stop() = 0;

	virtual OSStatus setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize);
	virtual OSStatus getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize);
	virtual OSStatus getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable);
protected:
	bool m_enableOutput, m_enableInput;
	AudioStreamBasicDescription m_configOutputPlayback, m_configInputPlayback, m_configInputCapture, m_configOutputCapture;
	AURenderCallbackStruct m_renderCallback;
};

#endif

