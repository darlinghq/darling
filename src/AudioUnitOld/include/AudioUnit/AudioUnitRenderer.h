#ifndef AUDIOUNITRENDERER_H
#define AUDIOUNITRENDERER_H
#include "AudioUnitBase.h"

// Wraps AURenderCallbackStruct
class AudioUnitRenderer : public AudioUnitComponent
{
public:
	AudioUnitRenderer(const AURenderCallbackStruct& cb);
	OSStatus init() override;
	OSStatus deinit() override;
	
	OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) override;
	
	OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) override;
private:
	AURenderCallbackStruct m_renderCallback;
};

#endif
