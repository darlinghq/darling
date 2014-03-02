#ifndef AOUCOMPONENT_H
#define AOUCOMPONENT_H
#include "AudioUnitBase.h"

class AudioOutputUnitComponent : public AudioUnitComponent
{
public:
	virtual OSStatus setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize) override;
	virtual OSStatus getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize) override;
	virtual OSStatus getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable) override;
	
	virtual OSStatus start() = 0;
	virtual OSStatus stop() = 0;
	
	virtual int cardIndex() const = 0;
protected:
	bool m_enableOutput = true, m_enableInput = false;
};

#endif
